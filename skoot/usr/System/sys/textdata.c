/*
**	Simply handle requests from ../lib/textdata.c
*/

inherit "/usr/System/lib/sys_auto";

# include "/usr/System/lib/string.c"
# include "/usr/System/lib/array.c"

# include <type.h>
# include <System.h>

static
void create() {
   compile_object("/usr/System/data/badobjref");
}

mixed ascii_to_private(string ascii) {
   string type;
   mixed *res;

   if (!ascii) {
      return nil;
   }
   ascii = strip(ascii);
   /* figure out module abstraction later */
   if (sscanf(ascii, "X[%s]%s", type, ascii) == 2) {
      /* extended types */
      if (type == "M") {
	 return new_object("/usr/SkotOS/data/merry", ascii);
      }
      if (type == "S") {
	 return ::call_other("/usr/XML/sys/xml", "parse", ascii);
      }
      error("unknown extended type");
   }
   res = parse_string(
      "whitespace = /[ \t\r\n]+/\n" +
      "int = /[+-]?[0-9]+/\n" +
      "float = /[+-]?([0-9]+\\.|[0-9]*\\.[0-9]+)([eE][+-]?[0-9]+)?/\n" +
      "str = /\"([^\\\\\"]*\\\\.)*[^\\\\\"]*\"/\n" +
      "obj = /<[^>]+>/\n" +
      "recarr = /#[0-9]+/\n" +
      "recmap = /#[0-9]+/\n" +
      "catchtoken = /./\n" +

      "ColouredValue: '(' int ')' Value ? colour_value\n" +
      "ColouredValue: Value\n" +
      "Value: 'nil' ? make_nil\n" +
      "Value: int ? td_parse_int\n" +
      "Value: float ? td_parse_flt\n" +
      "Value: str ? td_parse_str\n" +
      "Value: obj ? td_parse_obj\n" +
      "Value: 'O' '(' ArrList ')' ? td_make_lwo\n" +
      "Value: Array\n" +
      "Value: Mapping\n" +
      "Value: recmap ? disallow_recursion\n" +
      "Value: recarr ? disallow_recursion\n" +
      "Array: '{' '}' ? empty_array\n" +
      "Array: '({' '})' ? empty_array\n" +
      "Array: '{' ArrList '}' ? pick_second\n" +
      "Array: '{' ArrList ',' '}' ? pick_second\n" +
      "Array: '({' ArrList '})' ? pick_second\n" +
      "Array: '({' ArrList ',' '})' ? pick_second\n" +
      "Mapping: '[' ']' ? empty_mapping\n" +
      "Mapping: '([' '])' ? empty_mapping\n" +
      "Mapping: '[' MapList ']' ? pick_second\n" +
      "Mapping: '[' MapList ',' ']' ? pick_second\n" +
      "Mapping: '([' MapList '])' ? pick_second\n" +
      "Mapping: '([' MapList ',' '])' ? pick_second\n" +
      "ArrList: ColouredValue ? make_array\n" +
      "ArrList: ArrList ',' ColouredValue ? build_array\n" +
      "MapList: ColouredValue ':' ColouredValue ? make_mapping\n" +
      "MapList: MapList ',' ColouredValue ':' ColouredValue ? build_mapping\n",
      ascii);

   if (!res) {
      /* just accept it verbatim -- backwards compatibility? */
      return ascii;
   }
   return res[0];
}

string private_to_ascii(mixed value, int verbose, mapping seen) {
   string str;

# if 1
   /* we really have to solve this soon */
   if (query_colour(value) == 17 || query_colour(value) == 1) {
      return "X[S] " +
	 ::call_other("/usr/XML/sys/xml", "generate_pcdata", value);
   }
# endif

   switch(typeof(value)) {
   case T_NIL:
      str = "nil";
      break;
   case T_INT:
      str = (string) value;
      break;
   case T_FLOAT:
      str = (string) value;
      if (!sscanf(str, "%*s.") && !sscanf(str, "%*se")) {
	 str += ".0";
      }
      break;
   case T_STRING: {
      str = "\"" + SYS_TD_ESCAPE->escape_string(value) + "\"";
      break;
   }
   case T_OBJECT:
      if (verbose) {
	 if (function_object("query_source", value)) {
	    /* figure out module abstraction later */
	    str = "X[M] " + value->query_source();
	    break;
	 }
      }
      if (::sscanf(ur_name(value), "%*s#-1")) {
	 mixed *configuration;

	 configuration = ::call_other(value, "query_configuration");
	 if (configuration) {
	    str = "O(" + ur_name(value) + ", " +
	       implode(map(configuration, "private_to_ascii",
			   this_object(), verbose, seen), ", ") +
	       ")";
	    break;
	 }
      }
      str = "<" + name(value) + ">";
      break;
   case T_ARRAY:
      if (seen[value]) {
	 str = "#" + (seen[value] - 1);
      } else {
	 seen[value] = map_sizeof(seen) + 1;

	 str = "({ " +
	    implode(map(value, "private_to_ascii",
			this_object(), verbose, seen), ", ") +
	    " })";
      }
      break;
   case T_MAPPING: {
      string *out;
      mixed *ix;
      int i;

      if (seen[value]) {
	 str = "@" + (seen[value] - 1);
      } else {
	 seen[value] = map_sizeof(seen) + 1;

	 ix = map_indices(value);

	 out = allocate(sizeof(ix));
	 for (i = 0; i < sizeof(ix); i ++) {
	    out[i] =
	       private_to_ascii(ix[i], verbose, seen) + ":" +
	       private_to_ascii(value[ix[i]], verbose, seen);
	 }
	 str = "([ " + implode(out, ", ") + " ])";
      }
      break;
   }
   default:
      error("unknown type: " + typeof(value));
   }
   if (query_colour(value)) {
      return "(" + query_colour(value) + ")" + str;
   }
   return str;
}

static
mixed *td_make_lwo(mixed *tree) {
   tree = tree[2]; /* the only interesting bit */
   if (sizeof(tree) == 0 || typeof(tree[0]) != T_STRING) {
      error("O(lwo, arg1, arg2) construct without lwo string");
   }
   return ({ new_object(tree[0], tree[1 ..]...) });
}

static
mixed *colour_value(mixed *tree) {
   return ({ tree[3] });
}

static
mixed *make_nil(mixed *tree) {
   return ({ nil });
}

static
mixed *td_parse_int(mixed *tree) {
   return ({ (int) tree[0] });
}

static
mixed *td_parse_flt(mixed *tree) {
   return ({ (float) tree[0] });
}

static
mixed *td_parse_str(mixed *tree) {
   return ({
      SYS_TD_UNESCAPE->unescape_string(tree[0][1 .. strlen(tree[0])-2])
	 });
}

static
mixed *td_parse_obj(string *tree) {
   string str;
   object obj;

   str = strip(tree[0][1 .. strlen(tree[0])-2]);
   obj = find_object(str);
   if (!obj) {
      obj = ::new_object("/usr/System/data/badobjref", str);
   }
   return ({ obj });
}

static
mixed *pick_second(mixed *tree) {
   return tree[1 .. 1];
}

static
mixed *empty_array(mixed *tree) {
   return ({ ({ }) });
}

static
mixed *make_array(mixed *tree) {
   return ({ tree });
}

static
mixed *build_array(mixed *tree) {
   return ({ tree[0] + ({ tree[2] }) });
}

static
mixed *empty_mapping(mixed *tree) {
   return ({ ([ ]) });
}

static
mixed *make_mapping(mixed *tree) {
   return ({ ([ tree[0] : tree[2] ]) });
}

static
mixed *build_mapping(mixed *tree) {
   return ({ tree[0] + ([ tree[2] : tree[4] ]) });
}

