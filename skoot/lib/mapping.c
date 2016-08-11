/*
**	/lib/mapping.c
**
**	Useful functions for manipulating mappings.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

private inherit "/lib/array";

mapping prefixed_map(mapping map, string prefix, varargs int prune,
		     int reverse) {
   if (reverse) {
      map = map[.. prefix] + map[prefix + "\377" ..];
      map[prefix] = nil;
   } else {
      map = map[prefix .. prefix + "\377"];
   }
   if (prune) {
      string *ix;
      mixed *val;
      int i;

      /* remove the prefix */
      ix = map_indices(map);
      val = map_values(map);
      map = ([ ]);
      for (i = 0; i < sizeof(ix); i ++) {
	 map[ix[i][strlen(prefix) ..]] = val[i];
      }
   }
   return map;
}


mapping reverse_mapping(mapping map) {
   mapping out;
   mixed *ix, *val;
   int i;

   out = ([ ]);

   ix = map_indices(map);
   val = map_values(map);
   for (i = 0; i < sizeof(ix); i ++) {
      if (out[val[i]] != nil) {
	 error("mapping is not injective");
      }
      out[val[i]] = ix[i];
   }
   return out;
}

void write_mapping_into_mapping(mapping from, mapping to) {
   mixed *ix, *val;
   int i;

   ix  = map_indices(from);
   val = map_values(from);
   for (i = 0; i < sizeof(ix); i ++) {
      to[ix[i]] = val[i];
   }
}

mixed reverse_lookup(mapping map, mixed val) {
   int ix;

   ix = index(val, map_values(map));
   if (ix >= 0) {
      return map_indices(map)[ix];
   }
   return nil;
}
