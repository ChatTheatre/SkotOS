# include <limits.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/url";
private inherit "/lib/mapargs";

inherit "/lib/random";

mapping syls;
mapping sylcnt;

mapping garble_tab;

mapping languages;

SAM description;

int query_syl_cnt ( string topic );
void hardcoded_syls ( void );

string query_state_root() { return "Lang:Lang"; }

string valid_language ( string lang );
void garb_create();

static 
void create() {
   set_object_name("SkotOS:Lang");

   syls = ([ ]);
   sylcnt = ([ ]);
   languages = ([ ]);
   
   hardcoded_syls();
   garb_create();
		
   HTTPD->register_root("Lang");
   SAMD->register_root("Lang");
}


void lang_reset ( void ) {
   syls = ([ ]);
   sylcnt = ([ ]);
   languages = ([ ]);
   garble_tab = ([ ]);
}

void lang_hardcoded ( void ) {
   hardcoded_syls();
   garb_create();
}

string current_lang ( object body ) {
   mixed id;
   string lang;
   
   id = body->query_property("SkotOS:CurrentLang");
   if ( id ) {
      
   } else {
      body->set_property("SkotOS:CurrentLang", "0");
      return "common";
   }
}

void lang_status( object user, object body ) {
   int i;
   mixed prof;
   string id, langname;
   
   string *str;
   str = map_indices( sylcnt );
   user->message("Languages:\n");
   for ( i=0; i<sizeof(str); i++ ) {
       user->message("  "+str[i]+" " );
       sscanf( str[i], "%s=%s", id, langname );
       prof = body->query_property( "skill:lang"+id );
       if ( !prof ) {
	   prof = 0;
       }
       user->message( "prof: "+prof+"\n" );
   }
}

void set_lang_prof( object user, object body, string lang, string sprof ) {
   string id;
   int prof;

   sscanf( sprof, "%d", prof );
   if ( id = LANG->valid_language( lang ) ) {
       body->set_property( "skill:lang"+id, prof );
       user->message("Language proficiency set.\n");
   } else {
       user->message("invalid language: "+lang+"\n" );
   }
}

string lookup_id ( mixed id ) {
   int i;
   string *langs, langname, sid;

   id = (string)id;
   if ( id=="0" )
      return "common";
   
   langs = map_indices( sylcnt );
   for ( i=0; i<sizeof(langs); i++) {
      sscanf( langs[i], "%s=%s", sid, langname );
	  if ( sid==id )
         return langname;
   }
   return nil;
}

string lookup_lang ( string lang ) {
   int i;
   string *langs, langname, sid;

   if ( lower_case(lang)=="common" )
      return "0";
   
   langs = map_indices( sylcnt );
   for ( i=0; i<sizeof(langs); i++) {
      sscanf( langs[i], "%s=%s", sid, langname );
      if ( lang==langname )
         return sid;
   }
   return nil;
}

void lang_summarize ( object user, object body ) {
   string *langs, lid, lname;
   int i;
   mixed prof, cur;

   cur = body->query_property( "SkotOS:CurrentLang" );
   if ( cur ) {
       cur = lookup_id( cur );
       user->message("You are currently speaking in "+capitalize(cur)+".\n" );
   } else {
       user->message("You are currently speaking in Common.\n");
   }
   
   langs = map_indices( sylcnt );
   user->message("You are fluent in Common.\n");
   for ( i=0; i<sizeof(langs); i++ ) {
       sscanf( langs[i], "%s=%s", lid, lname );
       prof = body->query_property("skill:lang"+lid);
       if ( prof && prof > 0 ) {
	   if ( prof==10 )
	       user->message("You speak "+capitalize(lname)+" fluently.\n" );
	   else
	       user->message("You speak "+capitalize(lname)+" with "+prof+"0% proficiency.\n");
       } else {
	   /* nahh - say nothing
	    * user->message("You don't speak "+capitalize(lname)+" at all.\n");
	    * */
       }
   }
}

void set_current ( object user, object body, string lang ) {
   string id;
   mixed prof;
   
   if ( id = valid_language( lang ) ) {
       prof = body->query_property( "skill:lang"+id );
       if ( ( prof && prof > 0 ) || lang=="common" ) {
	   user->message( "You now speak in "+capitalize(lang)+".\n" );
	   body->set_property( "SkotOS:CurrentLang", id );
       } else {
	   user->message( "You don't speak "+capitalize(lang)+".\n" );
       }
   } else {
       lang_summarize( user, body );
   }
}

string valid_language ( string lang ) {
   string *str, id, langname;
   int i;

   if ( lang=="common" )
      return "0";
   
   str = map_indices( sylcnt );
   for ( i=0; i<sizeof(str); i++ ) {
      sscanf( str[i], "%s=%s", id, langname );
      if ( langname == lang )
         return id;
   }
   return nil;
}

string add_syl( string topic, SAM desc) {
   string syl_id;
   
   if ( sylcnt[ topic ] )
      sylcnt[ topic ]++;
   else
      sylcnt[ topic ] = 1;
   
   syl_id = topic+sylcnt[ topic ];  /* sword becomes swordX */
   syls[ syl_id ]=desc;
}

string show_syl( string topic, object user ) {
   string syl_id;
   int syl_num;
   object udat;
   
   udat = user->query_udat();
   
   syl_num = udat->next_syl_id( topic );
   if ( syl_num > query_syl_cnt( topic ) ) 
      syl_num = 1;
   if ( syl_num == query_syl_cnt( topic ) )
      udat->set_seen_all( topic, query_syl_cnt( topic ) );
   
   syl_id = topic + syl_num;
   
   if ( syls[ syl_id ] ) {
      udat->set_last_syl_seen( topic, syl_num );
      
      return syls[ syl_id ];  
   } else {
      return "No syl available";
   }
}

int query_syl_cnt ( string topic ) {
   if ( sylcnt[ topic ] )
      return sylcnt[ topic ];
   else
      return 0;
}

string *query_langs ( void ) {
   return map_indices( sylcnt );
}

string *query_syl ( string lang ) {
   int a;
   string *sy, langname;
   sy = ({  });
   
   for ( a=1; a<=sylcnt[lang]; a++ ) {
      langname = lang+a;
      sy += ({ syls[langname] });
   }
   return sy;
}

void clear_all_languages() {
  sylcnt = ([ ]);
  languages = ([ ]);
  syls = ([ ]);
}

void clear_langs ( string lang ) {
   int a;

   languages[lang] = nil;
   
   if ( sylcnt[lang] )
      for( a=1; a<=sylcnt[lang]; a++ ) 
         syls[ lang+a ] = nil;
   
   sylcnt[lang] = nil;
}

string test() {
   return "Here's a syl for you";
}

void hardcoded_syls( void ) {
   add_syl( "1=eastern", "gil" );
   add_syl( "1=eastern", "gal" );
   add_syl( "1=eastern", "ad" );
   add_syl( "1=eastern", "nun" );
   add_syl( "1=eastern", "erid" );
   add_syl( "1=eastern", "waith" );
   add_syl( "1=eastern", "ein" );
   add_syl( "1=eastern", "or" );
   add_syl( "1=eastern", "ian" );
   add_syl( "1=eastern", "gol" );
   add_syl( "1=eastern", "fin" );
   add_syl( "1=eastern", "win" );
   add_syl( "1=eastern", "er" );
   add_syl( "1=eastern", "wain" );
   add_syl( "1=eastern", "nul" );
   add_syl( "1=eastern", "'un" );
   add_syl( "1=eastern", "el" );
   add_syl( "1=eastern", "dor" );
   add_syl( "1=eastern", "onin" );
   add_syl( "1=eastern", "rian" );
   add_syl( "1=eastern", "is" );
   add_syl( "1=eastern", "thain" );
		
   add_syl( "2=western", "dim" );
   add_syl( "2=western", "flim" );
   add_syl( "2=western", "lim" );
   add_syl( "2=western", "mim" );
   add_syl( "2=western", "lum" );
   add_syl( "2=western", "um" );
   add_syl( "2=western", "im" );
   add_syl( "2=western", "um um" );
   add_syl( "2=western", "gum" );
   add_syl( "2=western", "rum" );

   add_syl( "3=northern", "pub" );
   add_syl( "3=northern", "gluck" );
   add_syl( "3=northern", "ti" );
   add_syl( "3=northern", "uk" );
   add_syl( "3=northern", "wibi" );
   add_syl( "3=northern", "clu" );
   add_syl( "3=northern", "uck" );
   add_syl( "3=northern", "glob" );
   add_syl( "3=northern", "ben" );
   add_syl( "3=northern", "gilck" );
   add_syl( "3=northern", "en" );
   add_syl( "3=northern", "ish" );
   add_syl( "3=northern", "ka" );
}

string clear() {
   syls = ([ ]);
}

SAM query_description() {
   return description;
}

void set_description ( SAM m ) {
   description = m;
}


void garb_create() {
   garble_tab = ([
      "E": 1, "e": 1,
      "T": 2, "t": 2,
      "A": 3, "a": 3,
      "I": 4, "i": 4,
      "N": 5, "n": 5,
	  "O": 6, "o": 6,
	  "S": 7, "s": 7,
	  "H": 8, "h": 8,
	  "R": 9, "r": 9,
	  "D": 10, "d": 10,
	  "L": 11, "l": 11,
	  "U": 12, "u": 12,
	  "C": 13, "c": 13,
	  "M": 14, "m": 14,
	  "F": 15, "f": 15,
	  "W": 16, "w": 16,
	  "Y": 17, "y": 17,
	  "G": 18, "g": 18,
	  "P": 19, "p": 19,
	  "B": 20, "b": 20,
	  "V": 21, "v": 21,
	  "K": 22, "k": 22,
	  "Q": 23, "q": 23,
	  "J": 24, "j": 24,
	  "X": 25, "x": 25,
	  "Z": 26, "z": 26,
      "1":5, "2":5, "3":5, "4":5, "5":5, "6":5, "7":5, "8":5, "9":5, "0":5
	  ]);
}

	
int garble_val ( string word )
{
    int i, len, sum;

    sum = 0;
    for (i = 0, len = strlen(word); i < len; i++) {
	sum += garble_tab[word[i..i]];
    }
    return sum;
}

string garble_word ( int seed, string language )
{
   int length, a, x;
   string output, sy;
		   
   output = "";
   eric_randomize( seed );
   
   length = eric_random(4)+1; 
   /* output = "["+length+"/"+rnd_seed+"]"; */
		  
   for( a=0; a<length; a++ ) {
      x = eric_random( sylcnt[language] );
      sy = syls[ language+(x+1) ];
	  output += sy;
   }

   return output; 
}

string garble( string input, string language, int prof )
{
   int    i, len, seedval, wordpos;
   string output, word;

   /* sscanf( level, "%d", prof ); */
   prof *= 7;

   input += " END";

   seedval = wordpos = 0;
   word = output = "";
   len = strlen(input);
   for (i = 0; i < len; i++) {
       switch (input[i]) {
       case 'A'..'Z':
       case 'a'..'z':
       case '0'..'9':
	   word += input[i..i];
	   seedval += garble_val(word) * (++wordpos*10);
	   SysLog("seed at i=" + i + ": " + seedval);
	   break;
       default:
	   if ( strlen(word) > 0 ) {
	       if ( garble_val(word) <= prof ) 
		   output += word;
	       else
		   /* output += "babble("+garble_val(word)+"/"+seedval+")";  */
		   output += garble_word( seedval,language ); 
	       output += input[i..i];
	       word = "";
	       wordpos = seedval = 0;
	   } else {
	       output += input[i..i];
	   }
	   break;
       }   
   }
   return output;
}

string get_cur_id ( object from ) {
   mixed id;

   if ( from->query_brain("name") ) {  /* npc */
      id = from->query_brain("lang");
      if (!id) {
	  id = "0";
      } else {
	  /*
	   * Well, you better make sure the rest of the world can see that you
	   * speak this fluently, then.
	   */
	  from->set_property("skill:lang" + id, 10);
      }
   } else {
      id = from->query_property( "SkotOS:CurrentLang" );
      if ( !id ) {
          from->set_property( "SkotOS:CurrentLang", "0" );
          id = "0";
      }
   }
   return (string)id;
}
   
string force_garble ( object who, string text ) {
   string lang;
   int degree;
   mixed id;

   if ( sscanf( who->query_property("force-garble"), "%s-%d", lang, degree ) < 2 ) {
      SysLog("Error in force-garble, bad format: "+who->query_property("force-garble") );
	  return text;
   }
		
   id = lookup_lang( lang );
   if ( id==nil ) {
      SysLog("Bad lang name: "+lang);
	  return text;
   } 

   if ( degree < 1 || degree > 10 ) {
      SysLog("Bad force-garble degree: "+degree );
	  return text;
   }

   if ( degree == 10 ) {
      SysLog("Turning off garble, degree 10");
	  who->set_property( "force-garble", nil );
	  return text;
   }
   
   text = garble( text, id+"="+lang, degree );
   SysLog( "force-garbled text: "+text );
   return text;
}

string zell_garble(string speech, string id, int prof_from, object to) {
   string lang;
   mixed prof_to;
   int prof;

   if ( id=="0" ) {
       /* Speaking Common */
       return speech;
   }
   prof_to = to->query_property("skill:lang" + id);
   if (!prof_to) {
       to->set_property("skill:lang"+id, 0);
       prof_to = 0;
   }
   prof = (prof_from * prof_to) / 10;
   lang = lookup_id( id );

   if (prof == 10) {
      return speech;
   }
   speech = capitalize( garble( speech, id+"="+lang, prof));
   return speech[.. strlen(speech)-2];
}

string do_garble(string speech, object from, object to) { 
  string xlated, lang;
   mixed id, prof_from, prof_to, prof;

   if (from->query_property("mute"))
       return "...";
   
   if ( from->query_property("force-garble") )
       speech = force_garble( from, speech );
   
   id = get_cur_id( from );
   if ( id=="0" ) {
       /* Speaking Common */
       return speech;
   }

   prof_from = from->query_property("skill:lang" + id);
   if (!prof_from) {
       from->set_property("skill:lang"+id, 0);
       prof_from = 0;
   }

   prof_to = to->query_property("skill:lang" + id);
   if (!prof_to) {
       to->set_property("skill:lang"+id, 0);
       prof_to = 0;
   }
   prof = (prof_from * prof_to) / 10;

   Debug("prof_to = " + prof_to + " / prof_from = " + prof_from +
	 " / prof = " + prof);

   lang = lookup_id( id );

   if (from == to || prof == 10) {
      return speech;
   }
    xlated = capitalize( garble( speech, id+"="+lang, prof));
   return xlated[0..strlen(xlated)-2];
}

string do_lang_name ( string text, object from, object to ) {
   string s1, s2;
   mixed prof, id;
  
   id = get_cur_id( from ); 
   if ( id=="0" )
      return text;
   
   prof = to->query_property("skill:lang"+id);
   if ( !prof ) {
       to->set_property("skill:lang"+id, 0);
       prof = 0;
   }
   if ( prof > 0 ) {
      if (text[strlen(text)-2 .. strlen(text)-1] == ", ") {
	 return text[.. strlen(text)-3] + " in " +
	    capitalize(lookup_id(id)) + ", ";
      }
   }
   return text;
}

void learn_lang ( object user, object body, object npc, string lang ) {
    mixed id, bname, prof, last_time;
    
    bname = npc->query_brain("name");
    if ( bname==nil ) {
	user->message("You may learn languages from castle servants, such as couriers and guards.\n");
	return;
    }
    id = valid_language( lang );
    if ( id==nil ) {
	user->message("That is not the name of a language.\n");
	return;
    }
    
    if ( npc->query_brain("lang") != id ) {
	user->message("That "+bname+" does not know how to speak "+capitalize(lang)+".\n" );
	return;
    }
    
    last_time = npc->query_brain( "lang_taught" );
    if ( last_time ) { 
	if ( last_time >= time()-60 ) {
	    user->message("The "+bname+" is too busy to help you, try again later.\n");
	    return;
	}
    }
    
    prof = body->query_property( "SkotOS:Lang"+id );
    if ( !prof ) {
	body->set_property( "SkotOS:Lang"+id, 0 );
	prof = 0;
    }
    if ( prof == 10 ) {
	user->message("You already speak "+capitalize(lang)+" fluently.\n" );
	return;
    }

    NPCBRAINS->temp_social( "A "+bname+" teaches $(PC) how to speak "+capitalize(lang)+".\n", npc, body, bname );

    body->set_property( "SkotOS:Lang"+id, prof + 1 );
    if ( prof==9 )
	user->message("You now speak "+capitalize(lang)+" fluently.\n");
    else
	user->message("You now speak "+capitalize(lang)+" with "+(prof+1)+"0% proficiency.\n" );
    npc->set_brain( "lang_taught", time() );
}
