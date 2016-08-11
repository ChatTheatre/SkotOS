/*
**      Output Object
**
**	This object is passed back and forth between the act and cmd layers.
**	The object is a mapping wrapped in functions to access/manipulate the
**      mapping. The indices for the mapping are output numbers and the values
**	are objects related to the output.
**
**	Skotos Tech Inc.
**	Copyright 2000
**	
**	Initial Revision: Wes Connell
**
*/

mapping output_mapping; /* this is where the related objects are stored */
mapping actors; /* these are the people who receive the output */

static
void create() {
   output_mapping = ([ ]);
   actors = ([ ]);
}

/* return a list of the output numbers */
string *query_output_indices() {
   return map_indices(output_mapping);
}

void set_output(string output, string arg, mixed related) {

   if (output_mapping[output] == nil)
      output_mapping[output] = ([ ]);

   if (!arg)
      output_mapping[output]["NoArgument"] = ({ });
   else
      output_mapping[output][arg] = related;
}

/* add a related object to an output */
void add_to_output(string output, string arg, mixed related) {
   string tmp_arg;

   if (output_mapping[output] == nil)
      output_mapping[output] = ([ ]);

   if (!arg)
      tmp_arg = "NoArgument";
   else
      tmp_arg = arg;

   if (output_mapping[output][tmp_arg] == nil)
      output_mapping[output][tmp_arg] = ({ related });
   else
      output_mapping[output][tmp_arg] += ({ related });
}

/* return the objects related to an output */
mapping query_output_related(string output_number) {
   return output_mapping[output_number];
}

/* was this output found */
int is_output(string output) {
   string *index_list;
   int index_size, i;

   index_list = map_indices(output_mapping);
   index_size = sizeof(index_list);
   
   for(i = 0; i < index_size; i++) {
      if (output == index_list[i])
	 return TRUE;
   }

   return FALSE;
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

/* cheap hack to makes output messages look nice */
string query_output_prep(string output_number, string arg) {
   mapping args_mapping;

   args_mapping = query_output_related(output_number);

   if (sizeof(args_mapping[arg]) > 1)
      return "are";
   else
      return "is";
}

void clear_output() {
   output_mapping = ([ ]);
}

/* set and query for the actors */
void set_actors(string type, mixed list) {
   actors[type] = list;
}

mixed query_actors(string type) {
   return actors[type];
}



