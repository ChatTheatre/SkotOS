/*
 * ~Tool/sys/config.c
 *
 * Parser for configuration-related +tool commands.
 *
 * Copyright Skotos Tech Inc 2003-2005.
 */

# include <type.h>
# include "~/include/tool.h"
# include <TextIF.h>

inherit "~/lib/config";

private mapping defaults;

static void
create()
{
   set_object_name("Tool:Config");

   SYS_TOOL->register_handler_ob(this_object());
   defaults = ([
       "text-height":     25,
       "text-width":      80,
       "text-background": "#ffffff",
       "text-foreground": "#000000",
       "text-fontsize":   0,
       "background":      "#ffffff",
       "foreground":      "#000000",
       "view-background": "#ffffff",
       "view-foreground": "#000000",
       "view-comment":    "red",
       "view-cpp":        "gray",
       "view-error":      "orange",
       "view-keyword":    "purple",
       "view-linenr":     "blue",
       "view-string":     "green",
       "view-type":       "green",
       "link-normal":     "#00c0ff",
       "link-hover":      "#00ffff",
       "menu-background": "#0b6e0b",
       "menu-foreground": "#ffffff",
       "work-background": "#0b6e80",
       "work-foreground": "#ffffff",
   ]);
}

void
patch()
{
   defaults = ([
       "text-height":     25,
       "text-width":      80,
       "text-background": "#ffffff",
       "text-foreground": "#000000",
       "text-fontsize":   0,
       "background":      "#ffffff",
       "foreground":      "#000000",
       "view-background": "#ffffff",
       "view-foreground": "#000000",
       "view-comment":    "red",
       "view-cpp":        "gray",
       "view-error":      "orange",
       "view-keyword":    "purple",
       "view-linenr":     "blue",
       "view-string":     "green",
       "view-type":       "green",
       "link-normal":     "#00c0ff",
       "link-hover":      "#00ffff",
       "menu-background": "#0b6e0b",
       "menu-foreground": "#ffffff",
       "work-background": "#0b6e80",
       "work-foreground": "#ffffff",
   ]);
   set_object_name("Tool:Config");
}

mapping
query_defaults()
{
   return defaults[..];
}

mixed **
query_grammar()
{
   return ({
      ({ "config list",                  "cmd_config_list"          }),
      ({ "config get <text>",            "cmd_config_get", "2"      }),
      ({ "config set <text> <anything>", "cmd_config_set", "2", "3" }),
      ({ "config clear <text>",          "cmd_config_clear", "2"    })
   });
}

string
help_config(varargs string topic)
{
   switch (topic) {
   case nil:
      return
	 "Commands to configure how Tool components respond/work.\n";
   case "list":
      return
	 "List the currently known settings and their defaults.\n";
   case "set":
      return
	 "Override the default setting.\n";
   case "get":
      return
	 "Show the current (and default) setting.\n";
   case "clear":
      return
	 "Clear the setting you chose, reverting to the default one.\n";
   default:
      return nil;
   }
}

string *
topics_config()
{
   return ({ "clear", "get", "list", "set" });
}

int
query_method(string name)
{
    switch (name) {
    case "query_list":
    case "query_config":
    case "query_default":
    case "set_config":
    case "set_default":
	return TRUE;
    default:
	return FALSE;
    }
}

mixed
call_method(string name, mapping args)
{
    switch (name) {
    case "query_list":
	return map_indices(defaults);
    case "query_default":
	if (!args["name"]) {
	    error("query_default expects a $name parameter");
	}
	if (defaults[args["name"]] == nil) {
	    error("query_default: No such default setting");
	}
	return defaults[args["name"]];
    case "set_default":
	if (!args["name"] || !args["value"]) {
	    error("set_default expects $name and $value parameters");
	}
	if (defaults[args["name"]] == nil) {
	    error("set_default: No such default setting");
	}
	defaults[args["name"]] = args["value"];
	return nil;
    case "query_config":
	if (!args["name"] || !args["udat"]) {
	    error("query_config expects $name and $udat parameters");
	}
	if (defaults[args["name"]] == nil) {
	    error("query_config: No such setting");
	}
	if (args["udat"]->query_property("tool:config:" + args["name"]) == nil) {
	    return defaults[args["name"]];
	}
	return args["udat"]->query_property("tool:config:" + args["name"]);
    case "set_config":
	if (!args["name"] || !args["value"] || !args["udat"]) {
	    error("set_config expects $name, $value and $udat parameters");
	}
	if (defaults[args["name"]] == "nil") {
	    error("set_config: No such setting");
	}
	if ((string)defaults[args["name"]] == args["value"]) {
	    args["udat"]->clear_property("tool:config:" + args["name"]);
	} else {
	    switch (typeof(defaults[args["name"]])) {
	    case T_INT:
		args["udat"]->set_property("tool:config:" + args["name"],
					   (int)args["value"]);
		break;
	    case T_STRING:
		args["udat"]->set_property("tool:config:" + args["name"],
					   (string)args["value"]);
		break;
	    default:
		break;
	    }
	}
        return nil;
    default:
	return nil;
    }
}
