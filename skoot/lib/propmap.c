/*
**      /lib/propmap.c
**
**	We talk of propmaps and structmaps, which represent the same
**	data in two formats:
**
**      A) A mapping where the indices are simple identifier style
**      strings and values can be of any type, but a mapping value
**      is treated as a recursive sub-propset. For example,
**
**          ([ "staff":   ([ "zell": ([ "state": "WI", "gender": 1 ]),
**                           "nino": ([ "state": "KS", "gender": 1 ]) ]),
**             "players": ([ "xena": ([ "state": "OH", "gender": 2 ]) ]) ])
**
**	Such mappings are very convenient in script code: they are
**      simple to recurse into, they are simple to navigate in SAM
**	(e.g. map.staff.nino.state) and they are easily enumerated
**	with map_indices() and map_values(). This is a structmap.
**
**	B) A flat mapping with colon-delimited string indices, like
**	the kind we use for properties. In this format, the data we
**	used above reads as:
**
**	    staff:zell:state    = "WI"
**	    staff:zell:gender   = 1
**	    staff:nino:state    = "KS"
**	    staff:nino:gender   = 1
**	    players:peon:state  = "TX"
**	    players:peon:gender = 2
**
**	This format is difficult to work with in scripts because its
**	structure is not exposed, but it is intuitive to humans and
**	easy to read and edit. This is a propmap.
**
**	030725	Zell		Initial Revision
**
**	Copyright Skotos Tech Inc 2003
*/

# include <type.h>

static
mapping flatten_structmap(mapping structmap, varargs string prefix,
			  mapping out) {
   string *ix, str;
   mixed *val;
   int i;

   if (!out) {
      out = ([ ]);
   }

   ix  = map_indices(structmap);
   val = map_values(structmap);

   for (i = 0; i < sizeof(ix); i ++) {
      str = prefix ? prefix + ":" + ix[i] : ix[i];

      if (typeof(val[i]) == T_MAPPING) {
	 flatten_structmap(val[i], str, out);
	 continue;
      }
      out[str] = val[i];
   }
   return out;
}

static
mapping structure_propmap(mapping propmap) {
   mapping out, map;
   string *ix, *bits;
   mixed newmap, *val;
   int i, j;

   out = ([ ]);

   ix  = map_indices(propmap);
   val = map_values(propmap);

   for (i = 0; i < sizeof(ix); i ++) {
      bits = explode(ix[i], ":");

      for (map = out, j = 0; j < sizeof(bits)-1; j ++) {
	 newmap = map[bits[j]];
	 if (typeof(newmap) != T_MAPPING) {
	    newmap = map[bits[j]] = ([ ]);
	 }
	 map = newmap;
      }
      map[bits[j]] = val[i];
   }
   return out;
}
