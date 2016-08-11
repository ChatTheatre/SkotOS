/*
 *	/core/sys/handler.c
 *
 *	Take care of the 'core:*' properties.
 *
 *	Copyright Skotos Tech Inc 2001
 */

# include <type.h>
# include <core.h>
# include <dp_handler.h>

inherit "/core/lib/dp_handler";

string query_derived_root() {
   return "core";
}

static
void create() {
   CORE_DP_REGISTRY->register_root_handler("core", this_object());
   set_object_name("Core:DerivedCore");
}

int r_query_method(string root, string method) {
   if (root != "core") {
      error("unexpected root: " + dump_value(root));
   }
   return !!function_object("method_" + method, this_object());
}

mixed r_call_method(object ob, string root, string method, mapping args) {
   if (root != "core") {
      error("unexpected root: " + dump_value(root));
   }
   if (function_object("method_" + method, this_object())) {
      return call_other(this_object(), "method_" + method, ob, args);
   }
   error("unknown method: " + dump_value(method));
}


mapping get_configuration() {
   return ([
	      "creation":
	      ({ C_F_READ, T_INT,
		    "when was this object created (if known)" }),
	      "delays":
	      ({ C_F_READ, C_F_READ | C_T_ARRFLAG,
		    "complex datastructure describing current delayed calls in object" }),
	      "ur:firstchild":
	      ({ C_F_READ, T_OBJECT,
		    "first entry in this object's list of ur-children" }),
	      "ur:sibling:next":
	      ({ C_F_READ, T_OBJECT,
		    "next sibling in this object's ur-parent's ur-child list" }),
	      "objectname":
	      ({ C_F_RW, T_STRING,
		    "this object's woe name" }),
	      "ur:sibling:previous":
	      ({ C_F_READ, T_OBJECT,
		    "previous sibling in this object's ur-parent's ur-child list" }),
	      "ur:parent":
	      ({ C_F_RW, T_OBJECT,
		    "this object's ur-parent" }),
	      ]);
}

mixed r_query_property(object ob, string root, string property) {
   if (root != "core") {
      return FALSE;
   }
   switch(property) {
   case "creation":
      return ob->query_creation_stamp();
   case "delays":
   case "delays:local":
      return ob->query_delayed_calls();
   case "ur:firstchild":
   case "ur:firstchild:local":
      return ob->query_first_child();
   case "ur:sibling:next":
   case "ur:sibling:next:local":
      return ob->query_next_ur_sibling();
   case "ur:sibling:previous":
   case "ur:sibling:previous:local":
      return ob->query_previous_ur_sibling();
   case "ur:parent":
   case "ur:parent:local":
      return ob->query_ur_object();
   case "objectname":
   case "objectname:local":
      return name(ob);
   }
}

mapping r_query_properties(object ob, string root, int derived) {
   mapping map, config;
   string *ix;
   int i;

   if (root != "core" || !derived) {
      return ([ ]);
   }
   
   config = get_configuration();

   ix = map_indices(config);

   map = ([ ]);

   for (i = 0; i < sizeof(ix); i ++) {
      map[ix[i]] = r_query_property(ob, root, ix[i]);
      if (config[ix[i]][CONFIG_FLAGS] & C_F_LOCAL) {
	 map[ix[i] + ":local"] = r_query_property(ob, root, ix[i] + ":local");
      }
   }
   return map;
}

mixed r_set_property(object ob, string root, string property, mixed value,
		     mixed *retval) {
   string str;
   mixed tmp;

   if (root != "core") {
      return FALSE;
   }

   sscanf(property, "%s:local", property);

   if (!get_configuration()[property]) {
      return FALSE;
   }
   switch(property) {
   case "objectname":
      ob->set_object_name(value);
      return TRUE;
   case "ur:parent":
      ob->set_ur_object(value);
      return TRUE;
   default:
      DEBUG("Attempt to set read-only property udat:" + property + " in " + name(ob));
      return TRUE;
   }
}
