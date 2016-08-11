inherit "/usr/SAM/lib/sam";

inherit "~/lib/bilbo";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args)
{
   object udat, obj;
   string obname, script;

   udat = context->query_udat();
   if (!udat) {
      return TRUE;
   }

   args += local;
   obname = args["obj"];
   if (!obname) {
      obname = "Marrach:players:H:hresh";
   }
   obj = find_object(obname);
   if (!obj) {
      return TRUE;
   }
   script = args["script"];
   if (!script) {
      return TRUE;
   }
   Output(source_to_html(pp_bilbo_script(obj, script, 10, 0, FALSE)));
   return TRUE;
}
