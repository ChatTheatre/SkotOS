# define DOLOG	1

private inherit "/base/lib/urcalls";

void find() {
   SysLog("Getting object tree...");
   call_out("check", 0, "/usr/zell/obtree"->get_obtree(""));
}

void check(object *arr) {
   mapping map;
   string *dets, *inherited, first;
   object ur;
   int i, j, icount, lcount;

   SysLog("Checking details...");

   map = ([ ]);

   for (i = 0; i < sizeof(arr); i ++) {
      dets = arr[i]->query_furry_details();
      if (!dets) {
	 continue;
      }
      if (ur = arr[i]->query_ur_object()) {
	 inherited = ur_details(ur);
	 if (!inherited) {
	    SysLog("What the HELL: " + name(arr[i]) + " / " + name(ur));
	    continue;
	 }
	 inherited &= dets;
	 for (j = 0; j < sizeof(inherited); j ++) {
	    if (!sizeof(ur_description_types(arr[i], inherited[j]))) {
	       inherited[j] = nil;
	    }
	 }
	 inherited -= ({ nil });
	 if (sizeof(inherited)) {
# if DOLOG
	    SysLog(name(arr[i]) + ": NewInherited -> " + implode(inherited, ", "));
# else
	    first = "I:" + explode(name(arr[i]), ":")[0];
	    if (!map[first]) {
	       map[first] = 0;
	    }
	    map[first] += sizeof(inherited);
	    if (!sscanf(name(arr[i]), "ABN:%*s")) {
	       icount += sizeof(inherited);
	    }
# endif
	 }
	 dets -= inherited;
      }
      if (sizeof(dets)) {
# if DOLOG
	 SysLog(name(arr[i]) + ": Local -> " + implode(dets, ", "));
# else
	    first = "L:" + explode(name(arr[i]), ":")[0];
	    if (!map[first]) {
	       map[first] = 0;
	    }
	    map[first] += sizeof(dets);
	    if (!sscanf(name(arr[i]), "ABN:%*s")) {
	       icount += sizeof(dets);
	    }
	 lcount += sizeof(dets);
# endif
	 for (j = 0; j < sizeof(dets); j ++) {
	    arr[i]->remove_detail(dets[j]);
	 }
      }
   }
   if (!DOLOG) {
      SysLog("INHERITED: " + icount + "; LOCAL: " + lcount);
      SysLog("MAP: " + dump_value(map));
   }
}
