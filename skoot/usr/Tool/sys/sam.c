/*
 * ~Tool/sys/sam.c
 *
 * SAM command parser.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include "~/include/tool.h"

private inherit "/lib/string";
        inherit "~/lib/sam";

void womble_xml() {
  ::womble_xml();
}

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
      ({ "sam exec quote",       "cmd_sam_exec"          }),
      ({ "sam exec <anything>",  "cmd_sam_exec",     "2" }),
      ({ "sam info",             "cmd_sam_info"          }),
      ({ "sam info <text>",      "cmd_sam_info",     "2" }),
      ({ "sam parse <anything>", "cmd_sam_parse",    "2" }),
      ({ "sam twiki",            "cmd_sam_twiki"         }),
      ({ "sam <anything>",       "cmd_sam_fallback", "1" }),
      ({ "sam",                  "cmd_sam_fallback",     }),
   });
}

void
cmd_sam_fallback(object user, object body, varargs string arg)
{
   if (arg) {
      user->message("+tool sam: Did not understand \"" + arg + "\", see \"+tool help sam\" for more information.\n");
   } else {
      user->message("+tool: See \"+tool help sam\" for more information.\n");
   }
}

string
help_sam(varargs string topic)
{
   switch (topic) {
   case nil:
      return
	 "Various commands to allow you to experiment with bits of SAM.";
   case "exec":
      return
	 "Execute a piece of SAM to see what it would do or look like.  " +
	 "By default the $(body), $(actor) and $(looker) are set to your " +
	 "own body, while $(here) refers to the environment you're in.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool sam exec <describe what=\"$(here)\" cap/>\n" +
	 "  +tool sam exec <describe-view view=\"$(body)\"/>\n";
   case "parse":
      return
	 "Parse a piece of SAM and show what the resulting tree looks " +
	 "like.";
   default:
      return nil;
   }
}

string *
topics_sam()
{
   return ({ "exec", "parse" });
}
