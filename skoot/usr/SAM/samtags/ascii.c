inherit "/lib/string";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   Output(replace_html(mixed_to_ascii(local["val"], TRUE)));
   return TRUE;
}
