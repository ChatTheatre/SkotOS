/*
**      Error Database
**
**	This object stands alone in limbo and answers requests for error
**	numbers.
**
**	Skotos Tech Inc.
**	Copyright 2000
**	
**	Initial Revision: Wes Connell
**
*/

inherit "/lib/womble";

# include <SAM.h>
# include <XML.h>
# include "/usr/SAM/include/sam.h"

mapping errors;

void womble_errors() {
   errors = womble(errors);
}

string query_state_root() { return "TextIF:ErrorDB"; }

static
void create() {
   set_object_name("TextIF:ErrorDB");
   errors = ([ ]);
}

/* return a list of the error numbers */
string *query_error_names() {
   return map_indices(errors);
}

void set_error(string name, SAM message) {
   errors[name] = message;
}

/* return the objects related to an error */
SAM query_error(string name) {
   return errors[name];
}

