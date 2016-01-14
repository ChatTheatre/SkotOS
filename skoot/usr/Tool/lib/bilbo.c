/*
 * ~Tool/lib/bilbo.c
 *
 * BILBO support.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <type.h>
# include <SID.h>
# include "~/include/tool.h"

private inherit "/lib/file";
private inherit "/lib/string";

inherit "utility";

/*
 * XXX: Basically reimplementing part of ~Skotos/lib/bilbo.c here, we can fix
 *      that later.
 */
private object
find_bilbo_code(object ob, string script)
{
   object target, new_obj;

   if (ob->query_property("bilbo:" + script) == "run") {
      return ob;
   }
   target = ob->query_property("bilbo:inherit:" + script);
   if (!target) {
      while (new_obj = ob->query_ur_object()) {
	 ob = new_obj;
	 if (ob->query_property("bilbo:" + script) == "run") {
	    return ob;
	 }
      }
      return nil;
   }
   new_obj = target;
   while (new_obj) {
      target = new_obj;
      new_obj    = target->query_property("bilbo:inherit:" + script);
   }
   return target;
}

static mapping
find_bilbo_scripts(object ob)
{
   int     i, sz;
   mapping map, result;
   mixed   *props, *values;

   result = ([ ]);

   while (ob) {
      map    = ob->query_properties();
      sz     = map_sizeof(map);
      props  = map_indices(map);
      values = map_values(map);

      for (i = 0; i < sz; i++) {
	 string mode, script;

	 if (!sscanf(props[i], "bilbo:%*s")) {
	    continue;
	 }
	 if (sscanf(props[i], "bilbo:%*s:%*s:%*d") == 3) {
	    continue;
	 }
	 if (sscanf(props[i], "bilbo:inherit:%s:%s", mode, script) == 2) {
	    mixed target, new_obj;

	    target = values[i];
	    if (typeof(target) != T_OBJECT) {
	       continue;
	    }
	    new_obj    = target->query_property(props[i]);
	    while (new_obj) {
	       target = new_obj;
	       new_obj    = target->query_property(props[i]);
	    }
	    if (!result[target]) {
	       result[target] = ([ ]);
	    }
	    if (!result[target][mode + ":" + script]) {
	       result[target][mode + ":" + script] = TRUE;
	    }
	 }
	 if (sscanf(props[i], "bilbo:%s:%s", mode, script) == 2) {
	    if (values[i] == "run") {
	       if (!result[ob]) {
		  result[ob] = ([ ]);
	       }
	       if (!result[ob][mode + ":" + script]) {
		  result[ob][mode + ":" + script] = TRUE;
	       }
	    }
	    continue;
	 }
      }
      ob = ob->query_ur_object();
   }
   return result;
}

/*
 * XXX: This needs to be moved to ~SkotOS/lib/bilbo.c for consistency's sake.
 */
private int
is_bilbo_command(string word)
{
   switch (word) {
   case "if":
   case "then":
   case "else":
   case "goto":
   case "function":
   case "call":
   case "return":
   case "delay":
   case "emit.actor":
   case "emit.target":
   case "emit.audience":
   case "emit.room":
   case "emit.distant":
   case "emit.object":
   case "emit.array":
   case "put":
   case "remove":
   case "action":
   case "exit":
   case "create":
   case "slay":
   case "empty":
   case "fill":
   case "delete":
   case "wear":
   case "wield":
   case "unwear":
   case "unwield":
   case "teleport":
   case "this.teleport":
   case "other.teleport":
   case "message":
   case "force":
   case "add":
   case "sub":
   case "mul":
   case "div":
   case "set":
      return TRUE;
   default:
      return FALSE;
   }
}

private mixed *
pp_bilbo_statement(string prefix, string line)
{
   string if_cond, if_code, linenr;
   mixed  *result;
      
   if (sscanf(line, "if %s then %s", if_cond, if_code) == 2) {
      string if_then, if_else;

      if (sscanf(if_code, "%s else %s", if_then, if_else) < 2) {
	 if_then = if_code;
	 if_else = nil;
      }
      result = ({ ({ prefix, CC_KEYWORD("if"), " " + if_cond + " ",
		     CC_KEYWORD("then") }) }) +
	       pp_bilbo_statement(prefix + "    ", if_then);
      if (if_else) {
	 result += ({ ({ prefix, CC_KEYWORD("else") }) }) +
	           pp_bilbo_statement(prefix + "    ", if_else);
      }
   } else if (sscanf(line, "rem%*s")) {
      result = ({ ({ prefix, CC_COMMENT(line) }) });
   } else if (sscanf(line, "goto %s", linenr)) {
      result = ({ ({ prefix, CC_KEYWORD("goto"), " ", CC_LINENR(linenr) }) });
   } else {
      string keyword, rest;

      if (sscanf(line, "%s %s", keyword, rest) == 2) {
	 rest = " " + rest;
      } else {
	 keyword = line;
	 rest = "";
      }
      if (is_bilbo_command(keyword)) {
	 result = ({ ({ prefix, CC_KEYWORD(keyword), rest }) });
      } else {
	 result = ({ ({ prefix, CC_ERROR(line) }) });
      }
   }
   return result;
}

private mixed
pp_bilbo_line(string line, string code)
{
   int    i, j, sz;
   string *commands, prefix;
   mixed  *result;

   commands = explode(code, " :: ");
   sz = sizeof(commands);
   result = ({ });
   for (i = j = 0; i < sz; i++) {
      string command, comment;

      if (sscanf(commands[i], "%s//%s", command, comment) < 2) {
	 command = commands[i];
	 comment = nil;
      }
      command = strip_left(command);
      if (strlen(command)) {
	 mixed *lines;

	 lines = pp_bilbo_statement("", command);
	 result += lines;
	 j += sizeof(lines);
      }
      if (comment) {
	 result += ({ ({ CC_COMMENT("//" + comment) }) });
	 j++;
      }
      if (i < sz - 1) {
	 result[j - 1] += ({ " ::" });
      }
   }
   if (sizeof(result) == 1) {
      return ({ CC_LINENR(line), "  " }) + result[0] + ({ "\n" });
   } else {
      mixed source;

      source = ({ CC_LINENR(line), "  " }) + result[0];
      sz = sizeof(result);
      for (i = 1; i < sz; i++) {
	 source += ({ "\n      " }) + result[i];
      }
      source += ({ "\n" });
      return source;
   }
}

private mapping
get_bilbo_properties(object ob, string script, int initial)
{
   mapping map;

   if (initial) {
      int     i, sz;
      string  *list;

      map  = ([ ]);
      list = ob->query_initial_properties();
      sz   = sizeof(list);
      for (i = 0; i < sz; i++) {
	 map[list[i]] = ob->query_initial_property(list[i]);
      }
   } else {
      map = ob->query_properties();
   }
   return map["bilbo:" + script + ":0010" .. "bilbo:" + script + ":9999"];
}

/*
 * Flag indicates whether to use core (false) or initial (true) properties.
 */
static mixed
pp_bilbo_script(object ob, string script, int l_from, int l_to, int initial)
{
   int     i, sz;
   string  s_from, s_to, *lines, *props;
   mapping map;
   mixed   *result;

   map = get_bilbo_properties(ob, script, initial);
   s_from = (string)l_from;
   s_from = "0000"[strlen(s_from)..] + s_from;
   if (l_to) {
      s_to = (string)l_to;
      s_to = "0000"[strlen(s_to)..] + s_to;
      map = map["bilbo:" + script + ":" + s_from..
	        "bilbo:" + script + ":" + s_to];
   } else {
      map = map["bilbo:" + script + ":" + s_from .. ];
   }
   if (!map_sizeof(map)) {
      return ({ });
   }
   sz     = map_sizeof(map);
   lines  = map_indices(map);
   props  = map_values(map);
   result = ({ });

   for (i = 0; i < sz; i++) {
      string line;

      sscanf(lines[i], "bilbo:" + script + ":%s", line);
      result += pp_bilbo_line(line, props[i]);
   }
   return result;
}

void
cmd_bilbo_list_all(object user, object body)
{
   int    i, sz;
   object *targets;

   targets =
      ({ body->query_environment() }) +
      (body->query_environment()->query_inventory() - ({ body })) +
      ({ body }) +
      body->query_inventory();
   sz = sizeof(targets);
   for (i = 0; i < sz; i++) {
      mapping map;

      map = find_bilbo_scripts(targets[i]);
      if (map_sizeof(map)) {
	 pre_message(user, dump_script_map("BILBO scripts", targets[i], map));
      }
   }
   user->message("Done.\n");
}

void
cmd_bilbo_list(object user, object body, varargs string arg)
{
   object target;
   mapping map;

   if (!arg) {
      map = find_bilbo_scripts(body);
      if (map_sizeof(map)) {
	 pre_message(user, dump_script_map("BILBO scripts", body, map));
      } else {
	 user->message("No BILBO scripts found in " + name(body) + ".\n");
      }
      return;
   }

   target = interpret_argument(user, body, arg);
   if (!target) {
      user->message("+tool bilbo list: Could not resolve \"" + arg + "\".\n");
      return;
   }
   map = find_bilbo_scripts(target);
   if (map_sizeof(map)) {
      pre_message(user, dump_script_map("BILBO scripts", target, map));
   } else {
      user->message("No BILBO scripts found in " + name(target) + ".\n");
   }
}

void
cmd_bilbo_view(object user, object body, string arg0, string arg1,
	       varargs int initial)
{
   string text;
   object target, code;
   mixed  result;

   target = interpret_argument(user, body, arg0);
   if (!target) {
      user->message("+tool bilbo view: Could not resolve \"" + arg0 +
		    "\".\n");
      return;
   }
   code = find_bilbo_code(target, arg1);
   if (!code) {
      user->message("+tool bilbo view: Could not find BILBO script " +
		    arg1 + " in " + name(target) + ".\n");
      return;
   }
   result = pp_bilbo_script(code, arg1, 10, 0, initial);
   if (!sizeof(result)) {
      user->message("+tool bilbo view: Could not find BILBO script " +
		    arg1 + " in " + name(target) + ".\n");
      return;
   }
   display_source(user,
		  "BILBO script " + arg1 + " in " + name(target) +
		  (code == target ? "" : " [" + name(code) + "]") + " [" +
		  (initial ? "initial" : "core") + " properties]",
		  result);
}

void
cmd_bilbo_write(object user, object body, string arg0, string arg1)
{
   object target;

   target = interpret_argument(user, body, arg0);
   if (!target) {
      user->message("+tool bilbo write: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   basic_editor(user,
		"End your BILBO code with a single period (~h for help).\n",
		"callback_bilbo_write", target, arg1);
}

private mapping
compact_bilbo_code(string *lines)
{
   int     i, sz;
   string  buffer, buffer_line;
   mapping result;

   result = ([ ]);
   sz = sizeof(lines);
   for (i = 0; i < sz; i++) {
      int    linenr;
      string text;

      if (sscanf(lines[i], "%d %s", linenr, text) == 2) {
	 string linenr_s;

	 if (buffer) {
	    result[buffer_line] = buffer;
	 }
	 linenr_s = (string)linenr;
	 linenr_s = "0000"[strlen(linenr_s)..] + linenr_s;

	 buffer_line = linenr_s;
	 buffer      = strip(text);
      } else {
	 if (buffer) {
	    buffer += " " + strip(lines[i]);
	 }
      }
   }
   if (buffer) {
      result[buffer_line] = buffer;
   }
   return result;
}

private atomic void
replace_bilbo_code(object target, string script, mapping code, int initial)
{
   int     i, sz;
   string  *linenrs, *lines, clear_func, set_func;
   mapping map;

   clear_func = initial ? "clear_initial_property" : "clear_property";
   set_func   = initial ? "set_initial_property"   : "set_property";

   /*
    * Erase original script.
    */
   map     = get_bilbo_properties(target, script, initial);
   sz      = map_sizeof(map);
   linenrs = map_indices(map);
   for (i = 0; i < sz; i++) {
      call_other(target, clear_func, linenrs[i]);
   }

   /*
    * Put in new script.
    */
   sz  = map_sizeof(code);
   if (sz > 0) {
       linenrs = map_indices(code);
       lines   = map_values(code);
       for (i = 0; i < sz; i++) {
	  call_other(target, set_func, "bilbo:" + script + ":" + linenrs[i],
		     lines[i]);
       }
       call_other(target, set_func, "bilbo:" + script, "run");
   } else {
       call_other(target, set_func, "bilbo:" + script, nil);
   }
}

private void
commit_bilbo_code(object target, string script, mapping code, int initial)
{
   /*
    * Replacing the code is done atomically.
    */
   replace_bilbo_code(target, script, code, initial);

   /*
    * Doing the XML thing can't be, as it involves filesystem interaction.
    */
   catch {
      if (target->query_object_name()) {
	 VAULT->store_snapshot(target);
      }
   }
}

static int
callback_bilbo_write(object user, object body, string *lines,
		     object target, string script)
{
   if (sizeof(lines)) {
      commit_bilbo_code(target, script, compact_bilbo_code(lines), TRUE);
      user->message("BILBO script written.\n");
   } else {
      user->message("No lines in buffer, nothing written.\n");
   }
   return TRUE;
}

void
cmd_bilbo_edit(object user, object body, string arg0, string arg1,
	       varargs int initial)
{
   string text;
   object target, code;

   target = interpret_argument(user, body, arg0);
   if (!target) {
      user->message("+tool bilbo edit: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   code = find_bilbo_code(target, arg1);
   if (code) {
      target = code;
      text = source_to_text(pp_bilbo_script(target, arg1, 10, 0, initial));
   } else {
      text = "";
   }
   edit_source(user,
	       "BILBO script " + arg1 + " in " + name(target) +
	       " [" + (initial ? "initial" : "core") + " properties]", text,
	       this_object(), "bilbo_edit_commit", target, arg1, "BILBO",
	       initial);
}

string
bilbo_edit_commit(object target, string script, string text, int initial)
{
   string *lines;

   lines = explode(text, "\r\n");
   commit_bilbo_code(target, script, compact_bilbo_code(lines), initial);
   return source_to_text(pp_bilbo_script(target, script, 10, 0, initial));
}

int
bilbo_edit_commit_text(object user, object body, string *lines,
		       object target, string script, int initial)
{
   if (lines && sizeof(lines)) {
      commit_bilbo_code(target, script, compact_bilbo_code(lines), initial);
      user->message("Commit successful.\n");
   } else {
      user->message("No lines in buffer, aborting.\n");
   }
   return TRUE;
}
