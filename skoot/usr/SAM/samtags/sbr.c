inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "layout";
   case "description":
      return "Convenience, inserts <BR> when used in a webpage, or a newline when used elsewhere.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   if (args["HTML"]) {
      Output("<BR>");
   } else {
      Output("\n");
   }
   return TRUE;
}
