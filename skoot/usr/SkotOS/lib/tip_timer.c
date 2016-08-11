/*
**	/usr/SkotOS/lib/tip_timer.c
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";

# include <SkotOS.h>

mapping tip_queue;

static
void create() {
   tip_queue = ([ ]);
}

static
void show_next_tip ( string topic, object user, int delay )
{
   object udat;

   if (!user || !user->query_udat() || !user->query_body()) {
      return;
   }
   udat = user->query_udat();

   if (!udat->query_seen_all(topic)) {
      user->query_body()->action("tip", ([ "topic": topic ]));
   }
}


static
void start_tip_timer(int seconds, string topic) {
   int total;
   string theatre;
   object user, udat, body;

   user    = this_object();
   udat    = user->query_udat();
   body    = user->query_body();
   theatre = body->query_property("theatre:id");
   total   = TIPD->query_tip_cnt(topic, theatre);

   if (udat->query_seen_all(topic) && total > udat->query_seen_all(topic)) {
      /* Someone has added new tips? */
      udat->set_seen_all(topic, 0);
   }
   if (!udat->query_seen_all(topic)) {
      call_out("show_next_tip", seconds, topic, user, seconds);
   }
}



