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
   args += local;

   XDebug(dump_value(args));
   if (args["who"] && args["obj"] && args["event"]) {
      string str;
      object ob;
      int time;

      if (sscanf(args["obj"], "OBJ(%s)", str)) {
	 XDebug("got obj=" + str);
	 if (ob = find_object(str)) {
	    args["time"] && sscanf(args["time"], "%d", time);
	    ob->run_event(args["event"], args["who"], args, time);
	    return TRUE;
	 }
      }
   }
   return FALSE;
}
