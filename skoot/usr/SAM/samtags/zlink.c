private inherit "/lib/url";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return FALSE;
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
   object udat;
   string base, str;
   int id;

   if (base = local["base"]) {
      if ((udat = context->query_udat()) && base[0] == '/') {
	 /* only do this with abslute URL's internal to our server */
	 id = udat->new_entry(({ local["base"],
				 local - ({ "base", "secure", "zlink_target", "zlink_title" }) }));
	 Output("<a href=\"/Request?__id=" + id + "\"" +
		(local["zlink_title"]  ? " title=\""  + local["zlink_title"]  + "\"" : "") +
		(local["zlink_target"] ? " target=\"" + local["zlink_target"] + "\"" : "") +
		">");
      } else {
	 str = "<a href=\"";
	 str += make_url(local["base"],
			 local - ({ "base", "secure", "zlink_target", "zlink_title" }),
			 !!local["secure"]) + "\"";
	 Output(str +
		(local["zlink_title"]  ? " title=\""  + local["zlink_title"]  + "\"" : "") +
		(local["zlink_target"] ? " target=\"" + local["zlink_target"] + "\"" : "") +
		">");
      }
      tf_elt(content, context, args);
      Output("</a>");
      return TRUE;
   }
}
