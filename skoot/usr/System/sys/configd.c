/*
**	~System/sys/configd.c
**
**	Store generic information about the configs of each game.
**
**	Copyright Skotos Tech Inc 2001
*/
# include <System/log.h>

inherit "/lib/string";

mapping id_to_type;
mapping boolean_config;
mapping integer_config;
mapping string_config;
mapping object_config;

static
void create() {
   id_to_type = ([ ]);
   boolean_config = ([ ]);
   integer_config = ([ ]);
   string_config = ([ ]);
   object_config = ([ ]);
   set_object_name("System:Config");
   /* Indicate that other objects can subscribe to our event. */
   add_event("updated");
}

#if 0
void
patch()
{
    /* A quick patch. */
    add_event("updated");
}
#endif

void
patch()
{
   int i, sz;
   string *indices;
   mixed *values;

    /* Lowercase indices in id_to_type */
    sz = map_sizeof(id_to_type);
    indices = map_indices(id_to_type);
    values = map_values(id_to_type);
    id_to_type = ([ ]);
    for (i = 0; i < sz; i++) {
	if (id_to_type[lower_case(indices[i])] != nil) {
	    error("duplicate entries for " + indices[i] + " in id_to_type");
	}
	id_to_type[lower_case(indices[i])] = values[i];
    }

    /* Lowercase indices in boolean_config */
    sz = map_sizeof(boolean_config);
    indices = map_indices(boolean_config);
    values = map_values(boolean_config);
    boolean_config = ([ ]);
    for (i = 0; i < sz; i++) {
	if (boolean_config[lower_case(indices[i])] != nil) {
	    error("duplicate entries for " + indices[i] + " in boolean_config");
	}
	boolean_config[lower_case(indices[i])] = values[i];
    }

    /* Lowercase indices in integer_config */
    sz = map_sizeof(integer_config);
    indices = map_indices(integer_config);
    values = map_values(integer_config);
    integer_config = ([ ]);
    for (i = 0; i < sz; i++) {
	if (integer_config[lower_case(indices[i])] != nil) {
	    error("duplicate entries for " + indices[i] + " in integer_config");
	}
	integer_config[lower_case(indices[i])] = values[i];
    }

    /* Lowercase indices in string_config */
    sz = map_sizeof(string_config);
    indices = map_indices(string_config);
    values = map_values(string_config);
    string_config = ([ ]);
    for (i = 0; i < sz; i++) {
	if (string_config[lower_case(indices[i])] != nil) {
	    error("duplicate entries for " + indices[i] + " in string_config");
	}
	string_config[lower_case(indices[i])] = values[i];
    }

    /* Lowercase indices in object_config */
    sz = map_sizeof(object_config);
    indices = map_indices(object_config);
    values = map_values(object_config);
    object_config = ([ ]);
    for (i = 0; i < sz; i++) {
	if (object_config[lower_case(indices[i])] != nil) {
	    error("duplicate entries for " + indices[i] + " in object_config");
	}
	object_config[lower_case(indices[i])] = values[i];
    }
}

int
allow_subscribe(object ob, string ev)
{
    switch (ev) {
    case "updated":
	/* Allow any and all objects to subscribe to this event. */
	return 1;
    default:
	/* We don't provide this event, what are you babbling about. */
	return 0;
    }
}

string query_state_root() { return "System:Config"; }

/* utility function to find a specific mapping from a type */
private mapping type_to_mapping(string type) {
   switch(type) {
   case "boolean": return boolean_config;
   case "integer": return integer_config;
   case "string": return string_config;
   case "object": return object_config;
   }

   return nil;
}

/* utility function to find a default value from a type */
/* Dead code? --Erwin */
private mixed type_to_default(string type) {
   switch(type) {
   case "boolean": return FALSE;
   case "integer": return 0;
   case "string": return "Default";
   case "object": return this_object();
   }

   return nil;
}

/* query an entire config mapping */
mapping query_config(string type) {
   return type_to_mapping(type);
}

/* clear all the mappings, not for production use */
void clear_configs() {
   id_to_type = ([ ]);
   boolean_config = ([ ]);
   integer_config = ([ ]);
   string_config = ([ ]);
   object_config = ([ ]);
}   

/* query an entry from a config mapping */
mixed query_config_entry(string index) {
   string type;
   mapping config;

   index = lower_case(index);

   type = id_to_type[index];

   if (!type)
      return nil;

   config = type_to_mapping(type);

   if (!config)
      error("bad config");

   return config[index];
}

/* this is a front-end function for the queries */
/* all external code should probably call this */
mixed query(string index) { return query_config_entry(index); }

/* query a list of indices from a config */
string *query_entry_list(string type) {
   return map_indices(type_to_mapping(type));
}

/* set a config entry */
atomic void set_config_entry(string index, string type, mixed value) {
   mapping config;

   index = lower_case(index);

   config = type_to_mapping(type);
   if (!config)
      error("incorrect type");

   config[index] = value;
   id_to_type[index] = type;
   event("updated", index);
}

/* remove a config entry */
atomic void remove_config_entry(string index) {
   string type;
   mapping config;

   type = id_to_type[index];

   if (!type)
      error("incorrect type");

   config = type_to_mapping(type);
   
   if (!config)
      error("bad config");

   config[index] = nil;
   id_to_type[index] = nil;
   event("updated", index);
}

/*
 * Fake properties.
 */

mixed
query_property(string name)
{
   return query(name);
}
