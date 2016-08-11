private inherit "/lib/string";
inherit "/usr/SAM/lib/sam";

# include <type.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return FALSE;
   case "description":
      return "Obsolete.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   string var, res;
   mixed arr;
   int j, index;

   AssertLocal("var",   "index");
   AssertLocal("val",   "index");
   AssertLocal("index", "index");

   var = lower_case(local["var"]);

   arr = local["val"];
   if (!arr) {
      arr = ({  });
   } else if (typeof(arr) != T_ARRAY) {
      arr = ({ arr });
   }

   index = (int)local["index"];
   if (index < 1) {
     index = 1;
   } else if (index > sizeof(arr)) {
     index = sizeof(arr);
   }
   args[var] = arr[index - 1];
   tf_elt(content, context, args);

   return TRUE;
}
