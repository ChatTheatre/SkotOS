/*
**	This subclass of /lib/parse.c allows the specification
**	of a grammar filename during initialization, making the
**	parse_string() call even simpler.
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/lib/parse";

private string grammar;
private string grammar_file;
private int grammar_stamp;

static
void create(string scratch, string file, varargs int debug) {
   ::create(scratch, debug);

   grammar_file = file;
   grammar_stamp = 0;
}

static
mixed *parse_string(string str, varargs int trees) {
   int stamp;

   stamp = file_stamp(grammar_file);
   if (stamp < 0) {
      error("grammar file not found");
   }
   if (stamp > grammar_stamp) {
      grammar_stamp = stamp;
      grammar = read_file(grammar_file);
   }
   return ::parse_string(grammar, str, trees);
}
