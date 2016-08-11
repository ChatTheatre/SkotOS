# include <type.h>
# include <XML.h>

private inherit "/usr/XML/lib/xmd";
        inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "description":
      return "Undocumented at the moment.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   int    i, sz;
   string *names;
   object  udat;
   mapping data;
   mixed   id,  *values;

   udat = context->query_udat();
   if (!udat) {
      error("missing udat in zsession");
   }
   id = local["zid"];
   if (id == nil) {
      data = ([ ]);
      id = udat->new_entry(data);
      args["zid"] = id;
   } else {
      id = (int)id;
      data = udat->query_data(id);
      if (!data) {
	 /* Invalid or expired zid? */
	 data = ([ ]);
	 id = udat->new_entry(data);
	 args["zid"] = id;
      }
   }

   /* Work with copy of local, just in case. */
   local -= ({ "zid" });
   sz = map_sizeof(local);
   names = map_indices(local);
   values = map_values(local);
   for (i = 0; i < sz; i++) {
      /* HACK: Need udat->add_data(id, name, value); */
      data[names[i]] = values[i];
   }

   tf_elt(content, context, args);
   return TRUE;
}
