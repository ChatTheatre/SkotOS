/*
**	~TextIF/sys/themes.c
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";
private inherit "/lib/mapping";

inherit "/lib/properties";

string query_state_root() { return "Core:Properties"; }

static
void create() {
   ::create();

   set_object_name("TextIF:Gender");
}

string query_possessive(string gender) {
   return query_property("gender:" + gender + ":possessive");
}

string query_pronoun(string gender) {
   return query_property("gender:" + gender + ":pronoun");
}

string query_nominative(string gender) {
   return query_property("gender:" + gender + ":nominative");
}

string query_genderstring(string gender) {
   return query_property("gender:" + gender + ":genderstring");
}
