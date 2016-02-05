/*
**
**	/usr/SkotOS/data/merry.c
**
**	Merry, short for Meriadoc, short for Mediated Execution
**	Registry Involving Another Dialect Of C. Each instance of
**	this LWO will maintain a copy of the LPC it represents as
**	well as an object pointer to the program itself.
**
**	Initial Revision by Zell 010910
**
**	Copyright (C) Skotos Tech 2001
*/

# include <SkotOS.h>
# include <SAM.h>
# include <status.h>

# include "/usr/SkotOS/include/merry.h"

inherit "/lib/womble";

private inherit "/lib/string";
private inherit "/lib/file";
private inherit "/usr/XML/lib/xmlgen";

# define MERRYNODE(n)	("/usr/SkotOS/merry/" + (n))

string source, hash;
mixed  *srcarr;
object program;

mapping constmap;
object *obarr;

SAM *samarr;

object query_program();
object get_program();
string expand_to_lpc(mixed *arr);

# define ST_VIRGIN	1
# define ST_STRING	2
# define ST_QUOTE	3
# define ST_COMMENT	4
# define ST_OBJREF	10
# define ST_ARGREF	11
# define ST_ARGREF_P	12
# define ST_SLEEP	13

void womble_merry() {
   samarr = womble(samarr);
}

string query_dtd_type() { return MERRY; }

static
void create(int clone) {
   ::create();
}

static
void configure(string lpc) {
   int    i ,sz;
   string result, *constants;
   mixed  *res;

   if (previous_program() != SYS_AUTO) {
      error("bad call");
   }

   lpc = strip(lpc);
   res = SYS_MERRY->parse_merry(lpc);

   srcarr   = res[0];
   obarr    = res[1];
   constmap = res[2];
   samarr   = res[3];

   /*
    * Generate LPC from Merry code:
    */
   result = expand_to_lpc(srcarr);
   result = replace_strings(result,
			    "ActArg", "Act",
			    "CallArg", "Call");

   if (!constmap) {
      constmap = ([ ]);
   }

   constants = map_indices(constmap);
   for (i = 0, sz = sizeof(constants); i < sz; i ++) {
      constants[i] = "# define " + constants[i] + " " +
	 expand_to_lpc(constmap[constants[i]]);
   }

   source = 
      "inherit \"/usr/SkotOS/lib/merrynode\";\n" +
      "# include \"/usr/SkotOS/include/merrynode.h\"\n" +
      implode(constants, "\n") + "\n" +
      "mixed merry(string mode, string signal, string label) {\n" +
      "    switch(label) { case \"virgin\": {\n" + result + "}\n" +
      "    }\n" +
      "}\n";

   hash = to_hex(hash_md5(source));

   get_program();	/* force the compile */
}

object query_program() { return program; }	/* for debugging */
mixed *query_srcarr() { return srcarr; }	/* for debugging */

private
int is_simple(string str) {
   int i;

   for (i = 0; i < strlen(str); i ++) {
      switch(str[i]) {
      case 'a'..'z': case 'A'..'Z': case '_': case '0'..'9':
	 break;
      default:
	 return FALSE;
      }
   }
   return TRUE;
}

void convert_sleep(mixed *bit);

private
string fixprop(string str) {
   string lc;
   int bad, i;

   if (strlen(str) && ((str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z'))) {
      for (i = 1; !bad && i < strlen(str); i ++) {
	 switch(str[i]) {
	 case 'a' .. 'z':
	 case 'A' .. 'Z':
	 case '0' .. '9':
	 case '_':
	    break;
	 default:
	    bad = TRUE;
	 }
      }
   } else {
      bad = TRUE;
   }
   if (bad) {
      return "\"" + str + "\"";
   }
   return str;
}


string expand_to_source(mixed arr) {
   string *result;
   mixed bit;
   int i;

   result = allocate(sizeof(arr));
   for (i = 0; i < sizeof(arr); i ++) {
      bit = arr[i];
      if (typeof(bit) == T_ARRAY) {
	 switch(bit[0]) {
	 case VAL_OBJREF:
	    if (obarr[bit[1]-1] == nil) {
	       /* object was destructed */
	       result[i] = "nil /* defunct */";
	    } else {
	       result[i] = "${" + name(obarr[bit[1]-1]) + "}";
	    }
	    break;
	 case VAL_ARGREF:
	    if (is_simple(bit[1])) {
	       result[i] = "$" + bit[1];
	    } else {
	       result[i] = "$(" + bit[1] + ")";
	    }
	    break;
	 case VAL_SLEEP:
	    convert_sleep(bit);
	    if (sizeof(bit) == 3) {
	       result[i] = "$delay(" + expand_to_source(bit[2]) +
		  ", FALSE, " + expand_to_source(bit[1]) + ");";
	    } else {
	       result[i] = "$delay(" + expand_to_source(bit[2]) +
		  ", " + expand_to_source(bit[3]) + ", " +
		  expand_to_source(bit[1]) + ");";
	    }
	    break;
	 case VAL_ARGLIST: {
	    string *tmp;
	    int j;

	    tmp = allocate(sizeof(bit)-1);
	    for (j = 1; j < sizeof(bit); j ++) {
	       if (is_simple(bit[j][0])) {
		  tmp[j-1] = "$" + bit[j][0] + ": " +
		     expand_to_source(bit[j][1]);
	       } else {
		  tmp[j-1] = "$(" + bit[j][0] + "): " +
		     expand_to_source(bit[j][1]);
	       }
	    }
	    result[i] = implode(tmp, ", ");
	    break;
	 }
	 case VAL_PROPGET:
	    result[i] = expand_to_source(bit[1]) + "." + fixprop(bit[2]);
	    break;
	 case VAL_PROPSET:
	    result[i] = expand_to_source(bit[1]) + "." + fixprop(bit[2]) +
	       " = " + expand_to_source(bit[3]);
	    break;
	 case VAL_PROPMOD:
	    result[i] = expand_to_source(bit[1]) + "." + fixprop(bit[2]) +
	       " " + bit[3] + "= " + expand_to_source(bit[4]);
	    break;
	 case VAL_PROPPOSTFIX:
	    result[i] = expand_to_source(bit[1]) + "." + fixprop(bit[2]) +
	       bit[3];
	    break;
	 case VAL_PROPPREFIX:
	    result[i] = bit[3] + expand_to_source(bit[1]) + "." +
	       fixprop(bit[2]);
	    break;
	 case VAL_CONSTANT:
	    result[i] = "constant " + bit[1] + " = " +
	       expand_to_source(constmap[bit[1]]) + ";";
	    break;
	 case VAL_LABELCALL: {
	    string *tmp;
	    int j;

	    tmp = allocate(sizeof(bit)-3);
	    for (j = 3; j < sizeof(bit); j ++) {
	       if (is_simple(bit[j][0])) {
		  tmp[j-3] = "$" + bit[j][0] + ": " +
		     expand_to_source(bit[j][1]);
	       } else {
		  tmp[j-3] = "$(" + bit[j][0] + "): " +
		     expand_to_source(bit[j][1]);
	       }
	    }
	    result[i] = (bit[1] ? bit[1] : "") + "::" + bit[2] + "(" +
	       implode(tmp, ", ") + ")";
	    break;
	 }
	 case VAL_SAM: {
	    string tmp;

	    tmp = generate_pcdata(samarr[bit[1]]);
	    tmp = replace_strings(tmp, "\"", "\\\"");
	    result[i] = "$\"" + tmp + "\"";
	    break;
	 }
	 case VAL_LABELREF:
	    result[i] = bit[1] + "::";
	    break;
	 }
      } else {
	 result[i] = bit;
      }
   }
   return implode(result, "");
}

string query_source() {
   return expand_to_source(srcarr);
}

mixed evaluate(mixed args...) { return get_program()->evaluate(args...); }


void convert_sleep(mixed *bit) {
   if (sizeof(bit) == 4 && typeof(bit[3]) == T_STRING) {
      /* backwards compatible conversion */
      SysLog("MERRY: Converting bit[3] = " + dump_value(bit[3]));
      bit[3] = ({ bit[3] });
   }
   if (typeof(bit[2]) == T_STRING) {
      /* backwards compatible conversion */
      SysLog("MERRY: Converting bit[2] = " + dump_value(bit[2]));
      bit[2] = ({ bit[2] });
   }
   if (typeof(bit[1]) == T_STRING) {
      SysLog("MERRY: Converting bit[1] = " + dump_value(bit[1]));
      if (bit[1][0] != '\"') {
	 /* backwards compatible conversion */
	 bit[1] = "\"" + bit[1] + "\"";
      }
      bit[1] = ({ bit[1] });
   }
}

string expand_to_lpc(mixed *arr) {
   string *result;
   mixed bit;
   int i;

   result = allocate(sizeof(arr));

   for (i = 0; i < sizeof(arr); i ++) {
      bit = arr[i];
      if (typeof(bit) == T_ARRAY) {
	 switch(bit[0]) {
	 case VAL_OBJREF:
	    result[i] = "obref(" + (bit[1]-1) + ")";
	    break;
	 case VAL_ARGREF:
	    result[i] = "(args[\"" + lower_case(bit[1]) + "\"])";
	    break;
	 case VAL_SLEEP: {
	    string label, ret;

	    convert_sleep(bit);

	    if (sizeof(bit) == 3) {
	       ret = "FALSE";
	    } else {
	       ret = expand_to_lpc(bit[3]);
	    }
	    label = expand_to_lpc(bit[1]);
	    result[i] = "{ do_delay(mode, signal, " +
	       expand_to_lpc(bit[2]) + ", " + label + "); return " +
	       ret + "; case " + label + ": ; } ";
	    break;
	 }
	 case VAL_ARGLIST: {
	    string *tmp;
	    int j;

	    tmp = allocate(sizeof(bit)-1);
	    for (j = 1; j < sizeof(bit); j ++) {
	       tmp[j-1] = "\"" + bit[j][0] + "\", " + expand_to_lpc(bit[j][1]);
	    }
	    result[i] = "({ " + implode(tmp, ", ") + " })";
	    break;
	 }
	 case VAL_PROPGET:
	    result[i] = "Get(" + expand_to_lpc(bit[1]) +
	       ", \"" + bit[2] + "\")";
	    break;
	 case VAL_PROPSET:
	    result[i] = "Set(" + expand_to_lpc(bit[1]) +
	       ", \"" + bit[2] + "\", " +
	       expand_to_lpc(bit[3]) + ")";
	    break;
	 case VAL_PROPMOD:
	    result[i] = "Set(" + expand_to_lpc(bit[1]) +
	       ", \"" + bit[2] + "\", Get(" + expand_to_lpc(bit[1]) +
	       ", \"" + bit[2] + "\") " + bit[3][0 .. 0] + " " +
	       expand_to_lpc(bit[4]) + ")";
	    break;
	 case VAL_PROPPOSTFIX:
	 case VAL_PROPPREFIX:
	    result[i] = "Set(" + expand_to_lpc(bit[1]) +
	       ", \"" + bit[2] + "\", Get(" + expand_to_lpc(bit[1]) +
	       ", \"" + bit[2] + "\") " + bit[3][0 .. 0] + " 1)";
	    break;
	 case VAL_CONSTANT:
	    result[i] = "";	/* top of file */
	    break;
	 case VAL_LABELCALL: {
	    string *tmp;
	    int j;

	    tmp = allocate(sizeof(bit)-3);
	    for (j = 3; j < sizeof(bit); j ++) {
	       tmp[j-3] = "\"" + bit[j][0] + "\", " + expand_to_lpc(bit[j][1]);
	    }
	    if (bit[1]) {
	       result[i] = "LabelCall(\"" + lower_case(bit[1]) + "\", \"" +
		  lower_case(bit[2]) + "\", " +
		  "({ " + implode(tmp, ", ") + " }))";
	    } else {
	       result[i] = "Call(this, \"" + lower_case(bit[2]) + "\", " +
		  "({ " + implode(tmp, ", ") + " }))";
	    }
	    break;
	 }
	 case VAL_SAM: {
	    result[i] = "samref(" + bit[1] + ")";
	    break;
	 }
	 case VAL_LABELREF:
	    result[i] = "LabelRef(\"" + bit[1] + "\")";
	    break;
	 default:
	    error("unknown merry nugget type: " + dump_value(bit));
	 }
      } else if (typeof(bit) == T_STRING) {
	 result[i] = bit;
      } else {
	 error("bad type of merry nugget: " + dump_value(bit));
      }
   }
   return implode(result, "");
}

#include "denew.h"

atomic
object get_program() {
   if (!source) {
      /*
       * Fixing already existing Merry nodes, hopefully.
       */
      int    i, sz;
      string result, *constants;

      /*
       * Generate LPC from Merry code:
       */
      result = expand_to_lpc(srcarr);
      result = replace_strings(result,
			       "ActArg", "Act",
			       "CallArg", "Call");

      if (!constmap) {
	 constmap = ([ ]);
      }

      constants = map_indices(constmap);
      for (i = 0, sz = sizeof(constants); i < sz; i ++) {
	 constants[i] = "# define " + constants[i] + " " +
	    expand_to_lpc(constmap[constants[i]]);
      }

      source = 
	 "inherit \"/usr/SkotOS/lib/merrynode\";\n" +
	 "# include \"/usr/SkotOS/include/merrynode.h\"\n" +
	 implode(constants, "\n") + "\n" +
	 "mixed merry(string mode, string signal, string label) {\n" +
	 "    switch(label) { case \"virgin\": {\n" + result + "}\n" +
	 "    }\n" +
	 "}\n";

      hash = to_hex(hash_md5(source));
   }

   if (!program) {
      string name;

      if (!hash) {
	 error("Internal error: Missing MD5 hash for Merry program");
      }

      name    = MERRYNODE(hash);
      program = find_object(name);

      if (!program) {
	 if (!source) {
	    error("Internal error: Missing source for Merry program");
	 }

	 program = compile_object(name, denew(source));
	 SYS_MERRY->new_merry_node(program);
      }
   }

   /* Configure the object with the right constants. */
   program->set_object_array(obarr);
   program->set_sam_array(samarr);
   return program;
}

mixed
query_property(string prop) {
   if (prop && sscanf(lower_case(prop), "merry:%s", prop)) {
      if (prop == "source") {
	 return query_source();
      }
      error("unknown merry: property");
   }
   error("only merry: properties served here");
}
