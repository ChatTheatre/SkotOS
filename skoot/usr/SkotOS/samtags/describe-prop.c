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
      return TRUE;
   case "description":
      return "Describe the object that is the content of the property.";
   case "required":
      return ([
	 "what": "The name of the property.",
	 "this": "The object in which you want to query the property."
      ]);
   case "optional":
      return ([
	 "def": "Whether to describe it in a definite way or not.",
	 "cap": "Whether to capitalize the resulting output."
      ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   object this;
   string res, what;
   mixed property, actor;

   AssertLocal("what", "describe-prop");
   AssertArg("this", "describe-prop");

   what = local["what"];
   this = args["this"];
   property = this->query_property(what);

   if (typeof(property) != T_STRING) {
      Output(dump_value(property));
      return TRUE;
   }

   if (local["def"]) {
      res = "the " + property;
   } else {
      switch(property[0]) {
      case 'a': case 'e': case 'i': case 'o': case 'u':
	 res = "an " + property; break;
      case 'A': case 'E': case 'I': case 'O': case 'U':
	 res = "an " + property; break;
      default:
	 res = "a " + property; break;
      }
   }

   if (local["cap"]) {
      res = capitalize(res);
   }

   Output(res);

   return TRUE;
}

