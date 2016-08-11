/*
 * ~Tool/lib/utility.c
 *
 * Various handy bits for the tool-commands.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <type.h>
# include <TextIF.h>
# include <SkotOS.h>
# include "~/include/tool.h"

        inherit "/usr/SAM/lib/sam";
        inherit "/usr/SkotOS/lib/describe";
private inherit "/usr/XML/lib/xmlparse";
private inherit "/lib/string";
private inherit "/lib/url";

/*** Interface ***/

static string dump_tree(string title, mapping t);
static void   sam_message(object user, string text);
static void   pre_message(object user, string text);
static string wrap_text(string prefix, string text, varargs int width);
static mixed  interpret_parameter(object user, object body, string param);
static void   prompt_to(object user, string prompt, string func, mixed args...);
static string *split_arguments(string arg);
static mixed  interpret_argument(object user, object body, string arg);
static void   display_source(object user, string title, mixed text);
static string source_to_html(mixed text);
static string source_to_text(mixed text);
static void   basic_editor(object user, string prompt, string func,
			   mixed args...);
static void   builtin_editor(object user, string text, string func, mixed args...);
static void   send_popup(object user, string url);

/*** Implementation ***/

private mapping prompting;
private mapping describe_cache;
private mapping editors;

private string
dump_tree_rec(mapping t, string prefix)
{
   int     i, sz;
   string *names, result;
   mixed  *values, nil_value;

   nil_value = t[nil];
   t[nil]    = nil;

   sz     = map_sizeof(t);
   names  = map_indices(t);
   values = map_values(t);

   result = "";

   for (i = 0; i < sz; i++) {
      string str1, str2;

      if (names[i] == nil) {
	 continue;
      }
      if (i == sz - 1) {
	 str1 = "`-- ";
	 str2 = "    ";
      } else {
	 str1 = "|-- ";
	 str2 = "|   ";
      }
      if (values[i] == TRUE) {
	 result += prefix + str1 + names[i] + "\n";
      } else if (typeof(values[i]) == T_MAPPING) {
	 if (values[i][nil] != nil && values[i][nil] > 1) {
	    result += prefix + str1 + names[i] + " [" + values[i][nil] + "]\n";
	 } else {
	    result += prefix + str1 + names[i] + "\n";
	 }
      } else {
	 result += prefix + str1 + names[i] + " [" + values[i] + "]\n";
      }
      if (typeof(values[i]) == T_MAPPING) {
	 result += dump_tree_rec(values[i], prefix + str2);
      }
   }
   t[nil] = nil_value;
   return result;
}

static string
dump_tree(string title, mapping t)
{
   return title + "\n" + dump_tree_rec(t, "");
}

static void
sam_message(object user, string text)
{
   mixed state;

   state = ::parse_xml(text);
   user->html_message(unsam(state, ([ ]), user));
}

static void
pre_message(object user, string text)
{
# if 1
   user->html_message("<PRE>");
   user->message(text);
   user->html_message("</PRE>\n");
# else
   user->html_message("<PRE>" + replace_html(text) + "</PRE>\n");
# endif
}

private string
wrap_line(string prefix, string text, int width)
{
    int    i, sz, total, current_len;
    string *lines, current, *words, spaces;

    /*
     * Allocate enough lines for a a worst-case scenario, one word per line.
     * Each word would then be at least floor(width / 2) + 1 characters.
     */

    width -= strlen(prefix);
    lines = allocate((strlen(text) + (width / 2)) / (width / 2));

    spaces = "                                        "[0..strlen(prefix) - 1];

    words = explode(text, " ");
    sz = sizeof(words);

    current     = words[0];
    current_len = strlen(current);
    total       = 0;

    for (i = 1; i < sz; i++) {
	if (current_len + 1 + strlen(words[i]) < width) {
	    current += " " + words[i];
	    current_len += 1 + strlen(words[i]);
	} else {
	    lines[total++] = current;
	    current        = words[i];
	    current_len    = strlen(current);
	}
    }
    lines[total++] = current;
    return prefix + implode(lines[..total - 1], "\n" + spaces) + "\n";
}

static string
wrap_text(string prefix, string text, varargs int width)
{
   int    i, sz, len;
   string *lines, spaces;

   spaces = "                                        "[0..strlen(prefix) - 1];

   width = width ? width : DEFAULT_WIDTH;
   lines = explode(text, "\n");
   sz    = sizeof(lines);
   for (i = 0; i < sz; i++) {
      if (strlen(lines[i])) {
	 lines[i] = wrap_line(prefix, lines[i], width);
      } else {
	 lines[i] = prefix + "\n";
      }
      prefix = spaces;
   }
   return implode(lines, "");
}

static mixed
interpret_parameter(object user, object body, string param)
{
   string str;

   if (!param || !strlen(param)) {
      return param;
   }

   /* Syntactic sugar stuff. */
   if (sscanf(param, "OBJ(%s)", str)) {
      return find_object(str);
   }
   if (sscanf(param, "[%s]", str)) {
      return find_object(str);
   }
   if (sscanf(param, "%*d.%*d") == 2) {
      return (float)param;
   }
   if (sscanf(param, "%*d")) {
      return (int)param;
   }
   if (param[0] == '"' && param[strlen(param) - 1] == '"') {
      return param[1..strlen(param) - 2];
   }
   switch (param) {
   case "$here":
      return body->query_environment();
   case "$me":
   case "$body":
      return body;
   default:
      break;
   }
   return nil;
}

static void
prompt_to(object user, string prompt, string func, mixed args...)
{
   if (!user) {
      return;
   }
   if (!function_object(func, this_object())) {
      return;
   }
   if (!prompting) {
      prompting = ([ ]);
   }
   prompting[user] = ({ func }) + args;
   user->set_input_to(this_object(), "prompt_to");
   user->message(prompt);
}

nomask void
input_prompt_to(object user, object body, string arg)
{
   mixed *data;

   if (!prompting) {
      return;
   }
   if (user != previous_object()) {
      return;
   }
   data = prompting[user];
   if (!data) {
      return;
   }
   if (arg && strlen(arg) && arg[0] == '!') {
      user->set_input_to(this_object(), "prompt_to");
      MAIN->command(user, body, strip(arg[1..]));
      return;
   }
   prompting[user] = nil;
   call_other(this_object(), data[0], user, body, arg, data[1..]...);
}

static string *
split_arguments(string arg)
{
   string *bits;

   bits = ({ });
   arg = strip(arg);
   while (strlen(arg)) {
      string str, rest;

      if (sscanf(arg, "%%(%s)%s", str, rest) == 2 && strlen(str)) {
	 bits += ({ "%" + str });
	 arg = strip(rest);
      } else if (sscanf(arg, "%%%s %s", str, rest) == 2 && strlen(str)) {
	 bits += ({ "%" + str });
	 arg = strip(rest);
      } else if (sscanf(arg, "%%%s", str) && strlen(str)) {
	 bits += ({ "%" + str });
	 arg = "";
      } else if (sscanf(arg, "%s %s", str, rest) == 2 && strlen(str)) {
	 bits += ({ str });
	 arg = strip(rest);
      } else {
	 bits += ({ arg });
	 arg = "";
      }
   }
   return bits;
}

private string
cached_describe_one(object ob)
{
   string str;
   mixed *data;

   if (describe_cache) {
      data = describe_cache[ob];
      if (data) {
	 if (time() < data[1] + 60) {
	    return data[0];
	 }
      }
   } else {
      describe_cache = ([ ]);
   }
   data = describe_cache[ob] = ({ describe_one(ob), time() });
   return data[0];
}

static mixed
interpret_argument(object user, object body, string arg)
{
   object target;

   if (arg[0] == '%') {
      int    i, sz;
      string *bits;
      object *users, result;

      if (arg[1] == '(') {
	 bits = explode(arg[2..strlen(arg) - 2], ";");
      } else {
	 bits = explode(arg[1..], ";");
      }
      switch (bits[0]) {
      case "me":
	 target = body;
	 break;
      case "here":
	 target = body->query_environment();
	 break;
      default:
	 users = INFOD->query_current_user_objects();
	 sz = sizeof(users);
	 if (bits[0][0] >= 'a' && bits[0][0] <= 'z') {
	    /* Look for username. */

	    for (i = 0; i < sz && !target; i++) {
	       if (!users[i]->query_body()) {
		  continue;
	       }
	       if (users[i]->query_name() == bits[0]) {
		  target = users[i]->query_body();
	       }
	    }
	 } else {
	    /* Look for bodyname. */

	    for (i = 0; i < sz && !target; i++) {
	       object ob;

	       ob = users[i]->query_body();
	       if (!ob) {
		  continue;
	       }
	       if (lower_case(cached_describe_one(ob)) == lower_case(bits[0])) {
		  target = ob;
	       }
	    }
	 }
	 if (!target) {
	    target = find_object(bits[0]);
	 }
	 break;
      }
      if (!target) {
	 return nil;
      }
      sz = sizeof(bits);
      for (i = 1; i < sz; i++) {
	 int d;

	 if (sscanf(bits[i], "%d", d)) {
	    object *inv;

	    inv = target->query_inventory();
	    if (1 <= d && d <= sizeof(inv)) {
	       target = inv[d - 1];
	    } else {
	       return nil;
	    }
	 } else {
	    /* Later. */
	    return nil;
	 }
      }
      return target;
   } else {
   }
}

static string
source_to_text(mixed text)
{
   int    i, sz;
   string result;

   if (typeof(text) == T_STRING) {
      return text;
   }
   sz = sizeof(text);
   result = "";
   for (i = 0; i < sz; i++) {
      if (typeof(text[i]) == T_STRING) {
	 result += text[i];
      } else {
	 result += text[i][1];
      }
   }
   return result;
}

static string 
source_to_html(mixed text)
{
   int    i, sz;
   string result;

   if (typeof(text) == T_STRING) {
      return text;
   }
   sz = sizeof(text);
   result = "";
   for (i = 0; i < sz; i++) {
      if (typeof(text[i]) == T_STRING) {
	 result += replace_html(text[i]);
      } else {
	 switch (text[i][0]) {
	 case "comment":
	    result += "<FONT color=red>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 case "cpp":
	    result += "<FONT color=gray>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 case "error":
	    result += "<FONT color=orange>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 case "keyword":
	    result += "<FONT color=purple>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 case "linenr":
	    result += "<FONT color=blue>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 case "string":
	    result += "<FONT color=green>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 case "type":
	    result += "<FONT color=green>" + replace_html(text[i][1]) + "</FONT>";
	    break;
	 default:
	    result += replace_html(text[i][1]);
	    break;
	 }
      }
   }
   return result;
}

static void
send_popup(object user, string url)
{
   url = "SKOOT 70 " + url_absolute(url) + "\n";
   if (function_object("raw_line", user)) {
      user->raw_line(url);
   } else {
      user->message(url);
   }
}

static void
display_source(object user, string title, mixed text)
{
   if (user->query_skotos_client()) {
      string id;

      /*
       * Store the output in an udat property 
       */
      id = to_hex(hash_md5(title));
      user->query_udat()->set_property("source-" + id + "-title", title);
      user->query_udat()->set_property("source-" + id + "-stamp", time());
      user->query_udat()->set_property("source-" + id + "-text", 
				       source_to_html(text));
      user->message("Showing source in popup window.\n");
      send_popup(user, "/Tool/View.sam?id=" + id);
      return;
   }
   pre_message(user, title + ":\n" + source_to_text(text));
}

private void
purge_scratch_data(object udat)
{
   int i, sz;
   string *props;
   string *post;

   props = map_indices(udat->query_properties());
   sz = sizeof(props);
   for (i = 0; i < sz; i++) {
      string id;

      if (sscanf(props[i], "source-%s-stamp", id) == 1) {
	 int stamp;

	 stamp = udat->query_property(props[i]);
	 if (stamp < time() - 7 * 86400) {
	    int j, sz_j;

	    DEBUG("Purging source-" + id + "-* from " + name(udat));
	    if (!post) {
	       post = ({ "title", "stamp", "text", "object", "language",
			 "script", "target", "cb-obj", "cb-fun", "target",
			 "extra" });
	    }
	    sz_j = sizeof(post);
	    for (j = 0; j < sz_j; j++) {
	       udat->clear_property("source-" + id + "-" + post[j]);
	    }
	 }
      }
   }
}

static void
edit_source(object user, string title, string text,
	    object callback_obj, string callback_fun,
	    object target, string script, string language,
	    mixed extra...)
{
   string id;
   object udat;

   if (user->query_skotos_client()) {
      id = to_hex(hash_md5(title));
      udat = user->query_udat();

      /* Clean out old properties before allowing new ones. */
      purge_scratch_data(udat);

      udat->set_property("source-" + id + "-title",    title);
      udat->set_property("source-" + id + "-stamp",    time());
      udat->set_property("source-" + id + "-text",     replace_html(text));
      udat->set_property("source-" + id + "-object",   name(target));
      udat->set_property("source-" + id + "-language", language);
      udat->set_property("source-" + id + "-script",   script);

      udat->set_property("source-" + id + "-target",   target);
      udat->set_property("source-" + id + "-cb-obj",   callback_obj);
      udat->set_property("source-" + id + "-cb-fun",   callback_fun);
      udat->set_property("source-" + id + "-target",   target);
      udat->set_property("source-" + id + "-extra",    extra);

      user->message("Editing source in popup window.\n");
      send_popup(user, "/Tool/Edit.sam?id=" + id);
   } else {
      user->message("Editing source in line-based editor.\n");
      builtin_editor(user, text, callback_fun + "_text", target, script, extra...);
   }
}

static void
basic_editor(object user, string prompt, string func, mixed args...)
{
   prompt_to(user, prompt, "input_basic_editor", func, ({ }), args);
}

nomask static void
input_basic_editor(object user, object body, string arg, string func,
		   string *lines, mixed *args)
{
   switch (arg) {
   case ".":
      if (!call_other(this_object(), func, user, body, lines, args...)) {
	 user->message("Buffer written.\n");
      }
      return;
   case "~d":
      if (sizeof(lines)) {
	 lines = lines[..sizeof(lines) - 2];
	 user->message("Deleted last line.\n");
      } else {
	 user->message("No lines in buffer.\n");
      }
      break;
   case "~h":
      pre_message(user,
		  "Editor commands:\n" +
		  ".    - Finish editing.\n" +
		  "~d   - Delete line last added.\n" +
		  "~h   - This help information.\n" +
		  "~l   - Review text edited so far.\n" +
		  "~q   - Abort editing.\n");
      break;
   case "~l":
      pre_message(user,
		  "---- Editor buffer: ----\n" +
		  implode(lines, "\n") + "\n" +
		  "----\n");
      break;
   case "~q":
      user->message("Aborting editor, discarding buffer!\n");
      return;
   default:
      lines += ({ arg });
      break;
   }
   prompt_to(user, "", "input_basic_editor", func, lines, args);
}

static void
builtin_editor(object user, string text, string func, mixed args...)
{
   object obj;
   string id, str;

   if (!editors) {
      editors = ([ ]);
   }
   id = to_hex(hash_md5(name(user)));
   editors[user] = clone_object(EDITOR_OBJ);
   user->message(editors[user]->initialize_editor(user, SCRATCH_PATH + id, text));
   prompt_to(user, ":", "input_builtin_editor", func, args);
}

nomask static void
input_builtin_editor(object user, object body, string arg, string func,
		     mixed *args)
{
   object ed;
   string str, *lines;

   ed = editors[user];
   if (!ed) {
      user->message("Editor object disappeared!\n");
      return;
   }
   str = ed->run_editor(arg);
   if (str) {
      user->message(str);
   }
   switch (query_editor(ed)) {
   case "insert":
      prompt_to(user, "", "input_builtin_editor", func, args);
      return;
   case "command":
      prompt_to(user, ":", "input_builtin_editor", func, args);
      return;
   case nil:
      str = read_file(ed->query_target_file());
      if (str) {
	 lines = explode("\n" + str + "\n", "\n");
      } else {
	 lines = ({ });
      }
      remove_file(ed->query_target_file());
      if (!call_other(this_object(), func, user, body, lines, args...)) {
	 user->message("Buffer written.\n");
      }
      ed->die();
      return;
   }
}

static string
dump_script_map(string desc, object body, mapping map)
{
   int    i, sz;
   object *targets;
   string result;
   mapping *scripts;

   sz      = map_sizeof(map);
   targets = map_indices(map);
   scripts = map_values(map);
   result  = desc + " found in " + LINK(name(body), "+tool popup %" + ur_name(body), "command") + ":\n";

   sz = sizeof(targets);
   for (i = 0; i < sz; i++) {
      int j, sz_j;
      string *list;

      if (targets[i] == body) {
	 result += "    In the object itself:\n";
      } else {
	 result += "    Inherited from " + LINK(name(targets[i]), "+tool popup %" + ur_name(targets[i]), "command") + ":\n";
      }
      list = map_indices(scripts[i]);
      sz_j = sizeof(list);
      for (j = 0; j < sz_j; j++) {
	 list[j] = LINK(list[j], "+tool popup %" + ur_name(targets[i]) + " merry:" + list[j], "command");
      }
      result += "        " + implode(list, "\n        ") + "\n";
   }
   return result;
}
