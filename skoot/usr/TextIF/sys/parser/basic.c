/*
 * This is the object taking care of regular verbs, not to be confused with the
 * extra '!..' and '+..' commands that storyguides and storyhosts have at their
 * disposal.
 */

# include <SkotOS.h>

inherit map   "/lib/mapping";
inherit parse "~/lib/parser";
inherit "~SkotOS/lib/ursocials";

private int    dirty;
private string grammar;

void read_grammar(varargs int verify);

private mapping verbs_free, verbs_other, verbs_total, verbs_raw;
private mapping verbs_soul, adverbs_soul;

static void create() {
    parse::create("~/tmp/basic_scratch");
    verbs_soul   = ([ ]);
    adverbs_soul = ([ ]);
    verbs_other  = ([ ]);
}

void boot(int block) {
    if (ur_name(previous_object()) == "/usr/TextIF/main") {
	read_grammar();
	catch(subscribe_event(find_object(CONFIGD), "updated"));
	catch(subscribe_event(find_object(SOCIALS), "updated"));
    }
}

void patch() {
	read_grammar();
	catch(subscribe_event(find_object(CONFIGD), "updated"));
	catch(subscribe_event(find_object(SOCIALS), "updated"));
}

/*
 * Return values:
 * nil:      Couldn't make -any- sense of it.
 * <string>: Rewritten commandline.
 * <array>:  First element contains the verb that was ambiguous, the second
 *           element contains a list of possible completions.
 */
mixed guess_the_verb(string str) {
    string *args;

    args = explode(str, " ");
    switch (sizeof(args)) {
    case 0:
    case 1:
    {
	string *verbs;

	/* 'str' has to be a verb, then. */
	if (verbs_total[args[0]]) {
	    return str;
	}
	verbs = map_indices(prefixed_map(verbs_total, args[0]));
	switch (sizeof(verbs)) {
	case 0:
	    return nil;
	case 1:
	    /* Good, no ambiguity, let's go for it */
	    return implode(verbs[0..0] + args[1..], " ");
	default:
	    /* 2+ is too ambiguous, so let the user know. */
	    return ({ "completions", args[0], verbs });
	}
    } /* case 1: */
    default: {
	/* 2+, possibly of the format <adverb> <verb> */
	string *verbs_arg0, *verbs_arg1, *adverbs_arg0;

	if (verbs_total[args[0]]) {
	    /* Let the grammar and verb-code deal with it. */
	    return str;
	}
        if (verbs_soul[args[1]]) {
	    /*
	     * Let the grammar and verb-code deal with it.  Checking the adverb
	     * is pointless since the verb-code does that rather nicely
	     * already.
	     *
	     * Maybe some basic checking can be moved elsewhere later on. 
	     *
	     * Only do this if args[0] is a potential adverb. 2005/01/26 - Erwin.
	     */
	    if (map_sizeof(prefixed_map(adverbs_soul, args[0])) > 0) {
		return str;
	    }
	}

	/*
	 * Now there are two possibilities:
	 * 1. <verb> [<args...>]
	 * 2. <adverb> <soul_verb> [<args...>]
	 *
	 * Investigate all options and if more than one is possibly, somehow
	 * notify the user of this ambiguity.  Otherwise resolve the problem.
	 */
	verbs_arg0 = map_indices(prefixed_map(verbs_total, args[0]));
	verbs_arg1 = map_indices(prefixed_map(verbs_soul + ([ ]), args[1]));
	adverbs_arg0 = map_indices(prefixed_map(adverbs_soul, args[0]));
	if (sizeof(verbs_arg0)   > 0 &&
	    sizeof(verbs_arg1)   > 0 &&
	    sizeof(adverbs_arg0) > 0) {
	    /* Okay, this is too ambiguous to fix automatically. */
	    return ({ "ambiguous", verbs_arg0, verbs_arg1, adverbs_arg0 });
	}
	switch (sizeof(verbs_arg0)) {
	case 0:
	    break;
	case 1:
	    return implode(verbs_arg0[0..0] + args[1..], " ");
	default:
	    /* 2+ options. */
	    return ({ "completions", args[0], verbs_arg0 });
	}
	switch (sizeof(verbs_arg1)) {
	case 0:
	    /* Can't make -any- sense of this, eh. */
	    return nil;
	case 1:
	    return implode(args[0..0] + verbs_arg1[0..0] + args[2..], " ");
	default:
	    /* 2+ options. */
	    return ({ "completions", args[1], verbs_arg1 });
	}
    } /* default */
    } /* switch */
}

/*
 * Interface for ~TextIF/main.c to use.
 */
mixed *parse(string str, object user, object body) {
    string verb, arg;
    mixed result;

    if (dirty) {
        read_grammar(TRUE);
        dirty = FALSE;
    }
    result = "~/sys/parser/generic"->parse(str, user, body, verbs_free);
    if (!result) {
	return nil;
    }
    str = result;
    result = guess_the_verb(str);
    switch (typeof(result)) {
    case T_STRING:
	str = result;
	break;
    case T_ARRAY:
	return ({ result[0], str }) + result[1..];
    default:
	return nil;
    }
    if (sscanf(str, "%s %s", verb, arg) < 2) {
        verb = str;
        arg = nil;
    }
    if (verbs_free && verbs_free[verb]) {
	object handler;

	switch (typeof(verbs_free[verb])) {
	case T_INT:
	    return ({ verb, arg });
	case T_STRING:
	    handler = find_object(verbs_free[verb]);
	    if (handler) {
		return ({ ({ handler, verb }), arg });
	    }
	    return nil;
	case T_OBJECT:
	    return ({ ({ verbs_free[verb], verb }), arg });
	case T_ARRAY:
	    return ({ verbs_free[verb][..], verb, arg });
	}
    }
    return parse_string(grammar, str, user, body);
}

string query_grammar() {
    return grammar;
}

int confirm_changes() {
    int i;
    string *ix, *old;

    old = map_indices(adverbs_soul);
    ix = SOCIALS->query_all_adverbs();
    if (sizeof(ix - old) || sizeof(old - ix)) {
	DEBUG("confirm_changes/basic: query_all_adverbs() changed.");
	return TRUE;
    }

    old = map_indices(verbs_soul);
    ix = SOCIALS->query_verbs();
    for (i = 0; i < sizeof(ix); i++) {
	if (ix[i][0] == '!' || ix[i][0] == '+') {
	    ix[i] = nil;
	}
    }
    ix -= ({ nil });

    if (sizeof(ix - old) || sizeof(old - ix)) {
	DEBUG("confirm_changes/basic: query_verbs() changed.");
	return TRUE;
    }

    if (!verbs_raw) {
	return TRUE;
    }
    old = map_indices(verbs_raw);
    for (i = 0; i < sizeof(ix); i++) {
	object *obs;

	obs = SOCIALS->query_verb_obs(ix[i]);
	if (sizeof(obs) != 1 || !ur_raw_verb(obs[0])) {
	    ix[i] = nil;
	}
    }
    ix -= ({ nil });
    if (sizeof(ix - old) || sizeof(old - ix)) {
	DEBUG("confirm_changes/basic: raw verbs changed.");
	return TRUE;
    }
    return FALSE;
}

void read_grammar(varargs int verify) {
   string pre, post;
   string *ix, *preps;
   mixed *data;
   int i;

   if (verify) {
       /*
	* Make sure something actually changed before doing a huge amount of
	* work on this.
	*/
       if (!confirm_changes()) {
	   DEBUG("read_grammar/basic: Nothing noticable changed, ignoring call.");
	   return;
       }
   }

   /* Update (ad)verbs_soul while we're here as well. */
   adverbs_soul = ([ ]);
   ix = SOCIALS->query_all_adverbs();
   for (i = 0; i < sizeof(ix); i++) {
       adverbs_soul[ix[i]] = 1;
   }

   verbs_soul = ([ ]);
   ix = SOCIALS->query_verbs();
   for (i = 0; i < sizeof(ix); i++) {
       if (ix[i][0] != '!' && ix[i][0] != '+') {
	   verbs_soul[ix[i]] = 1;
       }
   }

   /* Now read the actual grammar. */
   SysLog("Reading basic.y file...");
   grammar = read_file("~/grammar/basic.y");
   if (sscanf(grammar, "%s<!--SOUL_VERBS-->%s", pre, post)) {
      for (i = 0; i < sizeof(ix); i ++) {
	 if (strlen(ix[i]) && ix[i][0] != '!' && ix[i][0] != '+') {
	     object *obs;

	     obs = SOCIALS->query_verb_obs(ix[i]);
	     if (sizeof(obs) != 1 || !ur_raw_verb(obs[0])) {
		 ix[i] = "<VERB value=\"" + ix[i] + "\"/>";
	     }
	 }
      }
      grammar = pre + implode(ix, "\n") + "\n" + post;
   } else {
       error("no SOUL_VERBS in basic.y");
   }
   data = preprocess_grammar(grammar);
   grammar     = data[0];
   verbs_free  = data[1];
   verbs_other = data[2];
   verbs_raw   = ([ ]);
   ix = SOCIALS->query_verbs();
   for (i = 0; i < sizeof(ix); i++) {
       if (strlen(ix[i]) && ix[i][0] != '!' && ix[i][0] != '+') {
	   object *obs;

	   if (verbs_free[ix[i]]) {
	       verbs_free[ix[i]] = nil;
	   }
	   obs = SOCIALS->query_verb_obs(ix[i]);
	   if (sizeof(obs) == 1 && ur_raw_verb(obs[0])) {
	       verbs_free[ix[i]] = ({ find_object("/usr/TextIF/cmd/social"), "raw_social" });
	       verbs_raw[ix[i]] = 1;
	   }
       }
   }
   verbs_total = verbs_soul + ([ ]) + verbs_free + verbs_other;
   SysLog("Loaded basic.y file.");
}

void evt_updated(object ob, varargs mixed extra...) {
    switch (ur_name(ob)) {
    case CONFIGD:
	switch (extra[0]) {
	case "Base:Skills:Grammar":
	    /* Ah, this one we use. */
	    break;
	default:
	    /* Others aren't interesting to us. */
	    return;
	}
	/* Fall-through */
    case SOCIALS:
        dirty = TRUE;
	break;
    default:
	/* Who is this? */
	break;
    }
}

mapping query_raw_verbs() {
    return verbs_raw[..];
}
