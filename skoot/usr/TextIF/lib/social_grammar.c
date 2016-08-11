/*
 * /usr/TextIF/lib/social_grammar.c
 *
 * Utilities used in grammars that use social verbs.
 */

# include <type.h>
# include <SkotOS.h>

inherit "/usr/SkotOS/lib/noun";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SAM/lib/sam";

# include "/usr/SkotOS/include/noun.h"

# define PARSER_SOCIAL "/usr/TextIF/sys/parser/social"

# define Zell(s) if (FALSE && this_user() && this_user()->query_name() == "zell") SysLog(s)

# undef Debug
# define Debug(s) Zell(s)

mapping *make_social(mixed *tree) {
   mapping args;
   mixed *out;
   int i;

   out = allocate(sizeof(tree));

   args = ([ ]);

   for (i = 0; i < sizeof(tree); i ++) {
       if (typeof(tree[i]) == T_STRING) {
	  /* Verb is passed on as string, unlike the rest. */
	  out[i] = ({ "V" });
	  args["verb"] = tree[i];
	  continue;
       }
       if (typeof(tree[i]) != T_MAPPING) {
	  SysLog(dump_value(tree[i]));
	  error("foo");
       }
       if (!map_sizeof(tree[i])) {
	  /* an optional parameter returning an empty mapping */
	  continue;
       }
       if (tree[i]["prep"]) {
	  out[i] = ({ "P", tree[i]["prep"] });
	  continue;
       }
       switch(tree[i]["form"]) {
       case "V":
	  out[i] = ({ "V" });
	  args["verb"] = tree[i]["verb"];
	  break;
       case "A":
	  out[i] = ({ "A" });
	  args["adverb"] = tree[i]["adverb"];
	  break;
       case "E":
	  out[i] = ({ "E" });
	  args["raw-evoke"] = tree[i]["raw-evoke"];
	  args["evoke"] = tree[i]["evoke"];
	  break;
       case "O":
       case "OA":
       case "AOA":
	  out[i] = ({ tree[i]["form"], tree[i]["W"] });
	  break;
       default:
	  error("unknown form: " + dump_value(tree[i]));
       }
   }
   return ({ args + ([ "parts": out - ({ nil }) ]) });
}

static
NRef *try_locate(Ob *obs) {
    object user, body;
    mixed res;

    user = previous_object(1)->query_current_user();
    body = previous_object(1)->query_current_body();
    res = locate(user, obs, nil, body, nil,
		 ({ body->query_environment(), body }));
    if (res[0] == LOC_ONE || res[0] == LOC_MULTI) {
       Debug("Doing try_locate() on: " + dump_value(obs) + " -> SUCCESS");
       return res[1];
    }
    if (sscanf(res[1], "You see no %*s") ||
	sscanf(res[1], "%*s has no %*s") ||
	sscanf(res[1], "%*s holds no %*s")) {
       /* if we don't check the latter, 'slide my hand sideways' fails */
       Debug("Doing try_locate() on: " + dump_value(obs) + " -> FAILURE [" + dump_value(res[1]) + "]");
       return nil;
    }
    Debug("Doing try_locate() on: " + dump_value(obs) + " -> COZY ERROR (" + res[1] + ")");
    return ({ });
}

private mixed *sanitize_refs(Ob *ob1, Ob *ob2, Ob *ob3, string *tree);

/*
 * Flag is TRUE if the first word is allowed to be an adverb instead of part
 * of the SocOb component.
 */
mixed *sanitize_soc_adv(mixed *tree, varargs int flag, int verbose) {
   /*
    * Four possibilities:
    * 1. all of this indicates a SocOb.
    * 2. all but the last word indicates a SocOb, and the last word is
    *    an adverb or an abbreviation of one.
    * 3. all but the first word indicates a SocOb, and the first word is
    *    an adverb or an abbreviation of one.
    * 4. tree is only one word, in which case it would have to be a SocOb.
    * Zell:: I don't think this is true anymore, at least not 4)
    */
   mixed sub_tree_1, sub_tree_2, sub_tree_3;
   mixed res;
   int   sz;

   sz = sizeof(tree);

   Debug("Actual tree: " + dump_value(tree));

   sub_tree_1 = PARSER_SOCIAL->parse(implode(tree, " "));
   Debug("SocOb tree: " + dump_value(sub_tree_1));

   if (sz > 1) {
      sub_tree_2 = PARSER_SOCIAL->parse(implode(tree[..sz - 2], " "));
      if (sizeof(SOCIALS->expand_adverb_prefix(tree[sz - 1])) == 0) {
	 sub_tree_2 = nil;
      }
      Debug("SocOb Adv tree: " + dump_value(sub_tree_2) + "/" +
	    dump_value(SOCIALS->expand_adverb_prefix(tree[sz-1])));

      if (flag) {
	 sub_tree_3 = PARSER_SOCIAL->parse(implode(tree[1..], " "));
	 if (sizeof(SOCIALS->expand_adverb_prefix(tree[0])) == 0) {
	    sub_tree_3 = nil;
	 }
	 Debug("Adv SocOb tree: " + dump_value(sub_tree_3) + "/" +
	       dump_value(SOCIALS->expand_adverb_prefix(tree[0])));
      }
   }
   res = sanitize_refs(sub_tree_1, sub_tree_2, sub_tree_3, tree);
   if (!res) {
# if 0
      if (sz == 1 && sizeof(SOCIALS->expand_adverb_prefix(tree[0])) > 0) {
	 return ({ ([ "form": "A", "adverb": tree[0] ]) });
      }
# endif

      return ({ ([ "form": "W", "W": tree, "O": sub_tree_1 ]) });
   }
   return res;
}

private
mixed *sanitize_refs(Ob *ob1, Ob *ob2, Ob *ob3, string *tree) {
   int options, sz;
   NRef *refs_1, *refs_2, *refs_3;

   sz = sizeof(tree);

   /*
    * Give all 3 options a closer look, see if we can rule out any
    * of 'em by checking what they refer to.
    */

   Debug("in sanitize_refs; ob1 = " + dump_value(ob1));
   Debug("in sanitize_refs; ob2 = " + dump_value(ob2));
   Debug("in sanitize_refs; ob3 = " + dump_value(ob3));

   if (ob1) {
      catch {
	 if (refs_1 = try_locate(ob1)) {
	    options ++;
	 }
      }
   }
   if (ob2) {
      catch {
	 if (refs_2 = try_locate(ob2)) {
	    options ++;
	 }
      }
   }
   if (ob3) {
      catch {
	 if (refs_3 = try_locate(ob3)) {
	    options ++;
	 }
      }
   }
   if (options == 0) {
      Debug("sanitize_refs: no options, return nil");
      return nil;
   }
   if (options == 1) {
      /* Good, only one was unambiguous, return it. */
      if (refs_1 && sizeof(refs_1)) {
	 Debug("sanitize_refs returning O");
	 return ({ ([ "form": "O", "O": refs_1 ]) });
      }
      if (refs_2 && sizeof(refs_2)) {
	 Debug("sanitize_refs returning O, A");
	 return ({
	    ([ "form": "O", "O": refs_2 ]),
	    ([ "form": "A", "adverb": tree[sz - 1] ])
	       });
      }
      /* Must be #3 */
      if (refs_3 && sizeof(refs_3)) {
	 Debug("sanitize_refs returning A, O");
	 return ({
	    ([ "form": "A", "adverb": tree[0] ]),
	       ([ "form": "O", "O": refs_3 ])
	       });
      }
      if (refs_1) {
	 Debug("sanitize_refs returning X");
	 return ({ ([ "form": "X", "X": ob1 ]) });
      }
      if (refs_2) {
	 Debug("sanitize_refs returning X, A");
	 return ({
	    ([ "form": "X", "X": ob2 ]),
	    ([ "form": "A", "adverb": tree[sz - 1] ])
	       });
      }
      if (!refs_3) {
	 error("internal errors: refs_3 == nil");
      }
      Debug("sanitize_refs returning A, X");
      return ({
	 ([ "form": "A", "adverb": tree[0] ]),
	    ([ "form": "X", "X": ob3 ])
	    });
   }
   /* Bugger, options > 1, still ambiguous. */
   if (refs_1) {
      /*
       * Favour this one over the others, you can always put the adverb
       * in a different position.
       */
      if (sizeof(refs_1)) {
	 Debug("sanitize_refs returning ambiguous O");
	 return ({ ([ "form": "O", "O": refs_1 ]) });
      }
      Debug("sanitize_refs returning ambiguous X");
      return ({ ([ "form": "X", "X": ob1 ]) });
   }
   Debug("on your own: tree is " + dump_value(tree));
   /* You're on your own, kid. */
   return ({ ([ "form": "W", "W": tree, "O": ob1 ]) });
}

mixed *sanitize_prep_ob(mixed *tree) {
    int i, sz;
    string *prep_words;
    string *ob_words;

    prep_words = tree[0];
    ob_words = tree[1..];

    sz = sizeof(prep_words);
    for (i = sz - 1; i >= 0; i--) {
        mixed *sub_tree;

        sub_tree = PARSER_SOCIAL->parse(implode(prep_words[i + 1..] + ob_words, " "));
        if (!sub_tree) {
            /* No parse no */
            continue;
        }
        sub_tree = sub_tree[0];
        if (!try_locate(({ sub_tree }))) {
            /* Parses properly but cannot be found. */
            continue;
        }
        return ({ implode(prep_words[..i], " "), sub_tree });
    }
    return nil;
}
