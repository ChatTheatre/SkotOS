/*
 *	/core/lib/core_dp.c
 *
 *	Handle derived properties for an object.
 *
 *	Copyright Skotos Tech Inc 2001
 */

private inherit "/lib/mapping";
private inherit "/lib/string";

inherit properties   "/lib/properties";
inherit urproperties "/lib/urproperties";

static void post_hook(string hook, string name, mapping args);

string query_dp_registry();

static void
create()
{
    properties   :: create();
    urproperties :: create();
}

int query_method(string method) {
   mixed  *data;

   data = query_dp_registry()->query_root_handler(method);

   if (data) {
      return !!data[0]->r_query_method(data[1], data[2]);
   }
   return FALSE;
}

mixed
call_method(string method, mapping args) {
   mixed  *data;

   data = query_dp_registry()->query_root_handler(method);

   if (data) {
      return data[0]->r_call_method(this_object(), data[1], data[2], args);
   }
   error("unknown root in method " + dump_value(method));
}


mixed
query_downcased_property(string prop) {
    mixed *data, val;

    val = ::query_downcased_property(prop);
    if (val != nil) {
       switch (prop) {
       case "base:gait":
       case "base:parts-covered:local":
	  /* Sorry, need to hack this in. --Erwin. */
	  ::set_downcased_property(prop, nil);
	  val = nil;
	  break;
       default:
	  if (sscanf(prop, "initial:%*s")) {
	     /* More hacking needed.  Oops. */
	     ::set_downcased_property(prop, nil);
	     val = nil;
	     break;
	  }
	  return val;
       }
    }
    data = query_dp_registry()->query_root_handler(prop);
    if (data) {
       val = data[0]->r_query_property(this_object(), data[1], data[2]);
       if (val != nil) {
	  return val;
       }
    }
    return query_exported_property(prop);
}

mapping
query_properties(varargs int derived)
{
    int     sz;
    mapping map, root_map;

    if (derived) {
       map = imported_properties() + ::query_properties();
    } else {
       map = ::query_properties();
    }

    root_map = query_dp_registry()->query_root_handler_map();
    sz = map_sizeof(root_map);
    if (sz) {
	int    i;
	string *roots;
	object *handlers;

	roots    = map_indices(root_map);
	handlers = map_values(root_map);
	for (i = 0; i < sz; i++) {
	    string  root;
	    object  handler;
	    mapping sub;

	    root = roots[i];
	    handler = handlers[i];

	    sub = handler->r_query_properties(this_object(), root, derived);
	    if (sub) {
		int sz_j;

		sz_j = map_sizeof(sub);
		if (sz_j) {
		    int j;
		    string *props;
		    mixed  *vals;

		    props = map_indices(sub);
		    vals = map_values(sub);
		    for (j = 0; j < sz_j; j++) {
			map[root + ":" + props[j]] = vals[j];
		    }
		}
	    }
	}
    }
    return map;
}

string *query_property_indices(varargs int derived) {
   return map_indices(query_properties(derived));
}

private void
setprop_post_hooks(string prop, mapping data)
{
   int i;

   post_hook("setprop", prop, data);

   for (i = strlen(prop) - 1; i > 0; i--) {
       if (prop[i] == ':') {
	   post_hook("setprop", prop[..i - 1], data);
       }
   }
}

mixed
set_downcased_property(string prop, mixed val, varargs int noderived) {
   mixed  *data, old;

   data = query_dp_registry()->query_root_handler(prop);
   if (data) {
      mixed *retval;

      if (noderived) {
	 return nil;
      }
      retval = ({ nil });
      if (data[0]->r_set_property(this_object(), data[1], data[2],
				  val, retval)) {
	 /* Returns 1 if the handler deals with this one. */
	 setprop_post_hooks(prop, ([ "target": this_object(),
				     "hook-property": prop,
				     "hook-value": val ]));
	 return retval[0];
      }
   } else {
      old = query_downcased_property(prop);
   }

   ::set_downcased_property(prop, val);
   setprop_post_hooks(prop, ([ "target": this_object(),
			       "hook-oldvalue": old,
			       "hook-property": prop,
			       "hook-value": val ]));
}

void
clear_downcased_property(string prop) {
   mixed *data, old;

   data = query_dp_registry()->query_root_handler(prop);
   if (data) {
      mixed *retval;

      retval = ({ nil });
      if (data[0]->r_set_property(this_object(), data[1], data[2], nil,
				  retval)) {
	  setprop_post_hooks(prop, ([ "target": this_object(),
				      "hook-property": prop ]));
	 return;
      }
   } else {
      old = query_downcased_property(prop);
   }

   ::clear_downcased_property(prop);
   setprop_post_hooks(prop, ([ "target": this_object(),
			       "hook-oldvalue": old,
			       "hook-property": prop ]));
}

void
clear_all_properties()
{
    int     sz;
    mapping map, root_map;

    ::clear_all_properties();
    root_map = query_dp_registry()->query_root_handler_map();
    sz = map_sizeof(root_map);
    if (sz) {
	int    i;
	string *roots;
	object *handlers;

	roots    = map_indices(root_map);
	handlers = map_values(root_map);
	for (i = 0; i < sz; i++) {
	    handlers[i]->r_clear_all_properties(this_object(), roots[i]);
	}	
    }
}

void
add_properties(mapping map)
{
    if (map) {
	int    i, sz;
	string *props;
	mixed  *vals;

	sz    = map_sizeof(map);
	props = map_indices(map);
	vals  = map_values(map);
	for (i = 0; i < sz; i++) {
	    set_property(props[i], vals[i]);
	}
    }
}
