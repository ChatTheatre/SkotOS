/*
**	We just exit as a more or less empty return value.
*/

string ref;

static
void configure(string str) {
   if (!str) {
      error("badobjref must have a reference");
   }
   ref = str;
}

void _F_configure(string str) {
   if (sscanf(previous_program(), "/usr/System/%*s")) {
      configure(str);
   }
}

string query_ref() { return ref; }
