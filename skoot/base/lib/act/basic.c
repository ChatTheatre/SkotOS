/*
**	/base/lib/act/basic.c
**
**	Functionality for picking things up and dropping them.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/toolbox";
private inherit "/base/lib/urcalls";

inherit "/lib/type";
inherit "/base/lib/act/virtual";

# include "/base/include/act.h"
# include "/base/include/thing.h"
# include "/base/include/motion.h"
# include "/base/include/environment.h"
# include "/base/include/prox.h"
# include "/base/include/events.h"

# include <base/bulk.h>
# include <base/living.h>
# include <base/act.h>
# include <base/preps.h>
# include <base.h>

void act_quit(varargs object user) { action("quit"); }

static
/* broadcast that I am quiting */
float action_quit(object result, string state, int silently, mapping args) {
   if (query_environment()) {
      post_signal("quit", args);
      set_disconnect_room(query_environment());
      result->smack_output("Desc:quit");
      move(nil);
   }
}

void act_disconnect() { action("disconnect"); }

static
/* quit quietly */
float action_disconnect(object result, string state, int silently,
			mapping args) {
   if (query_environment()) {
      set_disconnect_room(query_environment());
      move(nil);
      this_object()->disconnect();
   }
}

void act_linkdie() { action("linkdie"); }

static
/* broadcast that I am linkdying */
float action_linkdie(object result, string state, int silently, mapping args) {
   if (query_environment()) {
      post_signal("linkdie", args);
      set_disconnect_room(query_environment());
      result->smack_output("Desc:linkdie");
      move(nil);
   }
}

void act_client_control(int id, string val) {
   action("client_control", ([ "id": id, "value": val ]));
}

static
/* move me to def, log me in, and send glance description */
float action_login(object result, string state, int silently, mapping args) {
   object def, ob;
   int res;

   if (!query_environment()) {
      def = ObArg("default");
      ob = query_disconnect_room();

      DEBUG("action_login: args = " + dump_value(args) + "; disconnect room: " + dump_value(ob));
      if (!ob) {
	 ob = def;
      }
      if (!ob) {
	 error("nowhere to go in login");
      }
      if ((res = move(ob)) < 0) {
	 if (ob != def) {
	    res = move(def);
	 }
      }
      if (res < 0) {
	 error("move() failed with code " + res);
      }
      post_signal("login", args);
      result = get_result_object();
      result->smack_output("Desc:login");
      clear_disconnect_room();

      insert_action("view/glance");
      return 0.0;
   }
}

static
float action_client_control(object result, string state, int silently,
			    mapping args) {
   occur("client_control", IntArg("id"), StrArg("val"));
}

void act_open(NRef *what) {
   action("open", ([ "what": what ]));
}

void act_close(NRef *what) {
   action("close", ([ "what": what ]));
}

void act_unlock(NRef *what, object *keys, int explicit) {
   action("unlock", ([ "what": what, "keys": keys, "explicit": explicit ]));
}

void act_lock(NRef *what, object *keys, int explicit) {
   action("lock", ([ "what": what, "keys": keys, "explicit": explicit ]));
}

static
float action_take(object result, string state, int silently, mapping args) {
   mapping sort, success;
   object *containers, *what, from, env;
   mixed *take;
   int i, j;

   take = ArrArg("what");

   sort = ([ ]);
   success = ([ ]);

   take = take[..];	/* copy */

   for (i = 0; i < sizeof(take); i ++) {
      env = NRefOb(take[i])->query_environment();
      if (!pre_signal("take", args, ([ nil: take[i], "from": env ]))) {
	 continue;
      }
      if (IsPrime(take[i])) {
	 if (env = NRefOb(take[i])->query_environment()) {
	    /* if we have it already or it's completely elsewhere */
	    if (!NRefOb(take[i])->contained_by(query_environment()) ||
		NRefOb(take[i])->query_environment() == this_object()) {
	       env = nil;
	    }
	 }
      } else {
	 env = nil;
      }
      if (!sort[env]) {
	 sort[env] = ([ ]);
      }
      if (env) {
	 sort[env][NRefOb(take[i])] = TRUE;
      } else {
	 /* preserve nref rather than just the ob */
	 sort[nil][take[i]] = TRUE;
      }
   }
   /* now sort[nil] contains any bad objects */
   if (sort[nil]) {
      Set_Error("Error:Take:CannotTake", "obs", map_indices(sort[nil]));
      sort[nil] = nil;
   }

   containers = map_indices(sort);

   for (i = 0; i < sizeof(containers); i ++) {
      from = containers[i];

      /* if we're outside this object, check for open doors */
      if (!contained_by(from) && sizeof(from->query_doors(this_object())) &&
	  !sizeof(from->query_open_doors(this_object()))) {
	 Add_Error("Error:Take:Closed", "obs", from);
	 continue;
      }

      what = map_indices(sort[from]);

      for (j = 0; j < sizeof(what); j++) {
	 if (what[j] == this_object()) {
	    Add_Error("Error:Take:Yourself", "obs", what[j]);
	    continue;
	 }

       	 /* check if I am taking a volitional or -from- a volitional */
	 if (!check_ownership(what[j], this_object()) &&
	     !this_object()->is_offered(what[j])) {
	    Add_Error("Error:Take:Volition", "obs", what[j]);
	    continue;
	 }

	 if (!new_signal("take", args, ([ nil: what[j], "from": from ]))) {
	    continue;
	 }

	 /* try to move the object into me */
	 switch(what[j]->move(this_object())) {
	 case GENERIC_FAILURE:
	    Add_Error("Error:Take:MoveFailed", "obs", what[j]); break;
	 case FAIL_MASS_CHECK:
	    Add_Error("Error:Take:Mass", "obs", what[j]); break;
	 case FAIL_FA_CHECK:
	    Add_Error("Error:Take:FrontalArea", "obs", what[j]); break;
	 case FAIL_DEPTH_CHECK:
	    Add_Error("Error:Take:Depth", "obs", what[j]); break;
	 case FAIL_CAP_CHECK:
	    Add_Error("Error:Take:Capacity", "obs", what[j]); break;
	 case FAIL_WORN_CHECK:
	    Add_Error("Error:Take:IsWorn", "obs", what[j]); break;
	 case FAIL_WOULD_SPILL:
	    Add_Error("Error:Take:WouldSpill", "obs", what[j]); break;
	 case FAIL_IMMOBILE:
	    Add_Error("Error:Take:Immobile", "obs", what[j]); break;
	 default:
	    success[what[j]] = TRUE;
	    post_signal("take", args, ([ nil: what[j], "from": from ]));
	    break;
	 }
      }
      if (map_sizeof(success)) {
	 result->new_output("Output:Take", map_indices(success),
			    ([ "from": (from == query_environment() ?
					nil : from) ]));
	 flush_result_object();
	 result = get_result_object();
	 success = ([ ]);
      }
   }
}

void act_inv(mixed blah) {
   action("inv");
}


void act_glance() {
   action("glance");
}

void act_emote(string what) {
   action("emote", ([ "what": what ]));
}

static
float action_emote(object result, string state, int flags, mapping args) {
   broadcast("emote", StrArg("what"));
}

static
float action_tip(object result, string state, int flags, mapping args) {
   broadcast("tip", StrArg("topic"));
}

void act_brain ( object npc, string msg ) {
   broadcast( "brain", npc, msg );
}

static
float action_drag(object result, string state, int flags, mapping args) {
   object env, victim;

   if (!pre_signal("drag", args, ([ nil: ObArg("victim") ]))) {
      return -1.0;
   }

   victim = ObArg("victim");

   if (this_object()->query_property("drag")) {
      Set_Error("Error:Drag:AlreadyDragging", "victim", victim);
      return -1.0;
   }

   if (!new_signal("drag", args, ([ nil: ObArg("victim") ]))) {
      return -1.0;
   }
   this_object()->set_property("drag", victim);
   victim->set_property("dragger", this_object());
   if (victim->query_property("follow")) {
      int i, sz;
      object *souls;

      souls = victim->query_souls();
      sz = sizeof(souls);
      for (i = 0; i < sz; i++) {
	 souls[i]->message("You will follow nothing now.\n");
      }
      victim->clear_property("follow");
   }

   result->new_output("Output:Drag", ({ victim }), ([
      "dragger": this_object(), "victim": victim
      ]));
   post_signal("drag", args, ([ nil: ObArg("victim") ]));
}

static
float action_undrag(object result, string state, int flags, mapping args) {
   object env, victim;

   victim = this_object()->query_property("drag");
   if (!victim) {
      Add_Error("Error:Undrag:NotDragging", nil, nil);
      return -1.0;
   }
   victim->clear_property("dragger");
   this_object()->clear_property("drag");

   result->new_output("Output:Undrag", ({ victim }), ([
      "dragger": this_object(), "victim": victim
      ]));
}

static
float action_dragwiggle(object result, string state, int flags, mapping args) {
   object victim, env;

   if (victim = this_object()->query_property("drag")) {
      result->new_output("Output:Dragwiggle", ({ victim }),
			 ([ "dragger": this_object(), "victim": victim ]));
   }
}

void act_drink(object what) {
   action("drink", ([ "what": what ]));
}

