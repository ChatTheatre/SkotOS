/*
**	/usr/XML/lib/entities.c
**
**	Perform the translation back and forth between quoted entities
**	in the XML manner. In true XML this is a dynamic database, but
**	for our (current) purposes a hard-coded list is preferable.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/mapping";

mapping ascii_to_entity;
mapping entity_to_ascii;

# include <faststr.h>

/* for now, just the predefined entity list in the XML spec */

static
create() {
   ascii_to_entity = ([
      '\'': "apos",
      '\"': "quot",
      '<': "lt",
      '>': "gt",
      '&': "amp",
      '|': "pipe",
      '{': "lbrace",
      '}': "rbrace",
      ]);

   entity_to_ascii = reverse_mapping(ascii_to_entity);
}

# if 0
nomask void patch() {
   ascii_to_entity = ([
      '\'': "apos",
      '\"': "quot",
      '<': "lt",
      '>': "gt",
      '&': "amp",
      '|': "pipe",
      '{': "lbrace",
      '}': "rbrace",
      ]);

   entity_to_ascii = reverse_mapping(ascii_to_entity);
}
# endif

static
string entify_string(string str, int ents...) {
   mapping set;
   string s;
   int i, l;

   set = ([ ]);
   for (i = 0; i < sizeof(ents); i ++) {
      if (s = ascii_to_entity[ents[i]]) {
	 set[ents[i]] = "&" + s + ";";
      }
   }

   FStrNew();
   l = 0;
   for (i = 0; i < strlen(str); i ++) {
      if (s = set[str[i]]) {
	 if (l < i) {
	    FStrApp(str[l .. i-1]);
	 }
	 FStrApp(s);
	 l = i+1;
      }
   }
   if (l < i) {
      FStrApp(str[l .. i-1]);
   }
   return FStrRes();
}

static
mixed entity_to_ascii(string ent) {
   return entity_to_ascii[ent];
}

static
string ascii_to_entity(int ascii) {
   return ascii_to_entity[ascii];
}
