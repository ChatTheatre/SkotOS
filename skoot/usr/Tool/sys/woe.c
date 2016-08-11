/*
 * ~Tool/sys/woe.c
 *
 * +tool woe command handler.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include "~/include/tool.h"

private inherit "/lib/string";
        inherit "~/lib/woe";

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
      ({ "woe spawn <object>",            "cmd_woe_spawn",    "2"      }),
      ({ "woe spawn silently <object>",   "cmd_woe_spawn",    "3", 1   }),
      ({ "woe duplicate <object>",        "cmd_woe_duplicate","2"      }),
      ({ "woe duplicate <object> <text>", "cmd_woe_duplicate","2", "3" }),
      ({ "woe clone <object>",            "cmd_woe_clone",    "2"      }), 
      ({ "woe rename <object> <text>",    "cmd_woe_rename",   "2", "3" }),
      ({ "woe slay <object>+",            "cmd_woe_slay",     "2"      }),
      ({ "woe slay override <object>+",   "cmd_woe_slay",     "3", 1   }),
      ({ "woe move <object> <object>",    "cmd_woe_move",     "2"      }),
      ({ "woe tree <text>",               "cmd_woe_tree",     "2"      }),
      ({ "woe tree <text> <number>",      "cmd_woe_tree",     "2", "3" }),
      ({ "woe urtree <object>",           "cmd_woe_urtree",   "2"      }),
      ({ "woe view <object>",             "cmd_woe_view",     "2"      }),
      ({ "woe edit xml <object>",         "cmd_woe_xml",      "3"      }),
      ({ "woe edit <object>",             "cmd_woe_edit",     "2"      }),
      ({ "woe xml edit <object>",         "cmd_woe_xml",      "3"      }),
      ({ "woe xml <object>",              "cmd_woe_xml",      "2"      }),
      ({ "woe karmode <object>",          "cmd_woe_karmode",  "2"      }),
      ({ "woe <anything>",                "cmd_woe_fallback", "1"      }),
      ({ "woe",                           "cmd_woe_fallback",          }),
   });
}

void
cmd_woe_fallback(object user, object body, varargs string arg)
{
   if (arg) {
      user->message("+tool woe: Did not understand \"" + arg + "\", see \"+tool help woe\" for more information.\n");
   } else {
      user->message("+tool: See \"+tool help woe\" for more information.\n");
   }
}

string
help_woe(varargs string topic)
{
   switch (topic) {
   case nil:
      return
	 "The woe support consists of two commands: \"tree\" and \"urtree\".";
   case "tree":
      return
	 "Display a subsection of the Tree Of Woe.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool woe tree Generic\n" +
	 "  +tool woe tree Marrach:rooms\n";
   case "urtree":
      return
	 "Display a tree of Ur-dependencies, checking the Ur-children of " +
	 "the given object and then their Ur-children, etc.  " +
	 "Be aware that since the the objects are checked 50/s this may " +
	 "take some time for objects with a lot of Ur-children.\n" +
	 "\n" +
	 "Example:\n" +
	 "  +tool woe urtree %Generic:UrFurniture\n" +
	 "\n" +
	 "For any objects with more than one Ur-child the total is shown " +
	 "behind the Woe name.";
   case "rename":
      return
	 "Rename an existing woe object, for whatever reason you may have. " +
	 "The command requires two parameters, the first being the original " +
	 "object and the second being the new woe-name.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool woe rename %nino;1 chatters:Nino:inv1\n" +
	 "  +tool woe rename %Examples:Foo Examples:Bar\n" +
	 "\n" +
	 "Note that the second parameter should refer to a name not yet " +
	 "existing and will have to obey the rules like any other woe object:\n" +
	 "\n" +
	 "  Names starting with a '/' are forbidden.\n" +
	 "  Names including '::' are forbidden.\n" +
	 "  Names including < > \ # % { } | \\ ^ ~ [ ]  ` ' are forbidden.\n" +
	 "  Names including control-characters are forbidden.\n";      
   case "duplicate":
      return
	 "Duplicate an existing woe object, optionally giving it a new name.\n" +
	 "\n" +
	 "Examples:\n" +
	 "\n" +
	 "  +tool woe duplicate %nino;2 chatters:Nino:dupe2\n" +
	 "  +tool woe duplicate %nino;3\n" +
	 "";
   case "spawn":
      return
	 "Spawn a fresh copy from a mold.  The new object will be given the " +
	 "name of \"Duplicate:<mold-name>:<number>\" where the number is of " +
	 "course picked such that it doesn't match with existing ones.\n" +
	 "\n" +
	 "Examples:\n" +
	 "\n" +
	 "  +tool woe spawn %Marrach:objects:pouch-leather-brown\n" +
	 "  +tool woe spawn silently %nino;2\n" +
	 "\n" +
	 "An attempt is made to move the created object to your inventory, " +
	 "and if that fails, into your environment.  If that too fails, it " +
	 "will be left in the void and of course you will be told this. " +
	 "\n" +
	 "If you're trying to spawn an object whose woe-name starts with " +
	 "\"Duplicate:\" and ends in a number, a fresh spawn is made of the " +
	 "ur-parent of the object.  Same goes if you are trying to spawn " +
	 "off of an unnamed woe-object where the ur-parent is a _named_ " +
	 "woe-object.\n";
	 "";
   case "slay":
      return
	 "Slay one or more woe objects, using the usual object-referring " +
	 "syntax.\n" +
	 "\n" +
	 "Examples:\n" +
	 "\n" +
	 "  +tool woe slay %me;1 %me;3\n" +
	 "  +tool woe slay override %me;4\n" +
	 "\n" +
	 "The override is required if you're trying to slay a virtual home, " +
	 "something that isn't an ur-object, or an object with ur-children.\n";
   case "move":
      return
	 "Move objects around, while of course doing all the necessary " +
	 "checks on mass, capacity, etc.\n" +
	 "\n" +
	 "Examples:\n" +
	 "\n" +
	 "  +tool woe move %nino;1 %here\n" +
	 "  +tool woe move %here;1 %nino\n";
   case "view":
      return
	 "View a single object in WOE style, without needing to fire up the " +
	 "tree of WOE.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool woe view %me\n";
   case "edit":
      return
	 "Edit a single object in WOE style, without needing to fire up the " +
	 "tree of WOE.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool woe edit %me\n";
   case "karmode":
      return
	 "Edit a single object in WOE's Karmode style, without needing to fire up the " +
	 "tree of WOE.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool woe karmode %me\n";
   case "xml":
      return
	 "Edit a single object in WOE's XML style, without needing to fire up the " +
	 "tree of WOE.\n" +
	 "\n" +
	 "Examples:\n" +
	 "  +tool woe xml %me\n";
   default:
      return nil;
   }
}

string *
topics_woe()
{
   return ({ "edit", "karmode", "view", "xml", "move", "tree", "urtree", "rename", "duplicate", "spawn", "slay" });
}
