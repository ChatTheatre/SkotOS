inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   return !!args["HTML"];
}


