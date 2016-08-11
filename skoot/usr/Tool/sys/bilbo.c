/*
 * ~Tool/sys/sam.c
 *
 * SAM command parser.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include "~/include/tool.h"

private inherit "/lib/string";
        inherit "~/lib/bilbo";

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
   ({ "bilbo list",                         "cmd_bilbo_list"               }),
   ({ "bilbo list all",                     "cmd_bilbo_list_all"           }),
   ({ "bilbo list <object>",                "cmd_bilbo_list", "2"          }),
   ({ "bilbo view <object> <text>",         "cmd_bilbo_view", "2", "3"     }),
   ({ "bilbo view <object> <text> core",    "cmd_bilbo_view", "2", "3"     }),
   ({ "bilbo view <object> <text> initial", "cmd_bilbo_view", "2", "3", 1  }),
   ({ "bilbo write <object> <text>",        "cmd_bilbo_write", "2", "3"    }),
   ({ "bilbo edit <object> <text>",         "cmd_bilbo_edit",  "2", "3"    }),
   ({ "bilbo edit <object> <text> core",    "cmd_bilbo_edit",  "2", "3"    }),
   ({ "bilbo edit <object> <text> initial", "cmd_bilbo_edit",  "2", "3", 1 }),
   ({ "bilbo <anything>",                   "cmd_bilbo_fallback", "1"      }),
   ({ "bilbo",                              "cmd_bilbo_fallback"           }),
   });
}

void
cmd_bilbo_fallback(object user, object body, varargs string arg)
{
   if (arg) {
      user->message("+tool bilbo: Did not understand \"" + arg + "\", see \"+tool help bilbo\" for more information.\n");
   } else {
      user->message("+tool: See \"+tool help bilbo\" for more information.\n");
   }
}

string
help_bilbo(varargs string topic)
{
   switch (topic) {
   case nil:
      return
	 "Various commands to allow you to (re)view BILBO code.";
   case "list":
      return
	 "Review what BILBO scripts exist in an object or what a particular " +
	 "BILBO script looks like in an object.\n" +
	 "\n" +
	 "'+tool bilbo list' will list all BILBO scripts found in your own " +
	 "body.\n" +

	 "'+tool bilbo list all' will list all BILBO scripts found in the " +
	 "room you are in, any objects in that room, yourself, or any " +
	 "objects that you have in your inventory.\n" +

	 "'+tool bilbo list %<object>' will list all BILBO scripts found " +
	 "in the object.\n";
   case "write":
      return
	 "Allows you to edit BILBO code in the same format that the " +
	 "'+tool bilbo list %<object> <script>' generates, which is then " +
	 "compacted and the result written into the relevant properties.";
   case "view":
      return
	 "'+tool bilbo view %<object> <script> [core|initial]' will show " +
	 "the indicated BILBO script in the object, by default using the " +
	 "core properties but you can specify to look at the initial " +
	 "properties instead.";
   case "edit":
      return
	 "'+tool bilbo edit %<object> <script> [core|initial]' will allow " +
	 "you to edit the indicated BILBO script in the object, by default " +
	 "using the core properties but you can specify to work with the " +
	 "initial properties instead.";
   default:
      return nil;
   }
}

string *
topics_bilbo()
{
   return ({ "edit", "list", "view", "write" });
}
