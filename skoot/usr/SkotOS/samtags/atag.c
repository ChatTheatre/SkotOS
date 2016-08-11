inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"


int tag(mixed content, object context, mapping local, mapping args) {
   AssertLocal("tag", "atag");
   Output("\000\003" + local["tag"] + "\001");
   tf_elt(content, context, args);
   Output("\002");

   return TRUE;
}
