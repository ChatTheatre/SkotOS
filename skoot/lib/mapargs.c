# include <type.h>

static
mixed arg_mixed(string ix, mixed val, string desc) {
   if (val != nil) {
      return val;
   }
   error(desc + "(" + ix + "): undefined");
}

static
object arg_object(string ix, mixed val, string desc) {
   object ob;

   switch(typeof(val)) {
   case T_OBJECT:
      return val;
   case T_STRING:
      ob = find_object(val);
      if (ob) {
	 return ob;
      }
      error(desc + "(" + ix + "): object " + val + " not found");
   case T_NIL:
      error(desc + "(" + ix + "): undefined");
   }
   error(desc + "(" + ix + "): bad argument type");
}

static
string arg_string(string ix, mixed val, string desc) {
   switch(typeof(val)) {
   case T_STRING:
      return val;
   case T_INT: case T_FLOAT:
      return (string) val;
   case T_NIL:
      error(desc + "(" + ix + "): undefined");
   default:
      error(desc + "(" + ix + "): bad argument type");
   }
}

static
int arg_int(string ix, mixed val, string desc) {
   switch(typeof(val)) {
   case T_STRING:
      if (sscanf(val, "%d", val)) {
	 return val;
      }
      error(desc + "(" + ix + "): bad integer");
   case T_INT:
      return val;
   case T_NIL:
      error(desc + "(" + ix + "): undefined");
   default:
      error(desc + "(" + ix + "): bad argument type");
   }
}
