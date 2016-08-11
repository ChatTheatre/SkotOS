/*
 * ~Tool/lib/help.c
 *
 * Tool help system.
 *
 * Copyright Skotos Tech Inc 2002, 2003.
 */

# include "~/include/tool.h"

private inherit "/lib/string";

inherit "~/lib/utility";

static string
display_list(string *list)
{
   int i, sz;

   if (!list) {
      return "none";
   }
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      list[i] = capitalize(list[i]);
   }
   switch (sz) {
   case 0:
      return "None";
   case 1:
      return list[0];
   default:
      return implode(list[..sz - 2], ", ") + " and " + list[sz - 1];
   }
}

static string
format_info(string topic, string text, string *topics, varargs int sub)
{
   return
      "About:\n" + wrap_text("        ", capitalize(topic)) +
      "\n" +
      "Description:\n" + wrap_text("        ", text) +
      ( (topics && sizeof(topics)) ?
	"\nSee also:\n" +
	wrap_text("        ", display_list(topics)) :
	"" );
}

void
cmd_help(object user, object body, varargs string arg0, string arg1)
{
   string topic, arg, info, *topics;
   object ob;

   if (!arg0) {
      pre_message(user, format_info("Tool", this_object()->help_tool(),
				    this_object()->topics_tool()));
      return;
   }
   if (!arg1) {
      topic = arg0;
      ob = SYS_TOOL->find_handler_ob("help", topic);
      if (ob && (info = call_other(ob, "help_" + topic))) {
	 topics = call_other(ob, "topics_" + topic);
	 pre_message(user, format_info(capitalize(topic), info, topics));
	 return;
      }
      info = this_object()->help_tool(topic);
      if (info) {
	 topics = this_object()->topics_tool();
	 pre_message(user, format_info(capitalize(topic), info, topics));
	 return;
      }
      topics = this_object()->topics_tool();
      pre_message(user, format_info(capitalize(topic),
				    "No help available for this topic!",
				    topics));
      return;
   }
   topic = strip(arg0);
   arg   = strip(arg1);

   ob = SYS_TOOL->find_handler_ob("help", topic);
   if (!ob) {
      /* No such topic exists, let alone such a sub-topic? */
      topics = this_object()->topics_tool();
      pre_message(user, format_info(capitalize(topic),
				    "No help available for this topic!",
				    topics));
      return;
   }
   info   = call_other(ob, "help_" + topic, arg);
   topics = call_other(ob, "topics_" + topic);
   if (info) {
      pre_message(user, format_info(capitalize(topic) + " - " +
				    capitalize(arg), info, topics));
      return;
   }
   pre_message(user, format_info(capitalize(topic) + " - " +
				 capitalize(arg),
				 "No help available for this topic!",
				 topics));
}

void
cmd_help_fallback(object user, object body, string arg)
{
   pre_message(user, format_info(capitalize(arg),
				 "No help available for this topic!",
				 this_object()->topics_tool()));
}
