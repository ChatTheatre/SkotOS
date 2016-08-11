/*
 * ~Tool/sys/merry.c
 *
 * +tool woe command handler.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include "~/include/tool.h"

private inherit "/lib/string";
inherit "~/lib/merry";

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
      ({ "merry edit <object> <text>",     "cmd_merry_edit",     "2", "3"        }),
      ({ "merry edit space <text> <text>", "cmd_merry_edit",     "3", "4", TRUE  }),
      ({ "merry eval <anything>",          "cmd_merry_eval",     "2"             }),
      ({ "merry exec quote",               "cmd_merry_exec"                      }),
      ({ "merry exec <anything>",          "cmd_merry_exec",     "2"             }),
      ({ "merry list",                     "cmd_merry_list"                      }),
      ({ "merry list all",                 "cmd_merry_list_all"                  }),
      ({ "merry list <object>",            "cmd_merry_list",     "2"             }),
      ({ "merry list space <text>",        "cmd_merry_list",     "3", TRUE       }),
      ({ "merry view <object> <text>",     "cmd_merry_view",     "2", "3"        }),
      ({ "merry view space <text> <text>", "cmd_merry_view",     "3", "4", TRUE  }),
      ({ "merry write <object> <text>",    "cmd_merry_write",    "2", "3"        }),
      ({ "merry cache",                    "cmd_merry_cache"                     }),
      ({ "merry <anything>",               "cmd_merry_fallback", "1"             }),
      ({ "merry",                          "cmd_merry_fallback",                 }),
   });
}

void
cmd_merry_fallback(object user, object body, varargs string arg)
{
   if (arg) {
      user->message("+tool merry: Did not understand \"" + arg + "\", see \"+tool help merry\" for more information.\n");
   } else {
      user->message("+tool: See \"+tool help merry\" for more information.\n");
   }
}

string
help_merry(varargs string topic)
{
   switch (topic) {
   case nil:
      return
	 "Various commands to allow you to experiment with bits of Merry.";
   case "edit":
      return
	 "Edit a new or existing Merry script in one of the WOE objects.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool merry edit %some:object lib:foo\n" +
	 "  +tool merry edit space somenamespace lib:foo\n" +
	 "";
   case "eval":
      return
	 "Evaluate a Merry expression, which is basically a shortcut for " +
	 "doing this:\n" +
	 "\n" +
	 "  +tool merry exec return <your expression here>;\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool merry eval (104 - 32) * 5 / 9\n" +
	 "  +tool merry eval 100.0 * 1.04 * 1.04 * 1.04\n";
   case "exec":
      return
	 "Execute a piece of Merry code, where 'this', 'args[\"body\"]', " +
	 "'args[\"actor\"]' and 'args[\"looker\"]' are set to your own " +
	 "body, while 'args[\"here\"]' refers to the environment you're " +
	 "in.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool merry exec int f,i; f=1; for(i=2;i<14;i++) f*=i; return f;\n";
   case "list":
      return
	 "List all known (to +tool) Merry scripts in an object.\n" +
	 "\n" +
	 "Example:\n" +
	 "  +tool merry list %some:object\n" +
	 "  +tool merry list space somenamespace\n";
   case "view":
      return
	 "Not yet documented.  Sorry.";
   case "write":
      return
	 "Not yet documented.  Sorry.";
   default:
      return nil;
   }
}

string *
topics_merry()
{
   return ({ "edit", "eval", "exec", "list", "view", "write" });
}
