private inherit "/base/lib/urcalls";
private inherit "/lib/string";
private inherit "/lib/type";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string det, res;
   mixed what, actor, looker;

   AssertLocal("what", "describe-holder");

   what = local["what"];
   actor = local["actor"];
   looker = local["looker"];

   if (what->query_holder()) {
      what = what->query_holder();
   }

   res = describe_one(what, actor, looker);

   if (local["poss"] && res && strlen(res)) {
      switch(res[strlen(res)-1]) {
      case 's': case 'z': case 'x':
               res += "'";
      break;
      default:
	       res += "'s";
      }
   }

   if (local["cap"]) {
      res = capitalize(res);
   }

   Output(res);

   return TRUE;
}

