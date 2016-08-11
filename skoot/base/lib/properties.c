/*
 *	/base/lib/properties.c
 *
 *	This is a bit of functionality on top of /core/lib/core_object.c
 *	which now supplies most of what we need.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <base.h>

inherit "/core/lib/core_object";

string query_detail(varargs object looker);

/* implement the abstract function required by core_object.c */
string query_dp_registry() {
   return BASE_PROPERTIES;
}

static
void create() {
   :: create();
}

mixed
query_downcased_property(string prop) {
    if (sscanf(prop, "detail:%s", prop)) {
       prop = "details:" + query_detail() + ":" + prop;
    }
    return ::query_downcased_property(prop);
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
   if (sscanf(prop, "detail:%s", prop)) {
      prop = "details:" + query_detail() + ":" + prop;
   }

   val = force_property_type(prop, val);

   return ::set_downcased_property(prop, val, noderived);
}
