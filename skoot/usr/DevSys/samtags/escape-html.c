private inherit "/lib/string";

inherit "/usr/XML/lib/entities";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   string str, out;
   int i, j, newline;

   str = unsam(content, args);
   out = str;

   i = j = 0;

   while (i < strlen(str)) {
      while (strlen(out) - j < 10) {
	 out += str;
      }
      switch(str[i]) {
      case '<':
	 newline = FALSE;
	 out[j ++] = '&';
	 out[j ++] = 'l';
	 out[j ++] = 't';
	 out[j ++] = ';';
	 break;
      case '>':
	 newline = FALSE;
	 out[j ++] = '&';
	 out[j ++] = 'g';
	 out[j ++] = 't';
	 out[j ++] = ';';
	 break;
      case '"':
	 newline = FALSE;
	 out[j ++] = '&';
	 out[j ++] = 'q';
	 out[j ++] = 'u';
	 out[j ++] = 'o';
	 out[j ++] = 't';
	 out[j ++] = ';';
	 break;
      case '&':
	 newline = FALSE;
	 out[j ++] = '&';
	 out[j ++] = 'a';
	 out[j ++] = 'm';
	 out[j ++] = 'p';
	 out[j ++] = ';';
	 break;
      case '\n':
	 newline = TRUE;
	 if (local["simple"]) {
	    out[j ++] = str[i];
	    break;
	 }
	 out[j ++] = '<';
	 out[j ++] = 'b';
	 out[j ++] = 'r';
	 out[j ++] = '>';
	 out[j ++] = '\n';
	 break;
      case ' ':
	 if (local["simple"]) {
	    out[j ++] = str[i];
	    break;
	 }
	 if (newline) {
	    out[j ++] = '&';
	    out[j ++] = 'n';
	    out[j ++] = 'b';
	    out[j ++] = 's';
	    out[j ++] = 'p';
	    out[j ++] = ';';
	    break;
	 }
	 /* fall through */
      default:
	 newline = FALSE;
	 out[j ++] = str[i];
	 break;
      }
      i ++;
   }

   Output(out[.. j-1]);
   return TRUE;
}
