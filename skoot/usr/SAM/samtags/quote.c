inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   if (args["HTML"]) {
      /* TODO: actually do something here */
      tf_elt(content, context, args);
      return TRUE;
   }
}
