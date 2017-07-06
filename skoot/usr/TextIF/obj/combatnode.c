/*
**	Copyright Skotos Tech Inc 1999
*/

# include <SAM.h>
# include <TextIF.h>

inherit "/lib/womble";

string name;	/* Name of this object. Its X axis name. */

/* The innards of the y mapping are still rolling in my head. */
mapping y;	/* The data of the Y axis. */

void womble_innards() {
   y = womble(y);
}

void set_entry(string index, SAM desc);
string query_state_root() { return "TextIF:CombatNode"; }

static
void create(int clone) {
   if (!clone) {
      set_object_name("TextIF:CombatNode");
   }
   y = ([ ]);
   set_entry("UNUSED", "This entry is simply a place holder to prime the" +
	     "mapping.");
}

void clear() {
   y = ([ ]);
}

void set_name(string value) {
   if (sscanf(value, "%*s:")) {
      error("name may not contain a colon");
   }
   name = value;
   set_object_name("TextIF:CombatNodes:" + name);
   COMBATDB->register_node(this_object());
}

void patch() {
   COMBATDB->register_node(this_object());
}

string query_name() {
   if (!name) {
      return "NONAME";
   }

   XDebug("query_name() name: " + dump_value(name));
   return name;
}

atomic
void del_entry(string index) {
   XDebug("Deleting entry: " + dump_value(index));

   y[index] = nil;
   if (y[index]) {
      XDebug("WTF!!!");
   }
   XDebug("y indices: " + dump_value(map_indices(y)));
   if (!map_sizeof(y)) {
      call_out("maybe_suicide", 0);
   }
   SysLog("Combat message entry " + name + ":" + index + " was deleted.");
}

static
void maybe_suicide() {
   if (!map_sizeof(y)) {
      destruct_object();
   }
}

void die() {
   destruct_object();
}

/* I'm making this atomic just incase I add anything else to it. */
atomic
void set_entry(string index, SAM desc) {
   y[index] = desc;
}

SAM query_entry(string index) {
   return y[index];
}

string *query_entry_names() {
   return map_indices(y);
}
