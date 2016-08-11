/*
**	/usr/TextIF/cmds/movement.c
**
**	This file handles all the movement. See title.
**	Room and prox movement.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>
# include <SkotOS.h>
# include <base/living.h>
# include <base/bulk.h>
# include <base/act.h>
# include <base/preps.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";

inherit "/usr/SkotOS/lib/bilbo";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";

# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"
# include "/usr/SkotOS/include/inglish.h"



void cmd_go(object user, object body, Ob *where) {
   NRef what;
   string dir, det;
   object env;

   dir = implode(ObWords(where[0]), " ");

   env = body->query_environment();

   if ( NPCBODIES->is_dragging( user, body ) )
      return;

   if (det = env->query_detail_by_dir(dir)) {
      if ( NPCBODIES->is_blocking( user, det ) )
         return;
      body->action("enter", ([ "what" : NewNRef(env, det) ]) );
      return;
   }
   if (what = locate_one(user, LF_HUSH_ALL, where,
			 body, body->query_environment())) {
      if ( NPCBODIES->is_blocking( user, NRefDetail(what) ) )
	 return;
      body->action("enter", ([ "what" : what ]) );
      return;
   }
   user->message("You can't go there. Type \"exits\" for a list of exits. \n");
}

void cmd_enter(object user, object body, Ob *what_noun) {
   NRef what;

   if ( NPCBODIES->is_dragging( user, body ) )
      return;

   if (what = locate_one(user, FALSE, what_noun, body,
			 body->query_environment())) {
      if ( NPCBODIES->is_blocking( user, NRefDetail(what) ) )
         return;
      body->action("enter", ([ "what" : what ]) );
      return;
   }
}

void cmd_leave(object user, object body, varargs Ob *what_noun) {
   NRef what;

   if (NPCBODIES->is_dragging( user, body ) )
      return;
   
   if (what_noun) {
      what = locate_one(user, FALSE, what_noun, body, 
			body->query_environment());
      if (!what) {
	 return;
      }
   } else {
      what = body->query_proximity();
      if (!what) {
	 if (sizeof(body->query_proximate())) {
	    user->paragraph("You move away from " + describe_many(body->query_proximate(), nil, body) + ".\n");
	    body->leave_proximates();
	 } else {
	    user->paragraph("You're not near anything.");
	 }
	 return;
      }
   }
   if (NRefsEqual(what, body->query_proximity())) {
      body->action("leave", nil);
   } else {
      user->paragraph("You're not near " + describe_one(what, body, body) + ".");
   }
}

void cmd_approach(object user, object body, varargs Ob *what_noun) {
   object env;
   NRef handler, what, what_prox, body_prox;;

   if (!what_noun) {
      user->paragraph("What do you want to approach?");
      return;
   }

   if ( NPCBODIES->is_dragging( user, body ) )
      return;

   if (what = locate_one(user, FALSE, what_noun, body,
			 body->query_environment())) {
      if (NRefOb(what)->query_holder()) {
	 what = NewNRef(NRefOb(what)->query_holder(),
			ur_prime_id(NRefOb(what)->query_holder(), "outside"));
      }
      
      XDebug("XX cmd_approach() what: " + dump_value(what));
      XDebug("XX cmd_approach() body: " + dump_value(body));
      
      if (what == body) {
	 user->paragraph("You cannot approach yourself.");
	 return;
      }
      what_prox = NRefOb(what)->query_proximity();
      body_prox = body->query_proximity();
      
      if (NRefsEqual(what, body_prox) ||
	  (what_prox && NRefOb(what_prox) == body)) {
	 user->paragraph("You're already near " + 
			 describe_one(what, body, body) + ".");
	 return;
      }
      /* are we dealing with another object in the room? */
      env = body->query_environment();
      
      if (IsPrime(what) && NRefOb(what)->query_environment() == env) {
	 /* yes; are we in their prox? */

	 if ((!body_prox && !what_prox) || NRefsEqual(body_prox, what_prox)) {
	    body->action("approach", ([ "what" : what ]) );
	    return;
	 }
	 /* no; get into their prox */
	 if (what_prox) {
	    body->action("approach", ([ "what" : what_prox ]) );
	 } else {
	    body->action("leave", ([ ]) );
	 }
	 return;
      }
      /* we're dealing with a detail; check it */

      if (!ur_id_preps(NRefOb(what), NRefDetail(what))) {
	 user->paragraph("You can't get near to " + describe_one(what) + ".");
	 return;
      }

      XDebug("XX - cmd_approach() what: " + dump_value(what));
      body->action("approach", ([ "what" : what ]) );
      return;
   }
}


void desc_change_prox(object *users, object who, NRef old, NRef new) {
   if (old && new) {
      tell(users,
	   "You move from " + describe_one(old, who, who) +
	   " to " + describe_one(new, who, who) + ".",
	   describe_one(who) + " moves from " + describe_one(old, who) +
	   " to " + describe_one(new, who) + ".",
	   who);
   } else if (old) {
      tell(users,
	   "You move away from " + describe_one(old, who, who) + ".",
	   describe_one(who) + " moves away from " + describe_one(old, who) + ".",
	   who);
   } else {
      tell(users,
	   "You approach " + describe_one(new, who, who) + ".",
	   describe_one(who) + " approaches " + describe_one(new, who) + ".",
	   who);
   }      
}

void desc_leave(object *users, object who, varargs string dir, NRef ref) {
   string leavestr, id, gait;
   object env;
   SAM me;

   if (ref) {
      env = NRefOb(ref);
      id = NRefDetail(ref);
   } else {
      env = who->query_environment();
   }

   if (ref) {
      if (me = ur_description(env, id, "myleave")) {
	 general_sam(users, who, ({ who }), me);
	 general_sam(users, who, ({ who }),
		     "/usr/XML/sys/xml"->parse("<br/>"));
      }
   }
   gait = ur_gait(who);
   if (!id) {
      if (gait) {
	 tell_all_but(users, capitalize(describe_one(who)) + " " +
		      gait + " out.", who);
	 return;
      }
      tell_all_but(users, capitalize(describe_one(who)) + " leaves.",  who);
      return;
   }

   leavestr = who->query_brain("leave");

   if (!leavestr) {
      /* hopefully almost always true */
      if (gait) {
	 leavestr = gait + " out through";
      } else {
	 leavestr = "leaves through";
      }
   }

   leavestr = " " + leavestr + " ";

   tell_all_but(users, capitalize(describe_one(who)) + leavestr +
		describe_one(ref, who) + ".", who);
}

void desc_arrive(object *users, object who, varargs NRef entrance) {
   string arrivestr, gait;
   object env;

   if (who->query_brain("inanimate")) {
      return;
   }
   gait = ur_gait(who);

   if (!entrance) {
      if (gait) {
	 tell_all_but(users, capitalize(describe_one(who)) + " " + gait +
		      " in.", who);
	 return;
      }
      tell_all_but(users, capitalize(describe_one(who)) + " arrives.",  who);
      return;
   }

   arrivestr = who->query_brain("arrive");
   if (!arrivestr) {
      if (gait) {
	 arrivestr = gait + " in through";
      } else {
	 arrivestr = "arrives through";
      }
   }
   arrivestr = " " + arrivestr + " ";
   tell_all_but(users, capitalize(describe_one(who)) + arrivestr +
		describe_one(entrance, who) + ".",  who);
}

void desc_teleport(object *users, object who, string msg_first, string msg_third) {
   tell_to(users, msg_first, who);
   tell_all_but(users, msg_third, who);
}
