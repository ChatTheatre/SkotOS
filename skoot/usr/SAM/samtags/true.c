inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "control";
   case "description":
      return "To be used inside an if-tag.  The contents of this tag are inserted into the output if the if-condition in combination with any and-conditions or or-conditions evaluates to false.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}
int tag(mixed content, object context, mapping local, mapping args) {
   tf_elt(content, context, args);
   return TRUE;
}


