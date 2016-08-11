/*
**	/base/sys/crafting.c
**
**	Storage of craftable items.
**
**	Copyright Skotos Tech Inc 1999-2000
*/

# include <base.h>

private inherit "/lib/string";

mapping i_to_o;		/* sets of craftables that require ingredient i */
mapping t_to_o;		/* sets of craftables that require tool t */
mapping o;		/* set of all craftables */


string query_state_root() { return "Base:CraftDaemon"; }

static atomic
void create() {
   set_object_name("Base:CraftDaemon");
   i_to_o = ([ ]);
   t_to_o = ([ ]);
   o = ([ ]);
}


mapping query_i_to_o() { return i_to_o; }
mapping query_t_to_o() { return t_to_o; }

object *query_objects() {
   return map_indices(o);
}

string *query_tools() {
   return map_indices(t_to_o);
}

string *query_ingredients() {
   return map_indices(i_to_o);
}

object *query_objects_needing_ingredient(string i) {
   mapping map;

   if (map = i_to_o[lower_case(i)]) {
      return map_indices(map);
   }
   return ({ });
}

object *query_objects_needing_tool(string t) {
   mapping map;

   if (map = t_to_o[lower_case(t)]) {
      return map_indices(map);
   }
   return ({ });
}


atomic
void clear(object obj) {
   string *arr;
   mapping map;
   int i;

   if (previous_program() == BASE_LIB_CRAFTING) {
      arr = obj->query_ingredients();

      for (i = 0; i < sizeof(arr); i ++) {
	 if (map = i_to_o[arr[i]]) {
	    map[obj] = nil;
	 }
      }

      arr = obj->query_tools();

      for (i = 0; i < sizeof(arr); i ++) {
	 if (map = t_to_o[arr[i]]) {
	    map[obj] = nil;
	 }
      }
      o[obj] = nil;
   }
}

atomic
void add_ingredient(object obj, string i) {
   mapping map;

   if (previous_program() == BASE_LIB_CRAFTING) {
      map = i_to_o[i];
      if (!map) {
	 i_to_o[i] = map = ([ ]);
      }
      map[obj] = obj;
      o[obj] = obj;
   }
}

atomic
void add_tool(object obj, string t) {
   mapping map;

   if (previous_program() == BASE_LIB_CRAFTING) {
      map = t_to_o[t];
      if (!map) {
	 t_to_o[t] = map = ([ ]);
      }
      map[obj] = obj;
      o[obj] = obj;
   }
}

