/*
 * ~Tool/lib/woe.c
 *
 * TreeOfWoe support.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <type.h>
# include <System.h>
# include <base/act.h>
# include <base/bulk.h>

# include "~/include/tool.h"

        inherit       "/lib/data";
private inherit       "/lib/string";
private inherit       "/lib/url";

private inherit       "/base/lib/urcalls";
private inherit       "/base/lib/toolbox";

private inherit       "/usr/DevSys/lib/karmod";
	inherit gen   "/usr/XML/lib/xmlgen";
	inherit parse "/usr/XML/lib/xmlparse";
private inherit xmd   "/usr/XML/lib/xmd";
        inherit       "/usr/SID/lib/stateimpex";
private inherit       "/usr/SkotOS/lib/basecontext";
	inherit       "utility";

static void
create()
{
   parse::create();
}

void
cmd_woe_duplicate(object user, object body, string arg0, varargs string arg1)
{
   string path, clonable;
   object target, clone, wiztool;
   mixed  state;

   target = interpret_argument(user, body, arg0);
   if (!target) {
      user->message("+tool woe duplicate: Could not resolve \"" + arg0 +
		    "\".\n");
      return;
   }
   if (sscanf(ur_name(target), "%s#%*d", clonable) < 2) {
      user->message("+tool woe duplicate: " + name(target) +
		    " is not a clone.\n");
      return;
   }
   wiztool = DEV_USERD->query_wiztool(user->query_name());
   state = export_state(target);
   clone = wiztool->clone(clonable);
   if (arg1) {
      clone->set_object_name(arg1);
   } else {
      int r;
      string *bits;

      path = name(target);
      bits = explode(":" + path + ":", ":");
      if (sizeof(bits) > 2 && bits[0] == "Duplicate" &&
	  sscanf(bits[sizeof(bits) - 1], "%*d")) {
	 path = implode(bits[1..sizeof(bits) - 2], ":");
      }
   }
   import_state(clone, state);
   user->message("+tool woe duplicate: Created " + name(clone) + ".\n");
}

void
cmd_woe_spawn(object user, object body, string arg, int silently)
{
   int    r;
   string path;
   object target, clone;

   target = interpret_argument(user, body, arg);
   if (!target) {
      user->message("+tool woe spawn: Could not resolve \"" + arg + "\".\n");
      return;
   }
   path = name(target);
   if (!target->query_object_name()) {
      object parent;

      parent = target->query_ur_object();
      if (parent && parent->query_object_name()) {
	 /* Must be dealing with an unnamed ur-child, pick the parent. */
	 target = parent;
	 path = name(target);
      }
   }

   clone = spawn_thing(target);

   clone->action("teleport", ([ "dest": body ]), silently ? ACTION_SILENT : 0);
   if (clone->query_environment() == body) {
      user->message("+tool woe spawn: Spawned and moved " + name(clone) +
		    " into your inventory.\n");
      return;
   }
   clone->action("teleport", ([ "dest": body->query_environment() ]),
		 silently ? ACTION_SILENT : 0);
   if (clone->query_environment() == body->query_environment()) {
      user->message("+tool woe spawn: Spawned and moved " + name(clone) +
		    " into your environment.\n");
      return;
   }
   user->message("+tool woe spawn: Spawned " + name(clone) +
		 " but could not move it into your inventory or environment.\n");
}

void
cmd_woe_clone(object user, object body, string arg)
{
   user->message("+tool woe clone: Not yet implemented.\n");
}

void
cmd_woe_rename(object user, object body, string arg0, string arg1)
{
   string old_name;
   object target;

   target = interpret_argument(user, body, arg0);
   if (!target) {
      user->message("+tool woe rename: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   catch {
      old_name = name(target);
      target->set_object_name(arg1);
      user->message("+tool woe rename: Renamed " + old_name + " to " + arg1 + ".\n");
   } : {
      user->message("+tool woe rename: Failed to rename the object: " +
		    SYSLOGD->query_last_error() + "\n");
   }
}

void
cmd_woe_slay(object user, object body, string *args, varargs int override)
{
   int    i, sz;
   string *unresolved;
   object *targets;

   sz = sizeof(args);
   targets = allocate(sz);
   unresolved = allocate(sz);
   for (i = 0; i < sz; i++) {
      object target;

      targets[i] = interpret_argument(user, body, args[i]);
      if (!targets[i]) {
	 unresolved[i] = "\"" + args[i] + "\"";
      }
   }
   if (sizeof(unresolved - ({ nil })) > 0) {
      user->message("+tool woe slay: Could not resolve " +
		    implode(unresolved - ({ nil }), ", ") + ", aborting.\n");
      return;
   }
   for (i = 0; i < sz; i++) {
      if (override || safe_to_slay(targets[i])) {
	 catch {
	    targets[i]->die();
	    targets[i] = nil;
	 }
      }
   }
   targets -= ({ nil });
   user->message("+tool woe slay: " + (sz - sizeof(targets)) + " slain." +
		 (sizeof(targets) ? " " + sizeof(targets) + " failed." : "") + "\n");
}

void
cmd_woe_move(object user, object body, string arg0, string arg1)
{
   int    res;
   string *args, prog;
   object obj, env;

   obj = interpret_argument(user, body, arg0);
   if (!obj) {
      user->message("+tool woe move: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   prog = function_object("is_thing", obj);
   if (!prog || prog != "/base/lib/thing") {
      user->message("+tool woe move: Object to be moved is not a valid 'thing'.\n");
      return;
   }
   env = interpret_argument(user, body, arg1);
   if (!env) {
      user->message("+tool woe move: Could not resolve \"" + arg1 + "\".\n");
      return;
   }
   prog = function_object("is_environment", env);
   if (!prog || !sscanf(prog, "/base/%*s")) {
      user->message("+tool woe move: Cannot move, target is not a valid environment.\n");
      return;
   }
   switch (res = obj->move(env)) {
   case RES_CYCLIC_CONT:
      user->message("+tool woe move: Cannot move, this would lead to a loop in environments.\n");
      return;
   case RES_CYCLIC_PROX:
      user->message("+tool woe move: Cannot move, this would lead to a loop in prox.\n");
      return;
   case FAIL_MASS_CHECK:
      user->message("+tool woe move: Cannot move, the object has too much mass.\n");
      return;
   case FAIL_CAP_CHECK:
      user->message("+tool woe move: Cannot move, \n");
      return;
   case FAIL_WOULD_SPILL:
      user->message("+tool woe move: Cannot move, \n");
      return;
   case FAIL_WORN_CHECK:
      if (ur_weapon(obj)) {
	 user->message("+tool woe move: Cannot move, the weapon is wielded.\n");
      } else {
	 user->message("+tool woe move: Cannot move, the item is worn.\n");
      }
      return;
   default:
      if (res < 0) {
	 user->message("+tool woe move: Cannot move, unrecognized failure: " + res + ".\n");
	 return;
      }
      user->message("+tool woe move: Successfully moved.\n");
   }
}

private mapping
woe_create_tree(string path, int depth)
{
   int     i, sz;
   string  *list;
   mapping t;

   t = ([ ]);

   list = IDD->query_objects(path);
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      t[list[i]] = TRUE;
   }

   if (depth != -1) {
      if (depth <= 1) {
	 return t;
      }
      depth--;
   }

   list = IDD->query_subdirs(path);
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      t[list[i]] = woe_create_tree(path + ":" + list[i], depth);
   }

   return t;
}

void
cmd_woe_tree(object user, object body, string path, varargs mixed depth)
{
   depth = depth ? (int)depth : -1;
   pre_message(user, dump_tree(path, woe_create_tree(path, depth)));
}

static void
woe_create_urtree(object user, object body, object obj,
		  int total, int start, mapping top, mixed **stack)
{
   int     count;
   object  parent, child, first;
   mapping current;

   child   = stack[0][0];
   first   = stack[0][1];
   current = stack[0][2];

   while (count < 25 && sizeof(stack)) {
      object next, gc; /* grandchild */

      current[nil]++;
      /* Process current child */
      if (gc = child->query_first_child()) {
	 mapping t;

	 /* Hm, another one to add to the stack. */
	 current[name(child)] = t = ([ nil: 0 ]);
	 stack += ({ ({  gc, gc, t }) });
      }

      /* Update stack. */
      next = child->query_next_ur_sibling();
      if (!next) {
	 error(name(child) + " breaks the chain for " + name(child->query_ur_object()));
      }
      child = next;
      count++;
      if (child == first) {
	 /* We're at the end of the line, pop from stack. */
	 stack = stack[1..];
	 if (sizeof(stack)) {
	    child   = stack[0][0];
	    first   = stack[0][1];
	    current = stack[0][2];
	 }
      } else {
	 stack[0][0] = child;
      }
   }
   total += count;

   if (sizeof(stack)) {
      call_out("woe_create_urtree", 1, user, body, obj, total, start, top, stack);
   } else {
      pre_message(user,
		  "Displaying Ur data (search took " + (time() - start) +
		  " seconds)\n" +
		  dump_tree(name(obj) +
			    (top[nil] > 0 ? " [" + top[nil] + "]" : ""),
			    top));
   }
}

void
cmd_woe_urtree(object user, object body, string arg)
{
   string *args;
   object  obj, child;
   mapping t;

   if (!arg) {
      user->message("Usage: +tool woe urtree %<object>\n");
      return;
   }
   args = split_arguments(arg);
   if (sizeof(args) != 1) {
      user->message("+tool woe urtree: Requires 1 parameter, see \"+tool help woe urtree\".\n");
      return;
   }
   obj = interpret_argument(user, body, args[0]);
   if (!obj) {
      user->message("+tool woe urtree: Could not find \"" + arg + "\".\n");
      return;
   }
   /*
    * Check for the existance of one of the nomask functions in /lib/ur.c
    */
   if (function_object("set_ur_object", obj) != "/lib/ur") {
      user->message("+tool woe urtree: That object has no Ur support.\n");
      return;
   }
   child = obj->query_first_child();
   if (!child) {
      user->message("+tool woe urtree: That object has no Ur children.\n");
      return;
   }
   user->message("+tool woe urtree: Collecting Ur data...\n");

   t = ([ nil: 0 ]);
   call_out("woe_create_urtree", 1, user, body, obj, 0, time(), t,
	    ({ ({ child, child, t }) }));
}

static void
woe_devsys(object user, object body, string cmd, string page, string arg)
{
   string *args;
   object target;

   if (!arg) {
      user->message("Usage: +tool woe " + cmd + " %<object>\n");
      return;
   }
   args = split_arguments(arg);
   if (sizeof(args) != 1) {
      user->message("+tool woe " + cmd + ": Requires 1 parameter, see \"+tool " +
		    "help woe " + cmd + "\".\n");
      return;
   }
   target = interpret_argument(user, body, args[0]);
   if (!target) {
      user->message("+tool woe " + cmd + ": Could not resolve \"" + args[0] +
		    "\".\n");
      return;
   }
   if (user->query_skotos_client()) {
      user->message("Viewing Woe object in popup window.\n");
      send_popup(user, "/Dev/" + page + ".sam?obj=" + url_encode(name(target)));
   } else {
      switch (cmd) {
      case "karmode":
	 builtin_editor(user, genkar(target), "woe_devsys_commit_text",
			"karmode", target);
	 return;
      case "xml": {
	 mixed state;

	 state = export_state(target, nil, nil, ([ ]));
	 state = xmd_elts("object", ({ "id", target }), nil, state);
	 clear();
	 generate_xml(state, this_object());
	 builtin_editor(user, implode(query_chunks(), ""),
			"woe_devsys_commit_text", "xml", target);
	 return;
      }
      default:
	 break;
      }
      user->message("Sorry, this is not supported.\n");
   }
}

void
cmd_woe_view(object user, object body, string arg)
{
   woe_devsys(user, body, "view", "View", arg);
}

void
cmd_woe_edit(object user, object body, string arg)
{
   woe_devsys(user, body, "edit", "Form", arg);
}

void
cmd_woe_xml(object user, object body, string arg)
{
   woe_devsys(user, body, "xml", "XML", arg);
}

void
cmd_woe_karmode(object user, object body, string arg)
{
   woe_devsys(user, body, "karmode", "Kar", arg);
}

int
woe_devsys_commit_text(object user, object body, string *lines,
		       string mode, object target)
{
   if (!lines || !sizeof(lines)) {
      user->message("No lines in buffer, or no changes made.  Aborting.\n");
      return TRUE;
   }
   if (!target) {
      user->message("Target object disappeared?  Aborting.\n");
      return TRUE;
   }
   switch (mode) {
   case "karmode":
      catch {
	 parskar(target, implode(lines, "\n"));
	 user->message("Woe object commited.\n");
      } : {
	 user->message("Error while commiting Woe object:\n" +
		       SYSLOGD->query_last_error() + "\n");
	 builtin_editor(user, implode(lines, "\n"),
			"woe_devsys_commit_text", mode, target);
      }
      break;
   case "xml":
      catch {
	 string  xml;
	 object  ob;
	 mapping local;
	 mixed   *state;

	 xml = implode(lines, "\n");
	 state = parse_xml(xml);
	 if (!state) {
	    user->message("Error: Failed to parse the XML.\n");
	    builtin_editor(user, implode(lines, "\n"),
			   "woe_devsys_commit_text", mode, target);
	    return TRUE;
	 }
	 state = query_colour_value(xmd_force_to_data(state));
	 if (sizeof(state) == 0 || xmd_element(state[0]) != "object") {
	    user->message("Error: Root element is not 'object'.\n");
	    builtin_editor(user, implode(lines, "\n"),
			   "woe_devsys_commit_text", mode, target);
	    return TRUE;
	 }
	 state = state[0];
	 if (xmd_attributes(state)[0] != "id" ||
	     typeof(xmd_attributes(state)[1]) != T_OBJECT) {
	    user->message("Error: Bad or missing 'id' attribute to 'object' element.\n");
	    builtin_editor(user, implode(lines, "\n"),
			   "woe_devsys_commit_text", mode, target);
	    return TRUE;
	 }
	 ob = xmd_attributes(state)[1];
	 if (ob != target) {
	    user->message("Error: Object does not match original target.\n");
	    builtin_editor(user, implode(lines, "\n"),
			   "woe_devsys_commit_text", mode, target);
	    return TRUE;
	 }
	 state = query_colour_value(xmd_force_to_data(xmd_content(state)));
	 if (xmd_element(state[0]) == "context") {
	    local = parse_xml_context(state[0]);
	    state = state[1 ..];
	 }
	 if (sizeof(state) > 1) {
	    user->message("Error: 'object' element has more than one child.");
	    builtin_editor(user, implode(lines, "\n"),
			   "woe_devsys_commit_text", mode, target);
	    return TRUE;
	 }
	 import_state(ob, state[0], local);
	 user->message("Woe object commited.\n");
      } : {
	 user->message("Error while commiting XML: " +
		       SYSLOGD->query_last_error() + "\n");
	 builtin_editor(user, implode(lines, "\n"),
			"woe_devsys_commit_text", mode, target);
      }
      break;
   default:
      user->message("Unknown woe edit mode.  Ignored.\n");
      break;
   }
   return TRUE;
}
