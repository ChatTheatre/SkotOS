/*
 * /base/sys/properties.c
 *
 * Keep track of the property-handlers for '<root>:*' format properties,
 * using /core/lib/dp_registry.c
 *
 * And, now that we've got your attention, also track property-types!
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <type.h>

private inherit "/lib/string";

inherit "/core/lib/dp_registry";

private mapping root_handlers;
private mapping typed_properties;
private mapping typed_comments;

string query_state_root() { return "Base:PropertyTypes"; }

nomask void patch() {
   if (root_handlers) {
      string *ix;
      int i;

      ::create();

      ix = map_indices(root_handlers);
      for (i = 0; i < sizeof(ix); i ++) {
	 register_root_handler(ix[i], root_handlers[ix[i]]);
      }
      root_handlers = nil;
   }
   register_root_handler("core", find_object("/core/sys/handler"));
}

static void
create()
{
    set_object_name("Base:PropertyTypes");

    ::create();

    typed_properties = ([
	"skill:advancedefensive":    T_INT,
	"skill:attackingdefensive":  T_INT,
	"skill:cutoffensive":        T_INT,
	"skill:dodgedefensive":      T_INT,
	"skill:fatigue":             T_FLOAT,
	"skill:feintoffensive":      T_INT,
	"skill:fumbledefensive":     T_INT,
	"skill:guarddefensive":      T_INT,
	"skill:idledefensive":       T_INT,
	"skill:lang1":               T_INT,
	"skill:lang2":               T_INT,
	"skill:lang3":               T_INT,
	"skill:lang4":               T_INT,
	"skill:lang5":               T_INT,
	"skill:lang6":               T_INT,
	"skill:lang7":               T_INT,
	"skill:recoverdefensive":    T_INT,
	"skill:restdefensive":       T_INT,
	"skill:retiredefensive":     T_INT,
	"skill:salutedefensive":     T_INT,
	"skill:teaching":            T_INT,
	"skill:thrustoffensive":     T_INT,
	"skill_study":               T_INT,
	"skotos:charname":           T_STRING,
	"skotos:creator":            T_STRING,
	"skotos:currentlang":        T_INT,
	"startstory:skilllist:done": T_ARRAY,
	"startstory:skilllist:left": T_ARRAY,
	"page:allow":                T_MAPPING,
	"page:allowall":             T_INT,
	"page:pending":              T_MAPPING,
	"virtualhome:home":          T_OBJECT,
	"virtualhome:oubliette":     T_OBJECT,
	]);
    typed_comments = ([ ]);
}

string *
list_typed_properties(int t)
{
    int i, sz, *types;
    string *props;

    sz = map_sizeof(typed_properties);
    props = map_indices(typed_properties);
    types = map_values(typed_properties);
    for (i = 0; i < sz; i++) {
	if (types[i] != t) {
	    props[i] = nil;
	}
    }
    return props - ({ nil });
}

void
set_typed_property(string prop, int t, varargs string comment)
{
    typed_properties[lower_case(prop)] = t;
    typed_comments[lower_case(prop)] = comment;
}

string
query_typed_comment(string prop) {
   return typed_comments[lower_case(prop)];
}

void
create_typed_property(int t)
{
    int    i;
    string prop;

    prop = "property";
    i = 1;
    while (typed_properties[prop + i] != nil) {
	i++;
    }
    typed_properties[prop + i] = t;
}

void
delete_typed_property(string prop)
{
    typed_properties[prop] = nil;
}

int
query_typed_property(string prop)
{
    mixed t;

    t = typed_properties[prop];
    return t == nil ? -1 : t;
}

void
clear_typed_properties(varargs int t)
{
    if (t) {
	/* Clear all properties of type t. */
	int i, sz, *types;
	string *props;
	mapping map;

	map = ([ ]);
	sz = map_sizeof(typed_properties);
	props = map_indices(typed_properties);
	types = map_values(typed_properties);
	for (i = 0; i < sz; i++) {
	    if (types[i] != t) {
		map[props[i]] = types[i];
	    }
	}
	typed_properties = map;
    } else {
	typed_properties = ([ ]);
    }
}
