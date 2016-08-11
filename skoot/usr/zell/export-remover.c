# define DOLOG	1

private inherit "/lib/mapping";

void find() {
   call_out("check", 0, "/usr/zell/obtree"->get_obtree("ABN"));
}

void check(object *arr) {
   mapping map;
   string *ix;
   int i, j;

   map = ([ ]);

   for (i = 0; i < sizeof(arr); i ++) {
      map = prefixed_map(arr[i]->query_properties(), "export:", TRUE);
      if (!map_sizeof(map)) {
	 continue;
      }
      ix = map_indices(map);
      for (j = 0; j < sizeof(ix); j ++) {
	 arr[i]->set_property(ix[j], map[ix[j]]);
	 if (!arr[i]->query_child_count()) {
	    arr[i]->clear_property("export:" + ix[j]);
	 }
      }
   }
}
