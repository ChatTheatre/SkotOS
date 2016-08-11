inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "layout";
   case "description":
      return "Adds a newline to the output.";
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   if (!args["HTML"]) {
      Output("\n");
      return TRUE;
   }
}
