inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/lib/string";

# include <SAM.h>
# include <type.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Describe a collection of objects.";
   case "required":
      return ([ "what": "The item(s) you want to describe." ]);
   case "optional":
      return ([
	 "style":   "Adjust the style in which the description is written.  Default is 'STYLE_NONPOSS'.",
	 "actor":   "The acting party.",
	 "looker":  "From whose perspective to describe the item(s).",
	 "lookers": "If 'looker' isn't used, the first entry from 'lookers' " +
	            "is used instead.",
	 "cap":     "Whether to capitalize the resulting output or not."
	 ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string res;
   mixed what, style, actor, looker, lookers;

   args += local;

   AssertLocal("what", "describemany");

   what = args["what"];
   style = args["style"];
   actor = args["actor"];
   looker = args["looker"];
   lookers = args["lookers"];

   if (typeof(what) == T_OBJECT || IsNRef(what)) {
      what = ({ what });
   }

   if (typeof(style) != T_INT) {
      if (typeof(style) == T_STRING)
	 style = (int) style;
      else if (typeof(style) == T_NIL)
	 style = STYLE_NONPOSS;
      else
	 error("describemany needs a string or integer for its style");
   }

   if (lookers && typeof(lookers) == T_ARRAY &&
       sizeof(lookers) && !looker) {
      looker = lookers[0];
   }

   res = describe_many(what, actor, looker, style);

   if (local["cap"]) {
      res = capitalize(res);
   }

   Output(res);

   return TRUE;
}

