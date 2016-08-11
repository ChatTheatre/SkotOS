private inherit "/lib/string";
private inherit "/lib/mapargs";

# include <mapargs.h>

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "settings";
   case "description":
      return "Set the property to a (new) value.";
   case "required":
      return ([ "what": "The object in which to set the property.",
		"prop": "The name of the property that needs to be set.",
		"val":  "The value for the property." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   args += local;

   call_other(ObArg("what", "set-property"),
	      "set_property",
	      StrArg("prop", "set-property"),
	      args["val"]);

   return TRUE;
}

