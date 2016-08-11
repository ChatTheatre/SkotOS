/*
**	Simply handle requests from ../lib/textdata.c
*/

inherit "/usr/System/lib/sys_auto";

string escape_string(string value) {
   mixed *res;

   if (!strlen(value)) {
      return value;
   }
   res = parse_string(
      "regular = /[^\000-\037\\\\\"]+/\n" +
      "binary = /[\000-\037]/\n" +
      "slash = /\\\\/\n" +
      "quote = /\"/\n" +

      "String: String Bit ? tde_add\n" +
      "String: Bit\n" +
      "Bit: regular\n" +
      "Bit: binary ? tde_escape\n" +
      "Bit: quote ? tde_escape\n" +
      "Bit: slash ? tde_escape\n" +
      "",
      value);
   if (res) {
      return res[0];
   }
   error("parse error: " + dump_value(value[0 .. 0]));
   error("parse error");
}

static
string *tde_escape(string *tree) {
   int c;

   switch(c = tree[0][0]) {
   case 0 .. 7:
      return ({ "\\00" + c });
   case 8: case 11: case 12: case 14: case 15:
      return ({ "\\01" + (c-8) });
   case 16 .. 23:
      return ({ "\\02" + (c-16) });
   case 24 .. 31:
      return ({ "\\03" + (c-24) });
   case '\n':
      return ({ "\\n" });
   case '\t':
      return ({ "\\t" });
   case '\r':
      return ({ "\\r" });
   case '\"':
      return ({ "\\\"" });
   case '\\':
      return ({ "\\\\" });
   }
   error("not expecting: " + c);
}

static
string *tde_add(string *tree) {
   return ({ implode(tree, "") });
}
