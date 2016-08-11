private inherit "/base/lib/urcalls";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/lib/type";
inherit "/lib/string";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   string res;
   mixed what, actor, looker;

   AssertLocal("what", "describe");

   what = local["what"];
   actor = local["actor"];
   looker = local["looker"];

   if (IsNRef(actor)) {
      actor = NRefOb(actor);
   }

   if (IsNRef(looker)) {
      looker = NRefOb(looker);
   }

   if (typeof(what) == T_ARRAY) {
      res = describe_many(what, actor, looker);
   } else {
      res = describe_one(what, actor, looker);
   }

   if (res && strlen(res)) {
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
