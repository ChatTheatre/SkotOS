private inherit "/lib/string";
inherit "/usr/SAM/lib/sam";

# include <type.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "control";
   case "description":
      return "Evaluate the contents of the tag, while assigning the variable as indicated in the 'var' parameter with a new value from the 'var' parameter, while also setting the 'iteration' variable to whichever iteration it is (starting at 1).";
   case "required":
      return ([
	 "var": "The name of the variable that you want to contain the " +
	        "different values from 'val'.",
	 "val": "The values you want to assign to 'var'."
     ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   string var, res;
   mixed arr;
   int j;

   AssertLocal("var", "for");
   AssertLocal("val", "for");

   arr = local["val"];
   if (!arr) {
      arr = ({  });
   } else if (typeof(arr) != T_ARRAY) {
      arr = ({ arr });
   }
   var = lower_case(local["var"]);

   XDebug("running through for-loop of size " + sizeof(arr));
   for (j = 0; j < sizeof(arr); j ++) {
      args[var] = arr[j];
      args["iteration"] = (j+1);
      tf_elt(content, context, args);
   }
   return TRUE;
}
