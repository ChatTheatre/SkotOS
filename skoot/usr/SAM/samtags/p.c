inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "layout";
   case "description":
      return "Nothing special, just insert two newlines to separate paragraphs from eachother.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   if (!args["HTML"]) {
      tf_elt(content, context, args);
      Output("\n\n");
      return TRUE;
   }
}
