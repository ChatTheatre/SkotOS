/*
**	This is a class that implements a very generic property
**	access mechanism. We use this for SAM variable access to
**	various objects in the system. Any module may inherit us.
*/

# include <type.h>

inherit "/lib/womble";
private inherit "/lib/string";
private inherit "/lib/mapping";

private mapping properties;

string query_state_root() { return "Core:Properties"; }

void womble_properties() {
    properties = womble(properties);
}

static
void create() {
   properties = ([ ]);
}

void clear_downcased_property(string prop) {
   properties[prop] = nil;
}

nomask
void clear_property(string prop) {
   clear_downcased_property(lower_case(prop));
}


void clear_all_properties ( void ) {
   properties = ([ ]);		
}

mixed set_downcased_property(string prop, mixed val, varargs mixed extra...) {
   properties[prop] = val;
}

nomask
mixed set_property(string prop, mixed val, varargs mixed extra...) {
   return set_downcased_property(lower_case(prop), val, extra...);
}

mixed query_downcased_property(string prop) {
   return properties[prop];
}

/* this one is intended for e.g. Merry script lookups */
nomask
mixed query_raw_property(string prop) {
   return properties[prop];
}

/* this one is intended for e.g. Merry script lookups */
nomask
mapping query_prefixed_properties(string prefix) {
   return prefixed_map(properties, prefix);
}

nomask
mixed query_property(string prop) {
   return query_downcased_property(lower_case(prop));
}

string query_ascii_property(string prop) {
   mixed val;

   val = query_property(prop);
   if (val != nil) {
      return mixed_to_ascii(val, TRUE);
   }
   return nil;
}

void set_ascii_property(string prop, string val, varargs int opaque) {
   set_property(prop, val ? ascii_to_mixed(val, "nullify") : nil, opaque);
}

static
object nullify(string ref) {
   return nil;
}

mapping query_properties(varargs int derived) {
   return properties[..];
}

void add_properties(mapping map) {
   properties += map;
}

string *query_property_indices(varargs int opaque) {
   return map_indices(query_properties(opaque));
}

mapping query_intrinsic_properties() {
   return properties[..];
}
