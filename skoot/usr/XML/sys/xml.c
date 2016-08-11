/*
**
*/

# include <type.h>
# include <DTD.h>
# include <XML.h>

inherit "/lib/data";

inherit "~/lib/xmlparse";
inherit "~/lib/xmlgen";
inherit "~/lib/xmd";

static
void create() {
   ::create();

   DTD->register_type(XML_ELEMENT);
   DTD->register_type(XML_SAMREF);
   DTD->register_type(XML_PCDATA);
   DTD->register_type(XML_BOOL);

   DTD->register_colour(COL_ELEMENT);
   DTD->register_colour(COL_SAMREF);
   DTD->register_colour(COL_PCDATA);
}

void patch() {
   DTD->register_type(XML_ELEMENT);
   DTD->register_type(XML_SAMREF);
   DTD->register_type(XML_PCDATA);
   DTD->register_type(XML_BOOL);

   DTD->register_colour(COL_ELEMENT);
   DTD->register_colour(COL_SAMREF);
   DTD->register_colour(COL_PCDATA);
}

mixed parse(string str) {
   return ::parse_xml(str);
}

string gen_xml(mixed xml) {
   clear();   
   ::generate_xml(xml, this_object());
   return implode(query_chunks(), "");
}


string generate_pcdata(mixed pcdata) {
   return ::generate_pcdata(pcdata);
}

mixed xmd_force_to_data(mixed xmd) {
   return ::xmd_force_to_data(xmd);
}


/* DTD queries */

int query_type_colour(string type) {
   switch(type) {
   case XML_ELEMENT:
      return COL_ELEMENT;
   case XML_SAMREF:
      return COL_SAMREF;
   case XML_PCDATA:
     return COL_PCDATA;
   case XML_BOOL:
      return 0;
   }
   error("unknown type: " + type);
}

string query_colour_type(int colour) {
  switch(colour) {
  case COL_ELEMENT:
     return XML_ELEMENT;
  case COL_SAMREF:
     return XML_SAMREF;
  case COL_PCDATA:
    return XML_PCDATA;
  }
  error("unknown colour: " + colour);
}

int query_checkboxed(string type, mapping args) {
   return type == XML_BOOL;
}

int test_raw_data(mixed val, string type) {
   switch(type) {
   case XML_PCDATA:
      return
	typeof(val) == T_NIL    ||
	typeof(val) == T_STRING ||
	typeof(val) == T_ARRAY  ||
	query_colour(val) == COL_PCDATA ||
	query_colour(val) == COL_SAMREF ||
	query_colour(val) == COL_ELEMENT;
   case XML_BOOL:
      return typeof(val) == T_INT;
   }
   error("unknown type: " + type);
}

mixed default_value(string type) {
   switch(type) {
   case XML_PCDATA:
      return "";
   case XML_BOOL:
      return 0;
   }
   error("unknown type: " + type);
}

string typed_to_ascii(mixed val, string type) {
   switch(type) {
   case XML_ELEMENT:
   case XML_SAMREF:
   case XML_PCDATA:
      return generate_pcdata(val);
   case XML_BOOL:
      return (HARD(val) ? "hard " : "") + (val ? "true": "false");
   }
   error("unknown type: " + type);
}

mixed ascii_to_typed(string ascii, string type) {
   mixed dat;

   switch(type) {
   case XML_PCDATA:
      return parse_xml(ascii);
   case XML_BOOL:
      switch(ascii) {
      case "true": case "yes":
	 return TRUE;
      case "false": case "no":
	 return FALSE;
      case "hard false":
	 return HARDEN(FALSE);
      }
      error("value is not a boolean");
   }
   error("unknown type: " + type);
}

int ascii_size(string type) {
   return 60;
}

int ascii_height(string type) {
   return 4;
}
