/*
**	/usr/SAM/obj/unsam.c
**
**	Convenient class doing double duty as data repository
**	and SAM context...
**
**	Copyright Skotos Tech Inc 1999
**
*/

inherit data	"/lib/data";
inherit context	"/usr/SAM/lib/context";

static
void create(int clone) {
   if (clone) {
      data::clear();
      context::create();
   }
}
