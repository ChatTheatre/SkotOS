/*
**	/usr/CSD/lib/stats.c
**
**     	This code handles the stats of a character
**
**	Copyright Skotos Tech Inc 2001
*/

# include <nref.h>
# include <base/living.h>

private inherit "/lib/properties";

mapping stat_table;		/* table of stats */
mapping stat_types;		/* table of stat types */

static
void create() {
   stat_table = ([ ]);
   stat_types = ([ ]);

   return;
}

void clear_all_stats() {
   stat_table = ([ ]);
   stat_types = ([ ]);
}

int is_derived_stat(string stat) {
   if (stat_types[stat] == "DERIVED")
      return TRUE;
   
   return FALSE;
}

int is_absolute_stat(string stat) {
   if (stat_types[stat] == "ABSOLUTE")
      return TRUE;
   
   return FALSE;
}

/* stat table */
string *query_stat_list() {
   return map_indices(stat_table);
}

string query_stat_type(string stat) {
   return stat_types[stat];
}

mapping query_stat_table() {
   return stat_table;
}

mapping query_stat_types() {
   return stat_types;
}

void remove_stat(string stat) {
   stat_table[stat] = nil;
   stat_types[stat] = nil;
}

void set_stat(string stat, string type) {
   stat_table[stat] = ([ ]);
   stat_types[stat] = type;
}

void new_stat(string stat, int absolute) {
   SysLog("Creating new stat: " + stat);
   if (absolute) {
      stat_table[stat] = ([ ]);
      stat_types[stat] = "ABSOLUTE";
   } else {
      stat_table[stat] = ([ ]);
      stat_types[stat] = "DERIVED";
   }
}

void add_to_derived_stat(string stat, string base, float portion) {
   mapping derived;
   int i;
   float total, *values;

   if (!stat_table[stat])
      new_stat(stat, FALSE);

   if (!is_absolute_stat(base))
      error("derived stats must be derived from absolute stats only. stat: " + stat + " base: " + base);

   derived = stat_table[stat];
   values = map_values(derived);

   for (i = 0; i < sizeof(values); i++) {
      total += values[i];
   }

   if (total + portion > 1.0)
      error("derived stat portions must not be greater than 1.0");

   derived[base] = portion;
}

void new_derived_stat(string stat) {
   string base, *list;
   int i;

   if (!is_derived_stat(stat))
      error("stat: " + stat + " is not derived, it is absolute");

   list = query_stat_list();

   for (i = 0; i < sizeof(list); i++) {
      if (stat_types[list[i]] == "ABSOLUTE") 
	 base = list[i];
   }

   if (stat_types[base] != "ABSOLUTE")
      error("there are no absolute stats to prime with, add one");

   add_to_derived_stat(stat, base, 0.0);
}

void remove_from_derived_stat(string stat, string base) {
   mapping derived;

   if (!stat_table[stat])
      return;

   derived = stat_table[stat];
   
   derived[base] = nil;
}

float query_derived_stat_portion(string stat, string base) {
   mapping derived;

   if (!stat_table[stat])
      return 0.0;
 
   derived = stat_table[stat];
  
   return derived[base];
}

string *query_derived_stat_list(string stat) {
   mapping derived;

   if (!stat_table[stat])
      return nil;

   derived = stat_table[stat];

   return map_indices(derived);
}

/* this is assumed to be safe */
mapping query_derived_stat(string stat) {
   return stat_table[stat];
}

/* statistic query functions */
float query_stat(object ob, string stat) {
   mapping derived;
   string *ab_stats;
   float total;
   int i;

   /* is this stat absolute? then lets shuffle it along */
   if (is_absolute_stat(stat)) {
      if (!ob->query_property("base:stat:" + stat)) {
	 ob->set_property(stat, 100.0);
	 return 100.0;
      }

      return ob->query_property("base:stat:" + stat);
   }

   /* maybe its derived */
   if (is_derived_stat(stat)) {
      derived = query_derived_stat(stat);
      ab_stats = map_indices(derived);

      for (i = 0; i < sizeof(ab_stats); i++) {
	 total += query_stat(ob, ab_stats[i]) * derived[ab_stats[i]];
      }

      return total;
   }

   /* not found, assume 0 */
   return 0.0;
      
}

