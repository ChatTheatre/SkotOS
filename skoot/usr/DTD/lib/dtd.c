/*
**	/usr/DTD/lib/dtd.c
**
**	Datatype Definitions: this acts as a poor man's abstract data
**	type repository, for the conversion to and from ascii of raw
**	values, validity checks, etc.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <nref.h>
# include <type.h>
# include <DTD.h>

static
int query_type_colour(string type) {
   return DTD->get_type_handler(type)->query_type_colour(type);
}

static
string query_colour_type(int colour) {
   return DTD->get_colour_handler(colour)->query_colour_type(colour);
}

static
string *query_ascii_enumeration(string type) {
   return DTD->query_enumeration(type);
}

static
int query_checkboxed(string type) {
   return DTD->get_type_handler(type)->query_checkboxed(type, ([ ]));
}

static
int test_raw_data(mixed val, string type) {
   return DTD->get_type_handler(type)->test_raw_data(val, type);
}

static
mixed default_value(string type) {
   return DTD->get_type_handler(type)->default_value(type);
}

static string typed_to_ascii(mixed val, string type);

static
string untyped_to_ascii(mixed val) {
   string type;
   int colour;

   if (colour = query_colour(val)) {
      if (type = query_colour_type(colour)) {
	 return typed_to_ascii(val, type);
      }
   }
   switch(typeof(val)) {
   case T_STRING:
      return val;
   case T_INT: case T_FLOAT:
      return val + "";
   case T_OBJECT:
      return "OBJ(" + name(val) + ")";
   case T_ARRAY:
      return "ARR[" + sizeof(val) + "]";
   case T_MAPPING:
      return "MAP[" + map_sizeof(val) + "]";
   case T_NIL:
      return "[NIL]";
   }
   error("eek");
}

static
string typed_to_ascii(mixed val, string type) {
   if (!type) {
      return untyped_to_ascii(val);
   }
   return DTD->get_type_handler(type)->typed_to_ascii(val, type);
}

static
string typed_to_html(mixed val, string type) {
   object handler;

   handler = DTD->get_type_handler(type);
   if (function_object("typed_to_html", handler)) {
      return handler->typed_to_html(val, type);
   }
}

static
mixed ascii_to_untyped(string ascii) {
   string o, i;
   object ob;
   int n;

   if (sscanf(ascii, "NREF(%s|%s)", o, i)) {
      if (ob = find_object(o)) {
	 return NewNRef(ob, i);
      }
   } else if (sscanf(ascii, "NREF(%s)", o)) {
      if (ob = find_object(o)) {
	 return ob;
      }
   } else if (sscanf(ascii, "OBJ(%s)", o)) {
      if (ob = find_object(o)) {
	 return ob;
      }
      error("no object: " + o);
   } else return ascii;
}


static
mixed ascii_to_typed(string ascii, string type) {
   if (!type) {
      return ascii_to_untyped(ascii);
   }
   return DTD->get_type_handler(type)->ascii_to_typed(ascii, type);
}

static
int ascii_size(string type) {
   return DTD->get_type_handler(type)->query_asciisize(type);
}

static
int ascii_height(string type) {
   return DTD->get_type_handler(type)->query_asciiheight(type);
}
