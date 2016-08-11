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
   }
}
