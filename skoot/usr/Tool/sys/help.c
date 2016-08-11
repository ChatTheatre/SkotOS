/*
 * ~Tool/sys/help.c
 *
 * Tool help handler.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include "~/include/tool.h"

private inherit "/lib/string";

inherit "~/lib/help";
inherit "~/lib/utility";

string help_tool(varargs string arg);
string *topics_tool();

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
      ({ "help",               "cmd_help"                    }),
      ({ "help <text>",        "cmd_help",          "1"      }),
      ({ "help <text> <text>", "cmd_help",          "1", "2" }),
      ({ "help <anything>",    "cmd_help_fallback", "1"      }),
   });
}

string
help_tool(varargs string arg)
{
   switch (arg) {
   case nil:
      return
	 "Use \"+tool help <topic>\" to get some " +
	 "general information and possibly some " +
	 "sub-topics to read about.";
   case "popup":
      return
	 "Provides a web-popup which includes the Property Editor.\n" +
	 "\n" +
	 "It can also be used to directly jump to a specific object or " +
	 "property in that object.\n" +
	 "\n" +
	 "Usage:\n" +
	 "    +tool popup\n" +
	 "    +tool popup %<object>\n" +
	 "    +tool popup %<object> <property name>\n";
   case "profiler":
      return
	 "Collected into one command you can review the results of profiling various parts of the system.\n" +
	 "\n" +
	 "Profiling the LPC functions that get called, this is a running average of the last 5 minutes:\n" +
	 "    +tool profiler commands\n" +
	 "\n" +
	 "Profiling the Merry scripts that get called:\n" +
	 "    +tool profiler merry\n" +
	 "    +tool profiler merry [daily] <minimum>\n" +
	 "    +tool profiler merry [daily] %<object>\n" +
	 "    +tool profiler merry [daily] archives\n" +
	 "    +tool profiler merry [daily] archive <index>\n" +
	 "\n" +
	 "At any point there are two data-sets for Merry script profiling, one that is " +
	 "archived and reset every hour, and one that is archive and " +
	 "reset once a day (midnight GMT)\n";
   case "quote":
      return
	 "Usage:\n" +
	 "    +tool quote\n" +
	 "    +tool quote to <account>\n" +
	 "\n" +
	 "In the first form it'll quote whatever text you enter to the environment you're in. " +
	 "The second form picks the first body with which <account> is logged in and sends the text " +
	 "its way.\n";
   case "ide":
      return
	 "This popup (which requires that you use one of the Skotos clients, to function) allows you to " +
	 "easily browse the Bilbo and Merry scripts on an object.\n" +
	 "\n" +
	 "Example:\n" +
	 "    +tool ide %me\n";
   case "summary":
      return
"Follows a brief summary of the commands that have been " +
"implemented so far:\n" +
"\n" +
"     +tool bilbo\n" +
"         +tool bilbo edit %<object> <scriptname> [ core | initial ]\n" +
"         +tool bilbo list [ all | %<object> ]\n" +
"         +tool bilbo view %<object> <scriptname> [ core | initial ]\n" +
"         +tool bilbo write %<object> <scriptname>\n" +
"\n" +
"     +tool ide <object>\n" +
"\n" +
"     +tool help [ <topic> [ <subtopic> ] ]\n" +
"\n" +
"     +tool merry\n" +
"         +tool merry cache\n" +
"         +tool merry eval <merry-expression>\n" +
"         +tool merry exec <merry-code>\n" +
"         +tool merry edit %<object> <scriptname>\n" +
"         +tool merry edit space <namespace> <scriptname>\n" +
"         +tool merry list\n" +
"         +tool merry list all\n" +
"         +tool merry list %<object>\n" +
"         +tool merry list space <namespace>\n" +
"         +tool merry view %<object> <scriptname>\n" +
"         +tool merry view space <namespace> <scriptname>\n" +
"         +tool merry write %<object> <scriptname> \n" +
"\n" +
"     +tool popup\n" +
"\n" +
"     +tool profiler commands\n" +
"     +tool profiler merry\n" +
"     +tool profiler merry [daily] <minimum>\n" +
"     +tool profiler merry [daily] %<object>\n" +
"     +tool profiler merry [daily] archives\n" +
"     +tool profiler merry [daily] archive <index>\n" +
"     +tool profiler socials [<minimum>]\n" +
"     +tool profiler socials (hourly|daily) [<minimum>]\n" +
"\n" +
"     +tool quote\n" +
"     +tool quote to <account>\n" +
"     +tool quote \"<text>\"\n" +
"\n" +
"     +tool sam\n" +
"         +tool sam exec <sam-tags>\n" +
"         +tool sam parse <sam-tags>\n" +
"\n" +
"     +tool status\n" +
"     +tool status %<object>\n" +
"\n" +
"     +tool woe\n" +
"         +tool woe spawn [silently] %<object>\n" +
"         +tool woe duplicate %<object> [<new-name>]\n" +
"         +tool woe rename %<object> <new-name>\n" +
"         +tool woe slay %<object> ...\n" +
"         +tool woe move %<object> %<environment>\n" +
"         +tool woe tree <woe-path-prefix>\n" +
"         +tool woe urtree <woe-object-name>\n" +
"         +tool woe edit %<object>\n" +
"         +tool woe karmode %<object>\n" +
"         +tool woe view %<object>\n" +
"         +tool woe xml %<object>\n" +
"\n" +
"     +tool config\n" +
"         +tool config list\n" +
"         +tool config get <setting>\n" +
"         +tool config set <setting> <value>\n" +
"         +tool config clear <setting>\n" +
	 "";
   default:
      return nil;
   }
}

string *
topics_tool()
{
   return ({ "bilbo", "config", "ide", "help", "merry", "profiler", "popup", "quote", "sam", "tool", "woe", "summary" });
}
