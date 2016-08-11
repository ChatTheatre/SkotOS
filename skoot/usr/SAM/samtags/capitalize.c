private inherit "/usr/XML/lib/xmd";
private inherit "/lib/string";

inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Capitalizes the contents of the 'what' parameter.";
   case "required":
      return ([ "what": "That which needs to be capitalized." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   Output(capitalize(local["what"]));
   return TRUE;
}
