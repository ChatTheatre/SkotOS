/*
**	/usr/TextIF/cmds/social.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <status.h>
# include <type.h>
# include <SkotOS.h>
# include <base/living.h>
# include <base/preps.h>
# include <base/prox.h>
# include <base.h>

# include "/usr/SkotOS/include/inglish.h"

# include <SAM.h>
# include <XML.h>
# include <TextIF.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/mapping";

private inherit "/usr/XML/lib/xmd";
private inherit "/base/lib/urcalls";
private inherit "/usr/SkotOS/lib/ursocials"; 
private inherit "/usr/SkotOS/lib/auth";

private inherit "/usr/TextIF/lib/misc";

private inherit "/usr/SkotOS/lib/merryapi";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";

# define PARSER_SOCIAL "/usr/TextIF/sys/parser/social"

# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"


/*
**	The function check_objects() returns:
**		-1 if we output an error message to the user,
**		 0 if no items that were not in body's inventory,
**		 1 if all items were in body's inventory
**		 2 if some but not all items were in body's inventory
*/

int check_objects(object user, object body, object verb, NRef *refs) {
   NRef bad, distant;
   int all_bee, some_bee, bee, i;

   all_bee = TRUE;
   for (i = 0; i < sizeof(refs); i ++) {
      if (!ur_target_abstracts(verb) &&
	  ur_abstract(NRefOb(refs[i]), NRefDetail(refs[i]))) {
	 bad = refs[i];
	 refs[i] = nil;
	 continue;
      }
# if 0
      /* if we're trying to slap a detail without prepositions
	 in our (extended) environment, we fail */
      if ((ur_approach(verb) == SA_CLOSE ||
	   ur_approach(verb) == SA_ARMSLENGTH) && !IsPrime(refs[i]) &&
	  !ur_id_prep(NRefOb(refs[i]), NRefDetail(refs[i]), PREP_NEAR) &&
	  body->contained_by(NRefOb(refs[i]))) {
	 distant = refs[i];
	 refs[i] = nil;
	 continue;
      }
# endif
      if (IsPrime(refs[i]) && NRefOb(refs[i]) == body && sizeof(refs) > 1) {
	 /* smiling at ourselves -and- others, skip ourselves */
	 refs[i] = nil;
	 continue;
      }
      bee = (NRefOb(refs[i])->contained_by(body) ||
		  (!IsPrime(refs[i]) && NRefOb(refs[i]) == body));
      all_bee  &= bee;
      some_bee |= bee;
   }
   refs -= ({ nil });
   if (!sizeof(refs)) {
      if (bad) {
	 user->paragraph(capitalize(describe_one(bad)) +
			 " is not a real thing.");
	 return -1;
      }
      if (distant) {
	 user->paragraph("You can't get near to " +
			 describe_one(distant) + ".");
	 return -1;
      }
   }
   if (some_bee) {
      if (all_bee) {
	 return 1;
      }
      return 2;
   }
   return 0;
}

# define TestAdverb(adv) (!!sizeof(SOCIALS->expand_adverb_prefix(adv)))
# define ExactAdverb(adv) (SOCIALS->query_adverb(adv))
# define SanitizeAdverb(adv) {					\
   string *advs;						\
								\
   advs = SOCIALS->expand_adverb_prefix(adv);			\
   if (sizeof(advs) == 0) {					\
      user->paragraph("I don't know the adverb '" + adv + "'."); \
      return;							\
   }								\
   if (sizeof(advs) > 1 && sizeof(advs & ({ adv })) == 0) {	\
      user->paragraph("Do you mean " +				\
		    implode(advs[.. sizeof(advs)-2], ", ") +	\
		    " or " + advs[sizeof(advs)-1] + "?");	\
      return;							\
   }								\
}

# define SetAdverb(adv) {					\
   string *advs;						\
								\
   advs = SOCIALS->expand_adverb_prefix(adv);			\
   if (sizeof(advs) == 0) {					\
      user->paragraph("I don't know the adverb '" + adv + "'."); \
      return;							\
   }								\
   if (sizeof(advs) > 1 && sizeof(advs & ({ adv })) == 0) {	\
      user->paragraph("Do you mean " +				\
		    implode(advs[.. sizeof(advs)-2], ", ") +	\
		    " or " + advs[sizeof(advs)-1] + "?");	\
      return;							\
   }								\
   if (!adverb) {						\
      if (sizeof(advs & ({ adv })) == 1) {			\
          adverb = adv;						\
      } else {							\
          adverb = advs[0];					\
      }								\
   }								\
}

void mutate_in_preposition(object verb, object body, object user,
			   mapping partmap, mapping prepositions,
			   mapping args) {
   string role, *roles;
   int i, j;

   role = verb->get_role_by_preposition("in");
   if (role && !partmap[role]) {
      /* the verb accepts the 'in' preposition and that role is unsatisfied */
      roles = map_indices(partmap);
      for (i = 0; i < sizeof(roles); i ++) {
	 object *envs;
	 mixed *part, tmp;
	 string *words;
	 int sz, j;

	 envs = ({ body->query_environment(), body });

	 if (roles[i]) {
	    if (ur_raw(verb, roles[i])) {
	       continue;
	    }
	    tmp = run_merry(verb, "searchenvs-" + role, "global", args);
	    if (typeof(tmp) == T_ARRAY) {
	       envs = tmp;

	    }
	 }
	 part = partmap[roles[i]];
	 words = part[1];
	 sz = sizeof(words);

	 for (j = sizeof(words)-2; j >= 1; j --) {
	    mixed *tree, *res;

	    if (words[j] == "in") {
	       /* see if all the words refer to an object */
	       if (tree = PARSER_SOCIAL->parse(implode(words[.. j-1], " "))) {
		  res = locate(user, tree, nil, body, nil, envs);

		  if (res[0] == LOC_ONE || res[0] == LOC_MULTI ||
		      res[0] == LOC_ERROR_MULTI) {
		     /* got one! modify various entries */
		     partmap[role] = ({ "O", words[j+1 ..] });
		     prepositions[role] = "in";
		     part[1] = words[.. j-1];
		     return;
		  }
	       }
	       /* see if they refer to an object, followed by an adverb */
	       if (j > 1 && (part[0] == "AOA" || part[0] == "OA")) {
		  if (tree = PARSER_SOCIAL->parse(implode(words[.. j-2], " "))) {
		     res = locate(user, tree, nil, body, nil, envs);
		     if (res[0] == LOC_ONE || res[0] == LOC_MULTI ||
			 res[0] == LOC_ERROR_MULTI) {
			/* got one! modify various entries */
			SanitizeAdverb(words[j-1]);
			partmap[role] = ({ "O", words[j+1 ..] });
			prepositions[role] = "in";
			part[1] = words[.. j-2];
			return;
		     }
		  }
	       }
	       /* see if they refer to an adverb followed by an object */
	       if (j > 1 && part[0] == "AOA") {
		  if (tree = PARSER_SOCIAL->parse(implode(words[1 .. j-1], " "))) {
		     res = locate(user, tree, nil, body, nil, envs);
		     if (res[0] == LOC_ONE || res[0] == LOC_MULTI ||
			 res[0] == LOC_ERROR_MULTI) {
			/* got one! modify various entries */
			SanitizeAdverb(words[0]);
			partmap[role] = ({ "O", words[j+1 ..] });
			prepositions[role] = "in";
			part[1] = words[1 .. j-1];
			return;
		     }
		  }
	       }
	    }
	 }
      }
   }
}

private object select_verb(object body, object *verbs) {
   int sz;
   string id;

   sz = sizeof(verbs);
   id = body->query_property("theatre:id");
   if (id) {
      int i;
      object *tverbs, *gverbs;

      tverbs = allocate(sz);
      gverbs = allocate(sz);
      for (i = 0; i < sz; i++) {
	 if (verbs[i]->query_property("theatre:id") == id) {
	    tverbs[i] = verbs[i];
	 } else if (!verbs[i]->query_property("theatre:id")) {
	    gverbs[i] = verbs[i];
	 }
      }
      tverbs -= ({ nil });
      gverbs -= ({ nil });
      if (sizeof(tverbs) > 0) {
	 verbs = tverbs;
	 DEBUG("[Social] Relevant Theatre verb object(s): " + dump_value(verbs));
      } else {
	 verbs = gverbs;
	 if (!sizeof(verbs)) {
	    return nil;
	 }
      }
   }
   if (sizeof(verbs) > 1) {
      DEBUG("[Social] More than one relevant verb object for " + verbs[0]->query_imperative() + ": " + dump_value(verbs));
   }
   return verbs[0];
}

void cmd_social(object user, object body, mapping args) {
   mapping prepositions, partmap, advposmap;
   string *roles, advpos;
   object verb, *verbs;
   string vstr, adverb, prep, *tmproles, *nop_roles, tmprole;
   mixed **parts, *part;
   int nop, i;
   int ticks;
   mixed *mtime;

   ticks = status()[ST_TICKS];
   mtime = millitime();

   vstr = args["verb"];
   if (!vstr || !strlen(vstr)) {
      error("empty verb");
   }
   parts = args["parts"];
   if (!parts) {
      error("no parts in social");
   }
   args["parts"] = nil;

   if (vstr[0] == '+' && !is_root(user->query_name())
       && !(vstr == "+sk" && sizeof(rfilter(user->query_udat()->query_bodies(), "query_property", "skotos:guest")))) {
      user->paragraph("What?");
      return;
   }

   if (vstr[0] == '!' &&
       !sizeof(rfilter(user->query_udat()->query_bodies(),
		       "query_property", "GUIDE")) &&
       !is_root(user->query_name())) {
      user->paragraph("What?");
      return;
   }

   verbs = SOCIALS->query_verb_obs(vstr);

   if (!verbs || !sizeof(verbs)) {
      /* this is an internal error */
      error("unknown social verb " + vstr);
   }

   args["verb"] = nil;

   verb = select_verb(body, verbs);
   if (!verb) {
      user->paragraph("What?");
      return;
   }

   prepositions = ([ ]);
   partmap = ([ ]);
   advposmap = ([ ]);

   advpos = ADVPOS_PRE_VERB;

   for (i = 0; i < sizeof(parts); i ++) {
      NRef *refs;

      part = parts[i];
      refs = nil;

      switch(part[0]) {
      case "V":
	 if (prep) {
	    error("verb immediately following preposition");
	 }
	 advpos = ADVPOS_PRE_ROLE;
	 break;
      case "P":
	 if (prep) {
	    error("preposition immediately following preposition");
	 }
	 prep = part[1];
	 advpos = ADVPOS_PRE_EVOKE;
	 break;
      case "E":
	 if (prep) {
	    error("evoke immediately following preposition");
	 }
	 break;
      case "O":		/* must be an object */
      case "OA":	/* must be an object, possibly with adverb */
      case "AOA": {	/* can be object or adverb or both */
	 string role;

	 if (prep) {
	    role = verb->get_role_by_preposition(prep);

	    if (!role) {
	       user->paragraph("The verb '" + vstr + "' doesn't accept the preposition '" + prep + "'.");
	       return;
	    }
	    if (prepositions[role]) {
	       if (prep == prepositions[role]) {
		  user->paragraph("Alas, the preposition '" + prep + "' cannot be used twice for this verb.");
	       } else {
		  user->paragraph("Alas, the prepositions '" + prep + "' and '" + prepositions[role] + "' cannot both be used for this verb.");
	       }
	       return;
	    }
	    partmap[role] = part;
	    prepositions[role] = prep;
	    prep = nil;
	    advposmap[role] = advpos = ADVPOS_PRE_EVOKE;
	    break;
	 }
	 if (nop) {
	    user->paragraph("Alas, you must supply more prepositions in this sentence.");
	    return;
	 }
	 partmap[nil] = part;
	 nop = TRUE;
	 prep = nil;
	 advposmap[nil] = advpos = ADVPOS_PRE_ROLE;
	 break;
      }
      case "A": {
	 if (prep) {
	    error("adverb immediately following preposition");
	 }
	 if (adverb) {
	    error("internal error: multiple adverbs");
	 }
	 SetAdverb(args["adverb"]);
         args["adverb:position"] = advpos;
	 break;
      }
      default:
	 error("unknown wombat: " + part[0]);
      }
   }

   if (nop) {
      /* no preposition: start looking for most likely candidate */

      nop_roles = ur_roles(verb) - map_indices(prepositions);

      /* nop_roles can be empty now if direct word is an adverb */
      if (sizeof(nop_roles) > 0) {
	 /* prioritize direct roles */
	 nop_roles = (verb->query_direct_roles() & nop_roles) | nop_roles;

	 /* now make preliminary guess at role */

	 if (sizeof(nop_roles) == 1) {
	    tmprole = nop_roles[0];
	 } else {
	    /* increasingly difficult to guess which role */

	    tmproles = nop_roles & verb->query_required_roles();
	    if (sizeof(tmproles) == 1) {
	       tmprole = tmproles[0];
	    }
	    tmproles = nop_roles & verb->query_direct_roles();
	    if (sizeof(tmproles) == 1) {
	       tmprole = tmproles[0];
	    }
	 }
	 if (tmprole) {
	    partmap[tmprole] = partmap[nil];
	    partmap[nil] = nil;
	    advposmap[tmprole] = advposmap[nil];
	    advposmap[nil] = nil;
	 }
      }
   }

   mutate_in_preposition(verb, body, user, partmap, prepositions, args);

   args["imperative"] = verb->query_imperative();
   args["actor"] = body;
   args["vob"] = verb;

   roles = ({ });

   advpos = nil;

   tmproles = map_indices(partmap);
   for (i = 0; i < sizeof(tmproles); i ++) {
      object *envs;
      string *words, err;
      mixed *res, *tree, ref;
      int bee, sz, advset;
      string role, oldrole;
      NRef *refs;

      tmprole = tmproles[i];
      part = partmap[tmprole];
      prep = prepositions[tmprole];

      refs = nil;
      role = nil;

      words = part[1];
      sz = sizeof(words);

      refs = nil;

      envs = ({ body->query_environment(), body });

      if (tmprole) {
	 mixed tmp;

	 if (ur_raw(verb, tmprole)) {
	    args[tmprole + ":words"] = words;
	    args[tmprole + ":preposition"] = prepositions[tmprole];
	    if (verb->query_second()) {
	       args[tmprole + ":description"] =
		  verb->query_second() + (prep ? " " + prep : "");
	    }
	    prep = nil;
	    continue;
	 }
	 tmp = run_merry(verb, "searchenvs-" + tmprole, "global", args);
	 if (typeof(tmp) == T_ARRAY) {
	    envs = tmp;
	    /* this means we're committed to this role */
	    role = tmprole;
	 }
      }

      err = "I don't understand '" + implode(words, " ") + "'.";


      /* see if all the words refer to an object */
      if (tree = PARSER_SOCIAL->parse(implode(words, " "))) {
	 res = locate(user, tree, nil, body, nil, envs);

	 if (res[0] == LOC_ONE || res[0] == LOC_MULTI) {
	    refs = res[1];
	 } else if (res[0] == LOC_ERROR_MULTI && role) {
	    ref = locate_first(user, 0, tree,
			       ur_filter_object(verb, role),
			       body, envs...);
	    if (!ref) {
	       /* there was an error after all! */
	       return;
	    }
	    refs = ({ ref });
	 } else {
	    err = res[1];
	 }
      }
      /* see if they refer to an object, followed by an adverb */
      if (!refs && sizeof(words) > 1 &&
	  (part[0] == "AOA" || part[0] == "OA")) {

	 if (tree = PARSER_SOCIAL->parse(implode(words[.. sz-2], " "))) {
	    res = locate(user, tree, nil, body, nil, envs);
	    if (res[0] == LOC_ONE || res[0] == LOC_MULTI) {
	       SetAdverb(words[sz-1]);
	       advpos = "after";
	       refs = res[1];
	    } else if (res[0] == LOC_ERROR_MULTI && role) {
	       ref = locate_first(user, 0, tree,
				  ur_filter_object(verb, role),
				  body, envs...);
	       if (!ref) {
		  /* there was an error after all! */
		  return;
	       }
	       refs = ({ ref });
	    } else if (ExactAdverb(words[sz-1]) ||
		       (res[0] == LOC_ERROR_GONE &&
			TestAdverb(words[sz-1]))) {
	       err = res[1];
	    }
	 }
      }

      /* see if they refer to an adverb followed by an object */
      if (!refs && sizeof(words) > 1 && part[0] == "AOA") {
	 if (tree = PARSER_SOCIAL->parse(implode(words[1 ..], " "))) {
	    res = locate(user, tree, nil, body, nil, envs);
	    if (res[0] == LOC_ONE || res[0] == LOC_MULTI) {
	       SetAdverb(words[0]);
	       advpos = "before";
	       refs = res[1];
	    } else if (res[0] == LOC_ERROR_MULTI && role) {
	       ref = locate_first(user, 0, tree,
				  ur_filter_object(verb, role),
				  body, envs...);
	       if (!ref) {
		  /* there was an error after all! */
		  return;
	       }
	       refs = ({ ref });
	    } else if (ExactAdverb(words[0]) ||
		       (res[0] == LOC_ERROR_GONE &&
			TestAdverb(words[0]))) {
	       err = res[1];
	    }
	 }
      }

      /* or maybe just an adverb */
      if (!refs) {
	 if (part[0] == "AOA" && sizeof(words) == 1 &&
	     TestAdverb(words[0])) {
	    SetAdverb(words[0]);
	    args["adverb:position"] = ADVPOS_PRE_PREP;
	    continue;
	 }
      }
      if (!refs) {
	 user->paragraph(err);
	 return;
      }

      bee = check_objects(user, body, verb, refs);
      if (bee < 0) {
	 /* error message already done, just leave */
	 return;
      }
      /* check_objects currently modifies 'refs', e.g. nil elements */
      refs -= ({ nil });

      locate_update(user, refs);

      oldrole = nil;
      if (!role) {
	 if (prep) {
	    role = verb->get_role_by_preposition(prep);
	 } else {
	    if (sizeof(nop_roles) == 0) {
	       user->paragraph("The verb '" + vstr +
				       " 'doesn't take that many objects.");
	       return;
	    }

	    if (sizeof(nop_roles) > 1) {
	       if (bee) {
		  string *r1, *r2;

		  /* prioritize direct, then owned */

		  r1 = nop_roles & verb->query_direct_roles();
		  r2 = nop_roles - r1;

		  nop_roles =
		     ((r1 & verb->query_owned_roles()) | r1) |
		     ((r2 & verb->query_owned_roles()) | r2);
	       } else {
		  string *r1, *r2;

		  /* prioritize unowned, then direct */

		  r2 = nop_roles & verb->query_owned_roles();
		  r1 = nop_roles - r2;

		  nop_roles =
		     ((r1 & verb->query_direct_roles()) | r1) |
		     ((r2 & verb->query_direct_roles()) | r2);
	       }
	    }
	    oldrole = nop_roles[0];
	    nop_roles = (verb->query_required_roles() & nop_roles) |
	       nop_roles;
	    role = nop_roles[0];
	 }
      }
      if (role) {
	 roles += ({ role });
      }

      if (bee != 1 && ur_distance(verb, role) == SA_OWNED) {
	 string tip;

	 /* objects aren't all owned and role demands owned */
	 if (ur_distance(verb, oldrole) != SA_OWNED) {
	    /* aha: all was fine until the requirement re-sort */
	    user->paragraph("The verb '" + vstr + "' needs to work on something in your possession.");
	    return;
	 }
	 /* no -- things were already bad */
	 if (bee == 2) {
	    tip = " Try restricting your selection with the 'my' keyword.";
	 } else {
	    tip = "";
	 }
	 if (!prep) {
	    user->paragraph("You can only '" + vstr + "' things in your possession." + tip);
	 } else {
	    user->paragraph("You can only '" + vstr + " " + prep + "' things in your possession." + tip);
	 }
	 return;
      }
      if (sizeof(refs) > 1 && ur_single(verb, role)) {
	 if (!prep) {
	    user->paragraph("You can only '" + vstr + "' one thing at a time.");
	 } else {
	    user->paragraph("You can only '" + vstr + " " + prep + "' one thing at a time.");
	 }
	 return;
      }
      if (!prep && !ur_direct(verb, role)) {
	 if (!sizeof(ur_prepositions(verb, role))) {
	    error("verb/role " + verb->query_imperative() + "/" + role + " must be either direct or have at least one preposition");
	 }
	 prep = ur_prepositions(verb, role)[0];
	 prepositions[role] = prep;
      }
      args[role] = refs;
      args[role + ":preposition"] = prepositions[role];

      if(adverb && !args["adverb:position"]) {
	 if (advpos == "after" && advposmap[role] == ADVPOS_PRE_ROLE) {
	    if (prep) {
	       args["adverb:position"] = ADVPOS_PRE_EVOKE;
	    } else {
	       args["adverb:position"] = ADVPOS_PRE_PREP;
	    }
	 } else {
	    args["adverb:position"] = advposmap[role];
	 }
      }
      if (verb->query_second()) {
	 args[role + ":description"] =
	    verb->query_second() + (prep ? " " + prep : "");
      }
      prep = nil;
   }

   tmproles = verb->query_required_roles();
   for (i = 0; i < sizeof(tmproles); i ++) {
      if (!member(tmproles[i], roles)) {
	 string *preps;

	 preps = ur_prepositions(verb, tmproles[i]);
	 if (sizeof(preps)) {
	    if (sizeof(preps) == 1) {
	       user->paragraph("The verb '" + vstr + "' needs an object prepositioned by '" + preps[0] + "'.");
	    } else {
	       user->paragraph("The verb '" + vstr + "' needs an object prepositioned by one of: '" + implode(preps, "', '") + "'.");
	    }
	 } else {
	    user->paragraph("The verb '" + vstr + "' needs a direct object.");
	 }
	 return;
      }
   }
   if (ur_evoke(verb) == VERB_REQUIRED && !args["evoke"]) {
      user->message("The verb '" + vstr + "' requires an evoke at the end (e.g. \"hello!\")\n");
      return;
   }

   if (ur_log_evokes(verb) && args["evoke"]) {
      object env;

      env = body->query_environment();

      LOGD->add_entry(ur_log_evokes(verb),
		      describe_one(body) + " in [" +
		      (env ? name(env) : "NO ENV") + "]",
		      args["evoke"]);
   }

   args["adverb"] = adverb;
   args["roles"] = roles;
   args["privacy"] = ur_privacy(verb);
   args["obscurity"] = ur_obscurity(verb);
   args["secrecy"] = ur_secrecy(verb);

   if (!run_merry(verb, "command", "global", args)) {
      catch {
	 "~TextIF/sys/profiler"->add_entry(verb->query_imperative(), ticks - status()[ST_TICKS], mtime, millitime());
      }
      return;
   }

   body->action("social", args);
   catch {
      "~TextIF/sys/profiler"->add_entry(verb->query_imperative(), ticks - status()[ST_TICKS], mtime, millitime());
   }
}

void cmd_GUIDE_social(object user, object body, mapping args) {
    args["verb"] = "!" + args["verb"];
    cmd_social(user, body, args);
}

void cmd_DEV_social(object user, object body, mapping args) {
    args["verb"] = "+" + args["verb"];
    cmd_social(user, body, args);
}

void cmd_raw_social(object user, object body, string verb, string arg) {
   object verb_ob;
   mapping args;

   verb_ob = SOCIALS->query_verb_obs(verb)[0];
   args = ([ ]);

   args["imperative"] = verb_ob->query_imperative();
   args["actor"] = body;
   args["vob"] = verb_ob;
   args["line"] = arg;
   args["is_raw_verb"] = TRUE;

   run_merry(verb_ob, "command", "global", args);
}

void cmd_GUIDE_raw_social(object user, object body, string verb, string arg) {
   object verb_ob;
   mapping args;

   verb = "!" + verb;
   verb_ob = SOCIALS->query_verb_obs(verb)[0];
   args = ([ ]);

   args["imperative"] = verb_ob->query_imperative();
   args["actor"] = body;
   args["vob"] = verb_ob;
   args["line"] = arg;
   args["is_raw_verb"] = TRUE;

   run_merry(verb_ob, "command", "global", args);
}

void cmd_DEV_raw_social(object user, object body, string verb, string arg) {
   object verb_ob;
   mapping args;

   verb = "+" + verb;
   verb_ob = SOCIALS->query_verb_obs(verb)[0];
   args = ([ ]);

   args["imperative"] = verb_ob->query_imperative();
   args["actor"] = body;
   args["vob"] = verb_ob;
   args["line"] = arg;
   args["is_raw_verb"] = TRUE;

   run_merry(verb_ob, "command", "global", args);
}

private
void tell_some_targets(NRef *targets, object *users, object who,
		       mixed ldef, string role, mapping args, mapping exclude) {
   mixed **output;
   int i, sz;

   /* help, what does describe_multi() really do? */
   output = describe_multi(targets, who, STYLE_PRONOUN);

   for (i = 0; i < sizeof(output); i ++) {
      string text;
      object ob;
      mixed them;

      ob = output[i][0];
      text = output[i][1];

      if (ob != who && text && !exclude[ob]) {
	 args += ([ role+"name": text ]);
	 them = capitalize(unsam(ldef, args));
	 them = TAG(them, (args["evoke"] ? "Evoke" : "Social") + ":Second");
	 tell_each(users, who, them, ob);
	 exclude[ob] = TRUE;
      }
   }
}

void desc_social(object *users, object who, object verb, mapping args) {
   mapping me_args, them_args, tob_map, verbs, exclude;
   object *some, *tobs;
   string me, them, form, *vix, str, *roles, drole;
   SAM ldef, ldef1, ldef2, ldef3;
   NRef *trefs;
   int i, j, k, obvious;

   if (typeof(args["exarg-propbase"]) == T_STRING) {
      object exob;

      if (typeof(args["exarg-object"]) == T_OBJECT) {
	 exob = args["exarg-object"];
      } else if (verb) {
	 exob = verb;
      } else {
	 error("exarg-base but neither exarg-object nor verb");
      }
      args = prefixed_map(exob->query_properties(),
			  args["exarg-propbase"] + ":",
			  TRUE) +
	 args;
   }

   vix = map_indices(prefixed_map(args, "vdef:", TRUE));
   verbs = ([ ]);
   for (i = 0; i < sizeof(vix); i ++) {
      mixed v;

      v = args["vdef:" + vix[i]];
      if (typeof(v) != T_ARRAY || sizeof(v) != 2) {
	 error("$vdef:" + vix[i] + " is not a two-element array");
      }
      verbs[vix[i]] = v;
   }

   args["actor"] = who;
   if (verb && !run_merry(verb, "describe", "global", args)) {
      return;
   }

   roles = args["roles"];
   if (!roles) {
      error("missing args[roles]");
   }
   if (!args["adverb"] || typeof(args["adverb:position"]) != T_STRING) {
      /* cleanup -- just in case somebody is confused */
      args["adverb:position"] = nil;
   }

   /* not sure this is the best approach, but let's try for now */
   for (i = 0; i < sizeof(roles); i ++) {
      if (typeof(args[roles[i]]) != T_ARRAY || !sizeof(args[roles[i]])) {
	 /* scripts and handlers have entirely ruled a role out: abort */
	 return;
      }
      if (!drole && verb && ur_direct(verb, roles[i])) {
	 drole = roles[i];
      }
   }
   ldef = args["ldef"];
   ldef1 = args["ldef-actor"];
   ldef2 = args["ldef-target"];
   ldef3 = args["ldef-witness"];

   if (verb && !ldef && !(ldef1 && ldef2 && ldef3)) {
      mixed tmp;

      ldef = ({ });
      if (tmp = args["verb-extra:" + ADVPOS_PRE_SUBJECT]) {
	 ldef += ({ tmp, " " });
      }
      ldef += ({ xmd_ref("you"), " " });
      if (tmp = args["verb-extra:" + ADVPOS_PRE_VERB]) {
	 ldef += ({ tmp, " " });
      }
      if (args["adverb:position"] == ADVPOS_PRE_VERB) {
	 ldef += ({ xmd_ref("adverb"), " " });
      }
      ldef += ({ xmd_ref("verb") });

      if (tmp = args["verb-extra:" + ADVPOS_PRE_ROLE]) {
	 ldef += ({ " ", tmp });
      }
      if (args["adverb:position"] == ADVPOS_PRE_ROLE) {
	 ldef += ({ " ", xmd_ref("adverb") });
      }
      if (drole) {
	 ldef += ({ " ", xmd_ref(drole + "name") });
      }
      if (tmp = args["verb-extra:" + ADVPOS_PRE_PREP]) {
	 ldef += ({ tmp });
      }
      /* make sure adverb is after a direct object but before any indirect */
      if (args["adverb"]) {
	 if (args["adverb:position"] == nil ||
	     args["adverb:position"] == ADVPOS_PRE_PREP) {
	    ldef += ({ " ", xmd_ref("adverb") });
	 }
      }
      for (i = 0; i < sizeof(roles); i ++) {
	 mixed prep;

	 if (roles[i] == drole) {
	    continue;
	 }
	 prep = args[roles[i] + ":preposition"];
	 if (typeof(prep) != T_STRING) {
	    prep = ur_prepositions(verb, roles[i])[0];
	 }
	 ldef += ({ " ", prep, " ", xmd_ref(roles[i] + "name") });
      }
      if (tmp = args["verb-extra:" + ADVPOS_PRE_EVOKE]) {
	 ldef += ({ " ", tmp });
      }
      if (args["adverb:position"] == ADVPOS_PRE_EVOKE) {
	 ldef += ({ " ", xmd_ref("adverb") });
      }
      if (args["evoke"]) {
	 ldef += ({ ", ", xmd_ref("evoke") });
      }
      if (str = args["evoke"]) {
	 switch(str[strlen(str)-2]) {
	 case 'a' .. 'z' : case 'A' .. 'Z': case '0' .. '9':
	    args["evoke"] = str[.. strlen(str)-2] + ".\"";
	 }
      } else {
	 ldef = ldef + ({ "." });
      }
   }

   ldef = args["ldef"] = typeof(ldef) == T_ARRAY ? NewXMLPCData(ldef) : ldef;	/* hacks r us */

   them_args = ([
      "verb": verb ? verb->query_third() : nil,
      ]);

   for (i = 0; i < sizeof(vix); i ++) {
      them_args["verb:" + vix[i]] = verbs[vix[i]][1];
   }


   if (ldef || ldef1) {
      me_args = ([
      "evoke": args["evoke-actor"] ? args["evoke-actor"] : args["evoke"],
      "verb": verb ? verb->query_second() : nil,
      "you": "you",
      "poss": "your",
      "pro": "you",
      ]);
      for (i = 0; i < sizeof(vix); i ++) {
	 me_args["verb:" + vix[i]] = verbs[vix[i]][0];
      }

      for (i = 0; i < sizeof(roles); i ++) {
	 me_args[roles[i]+"name"] = describe_many(args[roles[i]], who, who);
      }
      me = unsam(ldef1 ? ldef1 : ldef, args + me_args);
      me = TAG(capitalize(me), (me_args["evoke"] ? "Evoke" : "Social") + ":First");

      tell_each(users, who, me, who);
   }

   some = who->query_environment()->query_inventory();

   obvious = (verb && ur_audible(verb)) || args["evoke"];

   /* tell the targets */

   tobs = ({ });

   if (!args["secrecy"]) {
      exclude = ([ ]);

      for (i = 0; i < sizeof(roles); i ++) {
	 mixed **classes;

	 trefs = args[roles[i]];

	 tob_map = ([ ]);
	 for (j = 0; j < sizeof(trefs); j ++) {
	    tob_map[NRefOb(trefs[j])] = TRUE;
	 }
	 tobs |= map_indices(tob_map);

	 tob_map = ([ ]);

	 /* priority list: i.e. evoke-dobs, evoke-target, evoke */
	 if ((them_args["evoke"] = args["evoke-" + roles[i]]) == nil) {
	    if ((them_args["evoke"] = args["evoke-target"]) == nil) {
	       them_args["evoke"] = args["evoke"];
	    }
	 }

	 if (ldef || ldef2) {
	    classes = classify_bodies(trefs, tob_map);

	    for (j = 0; j < sizeof(classes); j ++) {
	       if (sizeof(classes[j])) {
		  for (k = 0; k < sizeof(roles); k ++) {
		     if (k != i) {
			them_args[roles[k]+"name"] =
			   describe_many(args[roles[k]], who, classes[j][0],
					 STYLE_WEAK_LOOKER | STYLE_PRONOUN);
		     }
		  }
		  them_args["you"] = describe_one(who, nil, classes[j][0],
						  STYLE_WEAK_LOOKER);
		  if (classes[j][0]->can_identify(who)) {
		     them_args += ([
				      "poss": who->query_possessive(),
				      "pro": who->query_pronoun(),
				      ]);
		     tell_some_targets(tob_map[j], users, who,
				       ldef2 ? ldef2 : ldef,
				       roles[i], args + them_args, exclude);
		  } else if (obvious /* || ur_approach(verb) == SA_CLOSE */) {
		     them_args += ([
				      "poss": "its",
				      "pro": "it",
				      ]);
		     tell_some_targets(tob_map[j], users, who,
				       ldef2 ? ldef2 : ldef,
				       roles[i], args + them_args, exclude);
		  } /* else don't tell */
	       }
	    }
	 }
      }
   }

   /* tell everyone else */

   if ((ldef || ldef3) && !args["privacy"]) {
      object **classes;
      int i, j;

      them_args["evoke"] =
	 args["evoke-witness"] ? args["evoke-witness"] : args["evoke"];

      if (them_args["evoke"] && args["obscurity"]) {
	 /* mask out the evoke */
	 if (ldef) {
	    ldef = query_colour_value(ldef);
	    ldef = NewXMLPCData(ldef[.. sizeof(ldef)-3] + ({ "." }));
	 }
	 if (ldef3) {
	    ldef3 = query_colour_value(ldef3);
	    ldef3 = NewXMLPCData(ldef3[.. sizeof(ldef3)-3] + ({ "." }));
	 }
      }

      some = some - tobs - ({ who });

      /* do a special pass to catch people holding objects */

      tob_map = ([ ]);

      for (i = 0; i < sizeof(some); i ++) {
	 for (j = 0; j < sizeof(tobs); j ++) {
	    if (tobs[j]->contained_by(some[i])) {
	       tob_map[some[i]] = TRUE;
	    }
	 }
      }
      tobs = map_indices(tob_map);
      classes = allocate(sizeof(tobs));
      for (i = 0; i < sizeof(tobs); i ++) {
	 classes[i] = ({ tobs[i] });
      }

      some -= tobs;

      classes += classify_bodies(some);
      
      for (i = 0; i < sizeof(classes); i ++) {
	 if (sizeof(classes[i])) {
	    if (classes[i][0]->can_identify(who)) {
	       them_args["poss"] = who->query_possessive();
	       them_args["pro"] = who->query_pronoun();
	    } else if (obvious) {
	       them_args["poss"] = "its";
	       them_args["pro"] = "it";
	    } else {
	       continue;
	    }
	    for (k = 0; k < sizeof(roles); k ++) {
	       them_args[roles[k]+"name"] =
		  describe_many(args[roles[k]], who, classes[i][0],
				STYLE_WEAK_LOOKER | STYLE_PRONOUN);
	    }
	    them_args["you"] = describe_one(who, nil, classes[i][0],
					    STYLE_WEAK_LOOKER);
	    them = capitalize(unsam(ldef3 ? ldef3 : ldef, args + them_args));
	    them = TAG(them, (them_args["evoke"] ? "Evoke" : "Social") + ":Third");
	    for (j = 0; j < sizeof(classes[i]); j ++) {
	       tell_each(users, who, them, classes[i][j]);
	    }
	 }
      }
   }
}


void desc_noise(object *users, object who, string what, NRef dest) {
   tell_all_but(users, "You hear a " + what + " from the other side of " +
		describe_one(dest) + ".\n");
}
