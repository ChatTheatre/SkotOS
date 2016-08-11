/*
 * /base/lib/properties.c
 *
 * Wrapper that checks for special properties of "<root>:*" format.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <base.h>

private inherit "/lib/mapping";
private inherit "/lib/string";


inherit "/lib/urproperties";
inherit properties "/lib/properties";

static void post_hook(string hook, string name, mapping args);

static void
create()
{
    properties :: create();
}

string query_detail(varargs object looker);


int query_method(string method) {
   mixed  *data;

   data = BASE_PROPERTIES->query_root_handler(method);

   if (data) {
      return !!data[0]->query_method(data[1], data[2]);
   }
   return FALSE;
}

mixed
call_method(string method, mapping args) {
   mixed  *data;

   data = BASE_PROPERTIES->query_root_handler(method);

   if (data) {
      return data[0]->call_method(this_object(), data[1], data[2], args);
   }
   error("unknown root in method " + dump_value(method));
}

mixed
query_downcased_property(string prop) {
    mixed *data, val;

    val = ::query_downcased_property(prop);
    if (val != nil) {
	return val;
    }
    if (sscanf(prop, "detail:%s", prop)) {
       prop = "details:" + query_detail() + ":" + prop;
    }
    data = BASE_PROPERTIES->query_root_handler(prop);
    if (data) {
       return data[0]->query_property(this_object(), data[1], data[2]);
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

    root_map = BASE_PROPERTIES->query_root_handler_map();
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

	    sub = handler->query_properties(this_object(), root, derived);
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

private mixed
force_property_type(string prop, mixed val)
{
    if (val != nil) {
	int t_prop;

	t_prop = BASE_PROPERTIES->query_typed_property(prop);
	if (t_prop != -1) {
	    int t_val;

	    /* -1 indicates no specific property is being enforced */
	    t_val  = typeof(val);
	    if (t_val != t_prop) {
		/* Ah, type is not the intended type. */
		switch (t_prop) {
		case T_INT:
		    switch (t_val) {
		    case T_FLOAT:
			val = (int)val;
			t_val = typeof(val);
			break;
		    case T_STRING:
			sscanf(val, "%d", val);
			t_val = typeof(val);
			break;
		    default:
			break;
		    }
		    break;
		case T_FLOAT:
		    switch (t_val) {
		    case T_INT:
			val = (float)val;
			t_val = typeof(val);
			break;
		    case T_STRING:
			sscanf(val, "%f", val);
			t_val = typeof(val);
			break;
		    default:
			break;
		    }
		    break;
		case T_STRING:
		    switch (t_val) {
		    case T_INT:
		    case T_FLOAT:
			val = (string)val;
			t_val = typeof(val);
		    default:
			break;
		    }
		    break;
		case T_OBJECT:
		case T_ARRAY:
		case T_MAPPING:
		    /* What can be done to help here? */
		    break;
		}
		if (t_val != t_prop) {
		    /* Still the wrong type. */
		    switch (t_val) {
		    case T_INT:
			error("Bad type (int) for property " + prop);
		    case T_FLOAT:
			error("Bad type (float) for property " + prop);
		    case T_STRING:
			error("Bad type (string) for property " + prop);
		    case T_OBJECT:
			error("Bad type (object) for property " + prop);
		    case T_ARRAY:
			error("Bad type (array) for property " + prop);
		    case T_MAPPING:
			error("Bad type (mapping) for property " + prop);
		    default:
			error("Bad type (" + t_val + ") for property " + prop);
		    }
		}
	    }
	}
    }
    return val;
}

mixed
set_downcased_property(string prop, mixed val, varargs int noderived) {
   mixed  *data, old;

   if (sscanf(prop, "detail:%s", prop)) {
      prop = "details:" + query_detail() + ":" + prop;
   }

   val = force_property_type(prop, val);

   data = BASE_PROPERTIES->query_root_handler(prop);
   if (data) {
      mixed *retval;

      if (noderived) {
	 return nil;
      }
      retval = ({ nil });
      if (data[0]->set_property(this_object(), data[1], data[2],
				val, retval)) {
	 /* Returns 1 if the handler deals with this one. */
	 post_hook("setprop", prop, ([ "target": this_object(),
				       "hook-property": prop,
				       "hook-value": val ]));
	 return retval[0];
      }
   } else {
      old = query_downcased_property(prop);
   }

   ::set_downcased_property(prop, val);
   post_hook("setprop", prop, ([ "target": this_object(),
				 "hook-oldvalue": old,
				 "hook-property": prop,
				 "hook-value": val ]));
}

void
clear_downcased_property(string prop) {
   mixed *data, old;

   data = BASE_PROPERTIES->query_root_handler(prop);
   if (data) {
      mixed *retval;

      retval = ({ nil });
      if (data[0]->set_property(this_object(), data[1], data[2], nil,
				retval)) {
	 post_hook("setprop", prop, ([ "target": this_object(),
				       "hook-property": prop ]));
	 return;
      }
   } else {
      old = query_downcased_property(prop);
   }

   ::clear_downcased_property(prop);
   post_hook("setprop", prop, ([ "target": this_object(),
				 "hook-oldvalue": old,
				 "hook-property": prop ]));
}

void
clear_all_properties()
{
    int     sz;
    mapping map, root_map;

    ::clear_all_properties();
    root_map = BASE_PROPERTIES->query_root_handler_map();
    sz = map_sizeof(root_map);
    if (sz) {
	int    i;
	string *roots;
	object *handlers;

	roots    = map_indices(root_map);
	handlers = map_values(root_map);
	for (i = 0; i < sz; i++) {
	    handlers[i]->clear_all_properties(this_object(), roots[i]);
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
