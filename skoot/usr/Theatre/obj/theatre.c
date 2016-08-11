/*
**      /usr/SkotOS/obj/chattreatre.c
**
**      Copyright Skotos Tech Inc 2000
*/

inherit "../lib/theatre";

static
void create(int clone) {
  ::create();
   if (!clone) {
     set_object_name("Theatre:TheatreNode");
   }
}
