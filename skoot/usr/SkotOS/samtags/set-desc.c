/*
**	set-desc.c
**
**	Sets a description type in a specified detail.
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
      return "Set the description of a certain type for an object or its detail..";
   case "required":
      return ([ "obj": "The object or NRef in which to set the description.",
		"type": "The type of description that you want to set.",
		"desc": "The (new) description." ]);
   case "optional":
      return ([ "detail": "The detail for which you want to set the description.  Only used if the 'obj' parameter isn't an NRef." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed desc, ref;
   string id, type;
   object ob;

   AssertLocal("type", "set-desc");
   AssertLocal("desc", "set-desc");
   AssertLocal("obj", "set-desc");

   type = local["type"];
   desc = local["desc"];
   ref = local["obj"];

   /* type sanity checks */
   if (typeof(type) != T_STRING) {
      error("SAM tag set-desc requires a string for 'desc'.");
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

   XDebug("SAM tag set-desc - ob: " + dump_value(ob));
   XDebug("SAM tag set-desc - id: " + dump_value(id));
   XDebug("SAM tag set-desc - type: " + dump_value(type));
   XDebug("SAM tag set-desc - desc: " + dump_value(desc));

   /* call set_description() */
   ob->set_description(id, type, desc);

   return TRUE;
}

