/*
 * /base/sys/brain.c
 *
 * Take care of the 'brain:*' properties.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <base.h>

private inherit "/lib/string";

static
void create() {
   BASE_PROPERTIES->register_root_handler("brain", this_object());
}

mixed r_query_property(object ob, string root, string property) {
   if (root == "brain") {
      return ob->query_brain(lower_case(property));
   }
}

mapping r_query_properties(object ob, string root, int derived) {
   return ob->query_braindata()[..];
}

mixed r_set_property(object ob, string root, string property, mixed value,
		   mixed *retval) {
   if (root == "brain") {
      ob->set_brain(lower_case(property), value);
      return TRUE;
   }
   return FALSE;
}
