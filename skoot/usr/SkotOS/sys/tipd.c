/*
 * ~SkotOS/sys/tipd.c
 *
 * Copyright Skotos Tech Inc 2004.
 */

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

mapping topics;
mapping r_topics;

static void create()
{
   topics   = ([ ]);
   r_topics = ([ ]);
}

void update_tip(object ob)
{
   string topic;

   topic = r_topics[ob];
   if (topic) {
      topics[topic][ob] = nil;
      if (!map_sizeof(topics[topic])) {
	 topics[topic] = nil;
      }
   }
   topic = ob->query_topic();
   if (topic) {
      if (!topics[topic]) {
	 topics[topic] = ([ ]);
      }
      topics[topic][ob] = sizeof(ob->query_descriptions());
   }
}

object *query_tips(string topic)
{
   return map_indices(topics[topic]);
}

int query_tip_cnt(string topic, varargs string theatre)
{
   mapping map;

   map = topics[topic];
   if (map) {
      int i, sz, *sum, total;
      object *list;

      sz   = map_sizeof(map);
      list = map_indices(map);
      sum  = map_values(map);
      if (theatre) {
	 for (i = 0; i < sz; i++) {
	    if (sum[i] == 0) {
	       continue;
	    }
	    /* Only skip the ones that are the -wrong- theatre, we include
	     * generic tips for everybody.
	     */
	    if (list[i]->query_property("theatre:id") &&
		list[i]->query_property("theatre:id") != theatre) {
	       sum[i] = 0;
	    }
	 }
      }
      total = 0;
      for (i = 0; i < sz; i++) {
	 total += sum[i];
      }
      return total;
   }
   return 0;
}

object *sort_tips(object *list)
{
    int i, sz;
    object *obs;
    mapping map, *maps;

    sz = sizeof(list);
    map = ([ ]);
    for (i = 0; i < sz; i++) {
	int    rank;
	string id;

	rank = Int(list[i]->query_property("tips:rank"));
	id   = list[i]->query_property("theatre:id");
	if (map[rank]) {
	    if (map[rank][id]) {
		map[rank][id] += ({ list[i] });
	    } else {
		map[rank][id] = ({ list[i] });
	    }
	} else {
	    map[rank] = ([ list[i]->query_property("theatre:id"): ({ list[i] }) ]);
	}
    }
    obs = ({ });
    sz = map_sizeof(map);
    maps = map_values(map);
    for (i = 0; i < sz; i++) {
	int     j, sz_j;
	object  **lists;

	sz_j  = map_sizeof(maps[i]);
	lists = map_values(maps[i]);
	for (j = 0; j < sz_j; j++) {
	    obs += lists[j];
	}
    }
    return obs;
}

mixed select_tip(string topic, string theatre, int index)
{
   mapping map;

   map = topics[topic];
   if (map) {
      int    i, sz, *sum;
      object *list;
      mixed  *descs;

      sz   = map_sizeof(map);
      list = map_indices(map);
      sum  = map_values(map);
      if (theatre) {
	 object *glist, *tlist;

	 glist = allocate(sz);
	 tlist = allocate(sz);

	 for (i = 0; i < sz; i++) {
	    if (sum[i] == 0) {
	       continue;
	    }
	    /*
	     * Only skip the ones that are the -wrong- theatre, we include
	     * generic tips for everybody.
	     */
	    if (!list[i]->query_property("theatre:id")) {
	       glist[i] = list[i];
	    } else if (list[i]->query_property("theatre:id") == theatre) {
	       tlist[i] = list[i];
	    }
	 }
	 list = glist  + tlist - ({ nil });
	 sz = sizeof(list);
      }
      descs = ({ });
      list = sort_tips(list);
      for (i = 0; i < sz; i++) {
	 descs += list[i]->query_descriptions();
      }
      if (index > sizeof(descs)) {
	 return nil;
      }
      return descs[index - 1];
   }
   return nil;
}

mixed show_tip(string topic, object user)
{
   int tip_num, total;
   string tip_id;
   string theatre;
   object udat, body;

   udat = user->query_udat();
   body = user->query_body();
   theatre = body->query_property("theatre:id");

   total = query_tip_cnt(topic, theatre);

   if (!total) {
      return "No tip available";
   }

   tip_num = udat->next_tip_id(topic);

   DEBUG("show_tip(" + dump_value(topic) + ", " + dump_value(user) + "): " + tip_num + ", " + total);

   if (tip_num > total) {
      tip_num = 1;
   }
   if (tip_num == total) {
      udat->set_seen_all(topic, total);
   }
   udat->set_last_tip_seen(topic, tip_num);
   return select_tip(topic, theatre, tip_num);
}

void reset_tips(object user)
{
   user->query_udat()->tipclear();
}
