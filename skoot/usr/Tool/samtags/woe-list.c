# include <System.h>

inherit "/usr/SAM/lib/sam";
inherit "/lib/url";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args)
{
   int    i, sz;
   string path, prefix, *arr;
   object udat;

   udat = context->query_udat();
   if (!udat) {
      return TRUE;
   }
   args += local;
   path = args["path"];
   if (!path) {
      path = "";
   }
   DEBUG("Tool:woe-list:" + udat->query_name() + ":" + path);
   arr = IDD->query_objects(path);
   sz = sizeof(arr);
   if (strlen(path)) {
      prefix = path + ":";
   } else {
      prefix = path;
   }
   for (i = 0; i < sz; i++) {
      Output("SET " + prefix + arr[i] + " " +
	     make_url("/Dev/View.sam", ([ "obj": prefix + arr[i] ])) +
	     "\n");
   }
   arr = IDD->query_subdirs(path);
   sz = sizeof(arr);
   for (i = 0; i < sz; i++) {
      Output("DIR " + prefix + arr[i] + "\n");
   }
   Output("SYNC\n");

   return TRUE;
}
