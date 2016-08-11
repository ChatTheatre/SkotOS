# include <type.h>
# include <SkotOS.h>

inherit map   "/lib/mapping";
inherit parse "~/lib/parser";
inherit "~SkotOS/lib/ursocials";

private int    dirty;
private string grammar;

private mapping verbs_free, verbs_other, verbs_total, verbs_raw;
private mapping verbs_soul, adverbs_soul;

void read_grammar(varargs int verify);

static void create() {
    parse::create("~/tmp/guide_scratch");
    verbs_free  = ([ ]);
    verbs_other = ([ ]);
    verbs_total = ([ ]);
    verbs_soul = ([ ]);
    adverbs_soul = ([ ]);
}

void patch() {
    subscribe_event(find_object(SOCIALS), "updated");
}

void boot(int block) {
    if (ur_name(previous_object()) == "/usr/TextIF/main") {
	read_grammar();
	catch(subscribe_event(find_object(SOCIALS), "updated"));
    }
}

string query_grammar() {
    return grammar;
}

mixed *parse(string str, object user, object body) {
    string verb, arg, result;

    if (dirty) {
       read_grammar(TRUE);
       dirty = FALSE;
    }
    result = "~/sys/parser/generic"->parse(str, user, body, verbs_free);
    if (!result) {
	return nil;
    }
    str = result;
    /* First check for one of the verbs with freeform arguments first. */
    if (sscanf(str, "%s %s", verb, arg) < 2) {
	verb = str;
	arg = nil;
    }
    if (!verbs_total[verb]) {
	/* See if it's an unambiguous prefix of either one. */
	string *verbs;

	verbs = map_indices(prefixed_map(verbs_total, verb));
	if (sizeof(verbs) > 1) {
	    return ({ "completions", str, verb, verbs });
	}
	if (sizeof(verbs) == 0) {
	    return nil;
	}
	verb = verbs[0];
	/* Reconstruct the original line with the completed verb. */
	str = verb + (arg ? " " + arg : "");
    }
    if (verbs_free[verb]) {
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

int confirm_changes() {
    int i;
    string *ix, *old;

    old = map_indices(adverbs_soul);
    ix = SOCIALS->query_all_adverbs();
    if (sizeof(ix - old) || sizeof(old - ix)) {
	DEBUG("confirm_changes/guide: query_all_adverbs() changed.");
	return TRUE;
    }

    old = map_indices(verbs_soul);
    ix = SOCIALS->query_verbs();
    for (i = 0; i < sizeof(ix); i++) {
        if (ix[i][0] != '!') {
	    ix[i] = nil;
	}
    }
    ix -= ({ nil });

    if (sizeof(ix - old) || sizeof(old - ix)) {
	DEBUG("confirm_changes/guide: query_verbs() changed.");
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
	DEBUG("confirm_changes/guide: raw verbs changed.");
	return TRUE;
    }
    return FALSE;
}

void read_grammar(varargs int verify) {
    string pre, post;
    string *ix;
    mixed *data;
    int i;

    if (verify) {
       /*
	* Make sure something actually changed before doing a huge amount of
	* work on this.
	*/
       if (!confirm_changes()) {
	   DEBUG("read_grammar/guide: Nothing noticable changed, ignoring call.");
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
        if (ix[i][0] == '!') {
	    verbs_soul[ix[i]] = 1;
	}
    }


    SysLog("Reading guide.y file...");
    grammar = read_file("~/grammar/guide.y");
    if (sscanf(grammar, "%s<!--SOUL_VERBS-->%s", pre, post)) {
        for (i = 0; i < sizeof(ix); i ++) {
	    if (strlen(ix[i]) && ix[i][0] == '!') {
		object *obs;

		obs = SOCIALS->query_verb_obs(ix[i]);
		if (sizeof(obs) != 1 || !ur_raw_verb(obs[0])) {
		    ix[i] = "<VERB value=\"" + ix[i][1..] + "\"/>";
		}
	    }
	}
	grammar = pre + implode(ix, "\n") + "\n" + post;
    } else {
        error("no SOUL_VERBS in guide.y");
    }
    data = preprocess_grammar(grammar);
    grammar     = data[0];
    verbs_free  = data[1];
    verbs_other = data[2];
    verbs_raw   = ([ ]);
    ix = SOCIALS->query_verbs();
    for (i = 0; i < sizeof(ix); i++) {
        if (strlen(ix[i]) && ix[i][0] == '!') {
	    object *obs;

	    if (verbs_free[ix[i][1..]]) {
		verbs_free[ix[i][1..]] = nil;
	    }
	    obs = SOCIALS->query_verb_obs(ix[i]);
	    if (sizeof(obs) == 1 && ur_raw_verb(obs[0])) {
		verbs_free[ix[i][1..]] = ({ find_object("/usr/TextIF/cmd/social"), "raw_social" });
		verbs_raw[ix[i]] = 1;
	    }
	}
    }
    verbs_total = verbs_free + verbs_other;
    SysLog("Loaded guide.y file.");
}

void evt_updated(object ob) {
    dirty = TRUE;
}
