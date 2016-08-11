/*
**	/base/lib/act/social.c
**
**	Communicative functionality.
**
**	Copyright Skotos Tech Inc 1999
*/

/* ARGH: REMOVE SOON SOON SOON */
private inherit "/usr/SkotOS/lib/ursocials";

private inherit "/lib/array";
private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";

# include <nref.h>
# include <base.h>
# include <base/act.h>
# include <base/prox.h>
# include <base/preps.h>
# include <base/toolbox.h>

# include "/base/include/events.h"
# include "/base/include/act.h"
# include "/base/include/living.h"
# include "/base/include/motion.h"
# include "/base/include/environment.h"
# include "/base/include/prox.h"
# include "/base/include/movement.h"

private
void rebuild_react_map(mapping args, mapping rmap) {
   string *roles;
   int i;

   if (roles = args["roles"]) {
      for (i = 0; i < sizeof(roles); i ++) {
	 if (typeof(args[roles[i]]) == T_OBJECT) {
	    args[roles[i]] = ({ args[roles[i]] });
	 }
	 if (typeof(args[roles[i]]) == T_ARRAY) {
	    if (member(nil, args[roles[i]])) {
	       args[roles[i]] -= ({ nil });
	    }
	    rmap[roles[i]] = TRUE;
	 }
      }
   }
}


private
int call_handler(string action, string phase, mapping args) {
   object handler;

   if (handler = ACTIONS->query_handler(action)) {
      args["current-phase"] = phase;
      args["current-action"] = action;
      args["actor"] = this_object();

      if (!call_handler(action + "/head", phase, args)) {
	 return FALSE;
      }
      if (!handler->handle_action(phase, args)) {
	 return FALSE;
      }
      /* IF a handler for X is found, call X/tail */
      if (!call_handler(action + "/tail", phase, args)) {
	 return FALSE;
      }
   }
   return TRUE;
}

static
int call_handlers(string *actions, string phase, mapping args, mapping rmap) {
   int i;

   for (i = 0; i < sizeof(actions); i ++) {
      if (!call_handler(actions[i], phase, args) && phase != "post") {
	 rebuild_react_map(args, rmap);
	 return FALSE;
      }
   }
   rebuild_react_map(args, rmap);
   return TRUE;
}

private
int call_signals(string *actions, string phase, mapping args, mapping rmap) {
   mixed ret;
   int i;

   args["current-phase"] = phase;
   args["actor"] = this_object();
   for (i = 0; i < sizeof(actions); i ++) {
      args["current-action"] = actions[i];
      switch(phase) {
      case "pre":
	 ret = pre_signal(actions[i], args, rmap);
	 break;
      case "prime":
	 ret = prime_signal(actions[i], args, rmap);
	 break;
      case "desc":
	 ret = desc_signal(actions[i], args, rmap);
	 break;
      case "post":
	 post_signal(actions[i], args, rmap);	
	 ret = TRUE;
	 break;
      default:
	 error("bad phase");
      }
      if (!ret) {
	 rebuild_react_map(args, rmap);
	 return FALSE;
      }
   }
   rebuild_react_map(args, rmap);
   return TRUE;
}

/* this function is recursive, which is a little too cute */

private
int do_armslength(mixed it) {
   object ob;
   NRef prox, their_prox;

   if (!it) {
      error("target is not present");
   }
   if (IsNRef(it)) {
      ob = NRefOb(it);
   } else {
      ob = it;
   }

   if (ob == this_object() || ob->contained_by(this_object())) {
      /* we are always proximate to ourselves & our details & inventory */
      return FALSE;	/* we're done */
   }
   prox = query_proximity();

   if (ob == query_environment()) {
      /* a detail in our current room */
      if ((!prox && IsPrime(it)) || NRefsEqual(prox, it)) {
	 return FALSE;
      }
      repeat_action();
      insert_action("approach", ([ "what": it ]));
      return TRUE;
   }
   if (ob->query_environment() == query_environment()) {
      /* an object in the same room */
      their_prox = ob->query_proximity();

      while (their_prox && ur_volition(NRefOb(their_prox))) {
	 their_prox = NRefOb(their_prox)->query_proximity();
      }
      if ((!prox && !their_prox) || NRefsEqual(prox, their_prox)) {
	 return FALSE;
      }
      if (prox && IsPrime(prox) && NRefOb(prox) == ob) {
	 return FALSE;
      }
      if (!prox && their_prox && IsPrime(their_prox) && NRefOb(their_prox) == query_environment()) {
	 return FALSE;
      }
      if (their_prox) {
	 if (IsPrime(their_prox) && NRefOb(their_prox) == this_object()) {
	    return FALSE;
	 }
	 repeat_action();
	 insert_action("approach", ([ "what": their_prox ]));
	 return TRUE;
      }
      repeat_action();
      insert_action("approach");
      return TRUE;
   }
   /* this is inefficient for long chains, obviously */
   return do_armslength(ob->query_environment());
}

/* do_close is not recursive, unlike do_armslength */

private
int do_close(NRef target, string v2, object result, mapping args) {
   object tob;
   NRef aprox;

   tob = NRefOb(target);

   if (contained_by(tob)) {
      if (tob == query_environment()) {
	 /* I'm really sick of 'nil' and prime being the same prox */
	 if (!NRefsEqual(query_proximity(), target) &&
	     (query_proximity() || !IsPrime(target))) {
	    /* if they're unequal, they may still be equal if
	       query_proximity() is nil and target is prime */
	    repeat_action();
	    insert_action("approach", ([ "what": target ]));
	    return 1;
	 }
      }
      return 0;
   }
   /* find the peer-level object that contains target */
   while (tob->query_environment() != query_environment()) {
      tob = tob->query_environment();
      if (tob == nil) {
	 /* we don't share an environment with this object */
	 Add_Error("Error:Social:NotPresent", "obs", target);
	 return -1;
      }
   }
   /* tob is now in our environment */
   if (tob == this_object()) {
      /* we can always touch ourselves and our inventory */
      return 0;
   }
   if (query_proximity() && NRefOb(query_proximity()) == tob) {
      /* are we close to them or their bits? */
      return 0;
   }
   aprox = tob->query_proximity();
   if (aprox && NRefOb(aprox) == this_object()) {
      /* are they close to us or our bits? */
      return 0;
   }
   if (ur_volition(tob)) {
      insert_action("approach", ([
	 "what": tob,
	 "consent_mapping": ([
	    "target": target,
	    "message": v2,
	    "action": "social",
	    "args": args
	    ]),
	 ]));
      return 1;
   }
   repeat_action();
   insert_action("approach", ([ "what": tob ]));
   return 1;
}


static
float action_social(object result, string state, int silently,
		    mapping args) {
   mapping rmap;
   string *roles, *actions;
   object vob;
   NRef *obs, target;
   int dist, i, j;

   vob = ObArg("vob");
   args["verb"] = vob ? vob->query_imperative() : nil;

   args["rob"] = result;
   args["origtarget"] = "none";

   args["delay"] = nil;

   roles = args["roles"];

   rmap = ([ ]);

   if (args["override-action"]) {
      /* this is a temporary situation I think */
      actions = ({ args["override-action"] });
      args["override-action"] = nil;
   } else if (vob) {
      actions = vob->get_actions();
   } else {
      error("action_social with neither vob nor action");
   }
   actions |= ({ "command" });

   if (vob && vob->query_default_role() &&
       member(vob->query_default_role(), roles)) {
      rmap[nil] = vob->query_default_role();
   }

   if (args["evoke"]) {
      actions |= ({ "evoke" });
   }

   rebuild_react_map(args, rmap);

   if (!call_signals(actions, "pre", args, rmap)) {
      return 0.0;
   }

   if (vob) {
      int err;

      for (i = 0; i < sizeof(roles); i ++) {
	 int ldist;
	 mixed target_peer, peer;

	 ldist = ur_distance(vob, roles[i]);
	 if (ldist == SA_FAR) {
	    continue;
	 }
	 if (typeof(args[roles[i]]) != T_ARRAY) {
	    continue;
	 }
	 obs = args[roles[i]];

	 for (j = 0; j < sizeof(obs); j ++) {
	    peer = find_peer(obs[j], this_object(),
			     ldist == SA_CLOSE || ldist == SA_OWNED);

	    if (peer == PEER_CONTAINER) {
	       /* i.e. the room itself */
	       continue;
	    }
	    if (peer == PEER_NONE) {
	       Add_Error("Error:Social:Unreachable", "obs", obs[j]);
	       err = TRUE;
	       continue;
	    }
	    if (peer == PEER_CLOSED) {
	       Add_Error("Error:Social:Closed", "obs", obs[j]);
	       err = TRUE;
	       continue;
	    }
	    if (typeof(peer) == T_OBJECT) {
	       if (peer == this_object()) {
		  /* we can always manipulate our own stuff */
		  continue;
	       }
	       if (ldist == SA_ARMSLENGTH) {
		  mixed tmp;

		  /* for armslength, only require shared prox */
		  tmp = peer->query_proximity();

		  if (typeof(tmp) == T_OBJECT || tmp == nil) {
		     peer = tmp;
		  } else if (typeof(tmp) == T_STRING) {
		     peer = NRefDetail(tmp);
		  }
	       }
	    }
	    if (typeof(peer) == T_STRING &&
		!ur_id_prep(NRefOb(obs[j]), peer, PREP_NEAR)) {
	       Add_Error("Error:Social:NonNear", "obs", obs[j]);
	       return -1.0;
	    }
	    if (target_peer && peer != target_peer) {
	       Add_Error("Error:Social:TooManyTargets", nil, nil);
	       return -1.0;
	    }
	    target_peer = peer;
	    target = obs[j];
	    dist = ur_distance(vob, roles[i]);
	 }
      }
      if (err) {
	 return -1.0;
      }
   }
   if (!call_handlers(actions, "pre", args, rmap)) {
      return 0.0;
   } 

   if (target) {
     if (dist == SA_ARMSLENGTH) {
       if (do_armslength(target)) {
	 /* do_armslength has inserted the approach; return here later */
	 return 0.0;
       }
     } else if (dist == SA_CLOSE) {
       int n;

       n = do_close(target, vob->query_second(), result, args);

       switch(n) {
       case -1:
	 /* an error has been added to the result object */
	 return -1.0;
       case 0:
	 /* nothing was done: we're already close */
	 break;
       case 1:
	 /* do_close has inserted the approach; we'll return here later */
	 return 0.0;
       }
     }
   }

   if (!call_signals(actions, "prime", args, rmap)) {
      return 0.0;
   }

   if (!call_handlers(actions, "prime", args, rmap)) {
      return 0.0;
   }

   if (call_signals(actions, "desc", args, rmap) &&
       call_handlers(actions, "desc", args, rmap) &&
       vob) {
      broadcast("social", vob, args);
   }

   call_signals(actions, "post", args, rmap);
   call_out("call_handlers", 0, actions, "post", args, rmap);

   /* this should go into a action handler later */
   if (vob && vob->query_connects()) {
      for (i = 0; i < sizeof(roles); i ++) {
	 obs = args[roles[i]];
	 for (j = 0; j < sizeof(obs); j ++) {
	    target = obs[j];
	    if (ur_is_door(target->query_object(),
			   target->query_detail(this_object()))) {
	       if (target = ur_exit_dest(target->query_object(),
					 target->query_detail(this_object()))) {
		  NRefOb(target)->broadcast_event("noise", this_object(),
						  vob->query_imperative(),
						  target);
	       }
	    }
	 }
      }
   }
   if (typeof(args["delay"]) == T_FLOAT || typeof(args["delay"]) == T_INT) {
      return (float) args["delay"];
   }
   return 0.0;
}

static
float action_stance(object result, string state, int silently, mapping args) {
   object env;
   NRef target;
   int old, prep, stance;

   target = NRefArg("target");

   if (!pre_signal("stance", args, ([ nil: target ]))) {
      return -1.0;
   }

   stance = IntArg("stance");

   old = query_stance();

   if (!args["prep"]) {
      if (!new_signal("stance", args)) {
	 return -1.0;
      }
      if (stance == query_stance()) {
	 return 0.0;
      }
      set_stance(stance);
      /* don't clear preposition: go from kneel before to sit before */
      post_signal("stance", args);
      broadcast("stance", old, stance);
      return 1.0;
   }
   target = NRefArg("target");
   prep = IntArg("prep");
   if (!target) {
      target = query_proximity();
   } else {
      if (IsPrime(target) && NRefOb(target) == query_environment()) {
	 /* the room itself */
	 target = nil;
      }

      if (target && !NRefsEqual(target, query_proximity())) {
	 Add_Error("Error:Stance:BadDestination", nil, nil);
	 return -1.0;
      }
   }
   env = query_environment();

/* TODO FIX THIS
   -- If you try to perfom a stance on the prime detail of a room this fails.
   if (IsPrime(target) ?
       (NRefOb(target)->query_environment() != env) :
       (NRefOb(target) != env)) {
      Add_Error("Error:Stance:Failure", nil, nil);
      broadcast("error", error);
      return;
   }
   */

   if (!new_signal("stance", args, ([ nil: target ]))) {
      return -1.0;
   }
   if (stance != query_stance() || prep != query_preposition()) {
      set_stance(stance);
      set_preposition(prep);
   }
   post_signal("stance", args, ([ nil: target ]));
   broadcast("stance", old, stance, prep, target);
   return 1.0;
}
