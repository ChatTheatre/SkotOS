# include <type.h>
# include <base.h>
# include <limits.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <HTTP.h>
# include <SAM.h>
# include <base/bulk.h>
# include <base/preps.h>
# include <base/living.h>

private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";

inherit "/usr/SID/lib/stateimpex";

inherit "/lib/string";
inherit "/lib/array";
inherit "/lib/url";
inherit "/lib/mapargs";
inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";

void _debug ( string s );
string xform_symbols ( string s, object brain );
string xform_target ( string s, object brain, string bname );
string get_brain_talk ( object o );
void temp_social ( string desc, object npc, object target, string bname );

mapping traits;
mapping traitcnt;

mapping npcinv;

mapping positions;

string query_state_root() { return "NpcBrains:Brains"; }

string desc_one ( object b ) { return describe_one(b); }

mapping query_npcinv() { return npcinv; }

static 
void create() {
   set_object_name("SkotOS:NPC-Brains");
		
   traits = ([ ]);
   traitcnt = ([ ]);
   npcinv = ([ ]);
}

void reset() {
   npcinv = ([ ]);
   positions = ([ ]);
}

int val ( string s ) {
   int i;
   sscanf( s, "%d", i );
   return i;
}

int active_brain ( string bname );

void _debug ( string s ) { /* SysLog(s); */  }

int is_punc ( string test ) {
   switch( test ) {
      case ":":
      case ".":
      case "?":
      case ",":
      case "-":
      case "\"":
      case "'":
      case ";":
         return TRUE;
   }
   return FALSE;
}

string strip_punc ( string str ) {
   int i, j;
   j = 0;
   for ( i=0; i<strlen(str); i++ ) {
      if ( !is_punc( str[i..i] ) )
         str[j++] = str[i];
   }
   return str[.. j-1];
}

string *parse_words ( string sentance ) {
   string *words, *output;
   int i, j;
   
   words = explode( sentance, " " );
   for ( i=0; i<sizeof(words); i++ ) 
      words[i] = strip_punc( words[i] );

   return words;
}

void dump_traits ( void ) {
   int i;
   string *s;
   s = map_indices( traits );
   for( i=0; i<sizeof(s); i++ )
      this_user()->message( s[i]+" = "+dump_value(traits[ s[i] ])+"\n" );
	
}


void add_trait ( string bname, SAM desc ) {
   string bid;

   if ( traitcnt[bname] )
      traitcnt[bname]++;
   else
      traitcnt[bname]=1;

   bid = bname + traitcnt[bname];
   traits[bid] = desc;
}

void scrolls_info ( void ) {
   int i, j;
   object *scrolls, to;
   string from;
  
   scrolls = (npcinv["courier"] - ({ nil })) +
             (npcinv["page"] - ({ nil }));

   this_user()->message( "Limbo scrolls: "+sizeof(scrolls)+"\n");
   for ( j=0; j<sizeof(scrolls); j++ ) {
      this_user()->message( "scroll: "+ur_name(scrolls[j]));
      if (to = scrolls[j]->query_brain("deliver-to")) {
	 this_user()->message( " to " + describe_one(to));
	 from = scrolls[j]->query_description( "objskin", "delivery-from" );
	 if ( from ) {
	    this_user()->message( " from " + from + "\n" );
	 } else {
	    this_user()->message( " (unsigned)\n" );
	 }
      }
   }
}

mixed *scrolls_data() {
   int    i, sz;
   object *scrolls;
   mixed  *result;

   scrolls = (npcinv["courier"] - ({ nil })) +
             (npcinv["page"] - ({ nil }));
   sz = sizeof(scrolls);
   result = allocate(sz);
   for (i = 0; i < sz; i++) {
      result[i] = ({
	 scrolls[i],
	 scrolls[i]->query_description("objskin", "delivery-from"),
	 scrolls[i]->query_brain("deliver-to"),
      });
      if (result[i][2]) {
	 result[i][2] = describe_one(result[i][2]);
      }
   }
   return result;
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
   return trlist;
}

void clear_all_brains() {
   traitcnt = ([ ]);
   traits = ([ ]);
}

void clear_parents ( string bname ) {
   int a;

   if (traitcnt[bname]) {
      for ( a=1; a<traitcnt[bname]; a++ ) {
	 traits[bname+a] = nil;
      }
      traitcnt[bname] = nil;
   }
}

int is_naked(object obj) {
   if (obj->query_male()) {
      return !sizeof(obj->query_worn_on_location("groin"));
   }
   if (obj->query_female()) {
      return !sizeof(obj->query_worn_on_location("groin")) ||
	 !sizeof(obj->query_worn_on_location("chest"));
   }
   return FALSE;	/* what to do with neuters? */
}

object nudist_present ( object brain, object *objs ) {
   int i, naked;

   for (i = 0; i < sizeof(objs); i++) {
      if (objs[i] && ur_clothes_expected(objs[i]) && is_naked(objs[i])) {
         return objs[i];
      }
   }
   return nil;
}

string pick_nudist_response ( string bname ) {
   int i;

   if ( traitcnt[bname+"-nude"] == nil )
      return nil;
				   
   i = random( traitcnt[bname+"-nude"] );
   return traits[ bname+"-nude"+(i+1) ];
}

int do_check_nudists ( object brain ) {
   object *ppl, nudist, parent;
   mixed env, dest;
   string dtl, brief, s1, s2, resp;
   int j, do_tele;
   
   env = brain->query_environment();
   if ( env==nil ) {
      return FALSE;
   }
   if ( env->query_property( "npc:disable-nude" ) ) {
      /* SysLog( "No checking for nudists here!" ); */
      return FALSE;	  
   }
   
   ppl = brain->query_environment()->query_inventory();

   if (brain->query_brain("brain") == nil) {
      return FALSE;
   }

   if (nudist = nudist_present(brain, ppl)) {
      brief = describe_one(nudist);

      do_tele = FALSE;
      parent = brain->query_brain("parent");
      if (parent == nil) {
         return FALSE;
      }
      if (nudist->query_male()) {
         dest = NPCBODIES->query_dragmale( parent->query_brain("name") );
      } else if (nudist->query_female()) {
         dest = NPCBODIES->query_dragfemale( parent->query_brain("name") );
      }
      if (dest == nil) {
         return FALSE;
      }

      resp = pick_nudist_response(brain->query_brain("brain"));
      if (!resp) {
         return TRUE;
      }
      resp = xform_target( resp, brain, brain->query_brain("brain") );
	  
      sscanf( resp, "%s$(PC)%s", s1, s2 );
      s1 = xform_symbols( s1, brain );
      s2 = xform_symbols( s2, brain );
      if ( sscanf( s1, "$(tele)%*s" ) ) {
         do_tele = TRUE;
	 sscanf( s1, "$(tele)%s", s1 );
      }
      for (j = 0; j < sizeof(ppl); j ++) {
         if (ppl[j] == nudist) {
            ppl[j]->act_brain(brain, s1 + "you" + s2 );
	 } else {
            ppl[j]->act_brain(brain, s1 + brief + s2 );
	 }
      }
      if (do_tele) {
	 nudist->act_teleport( dest );
      }
      return TRUE;
   }
   return FALSE;
}


string xform_symbols ( string action, object brain ) {
   string res1, res2, final, bname;

   final = action;
   if ( sscanf( final, "%*s$(talk)%*s") ) {
      sscanf( final, "%s$(talk)%s", res1, res2 );
      bname = get_brain_talk(brain);
      if (bname) {
	 final = res1 + capitalize(bname) + res2;
      }
   }
   return final;
}

string xform_target( string action, object brain, string bname ) {
   string final, res1, res2, name;

   final = action;
   if ( sscanf( final, "%*s$("+bname+")%*s") ) {
      sscanf( final, "%s$("+bname+")%s", res1, res2 );
      if (typeof(brain->query_brain("brief")) == T_STRING) {
	 final = res1 + brain->query_brain("brief") + res2;
      } else {
	 final = res1 + implode(brain->query_brain("brief"), " ") + res2;
      }
   }
   return final;
}

string xform_targets( string action, object brain, object target ) {
   string final;
   final = xform_target( action, brain, brain->query_brain("brain") );
   if ( target )
      final = xform_target( final, target, target->query_brain("brain") );
   final = xform_target( final, brain->query_brain("parent"), "parent" );
   /* SysLog( "parent: "+dump_value( brain->query_brain("parent") ) ); */
   return final;
}

string valid_brain_action ( object brain, string action ) {
   int i;
   string result, other, verb, res1, res2;
   object *ppl, room;
   
   if ( !action )
      return "";

   room = brain->query_environment();
   ppl = room->query_inventory();
   result = action = xform_symbols( action, brain );
   
   if ( sscanf( action, "$(%*s-%*s" ) ) {
	  res2 = ""; 
      if ( sscanf( action, "%*s$(PC)%*s" ) ) {
         sscanf( action, "$(%s-%s)%s$(PC)%s", verb, other, res1, res2 );		   
	  } else {
         sscanf( action, "$(%s-%s)%s", verb, other, res1 );
	  }
	  _debug( "res1: "+res1+" res2: "+res2 ); 
	  for ( i=0; i<sizeof(ppl); i++ ) {
         if ( ppl[i]==nil ) {
            continue;
	 }
	 if ( ppl[i]==brain ) {
            continue;  /* never count self as an other */
	 }
         if ( ppl[i]->query_brain("name") ) {
            if ( ppl[i]->query_brain("brain")==other ) {
               if ( res2 != "" )
                  result = res1 + ppl[i]->query_brain("name") + res2;
			   else
                  result = res1;

			   result = xform_targets( result, brain, ppl[i] );
			  
			   if ( ppl[i]->query_environment()==brain->query_environment() ) {
			       if ( verb=="kill" )
         			   ppl[i]->set_brain( "die", TRUE );
	    		   return result;
			   }
			}
		 }
	  }
	  return "abort";
   } else {
      result = xform_target( result, brain, brain->query_brain("brain") );
   }

   return result;
}

string pick_action( string bname ) {
   int i;

   i = random( traitcnt[bname+"-act"] );
   return traits[ bname+"-act"+(i+1) ];
}

int match_to ( object person, string sname ) {
   string name, *path;

   name = person->query_object_name();
   if ( !name )
      return FALSE;
   
   path = explode(name, ":");
   if (sizeof(path) >= 4) {
      return (lower_case(path[3]) == lower_case(sname));
   }

   if ( sscanf( name, "Marrach:NPCs:%s", name ) == 1 ) {
      return ( lower_case(name) == lower_case(sname) );
   }

   return FALSE;
}

void do_deliver ( object brain ) {
   string proto;
   object *inv;
   mixed to;
   int i;

   proto = brain->query_brain("name");
   /* extract the component of proto that comes before the slash */
   proto = explode(proto, "/")[0];
   inv = npcinv[proto];
   if (!inv) {
      return;
   }

   inv -= ({ nil });
   /* this can be sped up even further later with some set trickery */
   for (i = 0; i < sizeof(inv); i++) {
      to = inv[i]->query_brain("deliver-to");
      if (typeof(to) == T_STRING) {
	 object m;

	 m = UDATD->query_body(to);
	 if (!m) {
	    m = UDATD->query_body(lower_case(to));
	 }
	 if (!m) {
	    m = find_object("Marrach:players:" + capitalize(to[0..0]) +
			    lower_case(to));
	 }
	 if (!m) {
	    m = find_object("Marrach:NPCs:"+proper(to) );
	 }
	 if (!m) {
	    SysLog("WARNING:: Scroll " + dump_value(inv[i]) + " with deliver-to " + dump_value(to));
	    continue;
	 }
	 to = m;
	 inv[i]->set_brain("deliver-to", to);
      }
      if (!to) { 
        /* --------  Ahhhh spam
	SysLog("WARNING:: Scroll " + dump_value(inv[i]) + " deliver-to is NIL.");
        SysLog( "brain " + dump_value(brain) + " to " + dump_value(to) );
        ---------- */
        continue;
      }
     
      if (to->query_environment() == brain->query_environment()) {
	 /* this person is present */

	 inv[i]->move(to);
	 if (inv[i]->query_environment() == to) {
	    /* the move succeeded */
	    temp_social( "A $("+proto+") gives "+describe_one(inv[i],nil,nil,STYLE_NONPOSS)+" to $(PC).", brain, to, proto );

	    npcinv[proto] -= ({ inv[i] });
	 }
	 /* else just keep trying later */
      }
   }
}

void do_brain_action ( object brain ) {
   int i, x;
   string action, stripped;
   mixed bname, *ppl, room;

   if ( brain==nil ) {
      SysLog("Hey, Eric, nil brain!");
	  return;
   }
   
   bname = brain->query_brain("brain");
   if ( bname==nil )
      return;
   
   if ( traitcnt[bname+"-act"] == nil ) {
      return;
   }
   
   /* SysLog( "bname: "+bname ); */
   room = brain->query_environment();
   if ( room )
      ppl = room->query_inventory();

   if ( room == nil ) {
/*      SysLog( "Room is NIL for " + dump_value(brain) + " at " + dump_value(brain->query_environment() ) ); */
      return; 
   }
   if ( ppl == nil ) {
      return;
   }

   if ( random(100) < val( brain->query_brain("noisy") ) && random(10)==1 ) {
      action = pick_action( bname );
      action = valid_brain_action( brain, action );
	   
      if ( action==nil ) {
         SysLog("Hey, Eric, nil action for"+ur_name(brain));
		 return;
	  }
	  
	  if ( action!="abort" )
         for ( i=0; i<sizeof(ppl); i++ ) 
            if ( ppl[i] )
               ppl[i]->act_brain( brain, action );
   }
}


string get_brain_talk ( object brain ) {
   mixed val;
   string bname;
   int i;

   bname = brain->query_brain("brain");
   if ( traitcnt[bname+"-talk"] == nil )
      return "";

   i = random( traitcnt[bname+"-talk"] );
   val = traits[ bname+"-talk"+(i+1) ];
   if (typeof(val) == T_STRING) {
      return "\"" + val + "\"";
   }
   /* else it's SAM or NIL? What to do here? -- Zell 001118 */
}


void npc_give(object user, object body, object *what, object who) {
   string proto, creator, *ppl;
   object *objs, newobj;
   mixed i;
   int found;

   /* does the NPC in question accept trash for 'delivery'? */
   if (who->query_brain("deliver") == "trash") {
      found = FALSE;
      for(i = 0; i < sizeof(what); i++) {
	 if (!check_ownership(what[i], body) ||
	     what[i]->query_worn_by() ||
	     what[i] == body) {
	    continue;
	 }
         creator = what[i]->query_property("skotos:creator");
	 if (creator && sscanf(creator, "npc:%*s")) {
            what[i]->die();
	    found = TRUE;
	 }
      }
      if (found) {
         temp_social(capitalize(describe_one(who)) + " accepts " + 
		     "trash from $(PC).", who, body, who->query_brain("name"));
      } else {
         user->message(capitalize(describe_one(who)) +
		       " can't accept that.\n" );
      }
      return;
   }

   /* else if NPC is not a deliverer, or doesn't deliver this in particular */
   if (who->query_brain("deliver") == nil ||
       !what[0]->single_match(who->query_brain("deliver"))) {
      user->message(capitalize(describe_one(who)) +
		    " doesn't know what to do with that.\n");
      return;
   }

   if (what[0]->query_brain("deliver-to") == nil) {
      user->message( "You must address it to somebody first.\n" );
      return;
   }

   proto = who->query_brain("name");
   /* extract the component of proto that comes before the slash */
   proto = explode(proto, "/")[0];
   if (npcinv[proto] == nil) {
      npcinv[proto] = ({ });
   }

   npcinv[proto] |= ({ what[0] });
   what[0]->move(nil);

   temp_social("A $(" + proto + ") takes " +
	       describe_one(what[0], nil, nil, STYLE_NONPOSS) +
	       " from $(PC).", who, body, proto);

}

string has_trait ( object npc, string trait ) {
   int i;
   mixed bcnt;
   string bname;

   bcnt = npc->query_brain("numbrain");
   if ( bcnt==nil )
      return nil;

   for ( i=0; i<bcnt; i++ ) {
      bname = npc->query_brain("brain"+(i+1));
      if ( traitcnt[bname+trait] )
         return traits[bname+trait+"1"];
   }
   return nil;
}

object do_give ( object npc, string ur, object body, string str ) {
   object o, obj, error;
   int *ix;
   string s;

   o = find_object( ur );
   if ( o==nil ) { 
      SysLog( "Invalid name for UR object in do_give for NPCs: " + ur );
	  return nil;
   }
   
   sscanf( ur_name(o), "%s#", s );
   obj = clone_object( s );
   /* import_state( obj, export_state(o) ); */

   obj->set_ur_object( o );

   if ( npc->query_brain( "create_property" ) ) {
      obj->set_property( npc->query_brain( "create_property" ), TRUE );
   }
   
   obj->set_brain( "delivery", str );
  
   obj->set_property( "skotos:creator", "npc:"+npc->query_brain("name") );
   
   obj->move(body);

   return obj;
}

void ask ( object user, object npc ) {
   if ( npc->query_brain("ask") )
      user->message( npc->query_brain("ask")+"\n" );
   else
      user->message( capitalize(describe_one(npc))+" has nothing to offer you.\n" );
}

void ask_for ( object user, object body, object npc, string thing ) {
   string name, s, s2, s3, rawid;
   int i, pick;
   mixed max, lasttime;
   object o;
   
   name = npc->query_brain("name");
   if ( name==nil ) {
      user->message( capitalize(describe_one(npc))+ " has nothing to give you.\n" );
	  return;
   }
  
   max = npc->query_brain("numcreate-"+thing);
   if ( max==nil || !max ) {
      user->message( capitalize(describe_one(npc)) + " doesn't have that.\n" );
	  ask( user, npc );
	  return;
   }

   rawid = describe_one(body) + "-give-" + thing;
   lasttime = npc->query_raw_memory( rawid );

   if ( lasttime && lasttime > time() - 60 ) {
      user->message("You were just given that, ask again a bit later.\n");
	  return;
   }
   npc->set_raw_memory( rawid, time() );

   call_out("clear_memory", 60, npc, rawid);

   pick = random(max)+1;
   
   s = npc->query_brain( "create-"+thing+pick );
   sscanf( s, "%s/%s", s2, s3 );

   o = do_give( npc, s2, body, thing );
   if ( o ) {
      if (o->query_environment() == body) {
	 temp_social( "A $("+name+") gives "+
		      describe_one(o,nil,nil,STYLE_NONPOSS)+" to $(PC).",
		      npc, body, name );
      } else {
	 o->move(body->query_environment());
	 temp_social("A $(" + name + ") places " +
		     describe_one(o, nil, nil, STYLE_NONPOSS) + " on the ground.",
		     npc, body, name);
      }
   } else {
      user->message("The "+npc->query_brain("name")+" can't give you that at the moment.\n");
   }
}

static
void clear_memory(object npc, string rawid) {
   if (npc) {
      npc->set_raw_memory(rawid, nil);
   }
}


void temp_social ( string action, object npc, object target, string bname ) {
   int i;
   object *ppl;
   string output, s1, s2;

   /* SysLog( "temp_social "+action ); */
   if (!npc) {
      DEBUG("temp_social: npc = nil?");
      return;
   }
   if (!npc->query_environment()) {
      DEBUG("temp_social: npc = " + name(npc) + ", environment = nil?");
      return;
   }

   ppl = npc->query_environment()->query_inventory();
   output = xform_target( action, npc, bname );
   s1 = output; s2=nil;
   if ( sscanf( output, "%*s$(PC)%*s" ) ) {
      sscanf( output, "%s$(PC)%s", s1, s2 );
   }
   for ( i=0; i<sizeof(ppl); i++ ) {
      if ( s2 ) {
         if ( ppl[i]==target )
            ppl[i]->act_brain( npc, s1+"you"+s2 );
         else
            ppl[i]->act_brain( npc, s1+describe_one(target)+s2 );
	  } else {
         ppl[i]->act_brain( npc, output );
	  }
   }
}

string get_trait_token ( object npc, string trait, string token ) {
   mixed bcnt;
   int i, j;
   string bname, text, output, checktoken;
   mixed tcnt;

   bcnt = npc->query_brain("numbrain");
   if ( bcnt==nil )
      return nil;

   for ( i=0; i<bcnt; i++ ) {
      bname = npc->query_brain( "brain"+(i+1) );
      tcnt = traitcnt[bname+trait];
      if ( tcnt ) {
         for( j=0; j<tcnt; j++ ) {
            sscanf( traits[bname+trait+(j+1)], "%s=%s", checktoken, output );
			if ( checktoken==token )
               return bname+"="+output;
		 }
	  }
   }
   return nil;
}

string get_one_trait ( object npc, string trait ) {
   mixed bcnt;
   int i;
   string bname, output;
   mixed tcnt;

   bcnt = npc->query_brain("numbrain");
   if ( bcnt==nil )
      return nil;

   for ( i=0; i<bcnt; i++ ) {
      bname = npc->query_brain( "brain"+(i+1) ); 
      tcnt = traitcnt[bname+trait];
      if ( tcnt )
         return bname+"="+traits[ bname+trait+(random(tcnt)+1) ];
   }
   return nil;
}

void check_action ( object npc, object who, string verb ) {
   int i;
   mixed cnt;
   string bname, textverb, text, output;

   output = get_trait_token( npc, "-react", verb );
   if ( output ) {
      sscanf( output, "%s=%s", bname, output );
      call_out( "temp_social", 2, output, npc, who, bname );
   }
}

void check_speech ( object npc, object who, string speech ) {
   string *words, output, bname;
   int i;

   words = parse_words(lower_case(speech));
   for ( i=0; i<sizeof(words); i++ ) {
      output = get_trait_token( npc, "-resp", words[i] );
	  if ( output ) {
         sscanf( output, "%s=%s", bname, output );
         call_out( "temp_social", 2, output, npc, who, bname );
         return;      
	  }
   }

   output = get_one_trait( npc, "-noresp" );
   if ( output ) {
      sscanf( output, "%s=%s", bname, output );
      call_out( "temp_social", 2, output, npc, who, bname );
   }
}

void check_arrive ( object npc, object who ) {
   string output, bname;
   
   output = get_trait_token( npc, "-arrive", describe_one(who) );
   if ( output ) {
      sscanf( output, "%s=%s", bname, output );
      call_out( "temp_social", 2, output, npc, who, bname );
	  return;
   }
}
			
string test() {
   _debug( "Brain test" );
}

