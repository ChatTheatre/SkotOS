# include <base.h>
# include <limits.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>
# include <base/bulk.h>
# include <base/preps.h>
# include <base/living.h>

# define DELAY_FACTOR 2

private inherit "/base/lib/toolbox";
private inherit "/base/lib/urcalls";

inherit "/usr/SID/lib/stateimpex";

inherit "/lib/string";
inherit "/lib/array";
inherit "/lib/url";
inherit "/lib/mapargs";
inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/bilbo";

string g_trait;

int setup_obj ( object obj, object location );
void fill_body ( object brain, string bname );
void _debug ( string s );
void choose_brain ( object b );
void do_check_create ( object b );
void do_check_clean ( object b );
void kill_all_bodies ( void );
void start_parent ( object user, string input, string desc, object env );
void setup_body ( object body );
object make_child ( object parent, object env );
string set_rogueparent ( void );

mapping parents;
mapping children;
mapping brain_delay;
mapping brain_pop;
mapping brain_look;
mapping rumors;

mapping traits;
mapping traitcnt;

mapping positions;
mapping blocks;
mapping dragmale;
mapping dragfemale;

object rogueparent;
mapping rogues;

string query_state_root() { return "NpcBodies:Bodies"; }

string query_describer() { return ur_name(this_object()); }

static 
void create() {
   set_object_name("SkotOS:NPC-Bodies");

   rumors = ([ ]);
   parents = ([ ]);
   children = ([ ]);
   brain_delay = ([ ]);
   brain_pop = ([ ]);
   brain_look = ([ ]);
   positions = ([ ]);
   
   traits = ([ ]);
   traitcnt = ([ ]);

   blocks = ([ ]);
   dragmale = ([ ]);
   dragfemale = ([ ]);

   HTTPD->register_root("NPC-Bodies");
   SAMD->register_root("NPC-Bodies");
}

/* -------
void now ( object o, string s ) {
   SysLog( "--->>> " + s + " " + dump_value(o->query_brain("ur")) );
}
-------- */

void reset() {
   kill_all_bodies();
   
   rumors = ([ ]);
   parents = ([ ]);
   children = ([ ]);
   brain_delay = ([ ]);
   brain_pop = ([ ]);
   brain_look = ([ ]);
   positions = ([ ]);
   blocks = ([ ]);
   dragmale = ([ ]);
   dragfemale = ([ ]);

   /* traits = ([ ]);
   traitcnt = ([ ]); */
}


int val ( string s ) {
   int i;
   if (!s) return 0;
   sscanf( s, "%d", i );
   return i;
}

mixed query_children() { return children; }

void patch ( void ) {
   if (!blocks) {
      blocks = ([ ]);
   }
   if (!dragmale) {
      dragmale = ([ ]);
   }
   if (!dragfemale) {
      dragfemale = ([ ]);
   }
   if (!rogues) {
      rogues = ([ ]);
   }
}

void patch_1b ( string s ) {
   string *things;  
   object *obs, o;
   int i, j;
		    
   things = "/usr/System/sys/idd"->query_objects( s );
   for( i=0; i<sizeof(things); i++ ) {
      SysLog( "thing: " + things[i] );
	  o = find_object( s+":"+things[i] );
	  obs = o->query_ur_children();
	  for( j=0; j<sizeof(obs); j++ ) {
         /* SysLog( dump_value(obs[j]) ); */
         obs[j]->set_property( "skotos:creator", "npc:old" );
	  }
	  SysLog( "found " + sizeof(obs) );
   }  
}

void patch_1 ( void ) {
   string *things;
   object *obs, o;
   int i;

   patch_1b( "Ur:clothing" );
   patch_1b( "Ur:drink" );
   patch_1b( "NPC:misc" );
   patch_1b( "Ur:food" );
}

void patch_2 ( void ) {
   rogueparent = clone_object("/base/obj/thing");
   set_rogueparent();
}

string set_rogueparent ( void ) {
   object r;

   r = rogueparent;

   r->set_brain( "name", "rogueparent" );
   r->set_capacity( 200.0 );
   r->set_brain( "inanimate", TRUE );

   r->set_object_name( "rogueparent" );
   r->set_brain( "numchild", 0 );

   setup_body( r );

   r->set_brain( "population", 0 );
   r->set_brain( "noisy", 0 );
   r->set_brain( "delay", 10 );
   r->set_brain( "ignore", TRUE );

   return "Rogue parent set up again";   
}

void set_rumor ( string idx, string text ) {
   int cnt;
   if ( rumors[ "num"+idx ]==nil )
      rumors[ "num"+idx ] = 0;
   cnt = rumors[ "num"+idx ];

   rumors[ idx+(cnt+1) ] = text;
   rumors[ "num"+idx ]++;
}

string query_rumor ( string idx ) {
   int cnt, x;
  
   if ( rumors[ "num"+idx ]==nil )
      return nil;
   cnt = rumors[ "num"+idx ];		   
   
   x = random(cnt);
   return rumors[ idx+(x+1) ];
}

int active_parent ( string bname );

void setup_body ( object brain ) {
   brain->set_brain( "numchild", 0 );
   brain->set_brain( "numaction", 0 );
   brain->set_brain( "numproto", 0 );
   brain->set_brain( "lifespan", "0" );
   brain->set_brain( "child", FALSE );
   brain->set_brain( "die", FALSE );
   brain->set_brain( "sticky", "0" );
   brain->set_brain( "feeding", TRUE );
   brain->set_brain( "noisy", "0" );
   brain->set_brain( "busy", FALSE );
   brain->set_brain( "shutdown", FALSE );
   brain->set_brain( "npc", TRUE );
   brain->set_brain( "resp_key", ({ }) );
   brain->set_brain( "resp_val", ({ }) );

   brain->set_capacity( 2.0 );
}

void list_rogues ( object user ) {
   int i;
   mixed *ar, m;
   string s;

   ar = map_indices( rogues );

   user->message( "\nRogues ------------------------- \n" );
   for( i=0; i<sizeof(ar); i++ ) {
      m = ar[i]->query_souls();
      m -= ({ nil });

      if ( m )
         m = sizeof(m);
      s = "   " + ur_name(ar[i]);
      s += " ("+rogues[ar[i]]+") at ";
      if ( ar[i]->query_environment()==nil )
         s += "in the NIL ";
      else
         s += name( ar[i]->query_environment() )+" ";
      s += dump_value(m)+"\n";
      user->message(s);
   }
}

void slaynils ( object user ) {
   string *list;
   object env;
   int i, count;

   list = map_indices( parents );
   for( i=count=0; i<sizeof(list); i++ ) {
      env = parents[list[i]]->query_environment();
	  if ( env==nil ) {
         count++;
		 parents[list[i]]->die();
	  }
   }

   list = map_indices( children );
   for( i=0; i<sizeof(list); i++ ) {
      env = children[list[i]]->query_environment();
	  if ( env==nil ) {
         count++;
		 children[list[i]]->die();
	  }
   }
   
   user->message( count + " CNPCs killed.\n" );
   
}


void list_bodies ( object user, string which ) {
   int i;
   string *list, s;
   object obj, parent;

   user->clear_more();
   
   if ( which==nil ) {
      user->add_more("\nParents ------------------------- ");
      list = map_indices(parents);
      for ( i=0; i<sizeof(list); i++ ) {
         s = "  "+list[i]+": "+ur_name(parents[list[i]])+" ";
         if ( blocks[ list[i] ] ) {
            s += " (blocking "+ blocks[ list[i] ]+")";
         }
		 user->add_more( s );
         if ( dragmale[ list[i] ] )
            user->add_more("    (drag men to "+describe_one(dragmale[ list[i] ])+") " );
         if ( dragfemale[ list[i] ] )
            user->add_more("    (drag women to "+describe_one(dragfemale[ list[i] ])+") " );
      }
      user->add_more("Children ------------------------ ");
   }
   
   list = map_indices(children);
   for( i=0; i<sizeof(list); i++ ) {
      obj = children[ list[i] ];
      if ( which==nil || sscanf( obj->query_brain("name"), "%*s"+which+"%*s") ) {
         s = "  "+obj->query_brain("name")+": "+list[i];
	     parent = obj->query_brain("parent");
         if ( parent )
            s += " ("+parent->query_brain("name")+")";
         else
            s += " (no parent!)";
						 
         if ( obj->query_environment() )
            s += " at "+describe_one(obj->query_environment());
         else 
            s += " at NIL :(";

         user->add_more( s );
		 
	  }
   }
   user->show_more();
}
		

void kill_all_bodies ( void ) {
   object *objs;
   int i;

   objs = map_values(parents);
   for( i=0; i<sizeof(objs); i++ )
      objs[i]->suicide();
   objs = map_values(children);
   for( i=0; i<sizeof(objs); i++ )
      objs[i]->suicide();

   /* --- positions = ([ ]);
   blocks = ([ ]);
   dragmale = ([ ]);
   dragfemale = ([ ]); --- */
   
   this_user()->message("All brains killed.\n");
}

void stop_parent ( object user, string bname ) {
   if ( parents[bname]==nil ) {
      user->message("That parent name is not being used.\n");
	  return;
   }
   parents[bname]->set_brain( "shutdown", TRUE );
   call_out( "kill_parent", 20, user, bname );
   user->message("Parent being shut down, may take a minute.\n");

   positions[bname] = nil;
   dragmale[bname] = nil;
   dragfemale[bname] = nil;
   blocks[bname] = nil;
}

void goto_parent ( object user, object body, string bname ) {
   if ( parents[bname]==nil ) {
      user->message("Invalid brain name.\n");
	  return;
   }
   body->act_teleport( parents[bname]->query_environment() );
}

void dump_traits ( mixed text ) {
   int i;
   string *s;
   s = map_indices( traits );
   for ( i=0; i<sizeof(s); i++ )
      if ( text ) {
         if ( sscanf( s[i], "%*s"+text+"%*s" ) )
            this_user()->message( s[i] + " = " + dump_value(traits[s[i]]) + "\n" );
	  } else {
         this_user()->message( s[i] + " = " + dump_value(traits[s[i]])+ "\n" );
	  }
}

void dump_parent ( object user, string bname ) {
   if ( parents[bname]==nil ) {
      user->message("Brain not being used.\n");
	  return;
   }
   parents[bname]->dump_brain( this_user() );
}

void list_on ( object user, string bname ) {
   object *obs;
   int prep;
   int cnt, i;
   
   if ( parents[bname]==nil ) {
      user->message( "Brain not being used.\n" );
	  return;
   }
   this_user()->message("brain "+bname);

   obs = parents[bname]->query_proximate();
   cnt = sizeof(obs);
  
   if ( cnt ) {
      prep = obs[i]->query_preposition();
      SysLog( describe_one(obs[i]) + " " + prep );
	  if ( prep==PREP_ON )
         SysLog("on!");
   } 
}

void kill_parent ( object user, string bname ) {
   positions[bname] = nil;
   if (parents[bname]) {
      parents[bname]->set_brain( "die", TRUE );
   }
}

void show_positions ( object user, varargs int show_woe ) {
   string *sar;
   int i;

   user->message( "\nParent positions --------------------- \n");
   sar = map_indices( positions );
   for( i=0; i<sizeof(sar); i++ ) {
      if (!parents[sar[i]]) {
	 /*
	  * Sanity-check: Sometimes 'positions' entries linger while the
	  *               parents aren't actually there anymore.
	  */
	 continue;
      }
      user->message( "  " + sar[i] + ": " +
		     ( show_woe ?
		       name(positions[sar[i]]) :
		       describe_one( positions[sar[i]] ) )+ "\n" );
   }
   user->message("\n");
}

void npc_reboot ( object user ) {
   int i;
   string *sar;
   
   kill_all_bodies();

   rumors = ([ ]);
   parents = ([ ]);
   children = ([ ]);
   brain_delay = ([ ]);
   brain_pop = ([ ]);
   brain_look = ([ ]);

   sar = map_indices( positions );
   
   for ( i=0; i<sizeof(sar); i++ )
      start_parent( user, sar[i], sar[i], positions[sar[i]] );

   user->message( "\nParents rebooted: "+i+"\n" );
}

void start_parent ( object user, string input, string desc, object env ) {
   string delay, pop;
   string lookdesc;
   mixed bname, bnum;

   sscanf( input, "%s%d", bname, bnum );
   if ( bname==nil || bnum==nil ) {
      user->message("To start a parent npc, name it something like: object1.\n" );
	  return;
   }
   
   if ( active_parent(input) ) {
      user->message("That parent already exists, at: "+describe_one(positions[input])+".\n");
      return;
   }

   if ( traitcnt[bname]==nil ) {
      user->message("Invalid brain name.\n");
	  return;
   }
   
   positions[ input ] = env;
   
   parents[input] = clone_object( "/base/obj/thing" );
   parents[input]->set_brain( "name", desc );
   parents[input]->set_capacity( 200.0 );

   if ( brain_pop[input]==nil )
      brain_pop[input] = "10";
   if ( brain_delay[input] == nil )
      brain_delay[input] = "10";
   delay = brain_delay[input];

   if ( brain_look[input] == nil )
      brain_look[input] = "You see nothing unusual about it.";
 
   setup_body( parents[input] );
   fill_body( parents[input], bname );
   parents[input]->set_brain( "inanimate", TRUE );

   if ( setup_obj( parents[input], env ) == FALSE )
      return;
   
   call_out( "npc_pulse", val(delay) * DELAY_FACTOR, parents[input] );
  
   user->message("Started brain: " + desc + "\n");
   
   brain_pop[input] = parents[input]->query_brain("population"); 
}

int active_parent ( string bname ) {
   if ( parents[bname]==nil ) 
	  return FALSE;
   return TRUE;
}

void set_block ( object user, string *words, object env, string mode ) {
   string *details, exit_dir, *exit_dirs, dirstr, bname, dir;
   NRef *exits, exit;
   int details_size, i, choice, j, found, k;
   
   if ( sizeof(words) < 3 ) {
      user->message("Specify exit detail.\n");
      return;
   }

   /* user->message( "words: "+dump_value(words)+"\n" ); */
   bname = words[0];
   dir = "";
   for( i=2; i < sizeof(words); i++ ) {
      dir += words[i] + " ";
   }
   dir = dir[0..strlen(dir)-2];
   /* user->message("dir: " + dump_value(dir)+"\n" ); */
   
   details = ur_details(env);
   details_size = sizeof(details);
   exits = ({ });
   exit_dirs = ({ });

   if ( !active_parent(bname) ) {
      user->message("There is no such parent, remember to use the number after the name.\n");
	  return;
   }
  
   for( i=0; i<sizeof(details); i++ ) {
      if ( details[i] == dir ) {
         found = TRUE;
		 /* user->message( "Found: " + details[i] + "\n" ); */
		 if ( mode=="set" ) {
            parents[bname]->set_brain( "blocking", dir );
			blocks[bname] = dir;
			/* user->message("setting block for "+bname+"\n" ); */
		 } else {
            parents[bname]->set_brain( "blocking", nil );
			blocks[bname] = nil;
		 }
	  }
   }

   if ( !found )
      user->message("You did not specify a valid detail: "+dump_value(details)+"\n" );  
   else
      user->message("Done.\n"); 
}

void set_pop ( object user, string bname, string spop ) {
   if ( !active_parent(bname) ) {
      user->message("There is no such brain.\n");
      return;
   }

   parents[bname]->set_brain( "population", spop );
   user->message( "Population set.\n");
}

void set_delay ( object user, string bname, string sdelay ) {
   if ( !active_parent(bname) ) {
      user->message("There is no such brain.\n");
      return;
   }

   parents[bname]->set_brain( "delay", sdelay );
   user->message( "Delay set.\n" );
}

void set_desc ( string bname, string slook ) {
   if ( !active_parent(bname) ) {
      this_user()->message("There is no such brain.\n");
      return;
   }

   parents[bname]->set_description( "npcskin", "look", slook );
   this_user()->message("Look description set.\n");
}

void set_teleport ( object user, string bname, object env, string gender ) {
   if ( !active_parent(bname) ) {
      this_user()->message("There is no such brain.\n");
      return;
   }

   if ( gender=="male" ) {
      dragmale[ bname ] = env;
   } else {
      dragfemale[ bname ] = env;
   }

   /* ---------------
   parents[bname]->set_brain( "teleport-"+gender, env );
   ---------------- */
   
   user->message( "Destination set.\n" );
}

void _debug ( string s ) { /* SysLog(s); */  }

void add_trait ( string bname, SAM desc ) {
   string bid;

   if ( traitcnt[bname] )
      traitcnt[bname]++;
   else
      traitcnt[bname]=1;

   bid = bname + traitcnt[bname];
   traits[bid] = desc;
   /* SysLog( "bname: "+bname+ " bid: "+bid+" desc: "+desc ); */
}

string *query_parents ( void ) {
   return map_indices( traitcnt );
}

string *query_traits ( string bname ) {
   int a;
   string *trlist, s;
   trlist = ({ });
   
   for( a=1; a<=traitcnt[bname]; a++ ) {
      s = bname+a;
	  trlist += ({ traits[s] });
   }
   /* _debug( "list size: "+sizeof(trlist) ); */
   return trlist;
}

void clear_all_bodies() {
   traitcnt = ([ ]);
   traits = ([ ]);
}

void clear_parents ( string bname ) {
   int a;

   SysLog( "clear_parents "+dump_value(bname));
   
   if (traitcnt[bname] != nil) {
      for ( a=1; a<traitcnt[bname]; a++ ) {
	 traits[bname+a] = nil;
      }
      traitcnt[bname] = nil;
   }
}

void fill_body ( object brain, string bname ) {
   string *tridx, s1, s2, parent, id, *trlist;
   int i, j;

   /* trlist = query_traits( bname ); */

   tridx = query_parents();
  
   for(i = 0; i < sizeof(tridx); i++) {
      id = nil;
      if (!sscanf(tridx[i], "%s-%s", parent, id)) {
	 parent = tridx[i];
      }
      if (parent != bname) {
	 continue;
      }
      trlist = query_traits( tridx[i] );
      for( j=0; j<sizeof(trlist); j++ ) {
	 if (!trlist[j]) {
	    error("empty trait list for index: " + tridx[i]);
	 }
	 sscanf( trlist[j], "%s=%s", s1, s2 );
	 switch( s1 ) {
	 case "proto":
	 case "work":
	 case "play":
	 case "leave":
	 case "arrive":
	 case "brain":
	 case "create":
	 case "alias":
	 case "trait":
	    if ( id ) {
	       brain->set_brainX( id+"_"+s1, s2 ); 
	    } else {
	       brain->set_brainX( s1, s2 );
	    }
	    break;
	 default:
	    if ( id==nil ) {
	       brain->set_brain( s1, s2 );
	    } else {
	       brain->set_brain( id+"_"+s1, s2 );
	    }
	    break;
	 }
	 
      }
   }
}



void set_trait ( string str ) {
   add_trait( g_trait, str );
}

string pick_proto ( object brain ) {
   int i, max, weight, j, final;
   int *tbl, tblcnt;
   string proto;

   tbl = ({ });
   tblcnt = 0;
   max = brain->query_brain("numproto");
   for ( i=0; i<max; i++ ) {
      sscanf( brain->query_brain("proto"+(i+1)), "$(freq=%d)%s", weight, proto );
      for( j=0; j<weight; j++ ) {
         tbl += ({ i });
         tblcnt++;
	  }
   }

   final = tbl[ random(sizeof(tbl)) ];
   sscanf( brain->query_brain("proto"+(final+1)), "$(freq=%d)%s", weight, proto );
   return proto;
}

void fill_child ( object child, object parent ) {
   string proto, s, st1, st2, desc1, desc2, *sar1, *sar2, s2, s3, s4;
   int i, j, fnd;
   
   sar1 = sar2 = ({ });
   proto = pick_proto( parent );

   child->set_brain( "lifespan", parent->query_brain( proto+"_lifespan" ) );
   child->set_brain( "name", proto );
   child->set_brain( "child", TRUE );
   child->set_brain( "sticky", parent->query_brain(proto+"_sticky") );
   child->set_brain( "feeding", TRUE );
   child->set_brain( "noisy", parent->query_brain(proto+"_noisy") );
   child->set_brain( "busymsg", parent->query_brain(proto+"_busymsg") );
   child->set_brain( "delay", parent->query_brain("delay") );

   if ( parent->query_brain( "num"+proto+"_arrive" ) ) {
      i = parent->query_brain( "num"+proto+"_arrive" );
	  s = parent->query_brain( proto+"_arrive"+(random(i)+1) );
	  child->set_brain( "arrive", s );
   }
   if ( parent->query_brain( "num"+proto+"_leave" ) ) {
      i = parent->query_brain( "num"+proto+"_leave" );
	  s = parent->query_brain( proto+"_leave"+(random(i)+1) );
      child->set_brain( "leave", s );
   }
   
   if ( parent->query_brain(proto+"_blocker") ) { 
      child->set_brain( "blocker", parent->query_brain(proto+"_blocker") );
   }

   if ( parent->query_brain( proto+"_generous") ) 
      child->set_brain( "generous", parent->query_brain( proto+"_generous" ) );
   else
      child->set_brain( "generous", "0" );
 
   if ( parent->query_brain( proto+"_ask" ) )
      child->set_brain( "ask", parent->query_brain( proto+"_ask" ) );

   if ( parent->query_brain( proto+"_auto-accept" ) )
      child->set_brain( "auto-accept", parent->query_brain( proto+"_auto-accept" ) );

   if ( parent->query_brain( proto+"_deliver" ) )
      child->set_brain( "deliver", parent->query_brain( proto+"_deliver" ) );
  
   if ( parent->query_brain( proto+"_lang" ) )
      child->set_brain( "lang", parent->query_brain( proto+"_lang" ) );
   
   if ( parent->query_brain("numresp_"+proto) ) {
      i = parent->query_brain("numresp_"+proto);
      for ( j=0; j<i; j++ ) {
         sscanf( parent->query_brain("resp_"+proto+(j+1)), "%s=%s", desc1, desc2 );
		 sar1 += ({ desc1 });
		 sar2 += ({ desc2 });
	  }
	  child->set_brain( "resp_key", sar1 );
	  child->set_brain( "resp_val", sar2 );
   }
   
   if ( parent->query_brain( "lang_"+proto ) ) {
      child->set_brain( "lang", parent->query_brain( "lang_"+proto ) );
   }
 
   if ( parent->query_brain( "num"+proto+"_brain" ) ) {
      i = parent->query_brain("num"+proto+"_brain");
	  for ( j=0; j<i; j++ )
         child->set_brainX( "brain", parent->query_brain( proto+"_brain"+(j+1) ) );
   }

   if ( parent->query_brain( "num"+proto+"_create" ) ) {
      i = parent->query_brain( "num"+proto+"_create" );
	  for ( j=0; j<i; j++ ) {
	     s = parent->query_brain( proto+"_create"+(j+1)  );   
         sscanf( s, "%s/%s/%s", s2, s3, s4 );		 
         child->set_brainX( "create", s );
		 child->set_brainX( "create-"+s2, s3+"/"+s4 );
	  }
   }

   if ( parent->query_brain( proto+"_create_property" ) ) {
      child->set_brain( "create_property", parent->query_brain( proto+"_create_property" ) );
   }
  
   if ( parent->query_brain( proto+"_ur" ) ) {
      child->set_brain( "ur", parent->query_brain( proto+"_ur" ) );
   }
   
   if ( parent->query_brain( "numtalk_"+proto ) ) {
      i = parent->query_brain( "numtalk_"+proto );
	  for ( j=0; j<i; j++ )
         child->set_brainX( "talk", parent->query_brain("talk_"+proto+(j+1) ) );
   }
   
   if ( parent->query_brain( "numnude_"+proto ) ) {
      i = parent->query_brain( "numnude_"+proto );
      for ( j=0; j<i; j++ ) {
         child->set_brainX( "nude", parent->query_brain("nude_"+proto+(j+1) ) );
	  }
   }
   
   if ( parent->query_brain( proto+"_busy" ) ) {
      sscanf( parent->query_brain( proto+"_busy" ), "%d", i );
      if ( random(100) < i ) {
         child->set_brain( "busy", TRUE );
         j = random( parent->query_brain("num"+proto+"_work" ) );
         desc1 = parent->query_brain( proto+"_work"+(j+1) );
      } else {
         child->set_brain( "busy", FALSE );
         j = random( parent->query_brain("num"+proto+"_play" ) );
         desc1 = parent->query_brain( proto+"_play"+(j+1) );
      }
   
      child->set_property( "NPC-Activity", desc1 );
   }

}


void spawn_child ( object user, object body, string parent, string child ) {
   int i;
   object rogue;
   
   SysLog( "spawn parent = " + dump_value(parent) + " child = " + dump_value(child) );
   SysLog( "rogueparent = " + dump_value(rogueparent) );
   if ( traitcnt[ parent+"-"+child ] ) {
      rogueparent->erase_braindata();
	  setup_body( rogueparent );
	  rogueparent->set_brain( "name", parent );
      fill_body( rogueparent, parent );

      /* "/usr/TextIF/lib/cmds/debug"->stat_brain( user, rogueparent ); */
	  
      for( i=0; i<rogueparent->query_brain("numproto"); i++ ) {
         rogueparent->set_brain( "proto"+(i+1), nil );
	  }
      rogueparent->set_brain( "proto1", "$(freq=1)"+child );
	  rogueparent->set_brain( "numproto", 1 );

      rogue = make_child( rogueparent, body->query_environment() );
	  user->message("Rogue NPC spawned. You are responsible for deleting it.\n");
	 
      rogues[ rogue ] = parent+"@"+child+" by "+user->query_name();
	  
   } else {
      user->message("Unknown brain, use the format: +npc spawn parent child.\n" );
	  return;
   }
}


atomic
object make_child ( object brain, object env	) { 
   object newchild, npcbody, urobj;
   int i, x, num;
   string name, dtl;
   mixed brief;

   newchild = clone_object( "/base/obj/thing" );

   children[ur_name(newchild)] = newchild;
   setup_body( newchild );

   newchild->set_brain( "parent", brain );
   fill_child( newchild, brain );
   newchild->set_brain( "child", TRUE );
   newchild->set_volition( TRUE );

   urobj = find_object( newchild->query_brain("ur") );

   newchild->set_ur_object( urobj );

   i = brain->query_brain( "numchild" );
   name = newchild->query_brain( "name" );

   setup_obj( newchild, env );

   newchild->possess(this_object());

   dtl = ur_prime_id( urobj, "outside" );

   if (!urobj) {
      /* Eh? -EH */
      DEBUG("No urobj?  newchild = " + dump_value(newchild) +
	    ", query_brain(ur) = " + dump_value(newchild->query_brain("ur")));

      error("cannot find NPC ur-child: " +
	    dump_value(newchild->query_brain("ur")));
   }
   brief = urobj->query_description( dtl, "brief" );
   if ( brief )
      newchild->set_brain( "brief", brief );
   else
      SysLog( "ERROR: No brief for " + newchild->query_brain("ur") );

   choose_brain( newchild );
   call_out( "npc_pulse", (random(val(brain->query_brain("delay"))) + 1) * DELAY_FACTOR, newchild );

   return newchild;
}



void describe(string event, object *users, object body, mixed args...) {
   object *inv;
   int i;

   if (body->query_environment()) {
      inv = body->query_environment()->query_inventory();
      for (i = 0; i < sizeof(inv); i ++) {
	 if (!sizeof(inv[i]->query_souls() & users)) {
	    inv[i] = nil;
	 }
      }
      inv -= ({ nil });
      /* inv is now an array of NPCs in this room */

      call_other(this_object(), "handle_" + event, inv, body, args...);
   }
}

void handle_social (object *inv, object who, object pverb, mapping args) {
   NRef *targets;
   object ob;

   if (args["roles"] && sizeof(args["roles"])) {
      targets = args[args["roles"][0]];
      ob = NRefOb(targets[0]);
      if (member(ob, inv)) {
	 if (args["evoke"]) {
	    NPCBRAINS->check_speech(ob, who, args["evoke"]);
	 } else {
	    NPCBRAINS->check_action(ob, who, pverb->query_third());
	 }
      }
   }
}

void handle_arrive(object *inv, object who, varargs NRef entrance ) {
   int i;

   for (i = 0; i < sizeof(inv); i ++) {
      NPCBRAINS->check_arrive(inv[i], who);
   }
}

void check_population ( object brain ) {
   int i;
   i = brain->query_brain( "numchild" );
  
   catch {
      while ( i < val(brain->query_brain("population")) ) {
	 make_child( brain, brain->query_environment() );
	 brain->set_brain( "numchild", ++i );
      }
   } : {
      brain->set_brain("die", TRUE);
      brain->set_brain("shutdown", TRUE);
   }   
}

void check_lifespan ( object brain ) {
   int i;
   string s;

   if ( brain->query_brain("parent") == rogueparent ) {
      return;
   }
   
   if ( brain->query_brain("feeding")==FALSE || brain->query_brain("sticky")=="100" ) {
	  /* _debug("lifespan check for "+brain->query_brain("name") ); */
      i = val( brain->query_brain( "lifespan" ) );

      if ( i==1 ) {   /* time to die! */
         brain->set_brain( "die", TRUE );	
      } else {
	     s = ""+(i-1);
         brain->set_brain( "lifespan", s );
      }
   }
}

void child_move ( object brain, string movetext ) {
   object env, error;
   string *details;
   NRef *exits;
   int details_size, i, choice;
  
   env = brain->query_environment();   
   if (!env) {
      return;
   }
   details = ur_details(env);
   details_size = sizeof(details);
   exits = ({ });

   for( i=0; i<details_size; i++ ) {
      if (ur_exit_dest(env, details[i]) && !env->is_hidden(details[i])) {
         exits += ({ NewNRef( env, details[i]) });
      }
   }

   if (sizeof(exits) > 0) {
      choice = random(sizeof(exits));
      brain->action( "enter", ([ "what" : exits[choice] ]) );
   } /* else NPC is stuck, too bad -- patched by Zell 001118 */

   brain->set_query( "feeding", FALSE );
}


void do_check_sticky ( object brain ) {
   mixed m;
   int i;
   i = val( brain->query_brain("sticky") );

   m = brain->query_souls();
   if ( m ) {
      if ( sizeof(m) > 1 ) {     /* skip if NPC is possessed */
         return;
      }
   }

   if ( i==100 )
      return;
   
   if ( i && random(100) > i && random(10)==1 ) {
      child_move( brain, "" );
   }
}

int is_home ( object brain ) {
   if ( brain->query_brain("child")==FALSE )
      return TRUE;

   if ( brain->query_environment() == nil )
      return FALSE;
   
   if ( brain->query_brain("parent") == nil )
      return FALSE;
   
   if ( brain->query_brain("parent")->query_environment()==nil )
      return FALSE;
   
   return ( brain->query_environment() == brain->query_brain("parent")->query_environment() );
}

int is_dragging ( object user, object body ) {
   mixed dragger;
   object draguser;

   if (dragger = body->query_property("dragger")) {
      if (dragger->query_environment() == body->query_environment()) {
         dragger->action("dragwiggle");
         return TRUE;
      }
   }
   return FALSE;
}

int is_blocking ( object user, string exit ) {
   object *ppl, room, parent;
   int i;
   string dtl;

   room = user->query_body()->query_environment();
   if ( !room )
      return FALSE;
   ppl = room->query_inventory();

   /* user->message( "is_blocking ppl: " + dump_value(ppl) + "\n" ); */

   for( i=0; i<sizeof(ppl); i++ ) {
      if ( ppl[i]->query_brain("blocker") ) {
         if ( is_home( ppl[i]) ) {
            parent = ppl[i]->query_brain("parent");
	    if ( parent && blocks[ parent->query_brain("name") ]==exit ) {
               user->message("You are blocked by "+describe_one(ppl[i])+".\n");
	       SysLog("Blocking guard: " + ur_name(ppl[i]));
               return TRUE;
	    } 
	 }
      }
   }
   return FALSE;
}

void choose_brain ( object brain ) {
   int i;
   i = random( brain->query_brain("numbrain") );
   brain->set_brain( "brain", brain->query_brain("brain"+(i+1)) );
}

void npc_pulse ( object brain ) {
   int i;
   object error, parent;

   if ( brain==nil )  /* we're dead */
      return;

   parent = brain->query_brain("parent");
   if ( brain->query_brain("child")==TRUE && parent==nil ) {
      SysLog( "NPC child with no parent, killing "+ ur_name(brain) );
	  brain->act_quit();
	  brain->die();
	  return;
   }

   if ( brain->query_environment()==nil ) {
      SysLog( "NPC with no environment, killing " + ur_name(brain) );
	  brain->act_quit();
	  brain->die();
	  return;
   }

   if ( brain->query_brain("die")==FALSE ) {
      int delay;

      delay = val(brain->query_brain("delay"));
      if (delay < 2) {
	 /* SysLog("WARNING:: Brain " + dump_value(brain) + " with delay < 2!"); */
      }
      delay = random(delay) + 1;	/* test -- Zell */

      call_out("npc_pulse", delay * DELAY_FACTOR, brain);
   } else {
      if ( parent ) {
         i = parent->query_brain("numchild");
	 parent->set_brain( "numchild", i-1 );
         children[ ur_name(brain) ] = nil;
      } 
      brain->act_quit();
      brain->die();
      return;
   }
   if ( brain->query_brain("shutdown")==FALSE ) {
      if ( brain->query_brain("child")==FALSE ) {
         check_population( brain );
      } else {
         check_lifespan( brain );
   
	 choose_brain( brain );
         /* SysLog( "chosen brain: "+brain->query_brain("name") ); */
		 
	 NPCBRAINS->do_brain_action( brain );

	 do_check_create( brain );
	 do_check_clean( brain );
	  
     NPCBRAINS->do_check_nudists( brain ); 

     do_check_sticky( brain ); 

	 if ( brain->query_brain("deliver") )
            NPCBRAINS->do_deliver( brain );
      }
   }

   parent = brain->query_brain( "parent" );
   if ( parent==nil )
      return;
   
   if ( parent->query_brain("shutdown")==TRUE ) {
      brain->set_brain("die", TRUE);
   } else { 
      if ( brain->query_brain("child")==TRUE )
         brain->set_brain( "delay", parent->query_brain("delay") );
   }
}

int setup_obj ( object obj, object env ) {
   object error;
   string ur;

   ur = obj->query_brain("ur");
   /* SysLog( "obj: "+dump_value(ur_name(obj)) ); 
   SysLog( "ur: "+dump_value( ur ) ); */

   if ( ur==nil )
      return FALSE;

   obj->set_ur_object( find_object(ur) );
   
   if (!env) {
      SysLog("ACKK! No environment.");
	  return FALSE;
   }
   
   obj->act_teleport( env );
   return TRUE;
}

string test() {
   _debug( "Brain test" );
}

int parent_empty ( object parent, string ctype ) {
   object *obs;
   int prep, cnt, i;
   mixed thisctype;
   
   obs = parent->query_proximate();
   cnt = sizeof(obs);
   if ( !cnt )
      return TRUE;

   for ( i=0; i<cnt; i++ ) {
      prep = obs[i]->query_preposition();
	  if ( prep==PREP_ON ) {
         thisctype = obs[i]->query_brain("ctype");
		 if ( thisctype ) {
            if ( thisctype == ctype ) {
               return FALSE;
			}
		 }
	  }
   }   
   return TRUE;
}

object make_obj ( string ur, string ctype, object dest ) {
   object obj, gift;
   string s;
   
   obj = find_object( ur );

   gift = spawn_thing(obj);
   
   gift->set_brain( "ctype", ctype );
   
   gift->move(dest);
   return gift;
}

object *query_cleanable_things( object room ) {
   object *obs, *out;
   int i, j;

   obs = room->query_inventory();
   out = allocate( sizeof(obs) );

   for( i=j=0; i<sizeof(obs); i++ ) {
      if ( !obs[i]->query_proximity() || obs[i]->query_preposition() != PREP_ON ) {
         out[j++] = obs[i];
	  } else {
         /* ---- SysLog( "!!!! skipping " + describe_one(obs[i]) + " " + 
            dump_value( obs[i]->query_proximity() ) ); ---- */
	  }
   }

   return out[.. j-1];
}
   

void do_check_clean( object brain ) {
   mixed bname, age, lastdropped, *temp;
   object *obs, *skip;
   string creator;
   int i, j, prep;
   
   bname = brain->query_brain("brain");
   if ( bname==nil )
      return;

   if ( brain->query_brain("deliver") != "trash" ) {
      return;
   }

   if ( brain->query_environment()->query_property( "npc:disable-clean" ) ) {
      /* SysLog( "No cleaning in here!  Pay your bill!" ); */
	  return;
   }
   
   if ( random(3) != 1 )
      return;
   
   if ( random(100) < val( brain->query_brain("noisy") ) ) {
      obs = query_cleanable_things( brain->query_environment() );
	  
      for( i=0; i<sizeof(obs); i++ ) {
         if ( obs[i]==nil )
            continue;

         creator = obs[i]->query_property("skotos:creator");
	 if ( creator && sscanf( creator, "npc:%*s" ) ) {
            lastdropped = obs[i]->query_property("misc:lastdropped");
	    if ( lastdropped ) {
               sscanf( lastdropped, "%d %*s", age );
	       /* SysLog( describe_one(obs[i])+" drop: " + lastdropped ); */
	       if ( age ) {
                  age = ( time()-age ) / 60;

                  if ( age > 10 ) {   /* 10 minutes before gets cleaned */ 
                     NPCBRAINS->temp_social( capitalize(describe_one(brain))
					     + " picks up " +
					     describe_one(obs[i]) + ".", 
					     brain, nil, brain->query_brain("name") );

		     obs[i]->die();
		  }
	       }
	    }
	 }
      }
	  
   }
}


void do_check_create( object brain ) {
   mixed bname, num;
   object parent, obj;
   string ur, desc, s, ctype;
   NRef dest;
   int choice;

   bname = brain->query_brain("brain");
   if ( bname==nil )
      return;

   parent = brain->query_brain("parent");
   if ( parent==nil )
      return;

   num = brain->query_brain( "numcreate" );   
   if ( num == nil )
      return;

   /* SysLog( traits[bname+"-act"+1] ); */
   /* SysLog( traits[bname+"-create"+1] ); */
   
   if ( random(100) < val( brain->query_brain("noisy") ) ) {
       if ( brain->query_environment() == parent->query_environment() ) {
	   choice = random(num)+1;
	   s = "create"+choice;
	   sscanf( brain->query_brain(s), "%s/%s/%s", ctype, ur, desc );
	   if ( parent_empty( parent, ctype ) ) {
	       if (find_object(ur)) {
		   obj = make_obj( ur, ctype, brain );
		   dest = NewNRef( parent, ur_prime_id( parent, "outside" ) );
		   obj->move( parent->query_environment(), dest );
		   obj->set_preposition( PREP_ON );
		   obj->set_property( "skotos:creator", "npc:"+brain->query_brain("name") );
		   desc = NPCBRAINS->xform_targets( desc, brain, nil );
		   NPCBRAINS->temp_social( desc, brain, obj, brain->query_brain("brain") );
	       } else {
		 WARNING("npc-bodies: " + describe_one(brain) + " (" +
			 name(brain) + ") wants " + ur + " which does not exist (anymore?)");
	       }
	   }
       }
   }
}

mixed query_dragmale ( string idx ) {
   if (dragmale) {
      return dragmale[idx];
   }
}

mixed query_dragfemale ( string idx ) {
   if (dragfemale) {
      return dragfemale[idx];
   }
}


