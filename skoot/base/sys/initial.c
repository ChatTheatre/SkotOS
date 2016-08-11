/*
 * /base/sys/initial.c
 *
 * Take care of the initial:* properties.
 *
 * Copyright Skotos Tech Inc 2006
 */

# define INITIAL "initial"

# include <base.h>

inherit "/core/lib/dp_handler";

string query_derived_root() {
   return INITIAL;
}

static
void create() {
   BASE_PROPERTIES->register_root_handler(INITIAL, this_object());
   set_object_name("Base:DerivedInitial");
}

int r_query_method(string root, string method) {
   if (root != INITIAL) {
      error("unexpected root: " + dump_value(root));
   }
   return !!function_object("method_" + method, this_object());
}

mixed r_call_method(object ob, string root, string method, mapping args) {
   if (root != INITIAL) {
      error("unexpected root: " + dump_value(root));
   }
   if (function_object("method_" + method, this_object())) {
      return call_other(this_object(), "method_" + method, ob, args);
   }
   error("unknown method: " + dump_value(method));
}

mixed r_query_property(object ob, string root, string property) {
   if (root != INITIAL) {
      return nil;
   }
   return ob->query_initial_property(property);
}

mapping r_query_properties(object ob, string root, int derived) {
   mapping map;

   if (root != INITIAL || !derived) {
      return ([ ]);
   }

   map = ob->query_all_initial_properties();
   return map ? map : ([ ]);
}

mixed r_set_property(object ob, string root, string property, mixed value, mixed *retval) {
   if (root != INITIAL) {
      return FALSE;
   }
   if (value == nil) {
      ob->clear_initial_property(property);
   } else {
      ob->set_initial_property(property, value);
   }
   return TRUE;
}

void r_clear_all_properties(object ob, string root)
{
   /* NOOP */
}
