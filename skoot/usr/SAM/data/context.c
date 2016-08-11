/*
**	/usr/SAM/obj/unsam.c
**
**	Just make instantiable context.
**
**	Copyright Skotos Tech Inc 1999
**
*/

inherit context	"/usr/SAM/lib/context";

static
void create(int clone) {
   if (clone) {
      context::create();
   }
}
