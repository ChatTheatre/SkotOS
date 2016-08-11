# include <type.h>
# include <mapargs.h>

private inherit "/lib/mapargs";
private inherit "/usr/XML/lib/xmd";
inherit "/usr/XML/lib/xmlparse";
private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/DevSys/lib/formstate";

int tag(mixed content, object context, mapping local, mapping args) {
   mapping map;
   string xml, file;
   object ob;
   mixed state, *call, **calls;
   int i;

   args += local;

   ob = ObArg("obj", "/Dev/Post-Form");

   local = parse_form_context(args);

   map = form_to_state(args - ({ "request" }));

   if (state = map["state"]) {
      import_state(ob, state, local);
   }

   if (calls = map["calls"]) {
      /* the parameters have already been translated in form_to_state */
      for (i = 0; i < sizeof(calls); i ++) {
	 call = calls[i];
	 catch {
	    call_other(ob, call[0], call[1 ..]...);
	 }
      }
   }
   {
      mixed log;

      log = ob->query_property("revisions");
      if (typeof(log) != T_ARRAY) {
	 log = ({ });
      }
      log += ({
	 time(),
	 query_originator() ? query_originator()->query_name() : "???",
	 "E"
      });
      ob->set_property("revisions", log);
   }

   return TRUE;
}
