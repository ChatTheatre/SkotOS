# include <mapargs.h>
# include <SID.h>

private inherit "/lib/file";
private inherit "/lib/mapargs";

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string file;

   args += local;

   ob = ObArg("obj", "SID:WriteXML");

   if (ob->query_object_name()) {
      if (file = VAULT->store(ob)) {
	 copy_file(file, file + "." + time());
      }
   }
   return TRUE;
}
