/*
 * ~UserAPI/sys/searchdb.c
 *
 * Cross-referencing some data, to make searching this data easier, while
 * trying to keep it all as efficient as sanely possible.
 *
 * Copyright Skotos Tech Inc 2004.
 */

private inherit "/lib/bigmap";
private inherit "/lib/string";

mapping database;

static void create()
{
   database = ([ ]);
   database["ipdata"] = ([ ]);
}

void patch()
{
   create();
}

void add_reference(string field, string value, object udat)
{
   mapping map, data;

   if (!database[field]) {
      error("Unknown database field");
   }
   map = database[field];

   data = get_by_str(map, value);
   if (!data) {
      data = ([ ]);
      set_by_str(map, value, data);
   }
   if (data[udat] == nil) {
      data[udat] = 1;
   } else {
      data[udat]++;
   }
}

void remove_reference(string field, string value, object udat)
{
   mapping map, data;

   if (!database[field]) {
      error("Unknown database field");
   }
   map = database[field];

   data = get_by_str(map, value);
   if (!data) {
      return;
   }
   if (!data[udat]) {
      return;
   }
   data[udat]--;
   if (data[udat] == 0) {
      data[udat] = nil;
      if (map_sizeof(data) == 0) {
	 set_by_str(map, value, nil);
      }
   }
}

object *query_references(string field, string value)
{
   mapping map, data;

   if (!database[field]) {
      error("Unknown database field");
   }
   map = database[field];

   data = get_by_str(map, value);
   if (!data) {
      return ({ });
   }
   return map_indices(data);
}

mapping query_database(string field)
{
   return database[field];
}

mapping query_matches(string field, string pattern)
{
   int i, sz_i, match;
   string *pre;
   object *dats;
   mapping map, *data, results, found;
   mixed **stamps;

   if (!database[field]) {
      error("Unknown database field");
   }
   map = database[field];

   if (pattern) {
       string *bits;
       pattern = implode(explode("%" + lower_case(pattern) + "%", "%"), "%%");
       bits = explode("*" + pattern + "*", "*");
       if (sizeof(bits) == 1) {
	  pattern = "%*s" + pattern;
	  match = 1;
       } else {
	  pattern = implode(bits, "%*s");
	  match = sizeof(bits) - 1;
       }
   }

   found = ([ ]);

   sz_i = map_sizeof(map);
   pre = map_indices(map);
   data = map_values(map);
   for (i = 0; i < sz_i; i++) {
      int j, sz_j;
      string *post;
      mapping *udats;

      sz_j = map_sizeof(data[i]);
      post = map_indices(data[i]);
      udats = map_values(data[i]);

      for (j = 0; j < sz_j; j++) {
	 string value;

	 value = pre[i] + post[j];
	 if (sscanf(value, pattern) == match) {
	    int k, sz_k;
	    object *list;

	    sz_k = map_sizeof(udats[j]);
	    list = map_indices(udats[j]);
	    for (k = 0; k < sz_k; k++) {
	       int stamp;
	       object dat;

	       dat = list[k];
	       stamp = dat->query_last_event(value);
	       if (found[dat] == nil || stamp > found[dat][0]) {
		  found[dat] = ({ stamp, value });
	       }
	    }
	 }
      }
   }

   sz_i = map_sizeof(found);
   dats = map_indices(found);
   stamps = map_values(found);
   found = ([ ]);
   for (i = 0; i < sz_i; i++) {
      found[dats[i]->query_name()] = stamps[i];
   }
   return found;
}
