# include <type.h>
# include <mapargs.h>

inherit "/usr/XML/lib/xmlparse";

private inherit "/lib/mapargs";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/SID/lib/stateimpex";

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string xml, file;
   mixed state;

   SysLog("Trying to post");

   args += local;

   xml = StrArg("xml", "/Dev/Post-State");

   state = parse_xml(xml);

   if (!state) {
      error("parsing failed");
   }

   state = query_colour_value(xmd_force_to_data(state));
   if (sizeof(state) == 0 || xmd_element(state[0]) != "object") {
      error("root element is not 'object'");
   }
   state = state[0];
   if (xmd_attributes(state)[0] != "id" ||
       typeof(xmd_attributes(state)[1]) != T_OBJECT) {
      XDebug(dump_value(state[1]));
      error("bad or missing 'id' attribute to 'object' element");
   }
   ob = xmd_attributes(state)[1];

   state = query_colour_value(xmd_force_to_data(xmd_content(state)));
   if (xmd_element(state[0]) == "context") {
      local = parse_xml_context(state[0]);
      state = state[1 ..];
   }
   if (sizeof(state) > 1) {
      error("'object' element has more than one child");
   }

   import_state(ob, state[0], local);

   {
      mixed log;

      log = ob->query_property("revisions");
      if (typeof(log) != T_ARRAY) {
	 log = ({ });
      }
      log += ({
	 time(),
	 query_originator() ? query_originator()->query_name() : "???",
	 "X"
      });
      ob->set_property("revisions", log);
   }

   return TRUE;
}
