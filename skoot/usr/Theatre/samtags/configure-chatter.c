# include <base/living.h>
# include <UserAPI.h>

private inherit "/lib/mapargs";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

atomic
int tag(mixed content, object context, mapping local, mapping args) {
   object chatter;
   string property, *ix;
   int i;

   chatter = ObArg("chatter", "configure-chatter");

   args += local;

   SysLog(dump_value(args));

   ix = map_indices(args);
   for (i = 0; i < sizeof(ix); i ++) {
      if (sscanf(ix[i], "nugget:%s", property)) {
	 SysLog("setting " + name(chatter) + " / " + property + " to " + dump_value(args[ix[i]]));
	 chatter->set_property(property, args[ix[i]]);
      }
   }

   chatter->set_property("bilbo:solo:bugger", "run");
   chatter->set_property("bilbo:solo:bugger:0010", "call bilbo:react:descsub");

   chatter->run_signal("bugger");

   chatter->clear_property("bilbo:solo:bugger");
   chatter->clear_property("bilbo:solo:bugger:0010");

   return TRUE;
}
