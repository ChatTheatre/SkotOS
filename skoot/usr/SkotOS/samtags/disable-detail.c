private inherit "/base/lib/urcalls";
private inherit "/lib/string";
private inherit "/lib/type";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

# include <SAM.h>
# include <SkotOS.h>
# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args) {
   object ob;
   string id;

   AssertLocal("ob", "disable-detail");
   AssertLocal("id", "disable-detail");

   ob = local["ob"];
   id = local["id"];

   ob->set_id_hidden(id, TRUE);

   return TRUE;
}

