/*
**	unregister-body.c
**
**	Unregisters a body from a user's list of bodies.
*/

private inherit "/lib/string";
private inherit "/lib/mapargs";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

# include <mapargs.h>

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

   SysLog("Local is " + dump_value(local));
   SysLog("Args is " + dump_value(args));

   args += local;

   if (udat = context->query_udat()) {
      string cname;
      object body;

      cname = StrArg("cname", "unregister-body");
      body  = udat->query_body(cname);

      if (body) {
	 SysLog("Performing unregistry...");
	 udat->del_body(body);

	 if (!udat->query_body(cname)) {
	    /* Must have been successful, then, mark as such. */
	    body->set_property("skotos:unregistered", time());
	    catch {
	       "~UserAPI/sys/unregistered"->add_unregistered(body);
	    }
	 }
      }
   }

   return TRUE;
}

