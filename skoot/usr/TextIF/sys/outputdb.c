/*
**      Output Database
**
**	This object stands alone in limbo and answers requests for output
**	numbers.
**
**	Skotos Tech Inc.
**	Copyright 2000
**	
**	Initial Revision: Wes Connell
**
*/

inherit "/lib/womble";

private inherit "/lib/string";

# include <SAM.h>
# include <XML.h>
# include "/usr/SAM/include/sam.h"

/* we use a mapping for each form of output */
mapping output_msgs_first;
mapping output_msgs_second;
mapping output_msgs_third;
mapping lc2cs;	/* map lowercased version to case-sensitive version */

void womble_messages() {
   output_msgs_first  = womble(output_msgs_first);
   output_msgs_second = womble(output_msgs_second);
   output_msgs_third  = womble(output_msgs_third);
}

string query_state_root() { return "TextIF:OutputDB"; }

static
void create() {
   set_object_name("TextIF:OutputDB");
   output_msgs_first = ([ ]);
   output_msgs_second = ([ ]);
   output_msgs_third = ([ ]);
}

void patch() {
   string *entries;
   int i;

   lc2cs = ([ ]);

   entries = map_indices(output_msgs_first) |
      map_indices(output_msgs_second) |
      map_indices(output_msgs_third);

   for (i = 0; i < sizeof(entries); i ++) {
      lc2cs[lower_case(entries[i])] = entries[i];
   }
}

/* clear all this crap */
void clear_output_dbs() {
   output_msgs_first = ([ ]);
   output_msgs_second = ([ ]);
   output_msgs_third = ([ ]);
}   

/* return a list of the output numbers */
string *query_output_msg_names_first() {
   return map_indices(output_msgs_first);
}

string *query_output_msg_names_second() {
   return map_indices(output_msgs_second);
}

string *query_output_msg_names_third() {
   return map_indices(output_msgs_third);
}

/* create a new entry */
void create_output_msg_first(string name) {
   output_msgs_first[name] = "EMPTY";
}

void create_output_msg_second(string name) {
   output_msgs_second[name] = "EMPTY";
}

void create_output_msg_third(string name) {
   output_msgs_third[name] = "EMPTY";
}

/* set the output message */
void set_output_msg_first(string name, SAM message) {
   XDebug("Setting first person message '" + name + "'.");
   output_msgs_first[name] = message;
}

void set_output_msg_second(string name, SAM message) {
   XDebug("Setting second person message '" + name + "'.");
   output_msgs_second[name] = message;
}

void set_output_msg_third(string name, SAM message) {
   XDebug("Setting third person message '" + name + "'.");
   output_msgs_third[name] = message;
}

/* return the wonderful SAM for this type of output */
SAM query_output_msg_first(string name) {
   return output_msgs_first[lc2cs[lower_case(name)]];
}

SAM query_output_msg_second(string name) {
   return output_msgs_second[lc2cs[lower_case(name)]];
}

SAM query_output_msg_third(string name) {
   return output_msgs_third[lc2cs[lower_case(name)]];
}

/* delete this entry */
void del_output_msg_first(string name) {
   output_msgs_first[lc2cs[lower_case(name)]] = nil;
}

void del_output_msg_second(string name) {
   output_msgs_second[lc2cs[lower_case(name)]] = nil;
}

void del_output_msg_third(string name) {
   output_msgs_third[lc2cs[lower_case(name)]] = nil;
}
