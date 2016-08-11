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

   set_object_name("TextIF:Themes");
}


mixed query_downcased_property(string property) {
   string theme;

   if (sscanf(property, "themes:%s", theme) &&
       map_sizeof(prefixed_map(query_properties(), "colours:" + theme))) {
      return this_object();
   }
   return ::query_downcased_property(property);
}

string query_colour(string theme, string id) {
   return query_property("colours:" + theme + ":" + id);
}

void set_colour(string theme, string id, string colour) {
   set_property("colours:" + theme + ":" + id, colour);
}
