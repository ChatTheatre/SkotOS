/*
**	add-adj.c
**
**	Adds an adjective to a specified detail.
*/
private inherit "/base/lib/urcalls";
private inherit "/lib/string";

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
      return "Add an adjective to an object detail.  If the 'obj' parameter is an object, by default the primary detail will be used, unless the 'detail' parameter is used as well.";
   case "required":
      return ([ "adjective": "The adjective to add.",
		"obj": "The object or NRef in which to add the adjective.",
		]);
   case "optional":
      return ([ "detail": "The specific detail for which to add the adjective.  Only used if the 'obj' parameter isn't an NRef." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed adj, ref;
   string id;
   object ob;

   AssertLocal("adjective", "add-adj");
   AssertLocal("obj", "add-adj");

   adj = local["adjective"];
   ref = local["obj"];

   /* type sanity checks */
   if (typeof(adj) != T_STRING) {
      error("SAM tag add-adj requires a string for 'adjective'.");
   }

   if (IsNRef(ref)) {
      id = NRefDetail(ref);
      ob = NRefOb(ref);
   } else {
      error("SAM tag add-adj requires a NREF or OBJ for 'obj'.");
   }
   if (local["detail"]) {
      id = local["detail"];
   }

   XDebug("SAM tag add-adj - ob: " + dump_value(ob));
   XDebug("SAM tag add-adj - id: " + dump_value(id));
   XDebug("SAM tag add-adj - adj: " + dump_value(adj));

   /* call add_adjective() */
   ob->add_adjective(id, adj);

   return TRUE;
}

