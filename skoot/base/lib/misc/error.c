/*
**      Error Object
**
**	This object is passed back and forth between the act and cmd layers.
**	The object is a mapping wrapped in functions to access/manipulate the
**      mapping. The indices for the mapping are error numbers and the values
**	are objects related to the error.
**
**	Skotos Tech Inc.
**	Copyright 2000
**	
**	Initial Revision: Wes Connell
**
*/

mapping error_mapping;

static
void create() {
   error_mapping = ([ ]);
}

/* return a list of the error numbers */
string *query_error_indices() {
   return map_indices(error_mapping);
}

void set_error(string error, string arg, mixed related) {

   if (error_mapping[error] == nil)
      error_mapping[error] = ([ ]);

   if (!arg)
      error_mapping[error]["NoArgument"] = ({ });
   else
      error_mapping[error][arg] = related;
}

/* add a related object to an error */
void add_to_error(string error, string arg, mixed related) {
   string tmp_arg;

   if (error_mapping[error] == nil)
      error_mapping[error] = ([ ]);

   if (!arg)
      tmp_arg = "NoArgument";
   else
      tmp_arg = arg;

   if (error_mapping[error][tmp_arg] == nil)
      error_mapping[error][tmp_arg] = ({ related });
   else
      error_mapping[error][tmp_arg] += ({ related });
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

/* cheap hack to makes error messages look nice */
string query_error_prep(string error_number, string arg) {
   mapping args_mapping;

   args_mapping = query_related(error_number);

   if (sizeof(args_mapping[arg]) > 1)
      return "are";
   else
      return "is";
}

void clear_error() {
   error_mapping = ([ ]);
}






