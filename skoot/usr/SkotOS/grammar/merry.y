# include <type.h>
# include <SAM.h>
# include <XML.h>

# include "/usr/SkotOS/include/merry.h"

private inherit "/lib/string";
private inherit "/usr/XML/lib/xmlparse";

mapping labels;

mapping obmap;
object *obarr;

mapping constmap;

SAM *samarr;

static
void prepare() {
   labels = ([ ]);
   obmap = ([ ]);
   obarr = ({ });
   constmap = ([ ]);
   samarr = ({ });
}

/* go through args and implode consecutive strings */
mixed *imp(mixed *args) {
   int ix, lix;

   lix = -1;	/* initialize lix to 'no current sequence' */

   while (TRUE) {
      if (ix < sizeof(args) && typeof(args[ix]) == T_STRING) {
	 /* this is the first string in a potential sequence */
	 if (lix < 0) {
	    lix = ix;
	 }
	 ix ++;
	 continue;
      }
      if (lix < 0 || lix == ix-1) {
	 /* no potential sequence */
	 if (ix == sizeof(args)) {
	    return args;
	 }
	 ix ++;
	 lix = -1;
	 continue;
      }
      /* hurray, a sequence */	
      args =
	 args[.. lix-1] +
	 ({ implode(args[lix .. ix-1], "") }) +
	 args[ix ..];
      if (ix == sizeof(args)) {
	 return args;
      }
      ix = lix;
   }
}

/* deal with the assignment rules */

mixed *assign(mixed *tree) {
   if (sizeof(tree[0]) != 1 || typeof(tree[0][0]) != T_ARRAY ||
       tree[0][0][0] != VAL_PROPGET) {
      return imp(tree[0] + tree[1 ..]);
   }
   /* ok: foo.bar used as a lvalue */
   if (tree[2] == "=") {
      /* it's a straight assignment */
      return ({ ({ VAL_PROPSET, tree[0][0][1], tree[0][0][2], tree[4 ..] }) });
   }
   /* or it's a mod */	
   return ({ ({ VAL_PROPMOD, tree[0][0][1], tree[0][0][2],
		   tree[2][.. strlen(tree[2]) - 2], tree[4 ..] }) });   
}

/* deal with prefix and postfix lvalues */

mixed *postfix(mixed *tree) {
   if (sizeof(tree) != 3 || typeof(tree[0]) != T_ARRAY ||
       tree[0][0] != VAL_PROPGET) {
      return imp(tree);
   }
   return ({ ({ VAL_PROPPOSTFIX, tree[0][1], tree[0][2], tree[2] }) });
}

mixed *prefix(mixed *tree) {
   int last;

   last = sizeof(tree)-1;
   if (sizeof(tree) != 3 || typeof(tree[last]) != T_ARRAY ||
       tree[last][0] != VAL_PROPGET) {
      return imp(tree);
   }
   return ({ ({ VAL_PROPPREFIX, tree[last][1], tree[last][2], tree[0] }) });
}

GrammarBegin

space = /(([ \t\n\r\v\f]+)|(\/\*([^*]|\*+[^*/])*\*+\/))+/

FLT_CONST = /[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?/
FLT_CONST = /[0-9]+([eE][+-]?[0-9]+)?/

INT_CONST = /[1-9][0-9]*/
INT_CONST = /0[0-7]*/
INT_CONST = /0[Xx][0-9a-fA-F]+/
INT_CONST = /'[^\\]'/
INT_CONST = /'\\[^0-7xX]'/
INT_CONST = /'\\[0-7][0-7]?[0-7]?'/
INT_CONST = /'\\[xX][0-9a-fA-F][0-9a-fA-F]?'/
DOT_DOT = /\.\./
DOT_DOT_DOT = /\.\.\./
STRING_CONST = /\"([^\\\"\n]*(\\.)*)*\"/
IDENTIFIER = /[a-zA-Z_][a-zA-Z_0-9]*/

MERRY_REF = /$\([^\(\)\n\r]*\)/
MOB_CONST = /${[^\}\n\r]*}/

SAM = /$\"([^\\\"\n\r]*(\\\")*)*\"/

catchall = nomatch

program: S opt_dcltr_or_stmt_list S {{
   return ({ imp(tree), obarr, constmap, samarr });
}}

program: S opt_dcltr_or_stmt_list S trash {{
   return ({ imp(tree) });
}}

S: {{ return ({ "" }); }}
S: space

constant_declaration: 'constant' S IDENTIFIER S '=' S list_exp S ';' {{
   if (constmap[tree[2]]) {
      error("constant '" + tree[2] + "' redefined");
   }
   constmap[tree[2]] = tree[6 .. sizeof(tree)-3];
   return ({ ({ VAL_CONSTANT, tree[2] }) });
}}

local_data_declaration: type_specifier S list_dcltr S ';' ? imp

type_specifier: IDENTIFIER

opt_star_list:
opt_star_list: star_list

star_list: '*'
star_list: star_list S '*' ? imp

data_dcltr: opt_star_list S IDENTIFIER ? imp

dcltr: data_dcltr

list_dcltr: dcltr
list_dcltr: list_dcltr S ',' S dcltr ? imp

opt_dcltr_or_stmt_list:
opt_dcltr_or_stmt_list: dcltr_or_stmt_list ? imp

dcltr_or_stmt_list: dcltr_or_stmt ? imp
dcltr_or_stmt_list: dcltr_or_stmt_list S dcltr_or_stmt ? imp

dcltr_or_stmt: local_data_declaration
dcltr_or_stmt: constant_declaration
dcltr_or_stmt: stmt

stmt: list_exp S ';' ? imp
stmt: compound_stmt
stmt: 'if' S '(' S list_exp S ')' S stmt ? imp
stmt: 'if' S '(' S list_exp S ')' S stmt S 'else' S stmt ? imp
stmt: 'do' S stmt S 'while' S '(' S list_exp S ')' S ';' ? imp
stmt: 'while' S '(' S list_exp S ')' S stmt ? imp
stmt: 'for' S '(' S opt_list_exp S ';' S opt_list_exp S ';' S opt_list_exp S ')' S stmt ? imp
stmt: 'rlimits' S '(' S list_exp S ';' S list_exp S ')' S compound_stmt {{
   error("rlimits are not used in merry code");
}}
stmt: 'catch' S compound_stmt S opt_caught_stmt ? imp
stmt: 'switch' S '(' S list_exp S ')' S compound_stmt ? imp
stmt: 'case' S exp S ':' S stmt ? imp
stmt: 'case' S exp S DOT_DOT S exp S ':' S stmt ? imp
stmt: 'default' S ':' S stmt ? imp
stmt: 'break' S ';' ? imp
stmt: 'continue' S ';' ? imp
stmt: 'return' S opt_list_exp S ';' ? imp
stmt: ';'


structured_list_exp: exp {{
   return ({ ({ tree }) });
}}
structured_list_exp: structured_list_exp S ',' S exp {{
   return ({ tree[0] + ({ tree[4 ..] }) });
}}

opt_structured_list_exp: {{
   return ({ ({ }) });
}}
opt_structured_list_exp: structured_list_exp


stmt: '$' S IDENTIFIER S '(' S opt_structured_list_exp S ')' S ';' {{
   if (tree[2] == "delay") {
      mixed *args, label, retval, delay;

      args = tree[6];
      switch(sizeof(args)) {
      case 0:
	 error("empty $delay() statement");
      case 3:
	 label = args[2];
	 /* fall through */
      case 2:
	 retval = args[1];
	 /* fall through */
      case 1:
	 delay = args[0];
	 break;
      default:
	 error("too many arguments to $delay()");
      }
      while (!label || labels[label]) {
	 label = "\"0000\"";
	 label[1] = hex_digit(random(16));
	 label[2] = hex_digit(random(16));
	 label[3] = hex_digit(random(16));
	 label[4] = hex_digit(random(16));
      }
      if (retval) {
	 return ({ ({ VAL_SLEEP, label, delay, retval }) });
      }
      return ({ ({ VAL_SLEEP, label, delay }) });
   }
   error("unknown merry function: $" + tree[2]);
}}


compound_stmt: '{' S opt_dcltr_or_stmt_list S '}' ? imp

opt_caught_stmt:
opt_caught_stmt: ':' S stmt ? imp

primary_p1_exp: sam_exp

sam_exp: SAM {{
   string str;
   SAM sam;

   str = tree[0][2 .. strlen(tree[0])-2];
   /* replace escaped quotes with actual quotes */
   str = replace_strings(str, "\\\"", "\"");

   sam = parse_xml(str);
   if (typeof(sam) == T_STRING ||
       query_colour(sam) == COL_SAMREF ||
       query_colour(sam) == COL_ELEMENT) {
      sam = NewXMLPCData(({ sam }));
   }
   samarr += ({ sam });

   return ({ ({ VAL_SAM, sizeof(samarr)-1 }) });
}}

primary_p1_exp: 'nil'
primary_p1_exp: INT_CONST
primary_p1_exp: FLT_CONST
primary_p1_exp: STRING_CONST
primary_p1_exp: '(' S '{' S opt_arg_list_comma S '}' S ')' ? imp
primary_p1_exp: '(' S '[' S opt_assoc_arg_list_comma S ']' S ')' ? imp
primary_p1_exp: IDENTIFIER

primary_p1_exp: MOB_CONST {{
   string name;
   object ob;

   name = strip(tree[0][2 .. strlen(tree[0])-2]);
   if (!strlen(name)) {
      error("empty ${} constant");
   }
   ob = find_object(name);
   if (!ob) {
      error("cannot resolve ${" + name + "}");
   }
   if (!obmap[name]) {
      obarr += ({ ob });
      obmap[name] = sizeof(obarr);
   }
   return ({ ({ VAL_OBJREF, obmap[name] }) });
}}

primary_p1_exp: '(' S list_exp S ')' ? imp

structured_assoc_exp: '$' IDENTIFIER S ':' S exp {{
   return ({ tree[1], tree[5..] });
}}

structured_assoc_exp: MERRY_REF S ':' S exp {{
   return ({ tree[0][2 .. strlen(tree[0])-2], tree[4..] });
}}

opt_structured_assoc_arg_list: structured_assoc_arg_list
opt_structured_assoc_arg_list: {{ return ({ ({ }) }) }}

structured_assoc_arg_list: structured_assoc_exp {{
   return ({ ({ tree }) });
}}
structured_assoc_arg_list: structured_assoc_arg_list S ',' S structured_assoc_exp {{
   return ({ tree[0] + ({ tree[4 ..] }) });
}}

primary_p1_exp: IDENTIFIER S '(' S opt_arg_list S ')' ? imp

primary_p1_exp: IDENTIFIER S '(' S arg_list S ',' S opt_structured_assoc_arg_list S ')' {{
   int ix;

   /* we have to count backwards, since arg_list is of variable length */

   ix = sizeof(tree) - 3;
   return tree[.. ix-1] + ({ ({ VAL_ARGLIST }) + tree[ix] }) + tree[ix+1 ..];
}}

primary_p1_exp: IDENTIFIER S '::' {{
   return ({ ({ VAL_LABELREF, tree[0] }) });
}}

primary_p1_exp: '::' S IDENTIFIER S '(' S opt_structured_assoc_arg_list S ')' {{
   return ({ ({ VAL_LABELCALL, nil, tree[2] }) + tree[6] });
}}

primary_p1_exp: IDENTIFIER S '::' S IDENTIFIER S '(' S opt_structured_assoc_arg_list S ')' {{
   return ({ ({ VAL_LABELCALL, tree[0], tree[4] }) + tree[8] });
}}

primary_p1_exp: 'catch' S '(' S list_exp S ')' ? imp
primary_p1_exp: primary_p2_exp S '->' S IDENTIFIER S '(' S opt_arg_list S ')' {{
   error("The operator -> is not allowed in Merry");
}}

primary_p1_exp: primary_p2_exp S '.' S IDENTIFIER {{
   return ({ ({ VAL_PROPGET, tree[.. sizeof(tree)-5],
		   tree[sizeof(tree)-1] }) });
}}

primary_p1_exp: primary_p2_exp S '.' S STRING_CONST {{
   int last;

   last = sizeof(tree)-1;
   return ({ ({ VAL_PROPGET, tree[.. sizeof(tree)-5],
		   tree[last][1 .. strlen(tree[last])-2] }) });
}}

primary_p1_exp: '$' IDENTIFIER {{
   return ({ ({ VAL_ARGREF, tree[1] }) });
}}

primary_p1_exp: MERRY_REF {{
   return ({ ({ VAL_ARGREF, tree[0][2 .. strlen(tree[0])-2] }) });
}}


primary_p2_exp: primary_p1_exp
primary_p2_exp: primary_p2_exp S '[' S list_exp S ']' ? imp
primary_p2_exp: primary_p2_exp S '[' S opt_list_exp S DOT_DOT S opt_list_exp S ']' ? imp

postfix_exp: primary_p2_exp
postfix_exp: postfix_exp S '++' ? postfix
postfix_exp: postfix_exp S '--' ? postfix

prefix_exp: postfix_exp
prefix_exp: '++' S cast_exp ? prefix
prefix_exp: '--' S cast_exp ? prefix
prefix_exp: '-' S cast_exp ? imp
prefix_exp: '+' S cast_exp ? imp
prefix_exp: '!' S cast_exp ? imp
prefix_exp: '~' S cast_exp ? imp

cast_exp: prefix_exp
cast_exp: '(' S type_specifier S opt_star_list S ')' S cast_exp ? imp

mult_oper_exp: cast_exp
mult_oper_exp: mult_oper_exp S '*' S cast_exp ? imp
mult_oper_exp: mult_oper_exp S '/' S cast_exp ? imp
mult_oper_exp: mult_oper_exp S '%' S cast_exp ? imp

add_oper_exp: mult_oper_exp
add_oper_exp: add_oper_exp S '+' S mult_oper_exp ? imp
add_oper_exp: add_oper_exp S '-' S mult_oper_exp ? imp

shift_oper_exp: add_oper_exp
shift_oper_exp: shift_oper_exp S '<<' S add_oper_exp ? imp
shift_oper_exp: shift_oper_exp S '>>' S add_oper_exp ? imp

rel_oper_exp: shift_oper_exp
rel_oper_exp: rel_oper_exp S '<' S shift_oper_exp ? imp
rel_oper_exp: rel_oper_exp S '>' S shift_oper_exp ? imp
rel_oper_exp: rel_oper_exp S '<=' S shift_oper_exp ? imp
rel_oper_exp: rel_oper_exp S '>=' S shift_oper_exp ? imp

equ_oper_exp: rel_oper_exp
equ_oper_exp: equ_oper_exp S '==' S rel_oper_exp ? imp
equ_oper_exp: equ_oper_exp S '!=' S rel_oper_exp ? imp

bitand_oper_exp: equ_oper_exp
bitand_oper_exp: bitand_oper_exp S '&' S equ_oper_exp ? imp

bitxor_oper_exp: bitand_oper_exp
bitxor_oper_exp: bitxor_oper_exp S '^' S bitand_oper_exp ? imp

bitor_oper_exp: bitxor_oper_exp
bitor_oper_exp: bitor_oper_exp S '|' S bitxor_oper_exp ? imp

and_oper_exp: bitor_oper_exp
and_oper_exp: and_oper_exp S '&&' S bitor_oper_exp ? imp

or_oper_exp: and_oper_exp
or_oper_exp: or_oper_exp S '||' S and_oper_exp ? imp

cond_exp: or_oper_exp
cond_exp: or_oper_exp S '?' S list_exp S ':' S cond_exp ? imp

wrapped_cond_exp: cond_exp {{
   return ({ tree });
}}

exp: cond_exp
exp: wrapped_cond_exp S '=' S exp ? assign
exp: wrapped_cond_exp S '+=' S exp ? assign
exp: wrapped_cond_exp S '-=' S exp ? assign
exp: wrapped_cond_exp S '*=' S exp ? assign
exp: wrapped_cond_exp S '/=' S exp ? assign
exp: wrapped_cond_exp S '%=' S exp ? assign
exp: wrapped_cond_exp S '<<=' S exp ? assign
exp: wrapped_cond_exp S '>>=' S exp ? assign
exp: wrapped_cond_exp S '&=' S exp ? assign
exp: wrapped_cond_exp S '^=' S exp ? assign
exp: wrapped_cond_exp S '|=' S exp ? assign

list_exp: exp
list_exp: list_exp S ',' S exp ? imp

opt_list_exp:
opt_list_exp: list_exp

arg_list: exp
arg_list: arg_list S ',' S exp ? imp

opt_arg_list:
opt_arg_list: arg_list
opt_arg_list: arg_list S DOT_DOT_DOT ? imp

opt_arg_list_comma:
opt_arg_list_comma: arg_list
opt_arg_list_comma: arg_list S ',' ? imp

assoc_exp: exp S ':' S exp ? imp

assoc_arg_list: assoc_exp
assoc_arg_list: assoc_arg_list S ',' S assoc_exp ? imp

opt_assoc_arg_list_comma:
opt_assoc_arg_list_comma: assoc_arg_list
opt_assoc_arg_list_comma: assoc_arg_list S ',' ? imp

trash:	trash_list S {{
   return ({ });
}}

trash_list:	trash_bit
trash_list:	trash_list S trash_bit

trash_bit:	FLT_CONST
trash_bit:	INT_CONST
trash_bit:	DOT_DOT
trash_bit:	DOT_DOT_DOT
trash_bit:	STRING_CONST
trash_bit:	IDENTIFIER
trash_bit:	MERRY_REF
trash_bit:	MOB_CONST
trash_bit:      SAM
trash_bit:	__STRINGS__
trash_bit:	catchall

GrammarEnd
