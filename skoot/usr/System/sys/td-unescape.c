/*
**	Simply handle requests from ../lib/textdata.c
*/

inherit "/usr/System/lib/sys_auto";

string unescape_string(string value) {
   mixed *res;

   if (!strlen(value)) {
      return value;
   }

   res = parse_string(
      "regular = /[^\\\\]+/\n" +
      "binary = /\\\\[0-3][0-7][0-7]/\n" +
      "whatnot = /\\\\./\n" +

      "String: String Bit ? tdu_add\n" +
      "String: Bit\n" +
      "Bit: regular\n" +
      "Bit: binary ? tdu_unescape\n" +
      "Bit: '\\\\n' ? tdu_unescape\n" +
      "Bit: '\\\\r' ? tdu_unescape\n" +
      "Bit: '\\\\t' ? tdu_unescape\n" +
      "Bit: '\\\\\"' ? tdu_unescape\n" +
      "Bit: '\\\\\\\\' ? tdu_unescape\n" +
      "",
      value);
   if (res) {
      return res[0];
   }
}

static
string *tdu_unescape(string *tree) {
   switch(tree[0]) {
   case "\\000":      return ({ "\000" });
   case "\\001":      return ({ "\001" });
   case "\\002":      return ({ "\002" });
   case "\\003":      return ({ "\003" });
   case "\\004":      return ({ "\004" });
   case "\\005":      return ({ "\005" });
   case "\\006":      return ({ "\006" });
   case "\\007":      return ({ "\007" });
   case "\\010":      return ({ "\010" });
   case "\\011":      return ({ "\011" });
   case "\\012":      return ({ "\012" });
   case "\\013":      return ({ "\013" });
   case "\\014":      return ({ "\014" });
   case "\\015":      return ({ "\015" });
   case "\\016":      return ({ "\016" });
   case "\\017":      return ({ "\017" });
   case "\\020":      return ({ "\020" });
   case "\\021":      return ({ "\021" });
   case "\\022":      return ({ "\022" });
   case "\\023":      return ({ "\023" });
   case "\\024":      return ({ "\024" });
   case "\\025":      return ({ "\025" });
   case "\\026":      return ({ "\026" });
   case "\\027":      return ({ "\027" });
   case "\\030":      return ({ "\030" });
   case "\\031":      return ({ "\031" });
   case "\\032":      return ({ "\032" });
   case "\\033":      return ({ "\033" });
   case "\\034":      return ({ "\034" });
   case "\\035":      return ({ "\035" });
   case "\\036":      return ({ "\036" });
   case "\\037":      return ({ "\037" });
   case "\\\\":		return ({ "\\" });
   case "\\\"":		return ({ "\"" });
   case "\\n":		return ({ "\n" });
   case "\\r":		return ({ "\r" });
   case "\\t":		return ({ "\t" });
   }
   error("not expecting: " + tree[0]);
}

static
string *tdu_add(string *tree) {
   return ({ implode(tree, "") });
}
