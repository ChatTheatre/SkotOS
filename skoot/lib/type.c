/*
**      /lib/type.c
**
**      Useful functions for manipulating files and path names.
**
**      Copyright Skotos Tech Inc 1999, 2005
*/

#include <type.h>

/* ascii_typeof will return a string naming the typeof value */
string ascii_typeof(mixed value) {

   /* this code was ripped from untyped_to_ascii() */
   switch(typeof(value)) {	
   case T_STRING:
      return "string";
   case T_INT: 
      return "int";
   case T_FLOAT:
      return "float";
   case T_OBJECT:
      return "object";
   case T_ARRAY:
      return "array";
   case T_MAPPING:
      return "mapping";
   case T_NIL:
      return "nil";
   }
   error("eek");
}

string formatted_typeof(mixed value) {

   /* this code was ripped from untyped_to_ascii() */
   switch(typeof(value)) {	
   case T_STRING:
      return "string" + "[" + "\"" + value + "\"" + "]";
   case T_INT: 
      return "int" + "[" + value + "]";
   case T_FLOAT:
      return "float" + "[" + value + "]";
   case T_OBJECT:
      return "object" + "[" + dump_value(value) + "]";
   case T_ARRAY:
      return "array" + "[" + dump_value(value) + "]";
   case T_MAPPING:
      return "mapping" + "[" + dump_value(value) + "]";
   case T_NIL:
      return "nil";
   }
   error("eek");
}


/* some smart casting functions */

int Int(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return 0;
   case T_INT:
      return val;
   case T_FLOAT:
      return (int) val;
   case T_STRING:
      if (!strlen(val)) {
	 return 0;
      }
      if (sscanf(val, "%d", val)) {
	 return val;
      }
      break;
   }
   error("cannot convert value to integer");
}

float Flt(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return 0.0;
   case T_INT:
      return (float) val;
   case T_FLOAT:
      return val;
   case T_STRING:
      if (!strlen(val)) {
	 return 0.0;
      }
      if (sscanf(val, "%f", val)) {
	 return val;
      }
      break;
   }
   error("cannot convert value to float");
}

mixed Num(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return 0;
   case T_INT:
   case T_FLOAT:
      return val;
   case T_STRING:
      if (!strlen(val)) {
	 return 0;
      }
      if (sscanf(val, "%*d.%*d") == 2) {
	 return (float) val;
      }
      if (sscanf(val, "%*d") == 1) {
	 return (int) val;
      }
      break;
   }
   SysLog(dump_value(val));
   error("cannot convert value to number");
}


string Str(mixed val, varargs int err) {
   switch(typeof(val)) {
   case T_NIL:
      return "";
   case T_INT:
   case T_FLOAT:
      return (string) val;
   case T_STRING:
      return val;
   case T_OBJECT:
      return name(val);
   }
   if (err) {
      error("cannot convert value to string");
   }
   return "[nostr]";
}

object Obj(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return val;
   case T_STRING:
      return find_object(val);
   case T_OBJECT:
      return val;
   }
   error("cannot convert value to object");
}

mixed *Arr(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return ({ });
   case T_STRING:
   case T_OBJECT:
   case T_INT:
   case T_FLOAT:
   case T_MAPPING:
      return ({ val });
   case T_ARRAY:
      return val;
   }
   error("cannot convert value to array");
}

mapping Map(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return ([ ]);
   case T_MAPPING:
      return val;
   }
   error("cannot convert value to mapping");
}

