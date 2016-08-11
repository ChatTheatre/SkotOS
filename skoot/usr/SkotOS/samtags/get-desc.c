/*
**	get-desc.c
**
**	Queries a description type in a specified detail.
*/
private inherit "/base/lib/urcalls";
private inherit "/lib/string";

inherit "/usr/XML/lib/xmlgen";
inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "settings";
   case "description":
      return "Get the description of a certain type from an object or its detail.";
   case "required":
      return ([ "obj": "The object or NRef from which to get the description.",
		"type": "The type of description that you want to get." ]);
   case "optional":
      return ([ "detail": "The detail for which you want to get the description.  Only used if the 'obj' parameter isn't an NRef." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed ref, xmd;
   string id, type;
   object ob;

   AssertLocal("type", "set-desc");
   AssertLocal("obj", "set-desc");

   type = local["type"];
   ref = local["obj"];

   /* type sanity checks */
   if (typeof(type) != T_STRING) {
      error("SAM tag get-desc requires a string for 'type'.");
   }

   if (IsNRef(ref)) {
      id = NRefDetail(ref);
      ob = NRefOb(ref);
   } else {
      error("SAM tag get-desc requires a NREF or OBJ for 'obj'.");
   }

   if (local["detail"]) {
      id = local["detail"];
   }

   /* call query_description() */
   xmd = ur_description(ob, id, type);

   if (typeof(xmd) == T_STRING) {
      Output(xmd);
      return TRUE;
   }

   Output(generate_pcdata(xmd));

   return TRUE;
}

