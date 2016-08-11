inherit parse "/lib/parse";

private string grammar;

void read_grammar();

static void create() {
    parse::create("~/tmp/debug_scratch");
}

void boot(int block) {
    if (ur_name(previous_object()) == "/usr/TexIF/main") {
	read_grammar();
    }
}

mixed *parse(string str) {
    return parse_string(grammar, str);
}

void read_grammar() {
    grammar = read_file("~/grammar/debug.y");
}
