inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/lib/string";
inherit "/lib/type";

# include <SAM.h>
# include <type.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Describe the current preposition of an object.";
   case "required":
      return ([ "what": "The object whose preposition you want." ]);
   case "optional":
      return ([ "cap": "Whether to capitalize the resulting output or not." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed what;
   string res;

   args += local;

   AssertLocal("what", "describeprep");

   what = args["what"];

   if (typeof(what) != T_OBJECT) {
      XDebug("XX - $describeprep what: " + formatted_typeof(what));
      error("describeprep requires an object");
   }

   XDebug("XX - $describeprep prep: " + 
	  formatted_typeof(what->describe_preposition()));

   res = what->describe_preposition();

   if (local["cap"]) {
      res = capitalize(res);
   }

   Output(res);
   return TRUE;
}

