/*
**	/usr/DevSys/lib/karstate.c
**
**	We brutally interrogate a physical thing to generate an
**	easily read description of detail descriptions.
**
**	Copyright Skotos Tech Inc 1999
*/

inherit thing "/usr/DevSys/lib/karthing";
inherit tip "/usr/DevSys/lib/kartip";

static
string genkar(object ob) {
   if (!ob) {
      error("bad argument to genkar");
   }
   if (function_object("is_thing", ob) == "/base/lib/thing") {
      return thing::genkar(ob);
   }
   if (ur_name(ob) == "/usr/SkotOS/sys/tip") {
      return tip::genkar(ob);
   }
   error("no KarMode defined for this object yet");
}

void parskar(object ob, string str) {
   if (function_object("is_thing", ob) == "/base/lib/thing") {
      thing::parskar(ob, str);
      return;
   }
   if (ur_name(ob) == "/usr/SkotOS/sys/tip") {
      tip::parskar(ob, str);
      return;
   }
   error("no KarMode defined for this object yet");
}
