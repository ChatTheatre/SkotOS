# include <type.h>
# include <SAM.h>
# include <SkotOS.h>

inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "settings";
   case "description":
      return "Generate the requested form of a given verb.";
   case "required":
      return ([
	  "verb": "The verb for which you want to get the relevant form.",
	  "form": "Which form you want (first, second, third)"
      ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   string res;
   object *list;
   mixed  verb, form;

   AssertLocal("verb", "social");
   AssertLocal("form", "social");

   verb = local["verb"];
   form = local["form"];

   if (typeof(verb) != T_STRING) {
      error("SAM tag social requires a string for 'verb'.");
   }
   if (typeof(form) != T_STRING) {
      error("SAM tag social requires a string for 'form'.");
   }
   list = SOCIALS->query_verb_obs(verb);
   if (!list || !sizeof(list)) {
      switch (form) {
      case "first":
	 res = verb;
	 break;
      case "second":
	 res = verb;
	 break;
      case "third":
	 res = verb + "s";
	 break;
      }
   } else {
      switch (form) {
      case "first":
	 res = list[0]->query_imperative();
	 break;
      case "second":
	 res = list[0]->query_second();
	 break;
      case "third":
	 res = list[0]->query_third();
	 break;
      }
   }
   Output(res);

   return TRUE;
}
