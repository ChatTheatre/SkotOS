/*
**	/usr/SkotOS/lib/traversal.c
**
**	Helpful routines for doing pathfinding and similar room-
**	to-room traversals of exit graphs.
**
**	Copyright Skotos Tech Inc 2000
*/

# include <nref.h>

inherit "/base/lib/urcalls";

string *find_path(object from, object to) {
}

object *find_area(object around) {
   mapping seen;
   string *details;
   object dest, *queue;
   NRef ref;
   int i;

   queue = ({ around });
   seen = ([ around : TRUE ]);

   while (sizeof(queue)) {
      details = ur_details(queue[0]);
      for (i = 0; i < sizeof(details); i ++) {
	 if (IsNRef(ref = ur_exit_dest(queue[0], details[i]))) {
	    if (dest = NRefOb(ref)) {
	       if (!seen[dest]) {
		  seen[dest] = TRUE;
		  queue += ({ dest });
	       }
	    }
	 }
      }
      queue = queue[1 ..];
   }
   return map_indices(seen);
}
