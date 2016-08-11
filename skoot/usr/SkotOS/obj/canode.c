/*
**	~SkotOS/obj/canode.c
**
**	Be a clonable ~SkotOS/lib/canode.c
**
**	(C) Copyright Skotos Tech Inc 2002
*/

inherit "/usr/SkotOS/lib/canode";

static
void create(int clone) {
   ::create(clone);
   if (!clone) {
      set_object_name("SkotOS:ClonableCANode");
   }
}
