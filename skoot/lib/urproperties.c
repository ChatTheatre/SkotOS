/*
 * /lib/urproperties.c
 *
 * A toolkit class to help with ur-object-related property work.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <base.h>

private inherit "/lib/mapping";

inherit "/lib/ur";

mixed
query_exported_property(string prop) {
   if (sscanf(prop, "export:%*s")) {
      if (query_ur_object()) {
	 /* send the export: namespace unmodified up the ur-chain */
	 return query_ur_object()->query_downcased_property(prop);
      }
      return nil;
   }
   /* it it's not already an export query, do one */
   return this_object()->query_downcased_property("export:" + prop);
}

mapping imported_properties() {
   if (!query_ur_object()) {
      return ([ ]);
   }
   return prefixed_map(query_ur_object()->query_properties(FALSE),
		       "export:", TRUE);
}
