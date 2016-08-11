/*
**	This program is meant to help index a two-level mapping
**	more easily, i.e. one where you need to store many more
**	indices than DGD's mapping size limit.
*/

# include <status.h>

/* operations where the index is an integer */

mixed get_by_int(mapping bigmap, int ix) {
   mapping map;

   map = bigmap[ix / 1024];
   if (map == nil) {
      return nil;
   }
   return map[ix];
}

atomic
void set_by_int(mapping bigmap, int ix, mixed val) {
   mapping map;

   map = bigmap[ix / 1024];
   if (map == nil) {
      map = bigmap[ix / 1024] = ([ ]);
   }
   map[ix] = val;
}

atomic
void clear_by_int(mapping bigmap, int ix) {
   mapping map;

   map = bigmap[ix / 1024];
   if (map == nil) {
      return;
   }
   map[ix] = nil;
}


/* operations where the index is an object */

mixed get_by_ob(mapping bigmap, object ob) {
   mapping map;
   int ix;

   /* this is a little slow, but not horribly */
   if (!sscanf(ur_name(ob), "%*s#%d", ix)) {
      /* this is a little slow too */
      ix = status(ob)[O_INDEX];
   }
   map = bigmap[ix / 1024];
   if (map == nil) {
      return nil;
   }
   return map[ob];
}

atomic
void set_by_ob(mapping bigmap, object ob, mixed val) {
   mapping map;
   int ix;

   /* this is a little slow, but not horribly */
   if (!sscanf(ur_name(ob), "%*s#%d", ix)) {
      /* this is a little slow too */
      ix = status(ob)[O_INDEX];
   }
   map = bigmap[ix / 1024];
   if (map == nil) {
      map = bigmap[ix / 1024] = ([ ]);
   }
   map[ob] = val;
}

atomic
void clear_by_ob(mapping bigmap, object ob) {
   mapping map;
   int ix;

   /* this is a little slow, but not horribly */
   if (!sscanf(ur_name(ob), "%*s#%d", ix)) {
      /* this is a little slow too */
      ix = status(ob)[O_INDEX];
   }
   map = bigmap[ix / 1024];
   if (map == nil) {
      return;
   }
   map[ob] = nil;
}


/* operations where the index is a string */

# define PREFIX_SIZE	2	/* never change this on a running game */

mixed get_by_str(mapping bigmap, string str) {
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
void set_by_str(mapping bigmap, string str, mixed val) {
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
void clear_by_str(mapping bigmap, string str) {
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


