/*
 * This is the generic pre-parser that takes a command, assuming it's not a
 * freeform verb, and split it up into tokens, all lowercased except for the
 * optional evoke at the end.
 */

inherit "/lib/string";
inherit "/lib/mapping";

inherit parse "~/lib/parser";

private string grammar;

void read_grammar();

static void create() {
    parse::create("~/tmp/generic_scratch");
}

void boot(int block) {
    if (ur_name(previous_object()) == "/usr/TextIF/main") {
	read_grammar();
    }
}

string parse(string str, varargs object user, object body, mapping free) {
    /* Check for freeforms stuff first */
    string *args;
    mixed result;

    str = strip(str);
    if (!strlen(str)) {
	/* Not a lot to say, have you? */
	return nil;
    }
    args = explode(str, " ");
    if (free) {
	string *verbs;

	verbs = map_indices(prefixed_map(free, lower_case(args[0])));
	if (sizeof(verbs)) {
	    args[0] = lower_case(args[0]);
	    return implode(args, " ");
	}
    }
    result = parse_string(grammar, str, user, body);
    return result ? implode(result[0], " ") : nil;
}

string query_grammar() {
    return grammar;
}

void read_grammar() {
    SysLog("Reading generic.y file...");
    grammar = read_file("~/grammar/generic.y");
    SysLog("Loaded basic.y file.");
}
