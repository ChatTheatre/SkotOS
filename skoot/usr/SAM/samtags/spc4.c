inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "layout";
   case "description":
      return "Generate exactly four spaces in the output.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}
int tag(mixed content, object context, mapping local, mapping args) {
   Output("    ");
   return TRUE;
}

