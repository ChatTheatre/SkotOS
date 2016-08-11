inherit "/lib/url";
inherit "/usr/SAM/lib/sam";

inherit "~/lib/bilbo";
inherit "~/lib/merry";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args)
{
   int     i, sz;
   object  udat, obj, *list;
   string  obname;
   mapping map, *maps;

   udat = context->query_udat();
   if (!udat) {
      return TRUE;
   }

   args += local;
   obname = args["obj"];
   obj = find_object(obname);
   if (!obj) {
      return TRUE;
   }
   map = find_bilbo_scripts(obj);
   sz = map_sizeof(map);
   list = map_indices(map);
   maps = map_values(map);
   for (i = 0; i < sz; i++) {
      int j, sz_j;
      string *names;

      sz_j = map_sizeof(maps[i]);
      names = map_indices(maps[i]);
      for (j = 0; j < sz_j; j++) {
	 Output("SET Bilbo:" + names[j] +
		" /Tool/Bilbo.sam?" +
		"obj="    + url_encode(name(list[i])) + "&" +
		"script=" + url_encode(names[j]) + "\r\n");
      }
   }
   map = find_merry_scripts(obj);
   sz = map_sizeof(map);
   list = map_indices(map);
   maps = map_values(map);
   for (i = 0; i < sz; i++) {
      int j, sz_j;
      string *names;

      sz_j = map_sizeof(maps[i]);
      names = map_indices(maps[i]);
      for (j = 0; j < sz_j; j++) {
	 Output("SET Merry:" + names[j] +
		" /Tool/Merry.sam?" +
		"obj="    + url_encode(name(list[i])) + "&" +
		"script=" + url_encode(names[j]) + "\r\n");
      }
   }
   Output("SYNC\r\n");
   return TRUE;
}
