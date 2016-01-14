/*
 * ~Tool/lib/merry.c
 *
 * Merry support.
 *
 * Copyright Skotos Tech Inc 2002.
 */

/*
 * [OOC Page] from [Zell/Zell]: "perhaps ~SkotOS/lib/merryapi.c could hide some
 * of that."
 * 
 * [OOC Page] from [Zell/Zell]: "There's no reference doc on the Merry language,
 * but ~SkotOS/lib/merrynode.c and ~SkotOS/include/merrynode.h should show you
 * some basics."
 *
 * [OOC Page] from [Zell/Zell]: "Set, Get, Act, Spawn, Slay, EmitTo, EmitIn,
 * Call, Error, SAM -- that's basically it"
 */

# include <type.h>
# include <status.h>
# include <SID.h>
# include <System.h>
# include <SkotOS.h>
# include "~/include/tool.h"

# define MERRY_LWO "/usr/SkotOS/data/merry"

private inherit "/lib/string";
        inherit "utility";
        inherit "/usr/SkotOS/lib/merryapi";

private string commit_merry_code(object target, string script, string code);

private void
run_merry_code(object user, string code, string display,
	       object this, mapping params, varargs int multiline)
{
   int ticks;
   object merry;

   catch {
      mixed result;

      merry  = new_object(MERRY_LWO, code);
      /* Zell: changed to reflect new calling pattern -- soon in API */
      ticks = status()[ST_TICKS];
      result = merry->evaluate(this, nil, nil, params, nil);
      ticks = ticks - status()[ST_TICKS];
      pre_message(user,
		  ( multiline ?
		    "Merry code:\n" +
		    "\n" + display + "\n" :
		    "Merry code:   " + display + "\n" ) +
		  "Merry result: " + dump_value(result) + "\n" +
		  "Ticks used for this code: " + ticks + "\n");
   } : { 
      string err, merry_name, err_pre, err_post;
      mixed cerr;

      err = SYSLOGD->query_last_error();
      merry_name = "/usr/SkotOS/merry/" + to_hex(hash_md5(strip(code)));

      if (sscanf(err, "%s\"" + merry_name + "\"%s", err_pre, err_post) == 2) {
	 err = err_pre + "merry-code" + err_post;
      }
      cerr = SYSLOGD->query_last_compile_errors();
      if (typeof(cerr) == T_ARRAY) {
	 int i, sz;

	 sz = sizeof(cerr);
	 for (i = 0; i < sz; i++) {
	    cerr[i] = "Line " + (cerr[i][1] - 3) + ": " + cerr[i][2];
	 }
      } else {
	 cerr = nil;
      }
      pre_message(user,
		  ( multiline ?
		    "Merry code:\n" +
		    "\n" + display + "\n" :
		    "Merry code:       " + display + "\n" ) +
		  "Runtime error:    " + (err ? err : "<Unknown>") + "\n" +
		  (cerr ?
		   "Compile error(s): " + implode(cerr,
						  "\n                  ") +
		   "\n" : ""));
   }
}

void
cmd_merry_exec(object user, object body, varargs string arg)
{
   mixed   result;
   mapping params;

   if (!arg) {
      prompt_to(user, "End your Merry code with a single period.\n",
		"input_merry_exec", "");
      return;
   }
   params = ([
      "body":   body,
      "actor":  body,
      "looker": body,
      "here":   body ? body->query_environment() : nil ]);
   run_merry_code(user, arg, arg, body, params);
}

static void
input_merry_exec(object user, object body, string arg, string text)
{
   mapping params;

   if (arg != ".") {
      prompt_to(user, "", "input_merry_exec", text + arg + "\n");
      return;
   }
   params = ([
      "body": body,
      "actor": body,
      "looker": body,
      "here":  body ? body->query_environment() : nil ]);
   run_merry_code(user, text, text, body, params, TRUE);
}

void
cmd_merry_eval(object user, object body, string arg)
{
   mixed   result;
   mapping params;

   params = ([
      "body":   body,
      "actor":  body,
      "looker": body,
      "here":   body ? body->query_environment() : nil ]);
   run_merry_code(user, "return (" + arg + ");", arg, body, params);
}

void
cmd_merry_write(object user, object body, string arg0, string arg1)
{
   object target;

   target = interpret_argument(user, body, arg0);
   if (!target) {
      user->message("+tool merry write: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   basic_editor(user,
		"End your Merry code with a single period (~h for help).\n",
		"callback_merry_write", target, arg1);
}

static int
callback_merry_write(object user, object body, string *lines,
		     object target, string script)
{
   if (sizeof(lines)) {
      string errors;

      errors = commit_merry_code(target, script, implode(lines, "\n"));
      if (errors) {
	 user->message("Error while writing new Merry code:\n" +
		       errors + "\n");
      } else {
	 user->message("New Merrry code written.\n");
      }
   } else {
      user->message("No lines in buffer, nothing written.\n");
   }
   return TRUE;
}

private object
find_merry_code(object ob, string script)
{
   mixed code, new_val;

   code = ob->query_property("merry:" + script);
   if (typeof(code) == T_OBJECT) {
      return ob;
   }
   new_val = ob->query_property("merry:inherit:" + script);
   while (typeof(new_val) == T_OBJECT) {
      code = new_val;
      new_val = code->query_property("merry:inherit:" + script);
   }
   if (typeof(code) == T_OBJECT) {
      if (code->query_property("merry:" + script)) {
	 return code;
      }
   }
   while (new_val = ob->query_ur_object()) {
      ob = new_val;
      code = ob->query_property("merry: " + script);
      if (typeof(code) == T_OBJECT) {
	 return ob;
      }
   }
   return nil;
}

static mapping
find_merry_scripts(object ob)
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

	 if (!sscanf(props[i], "merry:%*s")) {
	    continue;
	 }
	 if (sscanf(props[i], "merry:%*s:%*s:%*d") == 3) {
	    continue;
	 }
	 if (sscanf(props[i], "merry:inherit:%s:%s", mode, script) == 2) {
	    object target, new_val;

	    target = values[i];
	    new_val    = target->query_property(props[i]);
	    while (new_val) {
	       target = new_val;
	       new_val    = target->query_property(props[i]);
	    }
	    if (!result[target]) {
	       result[target] = ([ ]);
	    }
	    if (!result[target][mode + ":" + script]) {
	       /* Don't overwrite already found scripts. */
	       result[target][mode + ":" + script] = TRUE;
	    }
	 }
	 if (sscanf(props[i], "merry:%s:%s", mode, script) == 2) {
	    if (typeof(values[i]) == T_OBJECT &&
		name(values[i]) == MERRY_LWO + "#-1") {
	       if (!result[ob]) {
		  result[ob] = ([ ]);
	       }
	       if (!result[ob][mode + ":" + script]) {
		  /* Don't overwrite already found scripts. */
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

void
cmd_merry_list_all(object user, object body)
{
   int i, sz;
   object *targets;
   mapping map;

   targets =
      ({ body->query_environment() }) +
      (body->query_environment()->query_inventory() - ({ body })) +
      ({ body }) +
      body->query_inventory();
   sz = sizeof(targets);
   for (i = 0; i < sz; i++) {
      map = find_merry_scripts(targets[i]);
      if (map_sizeof(map)) {
	 pre_message(user, dump_script_map("Merry code", targets[i], map));
      }
   }
   user->message("Done.\n");
}

void
cmd_merry_list(object user, object body, varargs string arg, int is_space)
{
   object  target;
   mapping map;

   if (!arg) {
      map = find_merry_scripts(body);
      if (map_sizeof(map)) {
	 pre_message(user, dump_script_map("Merry code", body, map));
      } else {
	 user->message("No Merry code found in " + name(body) + ".\n");
      }
      return;
   }
   if (is_space) {
      target = SYS_MERRY->query_script_space(arg);
   } else {
      target = interpret_argument(user, body, arg);
   }
   if (!target) {
      user->message("+tool merry list: Could not resolve \"" + arg + "\".\n");
      return;
   }
   map = find_merry_scripts(target);
   if (!map_sizeof(map)) {
      user->message("No Merry code found in " + name(target) + ".\n");
      return;
   }
   pre_message(user, dump_script_map("Merry code", target, map));
}

# define PP_VIRGIN  0
# define PP_COMMENT 1
# define PP_STRING  2
# define PP_QUOTE   3
# define PP_NLSEEN  4
# define PP_CPP     5

private int
pp_merry_type(string text)
{
   return categorize_merry_word(text) == "type";
}

private int
pp_merry_keyword(string text)
{
   switch (categorize_merry_word(text)) {
   case "keyword":
   case "kfun":
   case "merryfun":
      return TRUE;
   default:
      return FALSE;
   }
}

private int
pp_merry_kw_char(int char)
{
   switch (char) {
   case '0'..'9':
   case 'a'..'z':
   case 'A'..'Z':
      return TRUE;
   default:
      return FALSE;
   }
}

private mixed
pp_merry_text(string text)
{
   int    i, sz, start;
   mixed *result;

   sz = strlen(text);
   i = start = 0;
   result = ({ });
   while (i < sz) {
      if (pp_merry_kw_char(text[i])) {
	 int    word_start;
	 string word;

	 word_start = i;
	 while (i < sz && pp_merry_kw_char(text[i])) {
	    i++;
	 }
	 word = text[word_start..i - 1];
	 if (pp_merry_type(word)) {
	    result += ({ text[start..word_start - 1], CC_TYPE(word) });
	    start = i;
	 } else if (pp_merry_keyword(word)) {
	    result += ({ text[start..word_start - 1], CC_KEYWORD(word) });
	    start = i;
	 } else {
	    /* Nothing to see here, just move along. */
	 }
      } else {
	 i++;
      }
   }
   return result += ({ text[start..] });
}

static mixed *
pp_merry_code(object merry) {
   int    state, i, sz, state_start;
   string text;
   mixed  *result;

   if (!merry) {
      return ({ });
   }
   text = merry->query_source();

   result = ({ });
   state = PP_NLSEEN;
   state_start = 0;
   sz = strlen(text);
   for (i = 0; i < sz; i++) {
      switch (state) {
      case PP_CPP:
	 if (text[i] == '\n') {
	    result += ({ CC_CPP(text[state_start..i - 1]) });
	    state = PP_NLSEEN;
	    state_start = i;
	    break;
	 }
	 break;
      case PP_NLSEEN:
	 if (text[i] == '#') {
	    result += pp_merry_text(text[state_start..i - 1]);
	    state = PP_CPP;
	    state_start = i;
	    break;
	 }
	 /*
	  * Fall through, PP_NLSEEN is only important for the pre-processor.
	  */
      case PP_VIRGIN:
	 switch (text[i]) {
	 case '"':
	    result += pp_merry_text(text[state_start..i - 1]);
	    state_start = i;
	    state = PP_STRING;
	    break;
	 case '\'':
	    result += pp_merry_text(text[state_start..i - 1]);
	    state_start = i;
	    state = PP_QUOTE;
	    break;
	 case '\n':
	    state = PP_NLSEEN;
	    break;
	 case '/':
	    if (i < sz - 1 && text[i + 1] == '*') {
	       result += pp_merry_text(text[state_start..i - 1]);
	       state = PP_COMMENT;
	       state_start = i;
	       i++;
	       break;
	    }
	 default:
	    break;
	 }
	 break;
      case PP_COMMENT:
	 if (text[i] == '*' && i < sz - 1 && text[i + 1] == '/') {
	    result += ({ CC_COMMENT(text[state_start..i + 1]) });
	    state = PP_VIRGIN;
	    state_start = i + 2;
	    i++;
	    break;
	 }
	 break;
      case PP_STRING:
	 switch (text[i]) {
	 case '\\':
	    i++;
	    break;
	 case '"':
	    result += ({ CC_STRING(text[state_start..i]) });
	    state = PP_VIRGIN;
	    state_start = i + 1;
	    break;
	 default:
	    break;
	 }
	 break;
      case PP_QUOTE:
	 switch (text[i]) {
	 case '\\':
	    i++;
	    break;
	 case '\'':
	    result += ({ CC_STRING(text[state_start..i]) });
	    state = PP_VIRGIN;
	    state_start = i + 1;
	    break;
	 default:
	    break;
	 }
	 break;
      }
   }
   switch (state) {
   case PP_NLSEEN:
   case PP_VIRGIN:  result += pp_merry_text(text[state_start..]);     break;
   case PP_COMMENT: result += ({ CC_COMMENT(text[state_start..]) });  break;
   case PP_QUOTE:   result += ({ CC_STRING(text[state_start..]) });   break;
   case PP_STRING:  result += ({ CC_STRING(text[state_start..] )});   break;
   case PP_CPP:     result += ({ CC_CPP(text[state_start..]) });      break;
   }
   return result;
}

mixed *
pp_merry_script(object ob, string script)
{
   return pp_merry_code(ob->query_property("merry:" + script));
}

void
cmd_merry_view(object user, object body, string arg0, string arg1, varargs int is_space)
{
   object target, code;
   mixed  text;

   if (is_space) {
      target = SYS_MERRY->query_script_space(arg0);
   } else {
      target = interpret_argument(user, body, arg0);
   }
   if (!target) {
      user->message("+tool merry view: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   code = find_merry_code(target, arg1);
   if (!code) {
      string tmp;

      if (sscanf(arg1, "merry:%s", tmp)) {
	 code = find_merry_code(target, tmp);
	 if (code) {
	    user->message("+tool: It looks like you intended to use \"" + tmp + "\" instead.\n");
	    arg1 = tmp;
	 }
      }
   }
   if (!code) {
      user->message("+tool merry view: Could not find Merry script " +
		    arg1 + " in " + name(target) + ".\n");
      return;
   }
   text = pp_merry_script(code, arg1);
   display_source(user,
		  "Merry script " + arg1 + " in " + name(target) +
		  (code == target ? "" : " [" + name(code) + "]"),
		  text);
}

private atomic string
replace_merry_code(object target, string script, string code)
{
   object merry;

   catch {
      if (strlen(strip(code))) {
	 merry = new_object(MERRY_LWO, code);
	 target->set_property("merry:" + script, merry);
      } else {
	 target->set_property("merry:" + script, nil);
      }
   } : {
      string err, merry_name, err_pre, err_post;
      mixed *cerr;

      err = SYSLOGD->query_last_error();
      merry_name = "/usr/SkotOS/merry/" + to_hex(hash_md5(strip(code)));

      if (sscanf(err, "%s\"" + merry_name + "\"%s", err_pre, err_post) == 2) {
	 err = err_pre + "merry-code" + err_post;
      }
      cerr = SYSLOGD->query_last_compile_errors();
      if (typeof(cerr) == T_ARRAY) {
	 int i, sz;

	 sz = sizeof(cerr);
	 for (i = 0; i < sz; i++) {
	    cerr[i] = "Line " + (cerr[i][1] - 3) + ": " + cerr[i][2];
	 }
	 return err + "\n" + implode(cerr, "\n");
      } else {
	 return err;
      }
   }
}

private string
commit_merry_code(object target, string script, string code)
{
   string result;

   /*
    * Replacing the code is done atomically.
    */
   result = replace_merry_code(target, script, code);

   /*
    * Doing the XML thing can't be, as it involves filesystem interaction.
    */
   catch {
      if (target->query_object_name()) {
	 VAULT->store_snapshot(target);
      }
   }

   return result;
}

void
cmd_merry_edit(object user, object body, string arg0, string arg1, varargs int is_space)
{
   string text;
   object target, code;

   if (is_space) {
      target = SYS_MERRY->query_script_space(arg0);
   } else {
      target = interpret_argument(user, body, arg0);
   }
   if (!target) {
      user->message("+tool merry edit: Could not resolve \"" + arg0 + "\".\n");
      return;
   }
   code = find_merry_code(target, arg1);
   if (code) {
      target = code;
      text = source_to_text(pp_merry_script(target, arg1));
   } else {
      text = "";
   }
   edit_source(user, "Merry script " + arg1 + " in " + name(target), text,
	       this_object(), "merry_edit_commit", target, arg1, "Merry");
}

mixed
merry_edit_commit(object target, string script, string text)
{
   string *lines, errors;

   lines = explode(text, "\r\n");
   errors = commit_merry_code(target, script, implode(lines, "\n"));
   if (errors) {
      return ({ errors, text });
   }
   return source_to_text(pp_merry_script(target, script));
}

int
merry_edit_commit_text(object user, object body, string *lines,
		       object target, string script)
{
   if (lines && sizeof(lines)) {
      string errors;

      errors = commit_merry_code(target, script, implode(lines, "\n"));
      if (errors) {
	 user->message("The following error(s) while commiting:\n" +
		       errors + "\n" +
		       "Going back to the editor:\n");
	 builtin_editor(user, implode(lines, "\n"), "merry_edit_commit_text",
			target, script);
      } else {
	 user->message("Commit successful.\n");
      }
   } else {
      user->message("No lines in buffer, aborting.\n");
   }
   return TRUE;
}

void
cmd_merry_cache(object user, object body)
{
   int *stats;

   stats = SYS_MERRY->query_node_map_stats();
   user->message("The current Merry-to-LPC translation cache contains " + stats[0] + " nodes.\n" +
		 "A cleanup will be triggered when it reaches a size of " + (stats[1]) + " nodes.\n" +
		 "The most recent cleanup occured at " + ctime(stats[2]) + ".\n");
}
