inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Assign a value in a mapping at the given index.  The mapping needs to already have been initialized.";
   case "required":
      return ([
	 "map":   "The name of the mapping variable.",
	 "index": "The index value.",
	 "to":    "The value to be associated with the index."
      ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}
int tag(mixed content, object context, mapping local, mapping args) {
   if (typeof(args["map"]) != T_MAPPING) {
      /* TODO */
      Output("attribute 'map' in tag 'mapset' is not a mapping");
      return TRUE;
   }
   if (typeof(args["index"]) != T_STRING) {
      /* TODO */
      Output("attribute 'index' in tag 'mapset' is not a string");
      return TRUE;
   }
   args["map"][args["index"]] = args["to"];
   XDebug("setting args[" + args["index"] + "] = " +
	  dump_value(args["value"]));
   return TRUE;
}
