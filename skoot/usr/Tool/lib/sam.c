/*
 * ~Tool/lib/sam.c
 *
 * SAM support.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <status.h>
# include <kernel/access.h>

# include "~/include/tool.h"

private inherit "/lib/string";

private inherit "~XML/lib/xmlparse";
private inherit "~SAM/lib/sam";

inherit "utility";

void womble_xml() {
  ::womble_xml();
}

void
cmd_sam_exec(object user, object body, varargs string arg)
{
   mixed  state;
   string result;
   int ticks;

   if (!arg) {
      prompt_to(user, "End your SAM text with a single period.\n",
		"sam_exec_quote", "");
      return;
   }
   ticks = status()[ST_TICKS];
   state = ::parse_xml(arg);
   result = unsam(state,
		  ([
		     "this":   body,	/* added by Zell 030404 */
		     "body":   body,
		     "actor":  body,
		     "looker": body,
		     "here":   body ? body->query_environment() : nil
		   ]),
		  user,
		  body);
   ticks = ticks - status()[ST_TICKS];
   pre_message(user,
	       "Original input:\n" +
	       "        " + arg + "\n" +
	       "\n" +
	       "Output:\n" +
	       "        " + dump_value(result) + "\n" +
	       "\n" +
	       "Ticks used for this code: " + ticks + "\n");
}

static void
sam_exec_quote(object user, object body, string arg, string text)
{
   if (arg == ".") {
      mixed  state;
      string result;

      state = ::parse_xml(text);
      result = unsam(state, ([
	 "this":   body,
	 "body":   body,
	 "actor":  body,
	 "looker": body,
	 "here":   body ? body->query_environment() : nil
	 ]),
	 user,
	 body);
      pre_message(user,
		  "Original input:\n" +
		  "\n" +
		  text +
		  "\n" +
		  "Output:\n" +
		  "        " + dump_value(result) + "\n");
      return;
   }
   text += arg + "\n";
   prompt_to(user, "", "sam_exec_quote", text);
}

void
cmd_sam_parse(object user, object body, string arg)
{
   mixed state;

   state = ::parse_xml(arg);
   pre_message(user,
	       "Original input:\n" +
	       "        " + arg + "\n" +
	       "\n" +
	       "Parsed format:\n" +
	       wrap_text("        ", dump_value(state)));
}

private string
sam_category(string cat)
{
   switch (cat) {
   case nil:            return "Generic";
   case "control":      return "Flow control";
   case "layout":       return "Text formatting";
   case "settings":     return "Set & query";
   case "undocumented": return "Undocumented";
   default:             return "Unknown (" + cat + ")";
   }
}

void
cmd_sam_info(object user, object body, varargs string arg)
{
   int    i, sz, public;
   string *handlers, description, output, category;
   object ob;
   mapping required, optional, total;

   handlers = ({ "SAM", "SkotOS" });
   sz = sizeof(handlers);
   if (!arg) {
      string  result, *cats;
      mapping categories;

      categories = ([ ]);
      for (i = 0; i < sz; i++) {
	 int j, sz_j;
	 object *list;
	 mixed *dir;
	 string path;

	 path = "/usr/" + handlers[i] + "/samtags/";
	 catch {
	    dir = get_dir(path + "*.c");
	 } : {
	    continue;
	 }
	 list = dir[3];
	 sz_j = sizeof(list);
	 for (j = 0; j < sz_j; j++) {
	    string name;

	    if (!list[j]) {
	       continue;
	    }
	    name = dir[0][j];
	    name = name[..strlen(name) - 3];
	    if (!function_object("tag_documentation", list[j])) {
	       if (!categories["undocumented"]) {
		  categories["undocumented"] = ([ ]);
	       }
	       categories["undocumented"][name] = TRUE;
	    } else if (list[j]->tag_documentation("public")) {
	       string cat;

	       cat = list[j]->tag_documentation("category");
	       if (!categories[cat]) {
		  categories[cat] = ([ ]);
	       }
	       categories[cat][name] = TRUE;
	    }
	 }
      }
      result = "These SAM tags currently exist:\n";
      sz   = map_sizeof(categories);
      cats = map_indices(categories);
      for (i = 0; i < sz; i++) {
	 string *tags;

	 tags = map_indices(categories[cats[i]]);
	 result += "    " + sam_category(cats[i]) + ":\n" +
	           wrap_text("        ", implode(tags, ", "));
      }
      pre_message(user, result);
      user->message("Usage: +tool sam info <tagname>\n");
      return;
   }
   arg = lower_case(arg);
   for (i = 0; i < sz && !ob; i++) {
      ob = find_object("/usr/" + handlers[i] + "/samtags/" + arg);
   }
   if (!ob) {
      user->message("+tool sam info: No tag named \"" + arg + "\" found.\n");
      return;
   }
   if (!function_object("tag_documentation", ob)) {
      user->message("+tool sam info: No tag named \"" + arg + "\" found.\n");
      return;
   }
   public = ob->tag_documentation("public");
   if (!public) {
      user->message("+tool sam info: No tag named \"" + arg + "\" found.\n");
      return;
   }
   category    = ob->tag_documentation("category");
   description = ob->tag_documentation("description");
   required    = ob->tag_documentation("required");
   optional    = ob->tag_documentation("optional");
   output =
      "Tag:\n" +
      "    <" + arg + ">\n" +
      "\n" +
      "Category:\n" +
      "    " + sam_category(category) + "\n" +
      "\n" +
      "Description:\n" +
      wrap_text("    ", description);

   total = ([ ]);
   if (required) {
      string *params, *descs;

      sz     = map_sizeof(required);
      params = map_indices(required);
      descs  = map_values(required);
      for (i = 0; i < sz; i++) {
	 total[params[i]] = descs[i];
      }
   }
   if (optional) {
      string *params, *descs;

      sz     = map_sizeof(optional);
      params = map_indices(optional);
      descs  = map_values(optional);
      for (i = 0; i < sz; i++) {
	 total[params[i] + " (optional)"] = descs[i];
      }
   }
   sz = map_sizeof(total);
   if (sz > 0) {
      string *params, *descs;

      output += "\nParameters:\n";
      params = map_indices(total);
      descs = map_values(total);
      for (i = 0; i < sz; i++) {
	 output += "    " + params[i] + ":\n" +
	           wrap_text("        ", descs[i]);
      }
   }
   pre_message(user, output);
}

static void
sam_twiki(object user, object body)
{
   int     i, sz;
   string  *handlers, *cats, result, *names;
   mapping categories, all_tags, *tag_data;

   handlers = ({ "SAM", "SkotOS" });
   sz = sizeof(handlers);
   categories = ([ ]);
   all_tags = ([ ]);
   for (i = 0; i < sz; i++) {
      int    j, sz_j;
      object *list;
      mixed  *dir;
      string path;

      path = "/usr/" + handlers[i] + "/samtags/";
      catch {
	 dir = get_dir(path + "*.c");
      } : {
	 continue;
      }
      list = dir[3];
      sz_j = sizeof(list);
      for (j = 0; j < sz_j; j++) {
	 string name;

	 if (!list[j]) {
	    continue;
	 }
	 name = dir[0][j];
	 name = name[..strlen(name) - 3];
	 if (!function_object("tag_documentation", list[j])) {
	    if (!categories["undocumented"]) {
	       categories["undocumented"] = ([ ]);
	    }
	    categories["undocumented"][name] = list[j];
	 } else if (list[j]->tag_documentation("public")) {
	    string cat;

	    cat = list[j]->tag_documentation("category");
	    if (!categories[cat]) {
	       categories[cat] = ([ ]);
	    }
	    categories[cat][name] = list[j];
	    all_tags[name] = ([
		 "category":    cat,
		 "description": list[j]->tag_documentation("description"),
		 "required":    list[j]->tag_documentation("required"),
		 "optional":    list[j]->tag_documentation("optional")
            ]);
	 }
      }
   }
   result = "<H1>SAM tags documentation</H1>\n" +
            "\n" +
            "%TOC{depth=\"2\"}%\n" +
            "\n" +
            "---+ Quick reference tables\n";

   sz   = map_sizeof(categories);
   cats = map_indices(categories);
   for (i = 0; i < sz; i++) {
      int    j, sz_j;
      string *tags;
      object *list;

      result +=
	 "\n" +
	 "|*" + sam_category(cats[i]) + " SAM tags*|||\n" +
	 "|*Tag*|*Required*|*Optional*|\n";

      tags = map_indices(categories[cats[i]]);
      list = map_values(categories[cats[i]]);
      sz_j = sizeof(tags);
      for (j = 0; j < sz_j; j++) {
	 mapping required, optional;

	 required = list[j]->tag_documentation("required");
	 required = required ? required : ([ ]);
	 optional = list[j]->tag_documentation("optional");
	 optional = optional ? optional : ([ ]);
	 result +=
	    "|" +
	    "[[#" + capitalize(implode(explode(tags[j], "-"), "")) + "][" + tags[j] + "]]" +
	    "|" +
	    (map_sizeof(required) ? implode(map_indices(required), ", ") :
	     "&nbsp;") +
	    "|" +
	    (map_sizeof(optional) ? implode(map_indices(optional), ", ") :
	     "&nbsp;") +
	    "|\n";
      }
   }
   result +=  "---+ Full listing of the tags\n";
   sz = map_sizeof(all_tags);
   names = map_indices(all_tags);
   tag_data = map_values(all_tags);
   for (i = 0; i < sz; i++) {
      int     j, sz_j;
      mapping total, required, optional;

      required = tag_data[i]["required"];
      optional = tag_data[i]["optional"];
      total = ([ ]);
      if (required) {
	 string *params, *descs;

	 sz_j   = map_sizeof(required);
	 params = map_indices(required);
	 descs  = map_values(required);
	 for (j = 0; j < sz_j; j++) {
	    total[params[j]] = descs[j];
	 }
      }
      if (optional) {
	 string *params, *descs;

	 sz_j   = map_sizeof(optional);
	 params = map_indices(optional);
	 descs  = map_values(optional);
	 for (j = 0; j < sz_j; j++) {
	    total[params[j] + "&nbsp;(optional)"] = descs[j];
	 }
      }
      result +=
	 "\n" +
	 "#Tag" + capitalize(implode(explode(names[i], "-"), "")) + "\n" +
	 "---++ *&lt;" + names[i] + "&gt;*\n" +
         "   <B>Category</B>: " +
	 sam_category(tag_data[i]["category"]) + "\n" +
	 wrap_text("   <B>Description</B>: ",
		   replace_html(tag_data[i]["description"]));
      sz_j = map_sizeof(total);
      if (sz_j > 0) {
	 int j;
	 string *params, *descs;

	 result +=
	    "   <B>Parameters</B>: \\\n\n";
	 params = map_indices(total);
	 descs = map_values(total);
	 for (j = 0; j < sz_j; j++) {
	    result +=
	       wrap_text("      <I>" + params[j] + "</I>: ",
			 replace_html(descs[j]));
	 }
      }
   }
   pre_message(user, result);
}
