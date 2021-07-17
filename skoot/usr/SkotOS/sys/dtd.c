/*
**	/usr/SkotOS/sys/dtd.c
**
**	Answer queries for the DTD system on the data types that
**	SkotOS handles -- which includes the THING_ ones, since
**	BaseWorld can't depend on any modules.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";
private inherit "/lib/mapargs";

inherit "/usr/Tool/lib/merry";

# include <mapargs.h>

# include <base/living.h>
# include <notes.h>
# include <DTD.h>
# include <nref.h>
# include <SkotOS.h>
# include <base/prox.h>
# include <TextIF.h>

static
void create() {
   DTD->register_type(THING_PREP);
   DTD->register_type(THING_DESC_TYPE);
   DTD->register_type(THING_FACE);
   DTD->register_type(THING_GENDER);
   DTD->register_type(THING_STANCE);

   DTD->register_type(THING_NREF);
   DTD->register_colour(COL_NREF);

   DTD->register_type(LOG_TAG);

   DTD->register_type(NOTE_STATUS);

   DTD->register_type(SOCIAL_APPROACH);
   DTD->register_type(SOCIAL_REQUIREMENT);
   DTD->register_type(SOCIAL_ROLE);

   DTD->register_type(MERRY);

   DTD->register_type(CA_COND);
   DTD->register_type(BITMAP);
}

void patch() {
   DTD->register_type(MERRY);
   DTD->register_type(CA_COND);
   DTD->register_type(BITMAP);
}

int query_type_colour(string type) {
   if (type == THING_NREF) {
      return COL_NREF;
   }
   return 0;
}

string query_colour_type(int colour) {
   if (colour == COL_NREF) {
      return THING_NREF;
   }
   return nil;
}

int query_checkboxed(string type, mapping args) {
   return FALSE;
}

int test_raw_data(mixed val, string type) {
   switch(type) {
   case BITMAP:
      /* make a bit of an effort to make sure it's int ** */
      return typeof(val) == T_ARRAY &&
	 typeof(val[0]) == T_ARRAY &&
	 typeof(val[0][0]) == T_INT;
   case CA_COND:
      return typeof(val) == T_INT &&
	 val == 0 || val == 1 || val == 2;
   case MERRY:
      return typeof(val) == T_NIL ||
	 (typeof(val) == T_OBJECT &&
	  ur_name(val) == "/usr/SkotOS/data/merry#-1");
   case THING_DESC_TYPE:
   case THING_FACE:
   case LOG_TAG:
      return TRUE;
   case THING_NREF:
      return typeof(val) == T_NIL || IsNRef(val);
   case THING_GENDER:
      return typeof(val) == T_INT;
      /* return typeof(val) == T_INT && (val == 0 || val == 1 || val == 2); */
   case NOTE_STATUS:
      return typeof(val) == T_INT &&
	 (val == NOTE_CAT_OPEN ||
	  val == NOTE_CAT_COMMENT ||
	  val == NOTE_CAT_CLOSE);
   case SOCIAL_APPROACH:
       return typeof(val) == T_INT &&
	   (val == SA_FAR || val == SA_ARMSLENGTH ||
	    val == SA_CLOSE || val == SA_OWNED);
   case SOCIAL_REQUIREMENT:
       return typeof(val) == T_INT &&
	   (val == VERB_FORBIDDEN ||
	    val == VERB_OPTIONAL ||
	    val == VERB_REQUIRED);
   case SOCIAL_ROLE:
      return TRUE;
   case THING_STANCE:
      if (typeof(val) != T_INT) {
	 return FALSE;
      }
      switch(val) {
      case STANCE_CROUCH:
      case STANCE_KNEEL:
      case STANCE_LIE:
      case STANCE_SIT:
      case STANCE_STAND:
      case STANCE_HANG:
      case STANCE_FLOAT:
      case 0:
	 return TRUE;
      }
      return FALSE;
   }
   error("unknown type: " + type);
}

mixed default_value(string type) {
   switch(type) {
   case CA_COND:
   case THING_GENDER:
      return 0;
   case MERRY:
   case BITMAP:
   case THING_NREF:
      return nil;
   case THING_DESC_TYPE:
      return "brief";
   case LOG_TAG:
      return "Bug Log"; /* ?? */
   case THING_FACE:
      return nil;
   case NOTE_STATUS:
      return NOTE_CAT_COMMENT;
   case SOCIAL_APPROACH:
       return SA_FAR;
   case SOCIAL_REQUIREMENT:
      return VERB_FORBIDDEN;
   case SOCIAL_ROLE:
      return "object";
   case THING_STANCE:
      return 0;
   }
   error("unknown type: " + type);
}

string typed_to_ascii(mixed val, string type) {
   switch(type) {
   case MERRY:
      return val ? val->query_source() : "";
   case LOG_TAG:
   case THING_DESC_TYPE:
   case THING_FACE:
      if (typeof(val) == T_NIL) {
	 return "N/A";
      }
      return val;
   case THING_NREF:
      if (IsNRef(val) && NRefOb(val) && NRefDetail(val)) {
	 return "NREF(" + name(NRefOb(val)) + "|" + NRefDetail(val) + ")";
      }
      return "";
   case THING_GENDER:
      /* switch(val) {
      case GENDER_MALE:
	          return "male";      
      case GENDER_FEMALE:
	          return "female";
      case GENDER_NEUTER:
      default:
              return "neuter";
      } */
      return GENDER->query_gender(val);
   case NOTE_STATUS:
      switch(val) {
      case NOTE_CAT_OPEN:
	 return "Open";
      case NOTE_CAT_COMMENT:
      default:
	 return "Comment";
      case NOTE_CAT_CLOSE:
	 return "Close";
      }
   case SOCIAL_APPROACH:
       switch(val) {
       default:
       case SA_FAR:
	   return "far";
       case SA_ARMSLENGTH:
	   return "armslength";
       case SA_CLOSE:
	   return "close";
       case SA_OWNED:
	  return "owned";
       }	   
   case SOCIAL_REQUIREMENT:
       switch(val) {
       default:
       case VERB_FORBIDDEN:
	  return "forbidden";
       case VERB_OPTIONAL:
	  return "optional";
       case VERB_REQUIRED:
	  return "required";
       }	   
   case SOCIAL_ROLE:
      return val;
   case THING_STANCE:
      switch(val) {
      case STANCE_CROUCH:
	 return "crouching";
      case STANCE_KNEEL:
	 return "kneeling";
      case 0:
	 return "none";
      case STANCE_LIE:
	 return "lying";
      case STANCE_SIT:
	 return "sitting";
      case STANCE_STAND:
	 return "standing";
      case STANCE_HANG:
	 return "hanging";
      case STANCE_FLOAT:
	 return "floating";
      }
   case CA_COND:
      return ({ "wrap", "fixed", "differential" })[val];
   case BITMAP:
      return "";
   }
   error("unknown type: " + type);
}

string typed_to_html(mixed val, string type) {
   if (type == MERRY) {
      return "<PRE>\n" +
	 source_to_html(pp_merry_code(val)) +
	 "\n</PRE><HR>\n";
   }
   return typed_to_ascii(val, type);
}

mixed ascii_to_typed(string ascii, string type) {
   string str, id;
   object ob;

   switch(type) {
   case MERRY:
      return new_object("/usr/SkotOS/data/merry", ascii);
   case LOG_TAG:
   case THING_DESC_TYPE:
   case THING_FACE:
      if (ascii == "N/A") {
	 return nil;
      }
      return ascii;
   case THING_NREF:
      if (!ascii || !strlen(ascii)) {
	 return nil;
      }
      if (sscanf(ascii, "NREF(%s|%s)", str, id)) {
	 if (ob = find_object(str)) {
	    return NewNRef(ob, id);
	 }
      } else if (sscanf(ascii, "NREF(%s)", str)) {
	 if (ob = find_object(str)) {
	    return ob;
	 }
      } else {
	 error("value is not an nref");
      }
      error("cannot find object: " + dump_value(str));
   case THING_GENDER:
      /*switch(lower_case(ascii)) {
          case "male":	
                  return GENDER_MALE;
          case "female":	
                  return GENDER_FEMALE;
          case "neuter":	
                  return GENDER_NEUTER;
          default:
                  error("value is not a gender");
                  }*/
           
        return GENDER->query_gender_reversed(lower_case(ascii));
      
   case NOTE_STATUS:
      switch(lower_case(ascii)) {
      case "open":	return NOTE_CAT_OPEN;
      case "comment":	return NOTE_CAT_COMMENT;
      case "close":	return NOTE_CAT_CLOSE;
      default:
	 error("value is not a note status");
      }
   case SOCIAL_APPROACH:
      switch(lower_case(ascii)) {
      case "far":		return SA_FAR;
      case "armslength":	return SA_ARMSLENGTH;
      case "close":		return SA_CLOSE;
      case "owned":		return SA_OWNED;
      default:
	 error("value is not a approach value");
      }
   case SOCIAL_REQUIREMENT:
      switch(lower_case(ascii)) {
      case "forbidden":		return VERB_FORBIDDEN;
      case "optional":		return VERB_OPTIONAL;
      case "required":		return VERB_REQUIRED;
      default:
	 error("value is not a approach value");
      }
   case SOCIAL_ROLE:
      return lower_case(ascii);
   case THING_STANCE:
      switch(lower_case(ascii)) {
      case "crouching":
	 return STANCE_CROUCH;
      case "kneeling":
	 return STANCE_KNEEL;
      case "none":
	 return 0;
      case "lying":
	 return STANCE_LIE;
      case "sitting":
	 return STANCE_SIT;
      case "standing":
	 return STANCE_STAND;
      case "hanging":
	 return STANCE_HANG;
      case "floating":
	 return STANCE_FLOAT;
      }	   
      error("value is not a valid stance");
   case CA_COND:
      return ([ "wrap": 0,
		"fixed": 1,
		"differential": 2
		 ])[lower_case(ascii)];
   case BITMAP:
      return nil;
   }
   error("unknown type " + type);
}

int query_asciisize(string type, varargs mapping args) {
   if (type == THING_NREF) {
      return 60;
   }
   return 40;
}

int query_asciiheight(string type, varargs mapping args) {
   if (type == MERRY) {
      return 20;
   }
   return 1;
}
