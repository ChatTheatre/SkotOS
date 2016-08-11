# include <limits.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>

inherit "/lib/womble";

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/url";

inherit "/usr/SAM/lib/sam";

mapping tips;
mapping tipcnt;

SAM description;

void womble_description() {
   description = womble(description);
}

int query_tip_cnt ( string topic );

string query_state_root() { return "Tips:Tip"; }


static 
void create() {
   set_object_name("SkotOS:Tip");

   tips = ([ ]);
   tipcnt = ([ ]);
}

string add_tip( string topic, SAM desc) {
   string tip_id;

   if ( tipcnt[ topic ] )
      tipcnt[ topic ]++;
   else
      tipcnt[ topic ] = 1;

   tip_id = topic+tipcnt[ topic ];  /* sword becomes swordX */
   tips[ tip_id ]=desc;
}

string show_tip( string topic, object user ) {
   string tip_id;
   int tip_num;
   object udat;

   udat = user->query_udat();

   tip_num = udat->next_tip_id( topic );
   if ( tip_num > query_tip_cnt( topic ) ) 
      tip_num = 1;
   if ( tip_num == query_tip_cnt( topic ) )
      udat->set_seen_all( topic, query_tip_cnt( topic ) );

   tip_id = topic + tip_num;

   if ( tips[ tip_id ] ) {
      udat->set_last_tip_seen( topic, tip_num );
      return unsam(tips[ tip_id ], ([ ]), user);
   }
   return "No tip available";
}

void reset_tips ( object user ) {
   user->query_udat()->tipclear();   
}

int query_tip_cnt ( string topic ) {
   if ( tipcnt[ topic ] )
      return tipcnt[ topic ];
   else
      return 0;
}

string *query_topics ( void ) {
   return map_indices( tipcnt );
}

int *query_indices(string topic) {
   int i, *foo;

   if (tipcnt[topic]) {
      foo = allocate(tipcnt[topic]);
      for (i = 0; i < tipcnt[topic]; i ++) {
	 foo[i] = i;
      }
      return foo;
   }
   return ({ });
}

SAM query_one_desc(string topic, int ix) {
   return tips[topic + (ix+1)];
}

string *query_desc ( string topic ) {
   string *desc, topicname;
   int a;

   desc = ({  });

   if (tipcnt[topic]) {
      for ( a=1; a<=tipcnt[topic]; a++ ) {
	 topicname = topic+a;
	 desc += ({ tips[topicname] });
      }
   }
   return desc;
}

void clear_topic ( string topic ) {
   int a;

   if (tipcnt[topic]) {
      for( a=1; a<=tipcnt[topic]; a++ ) {
	 tips[ topic+a ] = nil;
      }
   }
   tipcnt[topic] = nil;
}

string test() {
   return "Here's a tip for you";
}



string clear() {
   tips = ([ ]);
}

SAM query_description() {
   return description;
}

void set_description ( SAM m ) {
   description = m;
}

