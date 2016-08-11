/*
**	~TextIF/data/theme.c
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/lib/properties";

static
void create(int clone) {
   ::create();
}

string query_colour(string theme, string id) {
   return query_property("colours:" + id);
}

void set_colour(string theme, string id, string colour) {
   set_property("colours:" + id, colour);
}

