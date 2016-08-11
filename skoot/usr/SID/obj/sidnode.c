/*
**	/usr/SID/obj/sidnode.c
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/usr/SID/lib/sidnode";

string query_state_root() { return "SID:Element"; }

static
void create(int clone) {
   ::create();
   if (!clone) {
      ::set_object_name("SID:UrNode");
   }
}

nomask
void set_object_name(string name) {
   if (query_namespace() || query_tag()) {
      error("configured SID nodes cannot be renamed");
   }
   /* but do allow the naming of a virgin node */
   ::set_object_name(name);
}

atomic
void set_name(string s, string t) {
   if (::set_name(s, t)) {
      ::set_object_name("SID:" + s + ":" + t);
   }
}
