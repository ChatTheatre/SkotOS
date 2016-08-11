/*
**	/base/lib/act/virtual.c
**
**	Implement functions for the virtual-room stuff.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/toolbox";

# include <nref.h>
# include "/base/include/motion.h"

static
int open_virtual_door(NRef door) {
   object door_ob, dest_ob, home;
   string door_det, virtual_id;
   NRef dest;

   door_ob  = door->query_object();
   door_det = door->query_detail(this_object());

   if (dest = door_ob->query_exit_dest(door_det)) {
      dest_ob  = NRefOb(dest);

      if (virtual_id = dest_ob->query_property("IsVirtual")) {
	 object owner;

	 /* first, are we already in a virtual room? */
	 owner = door_ob->query_property("HomeOwner");
	 if (!owner) {
	    owner = this_object();
	 }

	 if (home = owner->query_property("VirtualHome:" + virtual_id)) {
	    if (dest_ob != home->query_ur_object()) {
	       return FALSE;
	    }
	 } else {
	    /* we need to create a home? */
	    home = spawn_thing(NRefOb(dest));
	    owner->set_property("VirtualHome:" + virtual_id, home);
	    home->set_property("HomeOwner", owner);
	 }
      }
      /* register our virtual room as the current one */
      dest_ob->set_property("VirtualCurrent", home);
   }
   return TRUE;
}

static
object enter_virtual_room(NRef dest) {
   object dest_ob, home;
   string virtual_id;

   dest_ob = NRefOb(dest);

   if (virtual_id = dest_ob->query_property("IsVirtual")) {
      object owner;

      /* are we already in a virtual room? */
      if (owner = query_environment()->query_property("HomeOwner")) {
	 /* if so, extend current room's owner's virtual space */
	 home = owner->query_property("VirtualHome:" + virtual_id);
	 if (!home) {
	    /* create the room in owner's name */
	    home = spawn_thing(dest_ob);
	    owner->set_property("VirtualHome:" + virtual_id, home);
	    home->set_property("HomeOwner", owner);
	 }
	 return home;
      }
      home = dest_ob->query_property("VirtualCurrent");
      if (!home) {
	 /* this should not really happen; if it does, go to the ur */
	 home = dest_ob;
      }
      return home;
   }
   return nil;
}
