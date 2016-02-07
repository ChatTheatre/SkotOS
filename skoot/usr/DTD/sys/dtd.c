/*
**	/usr/DTD/sys/dtd.c
**
**	This is the repository of data type definitions. It's very
**	new. We'll see precisely what it does later.
**
**	Copyright SkotOS Tech Inc 1999
*/

# include <type.h>
# include <mapargs.h>
# include <DTD.h>
# include <SAM.h>

private inherit "/lib/string";
private inherit "/lib/url";
private inherit "/lib/mapargs";
        inherit module "/lib/module";

private inherit "/usr/DTD/lib/dtd";

/* SIGH */
inherit "/usr/XML/lib/entities";

mapping enumerations;
mapping type_handlers;
mapping colour_handlers;

string query_state_root() { return "DTD:Enumerations"; }

static
void create() {
   type_handlers = ([
      LPC_MIXED: this_object(),
      LPC_STR: this_object(),
      LPC_INT: this_object(),
      LPC_FLT: this_object(),
      LPC_OBJ: this_object(),
      ]);
   colour_handlers = ([ ]);
   enumerations = ([ ]);

   module::create("SAM");

   set_object_name("DTD:DTD");
}

void patch() {
   type_handlers = ([
      LPC_MIXED: this_object(),
      LPC_STR: this_object(),
      LPC_INT: this_object(),
      LPC_FLT: this_object(),
      LPC_OBJ: this_object(),
      ]);
   colour_handlers = ([ ]);
}

void SAM_loaded() {
   SAMD->register_root("DTD");
}

void boot(int block) {
   if (previous_program() == "/usr/DTD/initd") {
   }
}

/* handler API */

void register_type(string type) {
   SysLog("DTD:: Registered type '" + type + "' to " +
	  name(previous_object()));
   type_handlers[type] = previous_object();
}

object query_type_handler(string type) {
   return type_handlers[type];
}

object get_type_handler(string type) {
   if (type_handlers[type]) {
      return type_handlers[type];
   }
   error("no handler for type: " + dump_value(type));
}


void register_colour(int colour) {
   SysLog("DTD:: Registered colour '" + colour + "' to " +
	  name(previous_object()));
   colour_handlers[colour] = previous_object();
}

object query_colour_handler(int colour) {
   return colour_handlers[colour];
}

object get_colour_handler(int colour) {
   if (colour_handlers[colour]) {
      return colour_handlers[colour];
   }
   error("no handler for colour: " + colour);
}


/* enumerations */

string *query_enumerations() {
   return map_indices(enumerations);
}

void clear_enumeration(string enum) {
   enumerations[enum] = nil;
}

void extend_enumeration(string enum, string item) {
   if (!enumerations[enum]) {
      enumerations[enum] = ([ ]);
   }
   enumerations[enum][item] = TRUE;
}

string *query_enumeration(string enum, varargs mapping args) {
   mapping map;

   if (map = enumerations[enum]) {
      return map_indices(map);
   }
   return nil;
}

mixed eval_sam_ref(string service, string ref, object context,
		   mapping args) {
   string type;
   object handler;

   if (type = args["dtype"]) {
      if (ref == "enumeration") {
	 handler = this_object();
      } else {
	 type = lower_case(type);
	 handler = type_handlers[type];
      }
      if (handler) {
	 return call_other(handler, "query_" + ref, type, args);
      }
      error("no handler known for type: " + type);
   }
}

/* we handle queries on the raw LPC types ourselves */


int query_type_colour(string type) {
   return 0;
}

int test_raw_data(mixed value, string type) {
   switch(type) {
   case LPC_MIXED:
      return TRUE;
   case LPC_STR:
      return typeof(value) == T_STRING || typeof(value) == T_NIL;
   case LPC_OBJ:
      return typeof(value) == T_OBJECT || typeof(value) == T_NIL;
   case LPC_FLT:
      return typeof(value) == T_FLOAT;
   case LPC_INT:
      return typeof(value) == T_INT;
   }
   error("unknown type: " + type);
}

mixed default_value(string type) {
   switch(type) {
   case LPC_MIXED:
   case LPC_STR:
   case LPC_OBJ:
      return nil;
   case LPC_FLT:
      return 0.0;
   case LPC_INT:
      return 0;
   }
   error("unknown type: " + type);
}

string typed_to_ascii(mixed value, string type) {
   if (!type) {
      return ::untyped_to_ascii(value);
   }
   switch(type) {
   case LPC_MIXED:
      return mixed_to_ascii(value, TRUE);
   case LPC_STR:
      if (value == nil) {
	 return "";
      }
      if (typeof(value) == T_STRING) {
	 return value;
      }
      error("not a string");
   case LPC_OBJ:
      if (typeof(value) == T_OBJECT) {
	 return "OBJ(" + name(value) + ")";
      }
      if (typeof(value) == T_NIL) {
	 return "";
      }
      error("not an object");
   case LPC_INT: case LPC_FLT:
      return (string) value;
   }
   error("unknown type: " + type);
}

string typed_to_html(mixed value, string type) {
   if (type == LPC_MIXED && typeof(value) == T_OBJECT &&
       value->query_dtd_type()) {
      return ::typed_to_html(value, value->query_dtd_type());
   }
   if (type == LPC_OBJ) {
      if (typeof(value) == T_OBJECT) {
	 return
	    "<a href=\"" +
	    make_url("/Dev/View.sam", ([ "obj": name(value) ])) + "\">" +
	    name(value) +
	    "</a>";
      }
      if (typeof(value) != T_NIL) {
	 SysLog("Eh... value is: " + dump_value(value));
      }
   }
   return nil;
}

mixed ascii_to_typed(string ascii, string type) {
   string str;
   object ob;
   int n;
   float f;

   if (!type) {
      type = LPC_MIXED;
   }
   switch(type) {
   case LPC_MIXED:
      return ascii_to_mixed(ascii);
   case LPC_STR:
      if (!strlen(ascii)) {
	 return nil;
      }
      return ascii;
   case LPC_OBJ:
      if (!ascii || !strlen(ascii)) {
	 return nil;
      }
      if (sscanf(ascii, "OBJ(%s)", str)) {
	 ob = find_object(str);
	 if (!ob) {
	    error("no object: " + ascii);
	 }
	 return ob;
      }
      error("value is not an object");
   case LPC_INT:
      if (sscanf(ascii, "%d", n)) {
	 return n;
      }
      error("value is not an integer");
   case LPC_FLT:
      if (sscanf(ascii, "%f", f)) {
	 return f;
      }
      error("value is not an float");
   }
   error("unknown type: " + type);
}

int query_asciisize(string type, varargs mapping args) {
   switch(type) {
   case LPC_FLT:
      return 10;
   case LPC_INT:
      return 12;
   case LPC_STR:
   case LPC_OBJ:
   case LPC_MIXED:
      return 30;
   }      
   error("unknown type: " + type);
}

int query_asciiheight(string type, varargs mapping args) {
   switch(type) {
      case LPC_STR:
      case LPC_MIXED:
	return 1;
      case LPC_FLT:
      case LPC_INT:
      case LPC_OBJ:
	return 1;
   }
   error("unknown type: " + type);
}
