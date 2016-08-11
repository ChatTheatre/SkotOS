private inherit "/base/lib/urcalls";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/lib/string";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Describe how much of the object has been eaten, assuming that this is described by how much mass the object has lost since it was created.";
   case "required":
      return ([ "what": "The object that you want to describe." ]);
   case "optional":
      return ([ "cap": "Capitalize the resulting output." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string out;
   mixed what;
   float percent;
   SAM res;

   AssertLocal("what", "eaten");

   what = local["what"];

   if (typeof(what) != T_OBJECT)
   	error("SAM tag 'eaten' requires an object as arg");

   percent = what->query_intrinsic_mass() / what->query_original_mass() * 100.0;

   if (percent >= 100.0) 
      out = "";
   else if (percent < 100.0 && percent > 80.0)
      out = "nibbled on ";
   else if (percent <= 80.0 && percent > 60.0)
      out = "bitten into ";
   else if (percent <= 60.0 && percent > 40.0)
      out = "partly eaten ";
   else if (percent <= 40.0 && percent > 25.0)
      out = "half eaten ";
   else if (percent <= 25.0 && percent > 6.0)
      out = "mostly eaten ";
   else 
      out = "nearly devoured ";

   if (local["cap"]) {
      out = capitalize(out);
   }

   Output(out);

   return TRUE;
}

