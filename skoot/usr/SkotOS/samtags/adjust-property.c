private inherit "/lib/string";
private inherit "/lib/mapargs";

# include <type.h>
# include <mapargs.h>

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "settings";
   case "description":
      return "Set the property in the given object to a new value.  This differs from set-property in that it checks if the current property-value is an integer, in which case it will attempt to cast the new value to an integer as well.";
   case "required":
      return ([ "what": "The object in which to change the property.",
		"prop": "The name of the property that needs to be changed.",
		"val":  "The new value for the property." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   string prop;
   object ob;
   mixed val;

   args += local;

   ob = ObArg("what", "adjust-property");
   prop = StrArg("prop", "adjust-property");

   val = ob->query_property(prop);
   if (typeof(val) == T_INT) {
      ob->set_property(val + IntArg("val", "adjust-property"));
   }
   return TRUE;
}

