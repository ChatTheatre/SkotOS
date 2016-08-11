/*
 * /core/lib/dp_handler.c
 *
 * A toolkit class to help derived property handlers do their thing.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <base.h>

# include <dp_handler.h>

private inherit "/lib/mapping";
private inherit "/lib/string";

mapping get_configuration();
string query_derived_root();

mixed query_property(string property) {
   string type, str;
   if (sscanf(lower_case(property), "derived:%s", property)) {
      mixed *dat;

      if (property == "properties") {
	 return map_indices(get_configuration());
      }
      if (property == "root") {
	 return query_derived_root();
      }

      if (!sscanf(property, "%s:%s", type, property)) {
	 error("unknown property 'derived:" + property + "'");
      }

      dat = get_configuration()[property];

      if (!dat) {
	 error("unknown derived property for lookup: " + property);
      }

      switch(type) {
      case "has-local":
	 return !!(dat[CONFIG_FLAGS] & C_F_LOCAL);
      case "readable":
	 return !!(dat[CONFIG_FLAGS] & C_F_READ);
      case "writable":
	 return !!(dat[CONFIG_TYPE] & C_F_WRITE);
      case "accessdesc":
	 if (dat[CONFIG_FLAGS] & C_F_READ) {
	    if (dat[CONFIG_FLAGS] & C_F_WRITE) {
	       return "RW";
	    }
	    return "RO";
	 }
	 if (dat[CONFIG_FLAGS & C_F_WRITE]) {
	    return "WO";
	 }
	 return "NONE";
      case "type":
	 return dat[CONFIG_TYPE] & C_T_MASK;
      case "arrflag":
	 return !!dat[CONFIG_TYPE] & C_T_ARRFLAG;
      case "typedesc":
	 switch(dat[CONFIG_TYPE] & C_T_MASK) {
	 case 0:
	    str = "mixed";
	    break;
	 case T_STRING:
	    str ="string";
	    break;
	 case T_INT: 
	    str ="int";
	    break;
	 case T_FLOAT:
	    str ="float";
	    break;
	 case T_OBJECT:
	    str ="object";
	    break;
	 case T_ARRAY:
	    str ="array";
	    break;
	 case T_MAPPING:
	    str ="mapping";
	    break;
	 }
	 if (dat[CONFIG_TYPE] & C_T_ARRFLAG) {
	    str += " *";
	 }
	 return str;
      case "desc":
	 return dat[CONFIG_DESC];
      }
   }
}
