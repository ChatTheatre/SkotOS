private inherit "/base/lib/urcalls";
private inherit "/lib/string";
private inherit "/lib/type";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

# include <SAM.h>
# include <SkotOS.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Describe an object.";
   case "required":
      return ([ "what": "The item you want to describe (may be an NRef)." ]);
   case "optional":
      return ([
	 "actor":   "The acting party.",
	 "detail":  "Indicate (or override) the detail you want to describe.",
	 "looker":  "From whose perspective to describe the item.",
	 "lookers": "If 'looker' isn't used, the first entry from 'lookers' " +
	            "is used instead.",
	 "cap":     "Whether to capitalize the resulting output or not.",
	 "nonposs": "Adjust the style in which the description is written." ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string det, res;
   mixed what, actor, looker, lookers;
   int style;

   AssertLocal("what", "describe");

   what = local["what"];
   actor = local["actor"];
   looker = local["looker"];
   
   lookers = args["lookers"];

   if (local["nonposs"]) {
      style |= STYLE_NONPOSS;
   }
   if (local["self"]) {
      style |= STYLE_PRONOUN;
   }

   if (IsNRef(actor))
      actor = NRefOb(actor);

   if (IsNRef(looker))
      looker = NRefOb(looker);

   if (typeof(lookers) == T_ARRAY && sizeof(lookers) && !looker) {
      looker = lookers[0];
      if (IsNRef(looker))
	 looker = NRefOb(looker);
   }

   if (typeof(what) == T_ARRAY) {
      res = describe_many(what, actor, looker, style);
   } else {
      if (local["detail"]) {
	 if (IsNRef(what)) {
	    what = NewNRef(NRefOb(what), local["detail"]);
	 } else {
	    what = NewNRef(what, local["detail"]);
	 }
      }
      res = describe_one(what, actor, looker, style);
   }
   if (local["cap"]) {
      res = capitalize(res);
   }

   Output(res);

   return TRUE;
}

