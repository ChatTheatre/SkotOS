/*
**	/obj/data.c
**
**	Inherit /lib/data.c and be clonable.
**
**	Copyright Skotos Tech Inc 1999
**
*/

inherit "/lib/data";

static
void create(int clone) {
   if (clone) {
      clear();
      /* call_out("destroy", 0); TODO */
   }
}

/* unless explicitly frozen, these do not live long past end of thread */

void destroy() {
   if (query_frozen()) {
      error("data object clone is frozen");
   }
   destruct_object();
}
