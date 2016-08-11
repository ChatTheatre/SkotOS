/*
 * ~Tool/lib/list.c
 *
 * List support.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <TextIF.h>
# include "~/include/tool.h"

inherit "utility";

void
cmd_list_client(object user, object body)
{
   int     i, sz;
   string  result, *ids, *names;
   object  *list;
   mapping map_id, map_html, map_skoot, map_skotos, *maps;

   map_id     = ([ ]);
   map_html   = ([ ]);
   map_skoot  = ([ ]);
   map_skotos = ([ ]);
   list = INFOD->query_current_user_objects();
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      int    html, skoot, skotos;
      string id;
      object ob;

      ob     = list[i];

      id     = ob->query_client_id();
      html   = ob->query_html_client();
      skoot  = ob->query_skoot_client();
      skotos = ob->query_skotos_client();
      if (!map_id[id]) {
	 map_id[id] = ([ ]);
      }
      map_id[id][ob->query_name()] = TRUE;
      if (html) {
	 map_html[ob->query_name()] = TRUE;
      }
      if (skoot) {
	 map_skoot[ob->query_name()] = TRUE;
      }
      if (skotos) {
	 map_skotos[ob->query_name()] = TRUE;
      }
   }
   result = "Client IDs:\n";
   sz   = map_sizeof(map_id);
   ids  = map_indices(map_id);
   maps = map_values(map_id);
   for (i = 0; i < sz; i++) {
      result +=
	 "*** " + (ids[i] ? ids[i] : "<none>") + " (" + map_sizeof(maps[i]) + "):\n" +
	 "    " + implode(map_indices(maps[i]), " ") + "\n";
   }
   pre_message(user, result);
}
