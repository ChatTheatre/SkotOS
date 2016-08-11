/*
 * ~Tool/sys/list.c
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include "~/include/tool.h"

inherit "~/lib/list";

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
      ({ "list by client", "cmd_list_client" }),
      ({ "list clients",   "cmd_list_client" })
   });
}

string
help_list(varargs string topic)
{
   switch (topic) {
   case nil:
      return "Needs to be written still.";
   case "client":
   case "clients":
   default:
      return "Needs to be written still.";
   }
}

string *
topics_list()
{
   return ({ "client", "clients" });
}
