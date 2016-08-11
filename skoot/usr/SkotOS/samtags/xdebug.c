private inherit "/base/lib/urcalls";
private inherit "/lib/string";
private inherit "/lib/type";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return FALSE;
   case "description":
      return "Undocumented at the moment.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed what;

   AssertLocal("what", "xdebug");

   what = local["what"];

   XDebug("XDebug SAM tag what: " + formatted_typeof(what));

   Output(formatted_typeof(what));

   return TRUE;
}

