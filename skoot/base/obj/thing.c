/*
**	/base/obj/thing.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>

inherit LIB_THING;

static
void create(int clone) {
   ::create();

   if (!clone) {
      set_object_name("Base:Thing");
   }
}

