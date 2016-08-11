/*
**	rem-adj.c
**
**	Removes an adjective to a specified detail.
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
      return "Remove an adjective from an object detail.  If the 'obj' parameter is an object, by default the primary detail will be used, unless the 'detail' parameter is used as well.";
   case "required":
      return ([ "adjective": "The adjective to remove. ",
		"obj": "The object or NRef from which to remove the adjective.",
		]);
   case "optional":
      return ([ "detail": "The specific detail from which to remove the adjective.  Only used if the 'obj' parameter isn't an NRef." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed adj, ref;
   string id;
   object ob;

   AssertLocal("adjective", "rem-adj");
   AssertLocal("obj", "rem-adj");

   adj = local["adjective"];
   ref = local["obj"];

   /* type sanity checks */
   if (typeof(adj) != T_STRING) {
      error("SAM tag rem-adj requires a string for 'adjective'.");
   }

   if (IsNRef(ref)) {
      id = NRefDetail(ref);
      ob = NRefOb(ref);
   } else {
      error("SAM tag rem-adj requires a NREF or OBJ for 'obj'.");
   }
   if (local["detail"]) {
      id = local["detail"];
   }

   XDebug("SAM tag rem-adj - ob: " + dump_value(ob));
   XDebug("SAM tag rem-adj - id: " + dump_value(id));
   XDebug("SAM tag rem-adj - adj: " + dump_value(adj));

   /* call add_adjective() */
   ob->remove_adjective(id, adj);

   return TRUE;
}

