inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

private inherit "/base/lib/urcalls";

# include <type.h>
# include <SAM.h>
# include "/usr/SAM/include/sam.h"
# include "/usr/SkotOS/include/describe.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Generates 'is' or 'are' depending on what 'what' refers to.";
   case "required":
      return ([ "what": "The object(s) you are referring to." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string prep, *snames, prime_id;
   mixed what;
   SAM res;
   int i, snames_size, plural, style;

   args += local;

   AssertLocal("what", "obsprep");

   what = args["what"];

   if (typeof(what) != T_ARRAY) {
      what = ({ what });
   }

   XDebug("obsprep() what: " + dump_value(what));

   if (sizeof(what) > 1) {
	 prep = "are";
   } else if (typeof(what[0]) == T_OBJECT) {
      /* personally I think this is a cheesy hack to determine 
      ** if an object is considered a plural object.
      ** such as: boots, socks, and pants. -- wes
      */
      prime_id = ur_prime_id(what[0], "outside");

      /* New approach. */
      if (ur_plural(what[0], prime_id)) {
	 prep = "are";
      } else if (typeof(what[0]->query_property("trait:isare")) == T_STRING) {
	 prep = what[0]->query_property("trait:isare");
      } else {
	 snames = ur_snames(what[0], prime_id);
	 if (snames) {
	    snames_size = sizeof(snames);

	    /* basically all snames and pnames need to be the same */
	    for(i = 0; i < snames_size; i++) {
	       if (snames[i] == ur_pname_of_sname(what[0], snames[i]))
		  plural = TRUE;
	    }
	 }

	 if (plural) {
	    prep = "are";
	 } else {
	    prep = "is";
	 }
      }
   } else {
      prep = "is";
   }

   Output(prep);
   return TRUE;
}

