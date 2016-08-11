# include <type.h>
# include <XML.h>

private inherit "/lib/url";
private inherit "/usr/XML/lib/xmd";

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
   mixed action;
   int id, i;
   mixed content_value;

   udat = context->query_udat();
   if (!udat) {
      error("no udat in zform");
   }
   content_value = query_colour_value(content);
   if (query_colour(content) == COL_PCDATA && typeof(content_value) == T_ARRAY) {
      content = content_value[..];
      for (i = 0; i < sizeof(content) && !action; i ++) {
	 if (query_colour(content[i]) == COL_ELEMENT &&
	     xmd_element(content[i]) == "action") {
	    action = xmd_content(content[i]);
	    content[i] = nil;
	 }
      }
      content -= ({ nil });
      content = NewXMLPCData(content);
   }
   local += ([ "action": action, "this": context->query_this() ]);
   id = udat->new_entry(({ "/SAM/Post", local - ({ "zlink_target" }) }));
   Output("<form method=post action=\"/Request\"" +
	  (local["zlink_target"] ? " target=\"" + local["zlink_target"] + "\"" : "") +
	  ">" +
	  "<input type=hidden name=\"__id\" value=" + id + "/>");
   tf_elt(content, context, args);
   Output("</form>");
   return TRUE;
}
