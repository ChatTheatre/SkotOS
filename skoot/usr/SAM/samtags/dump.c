inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Dump in an easily readable format the data-structure found in the 'val' parameter.";
   case "required":
      return ([ "val": "The data-structure which you want to examine." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   Output(dump_value(local["val"]));
   return TRUE;
}
