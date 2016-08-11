# include <type.h>
# include <mapargs.h>

private inherit "/lib/mapargs";
private inherit "/lib/url";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   mixed arr;
   int i;

   if (arr = context->query_targets()) {
      string url;

      /* we ignore args; the redirect would be much too large */
      if (typeof(local["base"]) == T_STRING) {
	 url = local["base"];
      } else {
	 if (typeof(local["propob"]) == T_STRING) {
	    url = "/SAM/Prop/" + url_encode(local["propob"]);
	 } else if (typeof(local["propob"]) == T_OBJECT) {
	    url = "/SAM/Prop/" + url_encode(name(local["propob"]));
	 } else {
	    error("redirect SAM tag needs non-nil 'base' or 'obj' attribute");
	 }
	 if (typeof(local["prop"]) == T_STRING) {
	    url += "/" + local["prop"];
	 } else {
	    url += "/Index";
	 }
      }
      if (!url ){
      }
      local -= ({ "base", "propob", "prop" });
      for (i = 0; i < sizeof(arr); i ++) {
	 /* let the user object do what it will with this */
	 if (ur_name(arr[i]) == "/usr/SAM/data/unsam#-1") {
	    if (arr[i]->query_user()) {
	       arr[i]->query_user()->redirect_to(url, local);
	    }
	 } else {
	    arr[i]->redirect_to(url, local);
	 }
      }
   }
}
