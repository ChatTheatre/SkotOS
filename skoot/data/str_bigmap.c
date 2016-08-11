/*
**	This LWO maintains a two-level mapping indexed by strings.
**
**	Copyright Skotos Tech Inc 2003
*/

# include <status.h>

mapping bigmap;

static
void create(int clone) {
   if (clone) {
      bigmap = ([ ]);
   }
}

mapping query_bigmap() { return bigmap; }

/* operations where the index is an object */

# define PREFIX_SIZE	3	/* never change this on a running game */

mixed get(string str) {
   mapping map;
   string prefix;

   if (strlen(str) < PREFIX_SIZE) {
      prefix = "";
   } else {
      prefix = str[.. PREFIX_SIZE - 1];
      str = str[PREFIX_SIZE ..];
   }

   map = bigmap[prefix];
   if (map == nil) {
      return nil;
   }
   return map[str];
}

atomic
void set(string str, mixed val) {
   mapping map;
   string prefix;

   if (strlen(str) < PREFIX_SIZE) {
      prefix = "";
   } else {
      prefix = str[.. PREFIX_SIZE - 1];
      str = str[PREFIX_SIZE ..];
   }

   map = bigmap[prefix];
   if (map == nil) {
      map = bigmap[prefix] = ([ ]);
   }
   map[str] = val;
}

atomic
void clear(string str) {
   mapping map;
   string prefix;

   if (strlen(str) < PREFIX_SIZE) {
      prefix = "";
   } else {
      prefix = str[.. PREFIX_SIZE - 1];
      str = str[PREFIX_SIZE ..];
   }

   map = bigmap[prefix];
   if (map == nil) {
      return;
   }
   map[str] = nil;
}


