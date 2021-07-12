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

string query_gender(int gender) {
    return query_property("gender_map")[gender];
}

int query_gender_reversed(string gender) {
    return reverse_mapping(query_property("gender_map"))[gender];
}

string query_possessive(int gender) {
   return query_property("gender:" + query_property("gender_map")[gender] + ":possessive");
}

string query_pronoun(int gender) {
   return query_property("gender:" + query_property("gender_map")[gender] + ":pronoun");
}

string query_objective(int gender) {
   return query_property("gender:" + query_property("gender_map")[gender] + ":objective");
}

string query_nominative(int gender) {
   return query_property("gender:" + query_property("gender_map")[gender] + ":nominative");
}

string query_genderstring(int gender) {
   return query_property("gender:" + query_property("gender_map")[gender] + ":genderstring");
}
