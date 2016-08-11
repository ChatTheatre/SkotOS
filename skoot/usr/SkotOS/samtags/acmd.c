inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   string tag;

   AssertLocal("cmd", "atag");
   tag = local["tag"];
   if (!tag) {
      tag = "command";
   }
   Output("\000\004" + tag + "|" + local["cmd"] + "\001");
   tf_elt(content, context, args);
   Output("\002");

   return TRUE;
}
