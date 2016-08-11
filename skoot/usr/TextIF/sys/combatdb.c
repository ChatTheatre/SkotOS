/*
**      Combat message database
**
**	This object stands alone in limbo and answers requests for combat
**	message.
**
**	Skotos Tech Inc.
**	Copyright 2001
**	
**	Initial Revision: Wes Connell
**
*/

inherit "/lib/string";

# include <SAM.h>
# include <XML.h>
# include "/usr/SAM/include/sam.h"

mapping x;	/* List of X axis objects. */

void create_node(string name);
string query_state_root() { return "TextIF:CombatDB"; }

static
void create() {
   set_object_name("TextIF:CombatDB");
   compile_object("~TextIF/obj/combatnode");

   x = ([ ]);
}

/* clear all this crap */
void clear() {
   x = ([ ]);
}   

atomic
void register_node(object ob) {
   x[ob->query_name()] = ob;
}

object query_node(string name) {
   return x[name];
}

string *query_node_names() {
   return map_indices(x);
}
