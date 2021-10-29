/*
**	/base/lib/act/movement.c
**
**	Functionality for moving around - currently only between
**	standard directed-graph-style rooms, with exits.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/toolbox";
private inherit "/base/lib/urcalls";

inherit "/base/lib/stance";
inherit "/base/lib/properties";
inherit "/base/lib/act/virtual";

# include "/base/include/act.h"
# include "/base/include/motion.h"
# include "/base/include/thing.h"
# include "/base/include/events.h"

# include <base.h>
# include <base/act.h>
# include <base/bulk.h>
# include <base/preps.h>

private
object *find_followers(object body) {
   object *arr;
   int i;

   arr = body->query_environment()->query_inventory();
   for (i = 0; i < sizeof(arr); i ++) {
      if (body != arr[i]->query_property("follow")) {
	 /* we're not following arr[i] */
	 arr[i] = nil;
      }
   }
   return arr - ({ nil });
}

static
float action_enter(object result, string state, int silently, mapping args) {
   string id, dir, *arr;
   object ob, old;
   NRef dest, what;

   what = NRefArg("what");

   if (state == "virgin") {
      if (!what) {
	 Add_Error("Error:Enter:BadArgument", nil, nil);
	 return -1.0;
      }
      if (!pre_signal("enter", args, ([ nil: what ]))) {
	 return -1.0;
      }
      what = NRefArg("what");
   }

   old = query_environment();

   if (!old) {
      Add_Error("Error:Enter:BadEnvironment",nil, nil);
      return -1.0;
   }

   ob = NRefOb(what);

   if (state == "virgin" && this_object()->query_opponent()) {
      DEBUG("Cancelling duel before enter: " + name(this_object()));
      insert_action("end_duel",
		    ([ "who": ({ this_object(),
				 this_object()->query_opponent() }),
		       "loser": this_object() ]));
      repeat_action();
      return 0.0;
   }

   if (state == "virgin") {
      /* get self to the exit */
      if (NRefOb(what) == old) {
	 /* exit is a detail in the room */
	 if (!NRefsEqual(query_proximity(), what)) {
	    repeat_action("proximate");
	    insert_action("approach", ([ "what": what ]));
	    return 0.0;
	 }
      } else if (ob->query_environment() == old) {
	 /* object in room */
	 if (!query_proximity() || NRefOb(query_proximity()) != ob) {
	    repeat_action("proximate");
	    insert_action("approach", ([ "what": ob ]));
	    return 0.0;
	 }
      } else if (!ob->contained_by(this_object())) {
	 /* we allow people to enter e.g. magical rings they're carrying */
	 Add_Error("Error:Enter:BadArgument", nil, nil);
	 return -1.0;
      }
      /* we're already there */
      state = "proximate";
   }

   id = what->query_detail(this_object());
   if (ob->query_closed(id)) {
      Add_Error("Error:Enter:Door", "obs", what);
      return -1.0;
   }
   if (arr = ur_exit_dirs(ob, id)) {
      if (sizeof(arr)) {
	 dir = arr[0];
      }
   }
   dest = ur_exit_dest(ob, id);
   if (!IsNRef(dest) || !NRefOb(dest)) {
      Add_Error("Error:Enter:BadDestination", "exit", what);
      return -1.0;
   }
   if (ob->is_child_of(NRefOb(dest))) {
      /*
      **  If e.g. an ur-tent points to itself in an exit destination
      **  to signal 'out' and 'in', then make sure the children point
      **  to themselves rather than to the ur-objects.
      */
      dest = NewNRef(ob, dest->query_detail(this_object()));
   }

   args["dest"] = dest;

   if (state == "proximate") {
      /* backwards compatibility */
      if (!run_signal("move", what, ([
	 "exitname":NRefDetail(what), "roomname":name(ob) ]) ) ) {
	 return 0.0;
      }

      if (!new_signal("enter", args, ([ "into": what, "from": dest ]),
		      ([ "into": old, "from": NRefOb(dest) ]))) {
	 return 0.0;
      }

      result->new_output("Output:StartEnter", ({ what }), nil, TRUE);

      insert_action("cancel_offers"); /* first cancel offers */
      repeat_action("ready"); /* return for more */
      return 0.0;
   }
   if (state == "ready") {
      object home, victim, *followers;
      int res, i, desc;

      /* execute the move */

      if (home = enter_virtual_room(dest)) {
	 dest = NewNRef(home, dest->query_detail(this_object()));
      }

      desc = desc_signal("enter", args, ([ "into": what, "from": dest ]),
			 ([ "into": old, "from": NRefOb(dest) ]));


      if (desc_signal("movement/leave", args, ([ nil: what ]))) {
	 /* leave */
	 broadcast("leave", nil, what);
      }

      followers = find_followers(this_object());

      if (ur_face(NRefOb(dest),
		  dest->query_detail(this_object())) == "outside" &&
	  NRefOb(dest)->query_environment()) {
	 res = move(NRefOb(dest)->query_environment(), dest);
      } else if (IsPrime(dest)) {
	 res = move(NRefOb(dest));
      } else {
	 res = move(NRefOb(dest), dest);
      }

      switch (res) {
      case GENERIC_FAILURE:
	 Add_Error("Error:Enter:MoveFailed", "obs", this_object());
	 break;
      case FAIL_MASS_CHECK:
	 Add_Error("Error:Enter:Mass", "obs", this_object());
	 break;
      case FAIL_FA_CHECK:
	 Add_Error("Error:Enter:FrontalArea", "obs", this_object());
	 break;
      case FAIL_DEPTH_CHECK:
	 Add_Error("Error:Enter:Depth", "obs", this_object());
	 break;
      case FAIL_CAP_CHECK:
	 Add_Error("Error:Enter:Capacity", "obs", this_object());
	 break;
      case FAIL_WOULD_SPILL:
	 Add_Error("Error:Enter:WouldSpill", "obs", this_object());
	 break;
      case FAIL_IMMOBILE:
	 Add_Error("Error:Enter:Immobile", "obs", this_object()); break;
      case GENERIC_SUCCESS:
	 break;
      default:
	 error("unknown move code: " + res);
      }
      if (desc_signal("movement/arrive", args, ([ nil: dest ]))) {
	 broadcast("arrive", dest);
      }
      insert_action("view/glance");

      for (i = 0; i < sizeof(followers); i ++) {
	 followers[i]->action("enter", ([ "what": what ]) );
      }

      if (home && home->query_property("HomeOwner") == this_object()) {
	 /* it's our virtual room we're stepping into -- close it? */
	 if (old->query_property("HomeOwner") != this_object()) {
	    /* yep, we're not coming from another virtual room of ours */
	    old->door_closes(id);
	 }
      }
      post_signal("enter", args, ([ "into": what, "from": dest ]),
		  ([ "into": old, "from": NRefOb(dest) ]));
      return 0.5;
   }
   error("bad state: " + state);
}

void act_teleport(object dest, varargs int silently) {
   action("teleport", ([ "dest": dest ]), silently ? ACTION_SILENT : 0);
}

static
float action_teleport(object result, string stage, int silently,
		      mapping args) {
   object room, dest;
   string vid;
   int res;

   if (!pre_signal("teleport", args, ([ nil: dest ]))) {
      return -1.0;
   }

   dest = ObArg("dest");

   if (!new_signal("teleport", args, ([ nil: dest ]))) {
      return 0.0;
   }

   if (stage == "virgin" && this_object()->query_opponent()) {
      DEBUG("Cancelling duel before teleport: " + name(this_object()));
      insert_action("end_duel",
		    ([ "who": ({ this_object(),
				 this_object()->query_opponent() }),
		       "loser": this_object() ]));
      repeat_action();
      return 0.0;
   }

   if (!silently) {
      if (query_property("Msg:TeleportOutFirst") &&
	  query_property("Msg:TeleportOutThird")) {
	 broadcast("properties", "Msg:TeleportOutFirst", nil,
		   "Msg:TeleportOutThird");
      } else {
	 result->smack_output("Desc:leave");
      }
   }

   if (dest) {
      if (vid = dest->query_property("IsVirtual")) {
	 room = query_property("VirtualHome:" + vid);
	 if (!room) {
	    /* create the room in owner's name */
	    room = spawn_thing(dest);
	    set_property("VirtualHome:" + vid, room);
	    room->set_property("HomeOwner", this_object());
	 }
	 dest = room;
      }
   }

   res = move(dest);
   switch(res) {
   case GENERIC_FAILURE:
      Add_Error("Error:Teleport:MoveFailed", "obs", this_object()); 
      return -1.0;
   case FAIL_MASS_CHECK:
      Add_Error("Error:Teleport:Mass", "obs", this_object()); 
      return -1.0;
   case FAIL_FA_CHECK:
      Add_Error("Error:Teleport:FrontalArea", "obs", this_object()); 
      return -1.0;
   case FAIL_DEPTH_CHECK:
      Add_Error("Error:Teleport:Depth", "obs", this_object()); 
      return -1.0;
   case FAIL_CAP_CHECK:
      Add_Error("Error:Teleport:Capacity", "obs", this_object()); 
      return -1.0;
   case FAIL_WOULD_SPILL:
      Add_Error("Error:Teleport:WouldSpill", "obs", this_object());
      return -1.0;
   case FAIL_IMMOBILE:
      Add_Error("Error:Teleport:Immobile", "obs", this_object()); break;
   case GENERIC_SUCCESS:
      break;
   default:
      error("unknown move result: " + res);
   }
   result = get_result_object();
   if (dest) {
      if (!silently) {
	 if (query_property("Msg:TeleportInFirst") &&
	     query_property("Msg:TeleportInThird")) {
	    broadcast("properties", "Msg:TeleportInFirst", nil,
		      "Msg:TeleportInThird");
	 } else {
	    result->smack_output("Desc:arrive");
	 }
      }
      insert_action("view/glance");
   }
   post_signal("teleport", args, ([ nil: dest ]));
   return 0.0;
}

static
float action_approach(object result, string state, int silently,
		      mapping args) {
   mapping consent_mapping;
   object ob, env, tenv;
   mixed target;
   NRef old;
   int done, prep, res;

   target = Arg("what");

   if (!pre_signal("approach", args, ([ nil: target ]))) {
      return -1.0;
   }

   target = Arg("what");
   consent_mapping = MapArg("consent_mapping");

   env = query_environment();

   if (IsNRef(target)) {
      ob = NRefOb(target);
   } else {
      ob = target;
   }

   /* target must be room detail, object in room, or object's detail */

   if (target) {
      if (ob == env && IsPrime(target)) {
	 /* the actual room: equivalent to no prox */
	 target = nil;
      } else if (ob != env && ob->query_environment() != env) {
	 Add_Error("Error:ApproachProx:BadDestination", "target", target);
	 return -1.0;
      }
   }

   old = query_proximity();

   if (target) {
      if (ur_abstract(ob, target->query_detail(this_object()))) {
	 Add_Error("Error:ApproachProx:Abstract", "target", target);
	 return -1.0;
      }
      prep = ur_id_preps(ob, target->query_detail(this_object()));

      if ((prep & PREP_NEAR) == 0 && !IsPrime(target) && contained_by(ob)) {
	 int i;
	 if (prep == 0) {
	    Add_Error("Error:ApproachProx:NoPreposition", "target", target);
	    return -1.0;
	 }
	 /* pick the first integer that's valid */
	 for (i = 1; (prep & i) == 0; i *= 2);
	 prep = i;
      }
      if (IsPrime(target) && ob == query_environment()) {
	 /* the room itself */
	 target = nil;
      }
   }
   if (!old && !target) {
      done = TRUE;
   }
   if (old && NRefsEqual(target, old)) {
      done = TRUE;
   }
   if (!done && !Arg("consented") && target &&
       (ur_volition(ob) || ob->query_holder())) {
      if (consent_mapping) {
	 insert_action("demand_consent", ([
	    "target": target,
	    "message": consent_mapping["message"],
	    "action": "approach",
	    "args": ([
	       "what": target,
	       "consent_mapping": consent_mapping,
	       ]),
	    ]));
      } else {
	 insert_action("demand_consent", ([
	    "target": target,
	    "message": "approach",
	    "action": "approach",
	    "args": ([
	       "what": target,
	       ]),
	    ]));
      }
      return 0.0;
   }
   /* there may still be a post-approach action to run */
   if (consent_mapping) {
      action(consent_mapping["action"],
	     consent_mapping["args"]);
   }
   if (!new_signal("approach", args, ([ nil: target ]))) {
      /* Return -1.0 instead of 0.0, to prevent endless loops. */
      return -1.0;
   }
   /* remember, post_signal always executes in a callout */
   post_signal("approach", args, ([ nil: target ]));

   if (done) {
      return 0.0;
   }
   res = move(env, target);
   if (res != GENERIC_SUCCESS) {
      Add_Error("Error:Approach:MoveFailed", "obs", this_object());
      return -1.0;
   }

   if (prep) {
      set_preposition(prep);
   }
   broadcast("change_prox", old, target);
   return 1.0;
}

static
float action_leave(object result, string state, int silently, mapping args) {
   NRef old, new_ref;

   old = query_proximity();

   if (!pre_signal("leave", args)) {
      return -1.0;
   }

   if (!new_signal("leave", args)) {
      return -1.0;
   }

   post_signal("leave", args);

   if (!old) {
      return 0.0;
   }

   /* you actually -climb- the prox chain when you leave a leaf */
   new_ref = IsPrime(old) ? NRefOb(old)->query_proximity() : nil;
   move(query_environment(), new_ref);
   broadcast("change_prox", old, new_ref);
   return 1.0;
}
