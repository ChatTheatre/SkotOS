/*
**	Combined output and error object.
*/

private inherit "/lib/string";

# include <type.h>

mapping output_mapping; /* this is where the related objects are stored */
mapping error_mapping;
mapping actors; /* these are the people who receive the output */
mapping privmap;

mapping dobs;

static
void create(varargs int clone) {
   output_mapping = ([ ]);
   error_mapping = ([ ]);
   privmap = ([ ]);
   actors = ([ ]);
   dobs = ([ ]);
}

/* return a list of the output numbers */
string *query_output_indices() {
   return map_indices(output_mapping);
}

void smack_output(string output, varargs mixed args...) {
   if (output_mapping[output] == nil)
      output_mapping[output] = ([ ]);
   output_mapping[output]["args"] = args;
}

void new_output(string output, mixed *arr, varargs mapping args, int priv) {
   output_mapping[output] = args ? args : ([ ]);
   privmap[output] = priv;

   /* nasty lousy hack */
   if (typeof(dobs) != T_MAPPING) {
      dobs = ([ ]);
   }
   dobs[output] = arr;
}

int query_private(string output) {
   return !!privmap[output];
}

void set_output(string output, string arg, mixed related) {
   if (output_mapping[output] == nil)
      output_mapping[output] = ([ ]);
   output_mapping[output][arg] = related;
}

/* add a related object to an output */
void add_to_output(string output, string arg, mixed related) {
   if (output_mapping[output] == nil)
      output_mapping[output] = ([ ]);

   if (output_mapping[output][arg] == nil)
      output_mapping[output][arg] = ({ related });
   else
      output_mapping[output][arg] += ({ related });
}

/* return the objects related to an output */
mapping query_output_related(string output_number) {
   return output_mapping[output_number];
}

/* was this output found */
int is_output(string output) {
   return !!output_mapping[output];
}

/* query specific objects in a mapping */
object *query_output_objects(string output_number, string arg) {
   mapping args_mapping;

   args_mapping = query_output_related(output_number);

   if (args_mapping && args_mapping[arg]) {
      return args_mapping[arg];
   }

   return ({ });
}

void clear_output() {
   output_mapping = ([ ]);
}

mixed *query_dobs(string ix) { return dobs[ix]; }

/* set and query for the actors */
void set_actors(string type, mixed list) {
   actors[type] = list;
}

mixed query_actors(string type) {
   return actors[type];
}

/* return a list of the error numbers */
string *query_error_indices() {
   return map_indices(error_mapping);
}

void set_error(string error, string arg, mixed related) {
   if (error_mapping[error] == nil)
      error_mapping[error] = ([ ]);
   error_mapping[error][arg] = related;
}

/* add a related object to an error */
void add_to_error(string error, string arg, mixed related) {
   if (error_mapping[error] == nil)
      error_mapping[error] = ([ ]);

   if (error_mapping[error][arg] == nil)
      error_mapping[error][arg] = ({ related });
   else
      error_mapping[error][arg] += ({ related });
}

/* return the objects related to an error */
mapping query_related(string error_number) {
   return error_mapping[error_number];
}

/* was this error found */
int is_error(string error) {
   string *index_list;
   int index_size, i;

   index_list = map_indices(error_mapping);
   index_size = sizeof(index_list);
   
   for(i = 0; i < index_size; i++) {
      if (error == index_list[i])
	 return TRUE;
   }

   return FALSE;
}

void clear_error() {
   error_mapping = ([ ]);
}


mixed
query_property(string prop) {
   string role, action, arg;

   if (prop) {
      prop = lower_case(prop);
      if (sscanf(prop, "output:%s:%s", action, arg)) {
	 if (output_mapping[action]) {
	    return output_mapping[action][arg];
	 }
	 return nil;
      }
      if (sscanf(prop, "error:%s:%s", action, arg)) {
	 if (error_mapping[action]) {
	    return error_mapping[action][arg];
	 }
	 return nil;
      }
      error("unknown property: " + prop);
   }
   error("bad argument 1 (nil) to query_property");
}

void set_property(string prop, mixed val, varargs int opaque) {
   string action, arg, *bits;

   if (prop) {
      bits = explode(prop, ":");
      if (sizeof(bits) >= 3) {
	 action = implode(bits[0 .. sizeof(bits)-2], ":");
	 arg = bits[sizeof(bits)-1];
	 switch(lower_case(bits[0])) {
	 case "output": case "duelling": case "desc":
	    if (arg == "dobs") {
	       dobs[action] = val;
	       return;
	    }
	    if (!output_mapping[action]) {
	       output_mapping[action] = ([ ]);
	    }
	    output_mapping[action][arg] = val;
	    return;
	 case "error":
	    if (!error_mapping[action]) {
	       error_mapping[action] = ([ ]);
	    }
	    error_mapping[action][arg] = val;
	    return;
	 }
      }
      error("unknown property: " + prop);
   }
   error("bad argument 1 (nil) to set_property");
}
