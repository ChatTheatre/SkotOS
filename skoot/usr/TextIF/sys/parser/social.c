inherit parse "/lib/parse";

private string grammar;

void read_grammar();

static void create() {
    parse::create("~/tmp/social_scratch");
}

void boot(int block) {
    if (ur_name(previous_object()) == "/usr/TextIF/main") {
	read_grammar();
    }
}

mixed *parse(string str) {
   mixed *res;

   if (res = parse_string(grammar, str)) {
      return res[0];
   }
   return nil;
}

void read_grammar() {
    grammar = read_file("~/grammar/social.y");
}
