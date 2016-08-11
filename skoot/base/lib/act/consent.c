/*
**	/base/lib/act/consent.c
**
**	Functionality for allowing or deny people prox violations.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/urcalls";

# include "/base/include/act.h"
# include "/base/include/thing.h"
# include "/base/include/motion.h"
# include "/base/include/consent.h"
# include "/base/include/environment.h"
# include "/base/include/prox.h"
# include "/base/include/events.h"

# include <base/bulk.h>
# include <base/living.h>
# include <base/act.h>
# include <base.h>

static
float action_allow(object result, string state, int flags, mapping args) {
   string info, action;
   object env, me, who;

   me = this_object();
   env = me->query_environment();

   who = ObArg("who");


   if (!who) {
      Add_Error("Error:Allow:BadArgument", nil, nil);
      return -1.0;
   }

   if (!pre_signal("allow", args, ([ nil: who ]))) {
      return -1.0;
   }

   info = me->query_request_info(who);
   if (!info) {
      Add_Error("Error:Allow:NoRequest", "obs", who);
      return -1.0;
   }

   if (!new_signal("allow", args, ([ nil: who ]))) {
      return -1.0;
   }

   action = who->query_pending_action();
   args = who->query_pending_args();

   SysLog("ACTION_ALLOW: pending action = " + dump_value(action));
   SysLog("ACTION_ALLOW: pending args = " + dump_value(args));

   post_signal("allow", args, ([ nil: who ]));

   result->new_output("Output:Allow", ({ who }),
		      ([ "info": info,
			 "target": who->query_prox_target() ]),
		      TRUE);
   flush_result_object(); /* to preserve order */

   me->remove_request(who);
   who->set_prox_target(nil);

   who->action(action, args + ([ "consented": TRUE ]));
}

static
float action_deny(object result, string state, int flags, mapping args) {
   string info, action;
   object me, env, who;

   me = this_object();
   env = me->query_environment();

   who = ObArg("who");

   if (!who) {
      Add_Error("Error:Deny:BadArgument", nil, nil);
      return -1.0;
   }

   if (!pre_signal("deny", args, ([ nil: who ]))) {
      return -1.0;
   }

   info = me->query_request_info(who);
   if (!info) {
      Add_Error("Error:Deny:NoRequest", "obs", who);
      return -1.0;
   }

   if (!new_signal("deny", args, ([ nil: who ]))) {
      return -1.0;
   }

   me->remove_request(who);
   who->set_prox_target(nil);
   who->set_pending_action(nil,nil);

   post_signal("deny", args, ([ nil: who ]));

   result->new_output("Output:Deny", ({ who }), ([ "info": info ]));
}

static
float action_demand_consent(object result, string state, int flags,
			    mapping args) {
   mapping inner, dargs;
   object prox_target, me, handler;
   string message, action;
   NRef target;

   target = args["target"];

   if (!pre_signal("demand-consent", args, ([ nil: target ]))) {
      return -1.0;
   }

   message = StrArg("message");
   action = StrArg("action");
   dargs = MapArg("args");

   me = this_object();
   inner = dargs["args"];

   handler = NRefOb(target);

   if ((me->query_prox_target() == handler) &&
	(handler->query_request_info(me) == message) ) {
      Set_Error("Error:Consent:Pending", "msg", message);
      Add_Error("Error:Consent:Pending", "obs", NRefOb(target));
      return -1.0;
   } 

   if (!new_signal("demand-consent", args, ([ nil: target ]))) {
      return -1.0;
   }

   if (prox_target = me->query_prox_target()) {
      string old_message;

      old_message = me->query_consent_message();
      if (!old_message) {
	 old_message = message;	/* pointless hack? */
      }

      result->new_output("Output:ConsentMove", ({ prox_target }), ([
	 "target": prox_target, "msg": old_message
	 ]), TRUE);

      prox_target->remove_request(me);
      me->set_prox_target(nil);
      me->set_pending_action(nil,nil);
   }

   handler->add_request(me, message);
   me->set_prox_target(handler);
   me->set_pending_action(action, dargs);
   me->set_consent_message(message);

   if (handler->check_auto_allow(me)) {
      handler->action("allow", ([ "who": me ]));
      return 0.0;
   }
   if (handler->check_auto_deny(me)) {
      handler->action("deny", ([ "who": me ]));
      return 0.0;
   }

   post_signal("demand-consent", args, ([ nil: target ]));

   result->new_output("Output:ConsentStart", ({ handler }), ([
      "target": target, "msg": message, "handler": handler,
      "adv": (typeof(inner) == T_MAPPING ? inner["adv"] : nil)
      ]), TRUE);
}

static
float action_consent(object result, string state, int flags, mapping args) {
   string type;
   mixed what;

   type = StrArg("type");
   what = Arg("what");

   if (type == "remove") {
      remove_consent(what);
   } else if (what == "all") {
      clear_consent();
      add_consent("all", type);
   } else {
      add_consent(what, type);
   }
}
