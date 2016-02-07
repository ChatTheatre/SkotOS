# include <limits.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <HTTP.h>
# include <SAM.h>

inherit "/lib/womble";

private inherit "/base/lib/toolbox";

inherit "/usr/XML/lib/xmlparse";
inherit "/usr/SAM/lib/sam";
inherit "/usr/SID/lib/stateimpex";
inherit "/usr/SkotOS/lib/describe";
inherit "/base/lib/living/gender";

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/url";
private inherit "/lib/mapargs";
private inherit module "/lib/module";

private object start_handler;

mapping data;
mapping datacnt;

mapping badnames;

SAM description;

void womble_startstory() {
   data = womble(data);
   description = womble(description);
}

int query_data_cnt ( string topic );
void rexfer ( object udat, object body, string data );

string query_state_root() { return "StartStory:StartStory"; }

string strip_spaces ( string s ) {
   string *sar, n;
   int i;

   /* SysLog( "before: " + s ); */
   
   sar = explode( s, " " );  
   n = "";
   for( i=0; i<sizeof(sar); i++ ) {
      switch ( sar[i] ) {
         case "":
            break;
         case ".":
         case ",":
         case ";":
         case "-":
		    n += sar[i];
            break;
         default:
            if ( i )
               n += " ";
            n += sar[i];
			break;
	  }
   }

   /* SysLog( "after: " + n ); */
   
   return n;
}

void set_adjective ( object obj, string detail, string text ) {
   if ( strlen(text) > 0 )
      obj->add_adjective( detail, text );
}


static 
void create() {
   set_object_name("SkotOS:StartStory");

   data = ([ ]);
   datacnt = ([ ]);
   badnames = ([ ]);
}

void SAM_loaded() {
   SAMD->register_root("startstory");
}

void
register_start_handler(object ob)
{
    start_handler = ob;
}

void add_badname ( string name ) {
   string n, *sar, s;
   int i;
   sscanf( name, "\"%s\"", n );

   /* badnames[ lower_case(n) ] = TRUE; */
   
   sar = explode( n, " " );
   for( i=0; i<sizeof(sar); i++ )
      badnames[lower_case(sar[i])] = TRUE;

   if ( sizeof(sar) > 1 ) {
      s = "";
	  for ( i=0; i<sizeof(sar); i++ )
         s += sar[i];
	  badnames[ lower_case(s) ] = TRUE;
   }
}

void remove_badname ( string name ) {
   string n;
   sscanf( name, "\"%s\"", n );
   badnames[lower_case(n)] = nil;
}

mixed query_badname ( string name ) {
   string n, *sar, s;
   int i;
   
   sscanf( name, "\"%s\"", n );
   if ( badnames[lower_case(n)] )
      return TRUE;

   sar = explode( n, " " );
   /* SysLog( "sizeof sar: "+sizeof(sar) ); */
   if ( sizeof(sar) > 1 ) {
      s = "";
      for ( i=0; i<sizeof(sar); i++ )
         s += sar[i];
	  if ( badnames[ lower_case(s) ] )
         return TRUE;
   }

   s = lower_case(n);
   for( i=0; i<strlen(s); i++ ) {
      if ( s[i] < 'a' || s[i] > 'z' )
         return TRUE;
   }
   
   return nil;
}

void show_badnames(object user, varargs string pattern) {
   int    i, sz, match;
   string *names;

   if (pattern) {
       string *bits;
       pattern = implode(explode("%" + lower_case(pattern) + "%", "%"), "%%");
       bits = explode("*" + pattern + "*", "*");
       if (sizeof(bits) == 1) {
	  pattern = "%*s" + pattern;
	  match = 1;
       } else {
	  pattern = implode(bits, "%*s");
	  match = sizeof(bits) - 1;
       }
   }

   names = map_indices(badnames);
   for (i = 0, sz = sizeof(names); i < sz; i++) {
       if (pattern && sscanf(names[i], pattern) != match) {
	   names[i] = nil;
	   continue;
       }
       names[i] = capitalize(names[i]);
   }
   names -= ({ nil });
   if (sizeof(names)) {
       if (pattern) {
	   user->message("Matching bad names: ");
       } else {
	   user->message("Bad names: "); 
       }
       user->message(implode(names, ", ") + ".\n");
   } else {
     if (pattern) {
         user->message("No names match that pattern.\n");
     } else {
         user->message("No bad names stored.\n");
     }
   }
}

string add_data( string topic, SAM desc ) {
    string data_id;

    if ( datacnt[ topic ] )
       datacnt[ topic ]++;
    else
	   datacnt[ topic ] = 1;
   
    data_id = topic+datacnt[ topic ];  /* sword becomes swordX */
    data[ data_id ]=desc;
}

int query_data_cnt ( string topic ) {
   if ( datacnt[ topic ] )
      return datacnt[ topic ];
   else
      return 0;
}

string *query_topics ( void ) {
   return map_indices( datacnt );
}

string *query_desc ( string topic ) {
   int a;
   string *desc, topicname;
   desc = ({  });

   if (datacnt[topic]) {
      for ( a=1; a<=datacnt[topic]; a++ ) {
         topicname = topic+a;
         desc += ({ data[topicname] });
      }
   }
   return desc;
}

void clear_topic ( string topic ) {
   int a;

   if ( datacnt[topic] == nil )
      return;
   
   for( a=1; a<=datacnt[topic]; a++ ) {
      data[ topic+a ] = nil;
   }
   datacnt[topic] = nil;
}

void setup_skills ( object udat, string skillstr ) {
   string *list, *sar;
   int i;

   list = sar = ({ });
   
   if (skillstr == nil) {
       udat->set_property( "startstory:skilllist", nil );
       return;
   }		   
   sar = explode( skillstr, " " );
   
   for ( i=0; i<sizeof(sar); i++ ) {
       if ( sar[i] != "" ) {
	   list += ({ sar[i] });
       }
   }

   udat->set_property( "startstory:skilllist", list );
}

mixed query_page_data ( string str, string type ) {
   string s;
   int i;
  
   /* SysLog( "qpd "+str+" "+type ); */
   
   for( i=1; i<=datacnt[str]; i++ ) {
      /* SysLog( "dump_value="+dump_value(data[str+i]) ); */
      if ( sscanf( data[str+i], type+"=%*s" ) ) {
         sscanf( data[str+i], type+"=%s", s );
		 /* SysLog( "s: "+dump_value(s) ); */
		 if ( type=="pic" )
	        return s;
		 else
            return parse_xml(s);
      }
   }
   return "(page data undefined)";
}

string *query_body_data ( string type, string gender ) {
   string *sar, sdesc, sgen, stype, s;
   int i, j;
   sar = ({ });

   for ( i=1; data[type+i]; i++ ) {
      if ( sscanf( data[type+i], "%*s=%*s(%*s)" ) == 3 ) {
         sscanf( data[type+i], "%s=%s(%s)", sdesc, stype, sgen );
		 if ( gender=="he" ) {
            if ( sgen != "F" )
               sar += ({ sdesc });
		 } else {
            if ( sgen == "F" )
               sar += ({ sdesc });
		 }
	  } else {
         sscanf( data[type+i], "%s=%s", sdesc, stype );
		 sar += ({ sdesc });
	  }
   }
   return sar;
}

string *query_rumors ( void ) {
   string *sar;
   int i;

   sar = ({ });
   for ( i=1; data["Rumors"+i]; i++ )
      sar += ({ unsam( parse_xml(data["Rumors"+i]), ([ ]) ) });

   return sar;
}

string *query_first_memories ( void ) {
   string *sar, s1, s2;
   int i, j;

   sar = ({ });

   for( i=1; data["Memory"+i+"1"]; i++ ) {
       /* SysLog( dump_value( data["Memory"+i+"1"] ) ); */
       for( j=1; data[("Memory"+i)+j]; j++ ) {
	   sscanf( data[("Memory"+i)+j], "%s=%s", s1, s2 );
	   if ( s1=="first" ) {
	       sar += ({ unsam( parse_xml(s2), ([ ]) ) });
	       /* SysLog( "s2: "+s2 ); */
	       break;
	   }
       }
   }
   return sar;
}

/*
 * Split up as %s(%s) but make sure the 2nd %s only matches the _last_
 * pair of parenthesis.
 */
mixed *
split_memory(string line)
{
    int    d, len;
    string memory, start;

    line = strip(line);
    len = strlen(line);
    if (line[len - 1] == ')') {
	int i;

	for (i  = len - 2; i >= 0; i--) {
	    if (line[i] == '(') {
		start = line[i + 1..len - 2];
		line  = strip(line[..i - 1]);
		return ({ line, start });
	    }
	}
    }
    return nil;
}

string pick_second_memory ( object udat, string first ) {
   int i, j, found, freq, *tbl, final, val;
   string s1, s2, str, s, s3;
   string *sar;
   mixed *result;

   sar = ({ });
   found = 0; /* old habit */

   for ( i=1; datacnt["Memory"+i]; i++ ) {
      for ( j=1; j<=datacnt["Memory"+i]; j++ ) {
         sscanf( data[("Memory"+i)+j], "%s=%s", s1, s2 );
		 /* SysLog( "data: " + dump_value(data[("Memory"+i)+j]) + dump_value(s1) + dump_value(s2) ); */
         if ( s1=="first" && first==s2 ) {
            found = i;
		 }
		 if ( s1=="second" && found==i ) {
			sar += ({ s2 });
		 }
	  }
   }

   if ( !found ) {   /* Error!! Give first */
      SysLog( "No memory found for " + dump_value(first) );

      setup_skills( udat, "warriorclass" );
      return
	 "Your arms are lead, and your legs have rats gnawing on them " +
	 "from the inside. But the Master of Arms grasps your arm, and hauls " +
	 "you to your feet. 'Never give up.' Her voice is harsh and " +
	 "sharp. 'To surrender is to lose your life and your honor.' She " +
	 "thrusts the wooden sword back in your aching hands. 'Now stand " +
	 "ready!'";
   }

   tbl = ({ });
   for ( i=0; i<sizeof(sar); i++ ) {
      sscanf( sar[i], "(%d)%s", freq, s2 );
	  for( j=0; j<freq; j++ )
         tbl += ({ i });
   }
   
				   
   final = tbl[ random(sizeof(tbl)) ];
   sscanf( sar[final], "(%d)%s", freq, str );

   s3 = "";
   if ( sscanf( str, "List%d", val ) ) {
      tbl = ({ });
      for( i=1; data[("List"+val)+i]; i++ ) {
         if ( sscanf( data[("List"+val)+i], "first=%*s" ) == 1 ) {
            sscanf( data[("List"+val)+i], "first=%s", s3 );      
         } else {
            sscanf( data[("List"+val)+i], "(%d)%s", freq, s2 );
            for( j=0; j<freq; j++ )
               tbl += ({ i });
		 }
	  }
	  final = tbl[ random(sizeof(tbl)) ];
	  sscanf( data[("List"+val)+final], "(%d)%s", freq, str );
	  if ( strlen(s3) > 0 )
         str = s3 + " " + str;
   }

   if (result = split_memory(str)) {
       str = result[0];
       setup_skills(udat, result[1]);
   } else {
       setup_skills(udat, nil);
   }
   return unsam( parse_xml(str), ([ ]) );
}

string clear() {
   data = ([ ]);
}

SAM query_description() {
   return description;
}

void set_description ( SAM m ) {
   description = m;
}

void xfer(object udat, object body, string data) {
   string str;

   if (str = udat->query_property("startstory:" + data)) {
      body->set_property( "appearance:" + data, lower_case(str));
   }
}

string non_ordinary( string text, string type ) {
   int i;
   string s1, s2, s;
   
   for( i=1; data[type+i]; i++ ) {
      sscanf( data[type+i], "%s=%s", s1, s2 );
	  /* SysLog( "s2 dump value: "+dump_value(s2) ); */
	  if ( s2==nil )
	     s2 = "";
	  
      if ( s1==text ) {
		 if ( sscanf( s2, "%s(%s)", s1, s ) == 2 )
            return unsam( parse_xml(s1), ([ ]) );
		 else
            return unsam( parse_xml(s2), ([ ]) );
	  }
   }
   
   return "";
}

string construct_top ( object udat ) {
   string final, gen, s1, s2, s, haircover, *list;
   int i;
   mixed x;

   gen = udat->query_property("startstory:gender");
   
   list = ({ });
   s = final = "";
   
   s1 = non_ordinary( udat->query_property( "startstory:height" ), "Height" );
   s2 = udat->query_property( "startstory:build");
   if (s2) {
      s2 = lower_case(s2);
      if (s2 == "ordinary") {
	 s2 = "";
      }
   } else {
      s2 = "";
   }

/*   SysLog( udat->query_property("startstory:newname")+"s1 = "+s1+" s2 = " + s2 ); */
   
   if ( strlen(s1) > 0 )
      s += s1;

   if ( strlen(s2) > 0 )
      if ( strlen(s1) > 0 )
         s += ", " + s2;
      else
         s += s2;
   
   if ( strlen(s) > 0 ) {
      if ( s[0]=='a' || s[0]=='e' || s[0]=='i' || s[0]=='o' || s[0]=='u' )
         s = "an " + s;
	  else
         s = "a " + s;
      s2 = "";
   } else {
      s += "a ";
   }

   final += s;
   
   SysLog( "final = " + final );
   
   haircover = udat->query_property("startstory:haircoverage");
   
   if ( haircover=="Bald" )
      final += ", bald";
   
   if ( gen=="he" )
      final += " man with ";
   else
      final += " woman with ";
 
   if ( gen=="he" && udat->query_property("startstory:facialhair") != "None" ) {
      list += ({ non_ordinary( udat->query_property("startstory:faciallength"), "Faciallength" ) + " " +
         non_ordinary( udat->query_property("startstory:facialhair"), "Facialhair" ) });
   }
   
   if ( haircover!="Bald" ) {
      list += ({ non_ordinary( udat->query_property("startstory:haircoverage"), "Haircoverage" ) + " " +
	     non_ordinary( udat->query_property("startstory:hairtype"), "Hairtype" ) + " " +
	     non_ordinary( udat->query_property("startstory:haircolor"), "Haircolor" ) + " hair" });
		 
	  list += ({ non_ordinary( udat->query_property("startstory:hairlength"), "Hairlength" ) + " " + 
	     non_ordinary( udat->query_property("startstory:hairstyle"), "Hairstyle" ) });
   }

   list += ({ non_ordinary( udat->query_property("startstory:skin"), "Skin" )+" skin" });
  
   if ( sizeof(list)==1 )
      final += list[0] + ".";

   if ( sizeof(list)==2 )
      final += list[0] + " and " + list[1] + ".";

   if ( sizeof(list) > 2 ) {
      for( i=0; i<sizeof(list); i++ ) {
         if ( i==sizeof(list)-1 )
            final += " and " + list[i] + ".";
         else
            final += list[i] + ", ";
      }
   }
   
   return strip_spaces( final );
}


string construct_bot ( object udat ) {
   string *list, s, s1, s2, gen, final;
   int i;

   list = ({ });
   
   gen = udat->query_property("startstory:gender");

   final = capitalize(gen);
   final += " has ";

   s1 = non_ordinary( udat->query_property("startstory:browshape"), "Browshape" );
   s2 = non_ordinary( udat->query_property("startstory:eyebrowshape"), "Eyebrowshape" );

   if ( strlen(s1) > 0 && strlen(s2) > 0 ) {
      list += ({ s1 + " and " + s2 });
   } 
   if ( strlen(s1)==0 && strlen(s2) > 0 ) {
      list += ({ s2 });
   }
   if ( strlen(s1) > 0 && strlen(s2)==0 ) {
      list += ({ s1 });
   }

   s1 = non_ordinary( udat->query_property("startstory:eyeshape"), "Eyeshape" );
   s2 = non_ordinary( udat->query_property("startstory:eyecolor"), "Eyecolor" );
   
   if ( strlen(s1) > 0 && strlen(s2) > 0 ) {
      list += ({ (s1 + " " + s2 + " eyes") });
   }
   if ( strlen(s1)==0 && strlen(s2) > 0 ) {
      list += ({ (s2 + " eyes") });
   }
   if ( strlen(s1) > 0 && strlen(s2)==0 ) {
      list += ({ (s1 + " eyes") });
   }

   s1 = non_ordinary( udat->query_property("startstory:earshape"), "Earshape" );
   if ( strlen(s1) > 0 ) {
      list += ({ s1 });
   }
   
   s1 = non_ordinary( udat->query_property("startstory:noseshape"), "Noseshape" );
   if ( strlen(s1) > 0 ) {
      list += ({ s1 });
   }

   s1 = non_ordinary( udat->query_property("startstory:mouthshape"), "Mouthshape" );
   s2 = non_ordinary( udat->query_property("startstory:lipshape"), "Lipshape" );
  
   if ( strlen(s1) > 0 && strlen(s2) > 0 ) {
      list += ({ s1 + " with " + s2 });
   }
   if ( strlen(s1)==0 && strlen(s2) > 0 ) {
      list += ({ s2 });
   }
   if ( strlen(s1) > 0 && strlen(s2)==0 ) {
      list += ({ s1 });
   }

   s1 = non_ordinary( udat->query_property("startstory:cheekshape"), "Cheekshape" );
   s2 = non_ordinary( udat->query_property("startstory:jawshape"), "Jawshape" );

   if ( strlen(s1) > 0 ) {
      list += ({ s1 });
   }
   if ( strlen(s2) > 0 ) {
      list += ({ s2 });
   }

   if ( sizeof(list)==1 ) 
      final += list[0] + ".";

   if ( sizeof(list)==2 )
      final += list[0] + " and " + list[1] + ".";
   
   if ( sizeof(list) > 2 ) {
      for( i=0; i<sizeof(list); i++ ) {
         if ( i==sizeof(list)-1 )
            final += " and " + list[i] + ".";
         else
            final += list[i] + ", ";
	  }
   }
   
   return strip_spaces( final );
}

string construct_character ( object udat ) {
   string s;

   s = construct_top( udat );
   s += " ";
   s += construct_bot( udat );
   return s;
}

void set_body_skills ( object udat, object body ) {
   string *done, *left, *list;
   int    i, sz;

   list = udat->query_property( "startstory:skilllist" );
   if ( list==nil )
      return;

   done = ({ });
   left = ({ });
   for (i = 0, sz = sizeof(list); i < sz; i++) {
       list[i] = lower_case(list[i]);
       if (start_handler) {
	   if (start_handler->startstory_skills(body, list[i])) {
	       done += ({ list[i] });
	   } else {
	       left += ({ list[i] });
	   }
       } else {
	   /* Backward compatibility */
	   body->set_property("skill:" + list[i], TRUE);
	   left += ({ list[i] });
       }
   }
   body->set_property("startstory:skilllist:done", done);
   body->set_property("startstory:skilllist:left", left);
}

mixed name_already_used ( object udat, string name ) {
   mixed upgrade;
   upgrade = udat->query_property("upgrade-name");
   if ( upgrade )
      if ( lower_case(name) == lower_case(upgrade) )
         return FALSE;
   return UDATD->query_body( name );
}

atomic void make_character ( object udat ) {
   string gen, ur, name;
   object newchar, obj, clothing, *inv, home;
   int i, doing_upgrade;
   mixed upgrade;
		   
   gen = udat->query_property("startstory:gender");
   name = udat->query_property("startstory:newname");

   if ( gen=="he" )
      ur = data["Ur-Male1"];
   else
      ur = data["Ur-Female1"];

   /* --- Ur Male/Female --- */

   obj = find_object( ur );
   if ( !obj ) {
      SysLog( "Ur failed in make_character" );
      return;
   }

   upgrade = udat->query_property("upgrade-name");
   doing_upgrade = ( upgrade && (lower_case(upgrade)==lower_case(name)) ? TRUE : FALSE );

   if ( doing_upgrade ) {
      newchar = udat->query_property("upgrade-body");
   } else {
   
      newchar = spawn_thing(obj);

      newchar->set_object_name( "Marrach:players:" + 
				capitalize(name[0..0]) + ":" +
				lower_case(name) );
   }
   if ( gen=="he" )
      newchar->set_male();
   else
      newchar->set_female();

   newchar->set_id_prime( "default", TRUE );   
   newchar->add_alias( "default", name );
   newchar->add_alias( "default", lower_case(name) );
   newchar->add_alias( "default", capitalize(name) );

   xfer( udat, newchar, "build" );
   xfer( udat, newchar, "height" );
   xfer( udat, newchar, "skin" );
   xfer( udat, newchar, "hands" );
   xfer( udat, newchar, "eyeshape" );
   xfer( udat, newchar, "eyecolor" );
   xfer( udat, newchar, "earshape" );
   xfer( udat, newchar, "noseshape" );
   xfer( udat, newchar, "mouthshape" );
   xfer( udat, newchar, "lipshape" );
   xfer( udat, newchar, "jawshape" );
   xfer( udat, newchar, "cheekshape" );
   xfer( udat, newchar, "browshape" );
   xfer( udat, newchar, "eyebrowshape" );
   xfer( udat, newchar, "haircoverage" );
   xfer( udat, newchar, "hairlength" );
   xfer( udat, newchar, "haircolor" );
   xfer( udat, newchar, "hairtype" );
   xfer( udat, newchar, "hairstyle" );
   xfer( udat, newchar, "facialhair" );
   xfer( udat, newchar, "faciallength" );

   if (doing_upgrade) {
      return;
   }
   set_body_skills( udat, newchar );
   newchar->set_volition( TRUE );

   newchar->add_memory( "You vaguely remember a scene from the past..." + udat->query_property("startstory:secondmem") );

      /* --- Clothing --- */

   ur = data["Ur-Clothing1"];
   obj = find_object( ur );
   if ( obj ) {
      clothing = spawn_thing(obj);

      i = clothing->move( newchar );
      SysLog( "clothing: "+dump_value(clothing)+" i:"+i );
      SysLog( "query inv: "+ dump_value(newchar->query_inventory()) );
      newchar->action("wear", ([ "articles": ({ clothing }) ]));
   } else {
      SysLog("find object for clothing failed in makechar" );
   }

   /* --- Set up Virtual Room, etc --- */

   {
      object home, ur;

      ur = find_object("Marrach:rooms:outer:second:Guestroom" +
		       (1+random(5)));

      home = spawn_thing(ur);
      home->set_property("HomeOwner", newchar);
      newchar->set_property("VirtualHome:home", home);
      newchar->set_disconnect_room(home);
   }

   newchar->set_property("skotos:creation_time", time());

   /* this was previously in UserAPI, where it does NOT belong */
   newchar->set_property("SkotOS:CharName", name);
   newchar->set_property("SkotOS:Creator", name);

   UDATD->add_name_for_body(newchar, name, TRUE);
   UDATD->add_body_to_roster(newchar, udat->query_name());
}
