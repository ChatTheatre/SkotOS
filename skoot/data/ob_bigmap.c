/*
**	This LWO maintains a two-level mapping indexed by objects.
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

mixed get(object ob) {
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
void set(object ob, mixed val) {
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
void clear(object ob) {
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
