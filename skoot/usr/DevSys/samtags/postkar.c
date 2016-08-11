# include <type.h>
# include <mapargs.h>

inherit "/lib/mapargs";
inherit "/usr/DevSys/lib/karmod";

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;

   args += local;

   ob = ObArg("obj", "/Dev/PostKar");
   parskar(ob, StrArg("kar", "/Dev/PostKar"));

   {
      mixed log;

      log = ob->query_property("revisions");
      if (typeof(log) != T_ARRAY) {
	 log = ({ });
      }
      log += ({
	 time(),
	 query_originator() ? query_originator()->query_name() : "???",
	 "K"
      });
      ob->set_property("revisions", log);
   }

   return TRUE;
}
