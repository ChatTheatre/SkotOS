/*
**	/usr/TextIF/cmds/stance.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>
# include <base/living.h>
# include <base/act.h>
# include <base/preps.h>

private inherit "/lib/array";
private inherit "/base/lib/urcalls";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/noun";
inherit "/usr/SAM/lib/sam";

# include "/usr/SkotOS/include/inglish.h"
# include "/usr/SkotOS/include/noun.h"

void cmd_stance(object user, object body, string verb,
		string dir, string pstr, Ob *tref) {
   mapping stance_map;
   mixed *stance_data;
   NRef target;
   int resval, prep, badness;

   /*
   **	Data array for a specific stance is ({
   **		stance number, from <base/living.h>,
   **		array of pre-prepositions allowed for untargeted mode,
   **			e.g. 'sit down next to xena'
   **		array of prepositions for targeted peer mode,
   **			e.g. 'kneel before saint'
   **	})
   */

   stance_map = ([
      "crouch": ({ STANCE_CROUCH, ({ "down" }),
		   ({ PREP_BEFORE }) }),
      "kneel": ({ STANCE_KNEEL, ({ "down"}),
		  ({ PREP_BEFORE, "to" }) }),
      "lie": ({ STANCE_LIE, ({ "down" }),
		({ PREP_NEAR }) }),
      "sit": ({ STANCE_SIT, ({ "down", "up" }),
		({ PREP_NEAR }) }),
      "stand": ({ STANCE_STAND, ({ "up" }),
		  ({  }) })
      ]);


   stance_data = stance_map[verb];
   if (!stance_data) {
      error("unknown stance");
   }
   if (dir && !member(dir, stance_data[1])) {
      user->message("I don't know how to " + verb + " that way.\n");
      return;
   }

   switch(pstr) {
   case "close to": case "close by": case "close beside":
      prep = PREP_CLOSE_TO;
      break;
   case "against":
      prep = PREP_AGAINST;
      break;
   case "under": case "beneath": case "below":
      prep = PREP_UNDER;
      break;
   case "on": case "upon": case "on top of": case "atop":
      prep = PREP_ON;
      break;
   case "inside": case "in": case "within":
      prep = PREP_INSIDE;
      break;
   case "near": case "near to": case "by": case "nearby": case "at":
      prep = PREP_NEAR;
      break;
   case "over": case "above":
      prep = PREP_OVER;
      break;
   case "behind":
      prep = PREP_BEHIND;
      break;
   case "before": case "in front of": case "across from":
      prep = PREP_BEFORE;
      break;
   case "beside": case "next to":
      prep = PREP_BESIDE;
      break;
   case nil:
      prep = PREP_NEAR;
      pstr = "near";
      break;
   default:
      user->message("I'm confused by the preposition '" + pstr + "'.\n");
      return;
   }

   /* tref is the parse tree indicating the 'target' in the command */
   if (tref) {
      /* either e.g. 'kneel down before velvet' or 'sit on the floor' */
      target = locate_one(user, FALSE, ({ tref }), body, body->query_environment());
      if (target) {
	 /* are we already doing precisely what was ordered? */
	 if (stance_data[0] == body->query_stance() &&
	     NRefsEqual(target, body->query_proximity()) &&
	     prep == body->query_preposition()) {
	    user->message("You are already " + body->describe_stance() +
			  " " + body->describe_preposition() + " " +
			  describe_one(target) + "!\n");
	    return;
	 }
	 /* currently you cannot be your own target */
	 if (NRefOb(target) == body) {
	    user->message("You can't " + verb + " " + pstr +
			  " yourself!\n");
	    return;
	 }

	 if (ur_abstract(NRefOb(target), NRefDetail(target))) {
	    user->message("You can't " + verb + " " + pstr + " " +
			  describe_one(target) + ".\n");
	    return;
	 }

	 /* TODO: this should almost certainly go into the act_ */
	 if (IsPrime(target)) {
	    switch(prep) {
	    case PREP_CLOSE_TO:
	    case PREP_AGAINST:
	    case PREP_ON:
	    case PREP_NEAR:
	    case PREP_BEHIND:
	    case PREP_BEFORE:
	    case PREP_BESIDE:
	       badness = FALSE;
	       break;
	    default:
	       badness = TRUE;
	       break;
	    }
	 } else {
	    badness = !ur_id_prep(NRefOb(target), NRefDetail(target), prep);
	 }
	 if (badness) {
	    user->message("You can't " + verb + " " + pstr + " " +
			  describe_one(target) + ".\n");
	    return;
	 }

	 /* if we pass all these checks: do it */

	 body->action("approach", ([
	    "what" : target,
	    "consent_mapping" : ([
	       "message": verb + " " + pstr,
	       "action": "stance",
	       "args": ([
		  "stance" : stance_data[0], 
		  "prep" : prep,
		  "target" : target,
		  ]),
	       ]),
	    ]));
      }
      return;
   }
   /* e.g. 'sit', 'sit down', or 'sit on the floor' from above */
   if (stance_data[0] == body->query_stance()) {
      user->message("You are already " + body->describe_stance() + "!\n");
      return;
   }
   /* catch specifically standing up */
   if (stance_data[0] == STANCE_STAND &&
       body->query_preposition() == PREP_ON) {
      prep = PREP_NEAR;
   } else {
      /* special signal: don't clear current preposition */
      prep = 0;
   }

   body->action("stance", ([
      "stance" : stance_data[0],
      "prep": prep,
      "target" : body->query_proximity() ]));
		 
}

void desc_stance(object *users, object who, int old, int new_stance,
		 varargs int prop, NRef target) {
   string me, him, them;

   if (target) {
      string p1, p2, prep;

      prep = who->describe_preposition();

      p1 = prep + " " + describe_one(target) + ".\n";
      p2 = prep + " you.\n";

      switch(new_stance) {
      case STANCE_CROUCH:
	 if (old == STANCE_LIE) {
	    me = "You get up into a crouch " + p1;
	    him = describe_one(who) + " gets up into a crouch " + p2;
	    them = describe_one(who) + " gets up into a crouch " + p1;
	 } else {
	    me = "You crouch down " + p1;
	    him = describe_one(who) + " crouches down " + p2;
	    them = describe_one(who) + " crouches down " + p1;
	 }
	 break;
      case STANCE_KNEEL:
	 me = "You kneel " + p1;
	 him = describe_one(who) + " kneels " + p2;
	 them = describe_one(who) + " kneels " + p1;
	 break;
      case STANCE_LIE:
	 me = "You lie down " + p1;
	 him = describe_one(who) + " lies down " + p2;
	 them = describe_one(who) + " lies down " + p1;
	 break;
      case STANCE_SIT:
	 if (old == STANCE_LIE) {
	    /* let's not do 'sits up near zellski', ugly! */
	    me = "You sit " + p1;
	    him = describe_one(who) + " sits " + p2;
	    them = describe_one(who) + " sits " + p1;
	 } else {
	    me = "You sit down " + p1;
	    him = describe_one(who) + " sits down " + p2;
	    them = describe_one(who) + " sits down " + p1;
	 }
	 break;
      case STANCE_STAND:
	 me = "You stand up " + p1;
	 him = describe_one(who) + " stands up " + p2;
	 them = describe_one(who) + " stands up " + p1;
	 break;
      default:
	 error("unknown stance");
      }
      tell_to(users, me, who);
      if (IsPrime(target)) {
	 tell_to(users, him, NRefOb(target));
	 tell_all_but(users, them, who, NRefOb(target));
      } else {
	 tell_all_but(users, them, who);
      }
      return;
   }
   switch(new_stance) {
   case STANCE_CROUCH:
      if (old == STANCE_LIE) {
	 tell(users, "You get up on your hands and knees.\n",
	      describe_one(who) + " gets up on " +
	      who->query_possessive() + " hands and knees.\n", who);
	 return;
      }
      tell(users, "You get down on your hands and knees.\n",
	   describe_one(who) + " gets down on " +
	   who->query_possessive() + " hands and knees.\n", who);
      return;
   case STANCE_KNEEL:
      tell(users, "You kneel.\n",
	   describe_one(who) + " kneels.\n", who);
      return;
   case STANCE_LIE:
      tell(users, "You lie down.\n",
	   describe_one(who) + " lies down.\n", who);
      return;
   case STANCE_SIT:
      if (old == STANCE_LIE) {
	 tell(users, "You sit up.\n", describe_one(who) + " sits up.\n", who);
	 return;
      }
      tell(users, "You sit down.\n", describe_one(who) + " sits down.\n", who);
      return;
   case STANCE_STAND:
      tell(users, "You stand up.\n",
	   describe_one(who) + " stands up.\n", who);
      return;
   default:
      error("unknown stance");
   }
}
