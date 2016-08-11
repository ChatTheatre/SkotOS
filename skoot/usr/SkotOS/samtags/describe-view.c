# include <mapargs.h>

inherit "/lib/string";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   mixed what;
   string res;

   args += local;

   what = args["view"];
   if (!what) {
      return TRUE;
   }

   if (args["looker"]) {
       res = describe_item(what, args["looker"], args["type"], "look");
   } else if (args["actor"]) {
       res = describe_item(what, args["actor"], args["type"], "look");
   } else {
       error("No 'looker' or 'actor' parameter provided");
   }

   res = strip_left(res);

   if (local["cap"]) {
      res = capitalize(res);
   }
   if (local["nocr"] && res[strlen(res)-1] == '\n') {
      res = res[.. strlen(res)-2];
   }
   if (local["uncap"] && res[0] >= 'A' && res[0] <= 'Z') {
      res[0] = (res[0] - 'A') + 'a';
   }

   Output(res);
   return TRUE;
}
