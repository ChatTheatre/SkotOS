/*
 * ~Tool/sys/tool.c
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <type.h>
# include <TextIF.h>
# include "~/include/tool.h"

private inherit "/lib/string";

/*
 * Inherit the various modules, empty or not.
 */

inherit "~/lib/utility";
inherit "~/lib/state";

private mapping handler_obs;
private mapping handlers;

static void
create()
{
   handler_obs = ([ this_object(): TRUE ]);
   handlers    = ([ ]);
}

void
patch()
{
   create();
}

void
register_handler_ob(object ob)
{
   handler_obs[ob] = TRUE;
}

void
unregister_handler_ob(object ob)
{
   handler_obs[ob] = nil;
}

object *
query_handler_obs()
{
   return map_indices(handler_obs);
}

object
find_handler_ob(string prefix, string cmd)
{
   int    i, sz;
   string function;
   object ob, *list;

   function = prefix + "_" + cmd;
   ob = handlers[function];
   if (ob) {
      /* Check if the cache is still accurate. */
      if (function_object(function, ob)) {
	 return ob;
      }
      handlers[function] = nil;
   }
   list = map_indices(handler_obs);
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      ob = list[i];
      if (function_object(function, ob)) {
	 handlers[function] = ob;
	 return ob;
      }
   }
   return nil;
}

void
cmd_fallback(object user, object body, string arg)
{
   user->message("+tool: Did not understand \"" + arg + "\", see \"+tool help\" for more information.\n");
}

void
cmd_DEV_tool(object user, object body, string arg)
{
   string cmd;
   object handler;
#if 1
   mixed *data;
#endif

   if (!arg) {
      user->message("+tool: Use \"+tool help\" for more information.\n");
      return;
   }
   arg = strip(arg);
#if 1
   DEBUG("+tool parser: " + dump_value(data = "~Tool/sys/parser"->parse(arg)));
   if (data) {
      object ob;

      ob = find_object(data[0]);
      if (!ob) {
	 user->message("+tool: Command handler missing in action: \"" + data[0] + "\".\n");
	 return;
      }
      if (!function_object(data[1], ob)) {
	 user->message("+tool: Command function \"" + data[1] + "\" missing in \"" +
		       data[0] + "\".\n");
	 return;
      }
      call_other(data[0], data[1], user, body, data[2..]...);
   } else {
      user->message("+tool: Did not understand, see \"+tool help\" for more information.\n");
      return;
   }
#else
   if (sscanf(arg, "%s %s", cmd, arg) < 2) {
      cmd = arg;
      arg = nil;
   } else {
      arg = strip(arg);
   }
   cmd = lower_case(cmd);
   handler = find_handler_ob("cmd", cmd);
   if (!handler) {
      user->message("+tool: Did not recognize \"" + cmd + "\".\n");
      return;
   }
   call_other(handler, "cmd_" + cmd, user, body, arg);
#endif
}
