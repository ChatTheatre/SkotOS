inherit "/usr/XML/lib/entities";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   Output(entify_string(unsam(content, args), '\'', '\"'));
   return TRUE;
}
