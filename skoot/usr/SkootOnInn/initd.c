/*
**	~SkootOnInn/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/access.h>

# include <HTTP.h>
# include <SkotOS.h>
# include <base.h>

inherit vault "/usr/SID/lib/vaultnode";

object inn, oasis, mansion;

static
void create() {
   vault::create("/usr/SkootOnInn/data/vault");

   set_object_name("SkootOnInn:Init");

   claim_node("SkootOnInn");
   claim_node("Mansion");
   claim_node("Oasis");
   claim_node("clothing");
   claim_node("props");

   inn = clone_object("/usr/Theatre/obj/theatre");
   inn->set_name("Skoot On Inn", "SkootOnInn");
   inn->set_web_port(SYS_INITD->query_portbase() + 81);

   oasis = clone_object("/usr/Theatre/obj/theatre");
   oasis->set_name("Oasis", "Oasis");
   oasis->set_web_port(SYS_INITD->query_portbase() + 82);

   mansion = clone_object("/usr/Theatre/obj/theatre");
   mansion->set_name("Mansion", "Mansion");
   mansion->set_web_port(SYS_INITD->query_portbase() + 83);

   HTTPD->claim_port(SYS_INITD->query_portbase() + 80);
   HTTPD->register_root("Inn");
}

void patch() {
   HTTPD->register_root("Inn");
}
string query_http_root_url_document() {
   return "/Inn/Main.sam";
}   

string remap_http_request(string root, string url, mapping args) {
   if (root == "inn") {
      return "/usr/SkootOnInn/data/www" + url;
   }
   error("unknown URL root " + root);
}

atomic
void boot(int foo) {
   object obj;

   obj = find_object("SkootOnInn:rooms:Road");
   if (!obj) {
      error("SkootOnInn:rooms:Road not found");
   }
   inn->set_start_room(obj);

   obj = find_object("Mansion:rooms:Limo");
   if (!obj) {
      error("Mansion:rooms:Limo not found");
   }
   mansion->set_start_room(obj);

   obj = find_object("Oasis:rooms:DesertW");
   if (!obj) {
      error("Oasis:rooms:DesertW not found");
   }
   oasis->set_start_room(obj);

   obj = find_object("Generic:UrMale");
   if (!obj) {
      error("Generic:UrMale not found");
   }
   inn->set_male_body(obj);
   oasis->set_male_body(obj);
   mansion->set_male_body(obj);

   obj = find_object("Generic:UrFemale");
   if (!obj) {
      error("Generic:UrFemale not found");
   }
   inn->set_female_body(obj);
   oasis->set_female_body(obj);
   mansion->set_female_body(obj);
}
