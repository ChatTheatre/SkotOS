# include <limits.h>
# include <SkotOS.h>
# include <SAM.h>

private inherit "/lib/string";
private inherit "/lib/array";

mapping cmemories;  /* collective memories, shared by everyone, */
mapping cmemcnt;    /* NOT personal memories */

int query_mem_cnt ( string topic );
void hardcoded ( void );

string query_state_root() { return "Memories:Memories"; }

static 
void create() {
   set_object_name("SkotOS:Memories");

   cmemories = ([ ]);
   cmemcnt = ([ ]);
   
   hardcoded();
}


string add_memory( string topic, SAM desc) {
   string cmem_id;

   Debug( "topic "+topic );
   
   if ( cmemcnt[ topic ] )
      cmemcnt[ topic ]++;
   else
      cmemcnt[ topic ] = 1;
   
   cmem_id = topic+cmemcnt[ topic ]; 

   Debug("trying "+cmem_id);

   cmemories[ cmem_id ]=desc;

   Debug("done");
}

int show_cmemories ( string *wordarr, string username ) {
   int a, wordcnt, b, foundany, showthis, showall;
   string *cmem_ids, checkname;
 
   foundany = 0;
   wordcnt = sizeof( wordarr );
   cmem_ids = map_indices( cmemories );
   showall = wordcnt==1 && wordarr[0]=="all" ? 1 : 0;
   
   for( a=0; a<sizeof(cmem_ids); a++ ) {
      showthis = 0;
      for( b=0; b<wordcnt; b++ ) {
         if ( sscanf( lower_case(cmemories[ cmem_ids[a] ]), "%*s"+lower_case(wordarr[b])+"%*s") || showall ) {
            showthis = 1;
			break;
		 }		 
	  }
	  if ( showthis ) {
		 sscanf( cmem_ids[a], "%s%*d", checkname );
		 if ( lower_case(checkname)==lower_case(username) || lower_case(checkname)=="everyone" ) {
            this_user()->add_more( "\n" + cmemories[ cmem_ids[a] ] + "\n" );
			foundany = 1;
		 }
	  }	  
   }
      
   return foundany;
}


int query_cmem_cnt ( string topic ) {
   if ( cmemcnt[ topic ] )
      return cmemcnt[ topic ];
   else
      return 0;
}

string *query_topics ( void ) {
   return map_indices( cmemcnt );
}

string *query_desc ( string topic ) {
   int a;
   string *desc, topicname;
   desc = ({  });
   
   for ( a=1; a<=cmemcnt[topic]; a++ ) {
      topicname = topic+a;
      desc += ({ cmemories[topicname] });
   }
   return desc;
}

void clear_topic ( string topic ) {
   int a;

   Debug("clearing");
   Debug("topic "+topic);
   /* Debug("cnt "+cmemcnt[topic] ); */

   if ( cmemcnt[topic] ) {   
      for( a=1; a<=cmemcnt[topic]; a++ ) {
         cmemories[ topic+a ] = nil;
      }
      cmemcnt[topic] = nil;
   }

   Debug("done");

}

string cmem_test() {
   return "Thanks for the cmemories";
}

void hardcoded( void ) {
   add_memory( "Everyone", "It is customary to cook your trolls before eating them." );
   add_memory( "Everyone", "It is customary to have spare pizzas available for visiting royalty." );
   
   add_memory( "Everyone", "Giants are those really tall, sneering creatures.");
   add_memory( "Everyone", "Dwarves are generally as reclusive and untrustworthy as Texans." );

   add_memory( "Baron", "The queen has a temper." );
   add_memory( "Baron", "The king is dead." );
   add_memory( "Baron", "The princess is undefined.  How embaressing for her." );

   add_memory( "DukeLeto", "Leto memory 1." );
   add_memory( "DukeLeto", "Leto memory 2." );
   add_memory( "DukeLeto", "Leto memory 3." );
}

string mem_clear() {
   cmemories = ([ ]);
   cmemcnt = ([ ]);
}
