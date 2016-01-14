/*
**	/usr/TextIF/cmds/misc.c
**
**********************************************************************
**	991101 Zell	Complete rewrite; early version of new help system
**	00070x Wes	Added who and assist commands
**
**	Copyright Skotos Tech Inc 1999
*/

# include <XML.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <base.h>
# include <System.h>
# include <TextIF.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/date";

private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";

inherit xml "/usr/XML/lib/xmlparse";
inherit "/usr/SkotOS/lib/bilbo";

inherit authapi "/usr/UserAPI/lib/authapi";
inherit ctlapi "/usr/UserAPI/lib/ctlapi";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/auth";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";
inherit "/usr/SkotOS/lib/merryapi";

# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"
# include "/usr/TextIF/include/misc.h"

static
void create() {
   xml::create();
   authapi::create();
   ctlapi::create();
}

void patch() {
   ctlapi::create();
}

void cmd_follow( object user, object body, varargs mixed s);
void cmd_restrain ( object user, object body, varargs mixed s);


void cmd_exits(object user, object body, varargs string foo) {
  string *details;
  object env, *inv;
  NRef *exits;
  int i, j;

  exits = ({ });
  env = body->query_environment();

  if (!env)
    return;

  details = ur_match_exit_details(env, nil, TRUE);

  if (details) {
    for(i = 0; i < sizeof(details); i++) {
      exits += ({ NewNRef(env, details[i]) });
    }
  }

  inv = env->query_inventory() - ({ body });

  for(i = 0; i < sizeof(inv); i++) {
    details = ur_match_exit_details(inv[i], "outside", TRUE);
      
    if (!details)
      continue;

    for(j = 0; j < sizeof(details); j++) {
      exits += ({ NewNRef(inv[i], details[j]) });
    }
  }

  if (!sizeof(exits)) {
    user->paragraph("There are no obvious exits.");
    return;
  }

  if (body->query_property("client:theme")) {
     foo = "foo";
  }

  DEBUG("exits = " + dump_value(exits));
  user->html_message("You may leave through " + 
		     describe_exits(exits) + ".\n");
}

 
void cmd_tip( object user, object body, varargs string *topic ) {
  if (topic) {
    if ( topic[0]=="reset" ) {
      TIPD->reset_tips(user);
      user->paragraph("Tips reset.");
    }
    else
      body->action("tip", 
		   ([ "topic" : topic[0] ]) );
  } else {
    body->action("tip", 
		 ([ "topic" : "intro" ]) );
  }
}

void desc_tip( object *users, object body, string topic ) {
  object *souls;
  mixed tip;
  int i;

  souls = body->query_souls();
  tip = TIPD->show_tip( topic, souls[i] );

  for (i = 0; i < sizeof(souls); i ++) {
    souls[i]->html_message(unsam(tip, ([ ]), souls[i]));
  }
}

void desc_recall(object *users, object body, string *memories) {
    int i, sz, nr_memories;

    /* Only send to user-objects of relevant body */
    users &= body->query_souls();

    nr_memories = sizeof(memories);
    sz = sizeof(users);
    for (i = 0; i < sz; i++) {
	int j;

	users[i]->clear_more();
	for (j = 0; j < nr_memories; j++) {
	    users[i]->add_more(" " + memories[j]);
	}
	users[i]->show_more();
    }
}

void desc_remember( object *users, object body, string text ) {
  tell_to( users, "Memory added, use @recall to remember it.\n", body);
}

void desc_forget( object *users, object body, string result ) {
  tell_to( users, result, body );
}


/* TODO: this cmd should be intercepted in the user object probably */
void cmd_more ( object user, object body, varargs string *words ) {
  int chunk;
  object udat;

  udat = user->query_udat();
   
  if ( words && sizeof(words) ) {
    sscanf( words[0], "%d", chunk );
    if ( chunk > 0 ) {
      udat->set_property( "more", chunk );
      user->message( "Screen size set.");
    } else {
      user->message( "Usage: more 18    Displays 18 lines at a time.\n");
      return;
    }
  }
		
  user->message("\n");
  user->show_more();
}



void cmd_help(object user, object body, varargs mixed key) {
    object ob;
    SAM desc;

    if (key && typeof(key) == T_ARRAY) {
        key = implode(key, " ");
    }
    if (key && strlen(key)) {
	int    i;
        object *obarr;
	string *titles, id;

	key = lower_case(key);

	if (strlen(key) && key[0] == '+') {
	    if (!DEV_USERD->query_wiztool(user->query_name())) {
		user->paragraph("There's no help on '" + key + "'.");
		return;
	    }
	}
	if (strlen(key) && key[0] == '!') {
	    if (!sizeof(rfilter(user->query_udat()->query_bodies(), "query_property", "GUIDE")) &&
		!is_builder(user->query_name()) &&
		!is_root(user->query_name())) {
	        user->paragraph("There's no help on '" + key + "'.");
		return;
	    }
	}

	obarr = HELPD->match_keyword(key);
	if (!obarr) {
	    user->paragraph("There's no help on '" + key + "'.");
	    return;
	}
	if (id = body->query_property("theatre:id")) {
	   int i, sz;
	   object *tarr, *garr;

	   sz = sizeof(obarr);
	   tarr = allocate(sz);
	   garr = allocate(sz);
	   for (i = 0; i < sz; i++) {
	      if (obarr[i]->query_property("theatre:id") == id) {
		 tarr[i] = obarr[i];
	      } else if (!obarr[i]->query_property("theatre:id")) {
		 garr[i] = obarr[i];
	      }
	   }
	   tarr -= ({ nil });
	   garr -= ({ nil });
	   if (sizeof(tarr) > 0) {
	      obarr = tarr;
	   } else {
	      obarr = garr;
	   }
	}
	if (!sizeof(obarr)) {
	   user->paragraph("There's no help on '" + key + "'.");
	   return;
	}
	if (sizeof(obarr) > 1) {
	    int i;

	    titles = allocate(sizeof(obarr));
	    for (i = 0; i < sizeof(titles); i ++) {
	        titles[i] = obarr[i]->query_title();
	    }
	    user->message("There is help on '" + key + "' in sections:\n" +
			  tabulate(titles));
	    return;
	}
	ob = obarr[0];
    } else {
        /* TODO: another reminder that we need to dest-proof these */
        ob = find_object(HELPD);
    }
    /* TODO: how to handle the default trailing newline for sam()? */
    desc = ob->query_description();
    if (!desc) {
        desc = XML->parse("There is no help as yet.");
    }
    user->html_message(unsam(desc, ([ "keyword": key ]), user));
}

void cmd_DEV_npc( object user, object body, varargs string *words ) {
  if (!words || !sizeof(words)) {
    user->html_message(
       "<PRE>" + replace_html(
       "Usage:\n" +
       "  +npc list [<npc>]      ... List all currently or specific running npcs.\n" +
# if 0
       "  +npc killall           ... Kill all currently running npcs (careful!).\n" +
# endif
       "  +npc <npc> goto        ... Teleport to the location of <npc>.\n" +
       "  +npc spawn pipes cat   ... Create a rogue NPC cat from pipes parent.\n"+
       "  +npc slaynils          ... Slay any npcs without environment.\n" +
       "  +npc reboot            ... ?\n" +
       "  +npc positions [woe]   ... Show where the npcs are.\n" +
       "  +npc bodytraits <npc>  ... ?\n" +
       "  +npc braintraits       ... ?\n" +
       "  +npc rogues            ... Show where 'rogue' npcs are.\n" +
       "  +npc <npc> start       ... Creates an <npc> parent npc in this location.\n" +
       "  +npc <npc> stop        ... Stops an <npc> and all its npc children.\n" +
       "  +npc <npc> pop <...>   ... ?\n" +
       "  +npc <npc> delay <...> ... ?\n" +
       "  +npc <npc> dump        ... ?\n" +
       "  +npc <npc> liston      ... ?\n" +
       "  +npc <npc> kill        ... ?\n" +
# if 0
       "  +npc <npc> dragmale    ... ?\n" +
       "  +npc <npc> dragfemale  ... ?\n" +
       "  +npc <npc> block       ... ?\n" +
       "  +npc <npc> unblock     ... ?\n" +
# endif
       "  +npc scrolls           ... Show all scrolls, with sender and recipient.\n" +
       "  +npc scrolls-from      ... Same, sorted by sender.\n" +
       "  +npc scrolls-to        ... Same, sorted by recipient.\n" +
       ""
       ) + "</PRE>\n");
    return;
  }
  switch (words[0]) {
  case "reset":
     /* NPCBRAINS->reset(); */
     /* NPCBODIES->reset(); */
     user->paragraph("You don't want to do that.");
     return;
  case "killall":
     /* NPCBODIES->kill_all_bodies(); */
     user->paragraph("You don't want to do that.");
     return;
  case "hardcoded":
     NPCBRAINS->reset();
     /* NPCBRAINS->hardcoded_traits(); */
     NPCBODIES->reset();
     /* NPCBODIES->hardcoded_traits(); */
     user->paragraph("Reset funs called, but skipping hardcoded funs.");
     return;
  case "scrolls":
     NPCBRAINS->scrolls_info();
     return;
  case "scrolls-from": {
     int     i, sz, maxlen;
     mixed   *data;
     string  *names, result;
     mapping by_from, *to_data;

     data    = NPCBRAINS->scrolls_data();
     sz      = sizeof(data);
     by_from = ([ ]);

     for (i = 0; i < sz; i++) {
	string name_from, name_to;
	mapping map;

	name_from = data[i][1];
	name_to   = data[i][2];
	name_from = name_from ? capitalize(name_from) : "<Unsigned>";
	name_to   = name_to   ? capitalize(name_to) : "<Unknown>";
	if (strlen(name_from) > maxlen) {
	   maxlen = strlen(name_from);
	}
	map = by_from[name_from];
	if (!map) {
	   by_from[name_from] = map = ([ ]);
	}
	if (!map[name_to]) {
	   map[name_to] = 1;
	} else {
	   map[name_to]++;
	}
     }
     sz      = map_sizeof(by_from);
     names   = map_indices(by_from);
     to_data = map_values(by_from);
     result  = "Scrolls sent, grouped by sender:\n";

     for (i = 0; i < sz; i++) {
	int    j, sz_j, *totals;
	string *to_names;

	sz_j     = map_sizeof(to_data[i]);
	to_names = map_indices(to_data[i]);
	totals   = map_values(to_data[i]);
	for (j = 0; j < sz_j; j++) {
	   if (totals[j] > 1) {
	      to_names[j] += "[" + totals[j] + "]";
	   }
	}
	result  += pad_right(names[i] + ":", maxlen + 2) +
	           implode(to_names, ", ") + ".\n";
     }
     user->message(result);
     return;
  }
  case "scrolls-to": {
     int     i, sz, maxlen;
     mixed   *data;
     string  *names, result;
     mapping by_to, *from_data;

     data  = NPCBRAINS->scrolls_data();
     sz    = sizeof(data);
     by_to = ([ ]);

     for (i = 0; i < sz; i++) {
	string name_from, name_to;
	mapping map;

	name_from = data[i][1];
	name_to   = data[i][2];
	name_from = name_from ? capitalize(name_from) : "<Unsigned>";
	name_to   = name_to   ? capitalize(name_to) : "<Unknown>";
	if (strlen(name_to) > maxlen) {
	   maxlen = strlen(name_to);
	}
	map = by_to[name_to];
	if (!map) {
	   by_to[name_to] = map = ([ ]);
	}
	if (!map[name_from]) {
	   map[name_from] = 1;
	} else {
	   map[name_from]++;
	}
     }
     sz        = map_sizeof(by_to);
     names     = map_indices(by_to);
     from_data = map_values(by_to);
     result    = "Scrolls sent, grouped by recipient:\n";

     for (i = 0; i < sz; i++) {
	int    j, sz_j, *totals;
	string *from_names;

	sz_j       = map_sizeof(from_data[i]);
	from_names = map_indices(from_data[i]);
	totals     = map_values(from_data[i]);
	for (j = 0; j < sz_j; j++) {
	   if (totals[j] > 1) {
	      from_names[j] += "[" + totals[j] + "]";
	   }
	}
	result  += pad_right(names[i] + ":", maxlen + 2) +
	           implode(from_names, ", ") + ".\n";
     }
     user->message(result);
     return;
  }
  case "reboot":
     NPCBODIES->npc_reboot( user );
     return;
  case "slaynils":
     NPCBODIES->slaynils( user );
     return;
  case "positions":
     if (sizeof(words) > 1 && words[1] == "woe") {
	NPCBODIES->show_positions( user, TRUE );
     } else {
	NPCBODIES->show_positions( user );
     }
     return;
  case "bodytraits":
     NPCBODIES->dump_traits( words[1] );
     return;
  case "braintraits":
     NPCBRAINS->dump_traits();
     return;
  case "spawn":
     NPCBODIES->spawn_child( user, body, words[1], words[2] );
     return;
  case "rogues":
     NPCBODIES->list_rogues( user );
     return;
  case "list":
     if ( sizeof(words) > 1 )
	NPCBODIES->list_bodies( user, words[1] );
     else
	NPCBODIES->list_bodies( user, nil );
     return;
  default:
     break;
  }

  if ( sizeof(words) < 2 ) {
    user->message("Insufficient parameters.  You probably want one of the following:\n");
    user->message("  start stop dump goto kill teleport pop delay reboot positions slaynils\n" );
    return;
  }
   
  switch( words[1] ) {
  case "pop":
    NPCBODIES->set_pop( user, words[0], words[2] );
    break;
  case "delay":
    NPCBODIES->set_delay( user, words[0], words[2] );
    break;
  case "start":
    NPCBODIES->start_parent( user, words[0], words[0], body->query_environment() );
    break;
  case "stop":
    NPCBODIES->stop_parent( user, words[0] );
    break; 
  case "dump":
    NPCBODIES->dump_parent( user, words[0] );
    break;
  case "goto":
    NPCBODIES->goto_parent( user, body, words[0] );
    break;
  case "liston":
    NPCBODIES->list_on( user, words[0] );
    break;
  case "kill":
    NPCBODIES->kill_parent( user, words[0] );
    break;
# if 0
  case "dragmale":
    NPCBODIES->set_teleport( user, words[0], body->query_environment(), "male" );
    break;
  case "dragfemale":
    NPCBODIES->set_teleport( user, words[0], body->query_environment(), "female" );
    break;
  case "block":
    NPCBODIES->set_block( user, words, body->query_environment(), "set" );
    break;
  case "unblock":
    NPCBODIES->set_block( user, words, body->query_environment(), "unset" );
    break;
# endif
  default:
    user->message("What?\n");
    break;
  }
}

string fix_quotes ( string msg ) {
  int i;
  for ( i=0; i<strlen(msg); i++ )
    if ( msg[i]=='`' )
      msg[i]='\"';
  return msg;
}

string do_sam ( string msg ) {
  mixed xmd;
  xmd = parse_xml( msg );
  return unsam( xmd );
}

void desc_brain ( object *users, object who, object npc, string msg ) {
  msg = fix_quotes( msg );
  catch {
     msg = do_sam ( msg );
  } : {
     msg = "BAD SAM IN BRAIN: " + dump_value(msg);
     users = rfilter(users, "query_host");
     return;
  }
  tell_each( users, npc, msg, who );
}

void update_told ( object knower, object asker, string subject ) {
  object *objarr;
   
  if ( knower->query_brain("name") ) {  /* this is a NPC */
    if ( knower->query_brain("told_"+subject)==nil )
      knower->set_brain( "told_"+subject, ({ }) );

    objarr = knower->query_brain("told_"+subject);
    objarr += ({ asker });
    knower->set_brain( "told_"+subject, objarr );
  }
}

int already_told ( object knower, object asker, string subject ) {
  object *arr;
  int i;
  arr = knower->query_brain("told_"+subject);
  if (arr) {
    for( i=0; i<sizeof(arr); i++ )
      if ( arr[i]==asker )
	return TRUE;
  }
  return FALSE;
}

int set_writing_dtl ( object user, object body, object what, string dtlname, string text ) {
  string dtl, old, new_text;

  dtl = ur_prime_id( what, "outside" );
   
  if ( what->query_property( "writable" )==nil ) {
    user->paragraph( "You can't write on that." );
    return FALSE;
  }

  if ( dtlname=="writing" && what->query_property("lastwriter") == describe_one(body) ) {
    old = ur_description( what, dtl, dtlname );
    if ( old ) {
      sscanf( old, "\"%s\"", old );
      sscanf( text, "\"%s\"", new_text );
      what->set_description( dtl, dtlname, "\"" + old + "\n" + new_text + "\"" );
    } else {
      what->set_description( dtl, dtlname, text );
    }
  } else {
    what->set_description( dtl, dtlname, text );
  }
   
  if ( dtlname != "delivery-from" ) {
    if ( ur_description( what, dtl, "delivery-from") != proper(describe_one(body)) ) {
      what->set_description( dtl, "delivery-from", nil );
      what->set_description( dtl, "delivery-end", nil );
    }
  }

  return TRUE;
}

void cmd_address ( object user, object body, Ob *thing,
		   string prep, mixed words ) {
   NRef *what_refs, who_ref;
   object *obarr, *inv;
   object env;
   int i, inv_size, offer_found, fnd;
   NRef what;
   string *ppl, who;
   mixed m, m1;

   if (!words || !words["evoke"]) {
       user->message("You forgot to addressee!\n");
       return;
   }

   who = words["evoke"];
   who = who[1..strlen(who) - 2];
   who = lower_case(proper(who));

   if ( sizeof(thing) ) {
      what = locate_first(user, FALSE, thing,
			  new_object("~TextIF/data/filterfarm",
				     ({ "<carried" })),
			  body,
			  body->query_environment(), body);
      if (!what) {
	 return;
      }

      if (!NRefOb(what)->contained_by(body)) {
	 user->message("You must hold the scroll to address it.\n");
	 return;
      }
      m = UDATD->query_body(who);
      if (!m || m->query_property("skotos:unregistered")) {
	 m = find_object("Marrach:players:" + capitalize(who[0..0]) + 
			 ":" + lower_case(who));
	 /* user->message( "first m: " + dump_value(m)+"\n" ); */
      }

      if (!m || m->query_property("skotos:unregistered")) {
	 m = find_object("Marrach:NPCs:"+proper(who) );
	 /* user->message( "second m: " + dump_value(m) + "\n" ); */
      }

      if (!m || m->query_property("skotos:unregistered")) {
	 user->message("You must address it to a valid person.\n");
	 return;
      }
      if (set_writing_dtl( user, body, NRefOb(what), "delivery-to", who ) ) {
	 NRefOb(what)->set_brain( "deliver-to", m);
	 m->set_property("undisguised", TRUE);
	 user->paragraph( "You address " + describe_one(what) + " to " + describe_one(m) + ".");
	 m->clear_property("undisguised");
      }
   }
}

void cmd_ask ( object user, object body, Ob *thing, string prep, string subject ) {
  NRef things;
  object who;
  string text;

  if ( !thing ) {
    user->paragraph("What?");
    return;
  }

  things = locate_one(user, FALSE, thing, body,
		       body->query_environment(), body);
 
  if ( prep=="none" && things ) {
    who = NRefOb( things );

    if ( find_bilbo( who, "ask", "auto" ) ) {
      if ( !run_bilbo( who, body, "ask", "auto", NRefDetail(things), nil ) )
	return;
    }
	  
    NPCBRAINS->ask( user, who );
    return;
  }
   
  if ( prep=="for" && things ) {
    who = NRefOb( things );

    if ( find_bilbo( who, "ask-for", "auto" ) ) {
      if ( !run_bilbo( who, body, "ask-for", "auto", NRefDetail(things), ([ "request":subject ]) ) )
	return;
    }
    NPCBRAINS->ask_for( user, body, who, subject );
    return;
  }
   
  if ( prep=="about" && things ) {
    who = NRefOb(things);
    if (find_bilbo(who, "ask-about", "auto")) {
      if (!run_bilbo(who, body, "ask-about", "auto", NRefDetail(things), ([ "request":subject ]))) {
	return;
      }
    }
    if ( who->query_brain("busy") ) {
        if (who->query_brain("busymsg")) {
	  user->paragraph( who->query_brain("busymsg")+"\n" );
	} else {
	  user->paragraph(capitalize(describe_one(who)) + " is busy right now.\n");
	}
    } else {
      if ( already_told( who, user, subject ) ) {
	user->paragraph("He doesn't know anything else about that.\n");
      } else {
	text = NPCBODIES->query_rumor(subject);
	if ( text ) {
	  user->paragraph( text+"\n" );
	  update_told( who, user, subject );
	} else { 
	  user->paragraph( "He doesn't know anything about that.\n" ); /* fix */
	}
      }
    }
    return;
  }
}   

object
find_target(object user, object body, Ob who)
{
    NRef who_ref;

    if (who_ref = locate_one(user, LF_HUSH_ALL, ({ who }), body,
			     body->query_environment())) {
       return NRefOb(who_ref);
    }
    if (!ObPoss(who) && !ObArt(who) && !ObOC(who)) {
	string *words;

	words = ObWords(who);
	return UDATD->query_body(implode(words, " "));
    }	    
    return nil;
}

void cmd_DEV_skills(object user, object body, Ob who) {
   NRef who_ref;
   mapping skills;
   string *indices;
   int i, indices_size;
   object target;

   if (who) {
       target = find_target(user, body, who);
       if (!target) {
	   user->message("Could not find the target.\n");
	   return;
       }
   } else {
      target = body;
   }
   
   skills = target->query_properties()["skill:".."skill:\377"];
   indices = map_indices(skills);

   user->message("Skill listing for " + describe_one(target) + ".\n");

   if (!sizeof(indices)) {
      user->message("No skills found.\n");
      return;
   }

   indices_size = sizeof(indices);
   
   for (i = 0; i < indices_size; i++) {
      user->message(indices[i] + " " + skills[indices[i]] + "\n");
   }
}
   
void cmd_DEV_skills_list(object user, object body, string prop)
{
    int    i, sz;
    mixed *list;

    list = INFOD->query_current_user_objects();
    sz = sizeof(list);

    if (prop) {
	/*
	 * Check all currently online characters for that particular property
	 * and summarize what values there are
	 */
	mixed   *users;
	mixed   *values;
	mapping map;

	for (i = 0; i < sz; i++) {
	    list[i] = list[i]->query_body();
	}
	list -= ({ nil });
	sz = sizeof(list);

	prop = "skill:" + prop[1..strlen(prop) - 2];
	map = ([ ]);
	for (i = 0; i < sz; i++) {
	    mixed val;

	    val = list[i]->query_property(prop);
	    if (val) {
		if (map[val] == nil) {
		    map[val] = ({ list[i] });
		} else {
		    map[val] += ({ list[i] });
		}
	    }
	}
	sz = map_sizeof(map);
	values = map_indices(map);
	users = map_values(map);
	for (i = 0; i < sz; i++) {
	    values[i] = dump_value(values[i]);
	    users[i]  = describe_many(users[i]);
	}
	user->html_message("Summary for \"" + prop + "\":\n<PRE>" +
			   tabulate(({ "Value:", "------" }) + values,
				    ({ "Characters:", "-----------" }) + users) + "</PRE>\n");
    } else {
	/*
	 * Check all currently online characters and see what skill:*
	 * properties they have of > 0 value.
	 */
	string  *props;
	mixed   *users;
	string  *row1, *row2, *row3;
	mapping map, *maps;

	for (i = 0; i < sz; i++) {
	    list[i] = ({ list[i], list[i]->query_body() });
	}

	map = ([ ]);
	for (i = 0; i < sz; i++) {
	    int     j, sz_j;
	    string  list_name;
	    mixed   *values;
	    mapping prop_map;

	    if (is_root(list[i][0]->query_name()) ||
		is_builder(list[i][0]->query_name())) {
	        continue;
	    }
	    if (!list[i][1]) {
	        continue;
	    }
	    prop_map = list[i][1]->query_properties()["skill:".."skill:\377"];
	    sz_j = map_sizeof(prop_map);
	    props = map_indices(prop_map);
	    values = map_values(prop_map);

	    list_name = nil;
	    for (j = 0; j < sz_j; j++) {
		if (values[j]) {
		    if (!list_name) {
			list_name = describe_one(list[i][1]);
		    }
		    /* Not 0 and obviously not nil */
		    if (map[props[j]] == nil) {
			map[props[j]] = ([ values[j]: ([ list_name: TRUE ]) ]);
		    } else {
			if (map[props[j]][values[j]] == nil) {
			    map[props[j]][values[j]] = ([ list_name: TRUE ]);
			} else {
			    map[props[j]][values[j]][list_name] = TRUE;
			}
		    }
		}
	    }
	}

	sz = map_sizeof(map);
	props = map_indices(map);
	maps = map_values(map);
	row1 = ({ });
	row2 = ({ });
	row3 = ({ });
	for (i = 0; i < sz; i++) {
	    int j, sz_j;
	    mixed *values;
	    mapping *submaps;
	    string row1_entry;

	    row1_entry = props[i];

	    sz_j = map_sizeof(maps[i]);
	    values = map_indices(maps[i]);
	    submaps = map_values(maps[i]);
	    for (j = 0; j < sz_j; j++) {
		string row2_entry;
		string *entries;

		row2_entry = dump_value(values[j]);
		entries = map_indices(submaps[j]);
		row1 += ({ row1_entry });
		row2 += ({ row2_entry });
		row3 += ({ implode(entries, ", ") });
		row1_entry = "";
	    }
	}
	user->html_message("Summary of skills:\n<PRE>" +
			   tabulate(({ "Property:",   "---------" })   + row1,
				    ({ "Values:",     "-------" })     + row2,
				    ({ "Characters:", "-----------" }) + row3) + "</PRE>\n");
    }
}

void cmd_DEV_who( object user, object body, varargs string str ) {
   int     i, j, user_count, body_count;
   string  **result, text;
   mapping entry;
   mixed   *list;

   list = build_who_list();

   result = ({
       ({ "User", "----" }),
       ({ "Hostname", "--------" }),
       ({ "Character(s)", "------------" }) });

   for (i = 0; i < sizeof(list); i++) {
      string names;

      entry = list[i];

      if (str) {
	 int found;
	 object *bodies;

	 if (contains(lower_case(entry["user"]), lower_case(str))) {
	    found = TRUE;
	 }
	 bodies = entry["body"]; 
	 if (bodies) {
	    for (j = 0; j < sizeof(bodies); j++) {
	       if (contains(lower_case(describe_one(bodies[j])),
			    lower_case(str)))
		  found = TRUE;
	    }
	 }
	 if (!found) {
	    continue;
	 }
      }
      user_count++;
      if (entry["body"] && sizeof(entry["body"])) {
	 names = describe_many(entry["body"]);
	 body_count += sizeof(entry["body"]);
      } else {
	 names = "-";
      }

      result[0] += ({ entry["user"] });
      result[1] += ({ entry["ip"] });
      result[2] += ({ names });
   }

   if (user_count > 0) {
       user->html_message("<PRE>" + tabulate(result...) + "</PRE>");
   }
   if (user_count > 1) {
       text = user_count + " users and ";
   } else {
       text = user_count + " user and ";
   }

   if (body_count > 1) {
       user->message(text + body_count + " players online.\n");
   } else {
       user->message(text + body_count + " player online.\n");
   }
}

void theatre_who(object user, object body, mixed *list, string str)
{
    int     i, sz_i;
    string  *names;
    mapping characters, theatres;

    characters = ([ ]);
    theatres   = ([ ]);

    for (i = 0, sz_i = sizeof(list); i < sz_i; i++) {
       object *bodies;

       bodies = list[i]["body"]; 

       if (bodies) {
	   int j, sz_j;

	   for (j = 0, sz_j = sizeof(bodies); j < sz_j; j++) {
               if (str) {
		   if (!contains(lower_case(describe_one(bodies[j])),
				 lower_case(str)))
		       continue;
	       }
	       if (ur_def(bodies[j], "default")) {
		  string id, desc;

		  id   = bodies[j]->query_property("Theatre:ID");
		  desc = bodies[j]->query_property("Theatre:GameDesc");
		  if (!bodies[j]->query_property("theatre:notwhobody")) {
		     theatres[id] = desc;
		     if (!characters[id]) {
			characters[id] = ([ ]);
		     }
		     characters[id][describe_one(bodies[j])] = TRUE;
		  }
	       }
	   }
       }
    }

    names = map_indices(characters);
    sz_i = sizeof(names);

    if (sz_i == 0) {
       user->message("There is nobody online.\n");
    } else {
       string *result;

       result = ({ });
       for (i = 0; i < sz_i; i++) {
	  int sz_chars;
	  string *chars;

	  chars = map_indices(characters[names[i]]);
	  sz_chars = sizeof(chars);
	  result += ({
	     "Online in " +
		( theatres[names[i]] ?
		  "\"" + theatres[names[i]] + "\"" :
		  "no specific theatre" ) +
		": " +
	     ( sz_chars == 1 ?
	       chars[0] :
	       implode(chars[..sz_chars - 2], ", ") + " and " +
	       chars[sz_chars - 1] ) +
	     ".\n" });
       }
       user->message(TAG(implode(result, "\n"), "Who"));
    }
}

void generic_who(object user, object body, mixed *list, string str)
{
    int     i, sz_i;
    mapping charmap, idlemap, storymap;
    string  *chars, *idle, *story;

    charmap  = ([ ]);
    idlemap  = ([ ]);
    storymap = ([ ]);

    for (i = 0, sz_i = sizeof(list); i < sz_i; i++) {
       object *bodies;
       int *idles;

       bodies = list[i]["body"]; 
       idles  = list[i]["idle"];

       if (bodies && idles) {
	   int j, sz_j;

	   for (j = 0, sz_j = sizeof(bodies); j < sz_j; j++) {
              if (str) {
		 if (!contains(lower_case(describe_one(bodies[j])),
			       lower_case(str)))
		    continue;
              }
	      if (bodies[j]->query_property("theatre:whobody")) {
		 if (idles[j] < 3600) {
		    string title;

		    title = bodies[j]->query_property("trait:title");
		    if (title && strlen(title) > 5 && title[..4] == "Story") {
		       /* StorySomethings get added here. */
		       storymap[describe_one(bodies[j])] = TRUE;
		    } else {
		       charmap[describe_one(bodies[j])] = TRUE;
		    }
		 } else {
		    idlemap[describe_one(bodies[j])] = TRUE;
		 }
	      }
	   }
       }
    }

    chars = map_indices(charmap);
    idle = map_indices(idlemap);
    story = map_indices(storymap);

    if (!sizeof(chars) && !sizeof(idle) && !sizeof(story)) {
       user->message("There is nobody online.\n");
       return;
    }

    if (sizeof(chars)) {
       user->message("Characters online: ");
       if (sizeof(chars) == 1) {
	  user->message(TAG(chars[0] + ".\n", "Who"));
       } else {
	  user->message(TAG(implode(chars[.. sizeof(chars) - 2], ", ") +
			    " and " + chars[sizeof(chars) - 1] + ".\n",
			    "Who"));
       }
    }
    if (sizeof(story)) {
       user->message("Game Staff online: ");
       if (sizeof(story) == 1) {
	  user->message(TAG(story[0] + ".\n", "Who"));
       } else {
	  user->message(TAG(implode(story[.. sizeof(story) - 2], ", ") +
			    " and " + story[sizeof(story) - 1] + ".\n",
			    "Who"));
       }
    }
    if (sizeof(idle)) {
       user->message("Characters online but idle: ");
       if (sizeof(idle) == 1) {
	  user->message(TAG(idle[0] + ".\n", "Who"));
       } else {
	  user->message(TAG(implode(idle[.. sizeof(idle) - 2], ", ") +
			    " and " + idle[sizeof(idle) - 1] + ".\n",
			    "Who"));
       }
    }
}

void cmd_who( object user, object body, varargs string str ) {
    mixed *list;

    list = build_who_list();
    if (map_sizeof(MODULED->query_modules() & ({ "Theatre" }))) {
       theatre_who(user, body, list, str);
    } else {
       generic_who(user, body, list, str);
    }
}


void cmd_stop ( object user, object body, varargs string *str ) {
  if ( str[0]=="following" ) {
    cmd_follow(user, body, "nothing");
    return; 
  }
  if ( str[0]=="restraining" ) {
     cmd_restrain(user, body, "nobody");
    return;
  }
  user->message("Stop what?\n");
}

void cmd_DEV_badname(object user, object body, mapping str) {
    string name, prefix;

    if (is_root(user->query_name())) {
	prefix = "+";
    } else {
	prefix = "!";
    }
    name = str["evoke"];
    if (!name || strlen(name) <= 2) {
        user->message("Usage:\n" +
		      "    " + prefix + "badname \"name\"\n" +
		      "See also:\n" +
		      "    " + prefix + "goodname \"name\"\n" +
		      "    " + prefix + "badnames [ \"pattern\" ]\n");
	return;
    }
    if ( STARTSTORY->query_badname( name )==nil ) {
        STARTSTORY->add_badname( name );
	user->message( capitalize(name)+" added to bad name list.\n" );
    } else {
        user->message( capitalize(name)+" is already on the bad name list.\n" );
    }
}

 
void cmd_DEV_goodname(object user, object body, mapping str) {
    string name, prefix;

    if (is_root(user->query_name())) {
	prefix = "+";
    } else {
	prefix = "!";
    }

    name = str["evoke"];
    if (!name || strlen(name) <= 2) {
        user->message("Usage:\n" +
		      "    " + prefix + "goodname \"name\"\n" +
		      "See also:\n" +
		      "    " + prefix + "badname \"name\"\n" +
		      "    " + prefix + "badnames [ \"pattern\" ]\n");
        return;
    }
    if ( STARTSTORY->query_badname( name ) ) {
        STARTSTORY->remove_badname( name );
        user->message( capitalize(name)+" removed from the bad name list.\n" );
    } else {
        user->message( capitalize(name)+" is not on the bad name list.\n" );
    }
}

 
void cmd_DEV_badnames(object user, object body, varargs mapping str) {
    string prefix;

    if (str) {
	prefix = str["evoke"];
	if (prefix && strlen(prefix) > 2) {
	    prefix = prefix[1..strlen(prefix) - 2];
	} else {
	    prefix = nil;
	}
    }
    STARTSTORY->show_badnames(user, prefix);
}


void cmd_DEV_possess(object user, object body, Ob *things) {
  NRef target;

  if (target = locate_one(user, FALSE, things, body,
			  body->query_environment())) {
    if (!IsPrime(target)) {
      user->message("You can't possess a detail. It would hurt.\n");
      return;
    }

    if (sizeof(NRefOb(target)->query_souls())) {
      user->message("Warning: This object is already possessed. If you are trying to\npossess a CNPC, ignore this message.\n");
    }
    if (!NRefOb(target)->query_environment()) {
      user->message("You can't possess something without an environment. Summon it first, unless it's a room, in which case you shouldn't possess it.\n");
      return;
    }
    user->possess_body(NRefOb(target));
  }
}


void cmd_GUIDE_swap(object user, object body) {
   object to, *bodies;
   int i;

   if (body->query_property("GUIDE")) {
      /* leave our StoryGuide body and enter our previous body */
      to = body->query_property("GUIDE:SavedBody");
      if (!to) {
	 user->message("Please quit this body and log in the normal way.\n");
	 return;
      }
   } else {
      bodies = user->query_udat()->query_bodies();

      for (i = 0; i < sizeof(bodies) && !to; i ++) {
	 if (bodies[i]->query_property("GUIDE")) {
	    to = bodies[i];
	 }
      }
      if (!to) {
	 user->message("You don't seem to have a StoryGuide body yet!\n");
	 return;
      }
      to->set_property("GUIDE:SavedBody", body);
   }
   body->act_linkdie();
   user->possess_body(to);
}


void cmd_DEV_drag ( object user, object body, varargs mixed obj) {
    NRef thing;
    string dtl;
    object o;
    mixed target;

    target = body->query_property("drag");
   
    if (!obj) {
        if (target)
	  user->message( "You are restraining " + describe_one(target) + ".\n" );
	else
	  user->message( "You aren't restraining anybody.\n" );
	return;
    }
    switch (obj) {
    case "nothing":
    case "nobody":
    case "noone":
        if ( body->query_property("drag") )
	    body->action("undrag");
	else
	    user->message("You aren't restraining anybody.\n");
	return;
    default:
        break;
    }
    thing = locate_one( user, FALSE, obj, body, body->query_environment(),
			body);
    if (thing) {
        o = NRefOb( thing );
	if (!ur_volition(o)) {
	    user->message("You can't restrain that.\n");
	    return;
	}
	if (o == body) {
	    user->message("You can't restrain yourself!\n");
	    return;
	}
    } else {
        return;
    }
    body->action("drag", ([ "victim": o ]));
}

void cmd_DEV_transfer(object user, object body, varargs string *char_arr,
		      string from, string *old_user_arr, string to,
		      string *new_user_arr) {
   string old_user, new_user, char;
   object old_udat, new_udat, moving_body;

   if (!char_arr) {
      user->paragraph("Syntax: +transfer <character> from <user> to <user>");
      return;
   }
   old_user = implode(old_user_arr, " ");
   old_udat = UDATD->query_udat(old_user);
   if (!old_udat) {
      user->paragraph("There's no user named '" + old_user + "'");
      return;
   }
   new_user = implode(new_user_arr, " ");
   new_udat = UDATD->query_udat(new_user);
   if (!new_udat) {
      user->paragraph("There's no user named '" + new_user + "'");
      return;
   }
   char = implode(char_arr, " ");
   moving_body = old_udat->query_body(char);
   if (!moving_body) {
      user->paragraph("There's no body '" + char + "' associated with user '" + old_user + "'");
      return;
   }
   old_udat->del_body(moving_body);
   moving_body->set_property("SkotOS:Creator_orig",
			     moving_body->query_property("SkotOS:Creator"));
   moving_body->set_property("SkotOS:Creator", new_udat->query_name());
   new_udat->add_body(moving_body, TRUE);
   user->paragraph("Done.");
}







static atomic void
start_getprop(string callback, string user, string prop, mixed args...);

/*
 * The +finger command shows:
 * - username
 * - bodies
 * - name / hear / other / email / creation_time.
 * - email
 * - last-logon
 *
 * The +accfinger command shows:
 * - username
 * - bodies
 * - email
 * - play-time
 * - creation-time
 * - last-logon
 * - # of days
 * - total days
 * - % of days
 * - last client
 * - last OS
 * - last browser
 *
 * The new combined accfinger/finger command will show all of the above for
 * SH/SP, and only the following for SG:
 * - username
 * - bodies
 * - creation time
 * - last logon
 */
void
generic_finger(object user, object body, object udat, int access_level, varargs object target_body)
{
    int     i, sz;
    string  *properties, uname;
    mapping params;

    uname = udat->query_name();

    properties = ({
        "creation_time",
        "nextstamp",
        "card",
        "account_type",
        "account_status",
	"banned:when",
	"banned:reason"
    });

    if (access_level > 0) {
	properties += ({
	    "name",
	    "hear",
	    "other",
	    "email",
	});
    }
    sz = sizeof(properties);
    if (SYS_INITD->query_standalone()) {
	call_out("generic_finger_fetch", 0, FALSE, nil, nil,
		 ([ "_total_": 0 ]), uname, user, udat, body, access_level, target_body);
    } else {
	params = ([ "_total_": sz ]);
	for (i = 0; i < sz; i++) {
	    start_getprop("generic_finger_fetch", uname, properties[i],
			  properties[i], params, uname, user, udat, body,
			  access_level, target_body);
	}
    }
}

private string
relative_time(int s)
{
    int d, h, m;
    string *bits;

    bits = allocate(4);

    d = s / 86400;
    if (d) {
	bits[0] = d + "D";
	s -= d * 86400;
    }
    h = s / 3600;
    if (h) {
	bits[1] = h + "h";
	s -= h * 3600;
    }
    m = s / 60;
    if (m) {
	bits[2] = m + "m";
	s -= m * 60;
    }
    if (s) {
	bits[3] = s + "s";
    }
    return implode(bits - ({ nil }), " ");
}

static int
query_total_days(object udat, mixed creation_time)
{
   creation_time = creation_time ? (int)creation_time : 0;

   if (udat->query_number_of_days()) {
      int time;

      time = 972334800; /* "Mon Oct 23 14:00:00 2000 PST" */

      if (creation_time > time) {
         time = creation_time;
      }
      time -= (time % 86400);
      return (time() + 86400 - 1 - time) / 86400;
   }
   return 0;
}

static float
query_percent_of_days(object udat, mixed creation_time)
{
   int tot;

   if (tot = query_total_days(udat, creation_time)) {
      return 100.0 * (float) udat->query_number_of_days() / (float) tot;
   }
   return -1.0;
}

static void
generic_finger_fetch(int success, string val, string prop, mapping params,
		     string uname, object user, object udat, object body,
		     int access_level, varargs object target_body)
{
    int    i, sz, done, last_seen;
    string result, *trans, body_names, account_status;
    mixed  *data;

    params["_total_"]--;
    if (success) {
	params[prop] = val;
    }

    if (params["_total_"] > 0) {
	return;
    }
    /*
     * Using the fact that the mapping will stay shared between the several
     * start_getprop()-triggered call_outs, so that once all of them have
     * returned and indicated success or failure, the properties have been
     * collected and the counter is at 0.
     */
    if (!user || !body || !udat) {
	/* Sanity checking. */
	return;
    }
    if (map_sizeof(MODULED->query_modules() & ({ "Theatre" })) > 0 &&
	sizeof(IDD->query_objects("Theatre:Theatres")) > 1) {
	int    i, sz;
	object *bodies;
	string *names, str;

	bodies = udat->query_bodies();
	sz = sizeof(bodies);
	names = allocate(sz);

	for (i = 0; i < sz; i++) {
	    object *souls;

	    names[i] = describe_one(bodies[i]);
	    if (bodies[i]->query_property("Theatre:GameID")) {
		names[i] += " @ " + bodies[i]->query_property("Theatre:GameID");
	    } else {
		names[i] += " @ Unknown";
	    }
	    souls = bodies[i]->query_souls();
	    if (souls) {
	       souls -= ({ find_object("SkotOS:NPC-Bodies") });
	       if (sizeof(souls)) {
		  names[i] = "*" + names[i];
	       }
	    }
	}
	switch (sz) {
	case 0:
	    str = "None";
	    break;
	case 1:
	    str = capitalize(names[0]);
	    break;
	default:
	    str = capitalize(implode(names[..sz - 2], ", ")) + " and " + names[sz - 1];
	    break;
	}
	body_names = str;
    } else {
        int i, sz;
        object *bodies;

        string *names, str;


        bodies = udat->query_bodies();
        sz = sizeof(bodies);
        names = allocate(sz);

        for (i = 0; i < sz; i++) {
            object *souls;

            names[i] = describe_one(bodies[i]);
            souls = bodies[i]->query_souls();
            if (souls) {
               souls -= ({ find_object("SkotOS:NPC-Bodies") });
               if (sizeof(souls)) {
                  names[i] = "*" + names[i];
               }
            }
        }
        switch (sz) {
        case 0:
            str = "None";
            break;
        case 1:
            str = capitalize(names[0]);
            break;
        default:
            str = capitalize(implode(names[..sz - 2], ", ")) + " and " + names[sz - 1];
            break;
        }
	body_names = str;
	/* body_names = describe_many(udat->query_bodies()); */
    }

    result = "";
    if (target_body && target_body->query_property("skotos:unregistered") &&
	sizeof(udat->query_bodies() & ({ target_body })) == 0) {
       result += "Note: " + uname + " removed '" + describe_one(target_body) +
	         "' from his/her character list on " +
	         ctime(target_body->query_property("skotos:unregistered")) + "\n";
    }
    result +=
	"----- Finger Information -----\n" +
	"Account name:       " + uname + "\n";
    if (access_level > 0) {
	result += "Email:              " + (params["email"] ?
					    params["email"] :
					    "Unknown") + "\n";
    }
    if (!params["nextstamp"]) {
	account_status = "Expired (never logged on)";
    } else if ((int)params["nextstamp"] < time() &&
	(!params["card"] || !strlen(params["card"]))) {
	string when;

	when = ctime((int)params["nextstamp"]);
	account_status = "Expired (" + when[4..9] + " " + when[20..] + ")";
    } else {
	account_status = "Active";
	switch (params["account_type"]) {
	case "staff":
	case "free":
	case "developer":
	    account_status += " (free/staff)";
	    break;
	case "trial":
	    if (params["card"] && strlen(params["card"])) {
		account_status += " (preauth)";
	    } else {
		account_status += " (trial)";
	    }
	    break;
	case "regular":
	    account_status += " (regular)";
	    break;
	default:
	    account_status += " (unknown: tell Nino you saw this!)";
	    break;
	}
    }
    if (params["banned:when"] && strlen(params["banned:when"])) {
       account_status += " (Banned: " + ctime((int)params["banned:when"]) + ")";
    }
    if (udat->query_suspended()) {
       account_status += " (SUSPENDED: " + udat->query_suspended() + ")";
    }
    result +=
	"Account status:     " + account_status + "\n" +
       ((params["banned:reason"] && strlen(params["banned:reason"])) ?
	"Ban reason:         " + params["banned:reason"] + "\n" : ""
	  ) +
	"--\n" +
	"Created:            " + (params["creation_time"] ?
				  ctime((int)params["creation_time"]) :
				  "Unknown") + "\n" +
	"Characters played:  " + body_names + "\n";

    data = udat->query_last_ipnumber("text");
    if (data) {
        if (access_level == 0) {
	    result +=
		"Last seen [text]:   " +
		relative_time(time() - data[LI_WHEN]) + " ago.\n";
	} else {
	    result +=
		"Last seen [text]:   " +
		relative_time(time() - data[LI_WHEN]) + " ago from " +
		(data[LI_IPNR] == data[LI_IPNAME] ?
		 data[LI_IPNAME] :
		 data[LI_IPNAME] + "[" + data[LI_IPNR] + "]") + ".\n";
	}
	last_seen = data[LI_WHEN];
    }

    data = udat->query_last_ipnumber("http");
    if (data) {
       if (access_level == 0) {
	    result +=
		"Last seen [http]:   " +
		relative_time(time() - data[LI_WHEN]) + " ago.\n";
       } else {
	    result +=
		"Last seen [http]:   " +
		relative_time(time() - data[LI_WHEN]) + " ago from " +
		(data[LI_IPNR] == data[LI_IPNAME] ?
		 data[LI_IPNAME] :
		 data[LI_IPNAME] + "[" + data[LI_IPNR] + "]") + ".\n";
	}
	if (!last_seen) {
	    last_seen = data[LI_WHEN];
	}
    }
    if (!last_seen) {
	last_seen = udat->query_last_logon();
	if (last_seen) {
	    result +=
		"Last seen:          " +
		relative_time(time() - last_seen) + " ago.\n";
	}
    }

    result += "--\n";

    trans = ({ });
    if (access_level > 0) {
       trans += ({ "name",    "Name" });
    }
    trans += ({
	"hear",    "Heard about Skotos",
	"other",   "Heard if other",
    });
    done = FALSE;
    for (i = 0, sz = sizeof(trans); i < sz; i += 2) {
	string s;

	s = params[trans[i]];
	if (s && strlen(s)) {
	    result += pad_right(trans[i + 1] + ":", 20) + s + "\n";
	    done = TRUE;
	}
    }
    if (done) {
	result += "--\n";
    }
    if (access_level > 0) {
	mixed *funcs;

	done = FALSE;
	funcs = ({
	    "# of Days",    ({ udat, "query_number_of_days" }),
	    "Total Days",   ({ this_object(), "query_total_days", udat, params["creation_time"] }),
	    "% of Days",    ({ this_object(), "query_percent_of_days", udat, params["creation_time"] }),
	    "Last Client",  ({ udat, "query_last_client" }),
	    "Last OS",      ({ udat, "query_last_os" }),
	    "Last Browser", ({ udat, "query_last_browser" })
	});
	for (i = 0, sz = sizeof(funcs); i < sz; i += 2) {
	    result +=
		pad_right(funcs[i] + ":", 20) +
		call_other(funcs[i + 1]...) + "\n";
	    done = TRUE;
	}
	if (done) {
	    result += "--\n";
	}
    }
    user->html_message("<PRE>\n" + replace_html(result) + "</PRE>\n");
}

void
cmd_DEV_finger(object user, object body, varargs string *bits)
{
    int    access_level;
    string verb, t_uname;
    object t_body, t_udat;

    if (is_root(user->query_name())) {
	verb = "+finger";
	access_level = 1;
    } else {
	verb = "!finger";
	access_level = 0;
    }
    if (!bits || !sizeof(bits)) {
	user->paragraph("Usage: " + verb + " [character name]");
	return;
    }
    t_body = UDATD->query_body(implode(bits, " "));
    if (!t_body) {
	user->paragraph("Unknown character.");
	return;
    }
    t_uname = t_body->query_property("skotos:creator");
    if (!t_uname) {
	user->paragraph("That body has no known creator.");
	return;
    }
    t_udat = UDATD->query_udat(t_uname);
    if (!t_udat) {
	user->paragraph("That body is created by an unknown user: '" +
			t_uname + "'");
	return;
    }
    generic_finger(user, body, t_udat, access_level, t_body);
}

void cmd_DEV_force(object user, object body,
		   varargs Ob who_ref, mapping emap) {
  string voke;
  NRef who;

  if (emap) {
    voke = emap["evoke"];
    if (voke) {
      voke = voke[1 .. strlen(voke)-2];
    }
  }
  if (!voke) {
    user->message("Syntax: +force <body> \"drop skeleton key\n");
    return;
  }
  if (who = locate_one(user, FALSE, who_ref, body,
		       body->query_environment())) {
    if (!IsPrime(who)) {
      user->message("Alas, details cannot do anything.\n");
      return;
    }
    user->paragraph("Forcing " + describe_one(who, body, body) + " to: " + voke);
    "/usr/TextIF/main"->command(user, NRefOb(who), voke);
    return;
  }
}

private mapping woe_handles;

void cmd_DEV_woe(object user, object body, varargs string *words) {
    string str;

    if (!words) {
	user->paragraph(
	   "Usage: +woe [WoePath substring]\n" +
	   "       +woe abort | stop\n");
	return;
    }
    str = lower_case(implode(words, " "));
    if (!woe_handles) {
	woe_handles = ([ ]);
    }
    if (str == "abort" || str == "stop") {
	if (woe_handles[user]) {
	    user->message("Aborting current Tree of Woe search!\n");
	    remove_call_out(woe_handles[user]);
	    woe_handles[user] = nil;
	} else {
	    user->message("You are not currently searching through the Tree of Woe.\n");
	}
	return;
    }
    if (woe_handles[user] != nil) {
	user->message("Aborting previous Tree of Woe search!\n");
	remove_call_out(woe_handles[user]);
	woe_handles[user] = nil;
    }
    user->paragraph("Seeking through the Tree of Woe...");
    woe_handles[user] = call_out("traverse_woe", 0, user, ({ "" }), str);
}

void traverse_woe(object user, string *queue, string seek) {
    if (!user) {
	return;
    }
    if (sizeof(queue) > 0) {
	int i;
	string *arr, prepath;

	prepath = queue[0];

	if (strlen(prepath) > 0) {
	    prepath += ":";
	}
	arr = IDD->query_objects(queue[0]);
	for (i = 0; i < sizeof(arr); i ++) {
	   if (sscanf(prepath + lower_case(arr[i]), "Duplicate:%*s")) {
	      call_other(find_object(prepath + lower_case(arr[i])),
			 "set_object_name",
			 nil);
	   }
	    if (sscanf(lower_case(prepath + arr[i]), "%*s" + seek)) {
		user->paragraph("Woe Match: " + prepath + arr[i]);
	    }
	}
	arr = IDD->query_subdirs(queue[0]);
	for (i = 0; i < sizeof(arr); i ++) {
	    queue += ({ prepath + arr[i] });
	}
	woe_handles[user] = call_out("traverse_woe", 0.01, user, queue[1 ..], seek);
    } else {
	user->paragraph("Woe search finished.");
	woe_handles[user] = nil;
    }
}

/*** Support for the verb-completion code: ***/

void cmd_completions(object user, object body, string line,
                   string verb, string *verbs) {
    if (line && verb && verbs && sizeof(verbs) > 1) {
      user->paragraph("The command \"" + verb +
                      "\" is ambiguous, possible completions are: " +
                      implode(verbs, ", ") + ".\n");
    } else {
      user->paragraph("What?");
    }
}

void cmd_DEV_completions(object user, object body, string line,
                       string verb, string *verbs) {
    if (line && verb && verbs && sizeof(verbs) > 1) {
      user->paragraph("The command \"+" + verb +
                      "\" is ambiguous, possible completions are: " +
                      "+" + implode(verbs, ", +") + ".\n");
    } else {
      user->paragraph("What?");
    }
}

void cmd_GUIDE_completions(object user, object body, string line,
                         string verb, string *verbs) {
    if (line && verb && verbs && sizeof(verbs) > 1) {
      user->paragraph("The command \"!" + verb +
                      "\" is ambiguous, possible completions are: " +
                      "!" + implode(verbs, ", !") + ".\n");
    } else {
      user->paragraph("What?");
    }
}

void cmd_ambiguous(object user, object body, string line, string *verbs_arg0,
                 string *verbs_arg1, string *adverbs_arg0) {
    if (line && verbs_arg0 && verbs_arg1 && adverbs_arg0) {
      user->paragraph("That is ambiguous, possible interpretations are \"" +
                      verbs_arg0[0] + " ...\" and \"" +
                      adverbs_arg0[0] + " " + verbs_arg1[0] + " ...\"" +
                      ", others may be possible.");
    } else {
      user->paragraph("What?");
    }
}

void cmd_DEV_rename(object user, object body, string old, string new_name)
{
    int    flag;
    string verb, err;

    if (is_root(user->query_name())) {
	verb = "+rename";
	flag = TRUE;
    } else {
	verb = "!rename";
	flag = FALSE;
    }
    if (!old || !new_name) {
	user->message("Usage: " + verb + " <old name> <new name>\n");
	return;
    }

    old = lower_case(old);
    new_name = lower_case(new_name);

    /* Find if this isn't going to mess things up */
    if (!UDATD->query_body(old)) {
	user->message("Error: \"" + old + "\" does not exist.\n");
	return;
    }
    if (UDATD->query_body(new_name)) {
	user->message("Error: \"" + new_name + "\" already exists.\n");
	return;
    }
    if (!UDATD->query_body(old)->query_property("SkotOS:Creator")) {
	user->message("Error: \"" + old + "\" has no known creator.\n");
	return;
    }
    if (STARTSTORY->query_badname("\"" + new_name + "\"") == TRUE) {
	if (flag) {
	    user->message("Warning: \"" + new_name + "\" is on the 'badnames' list.\n");
	} else {
	    user->message("Error: \"" + new_name + "\" is on the 'badnames' list.\n");
	    return;
	}
    }

    err = catch(UDATD->rename_body(lower_case(old), lower_case(new_name)));
    if (err) {
       user->message("Unexpected error: " + err + "\n");
       return;
    }
    user->message("Done!\n");
}

void
cmd_DEV_infoline(object user, object body, varargs string what)
{
    int    i, sz;
    string verb, result;
    mixed  *recall;

    if (is_root(user->query_name())) {
	verb = "+infoline";
    } else {
	verb = "!infoline";
    }
    switch (what) {
    case "on":
	if (INFOLINE->query_listener(user, body)) {
	    user->message("You are already listening to the infoline.\n");
	    return;
	}
	INFOLINE->add_listener(user, body);
	user->message("You are now listening to the infoline.\n");
	return;
    case "off":
	if (!INFOLINE->query_listener(user, body)) {
	    user->message("You were not listening to the infoline.\n");
	    return;
	}
	INFOLINE->remove_listener(user, body);
	user->message("You are no longer listening to the infoline.\n");
	return;
    case "recall":
        recall = INFOLINE->query_recall();
	sz = sizeof(recall);
	result = "";
	for (i = 0; i < sz; i++) {
	   result += ctime(recall[i][0])[11..18] + ": " + recall[i][1] + "\n";
	}
	user->message("These are the most recent infoline messages:\n" +
		      result);
	return;
    default:
	if (INFOLINE->query_listener(user, body)) {
	    user->message("You are listening to the infoline.\n");
	} else {
	    user->message("You are not listening to the infoline.\n");
	}
	user->message("Usage: " + verb + " [on | off]\n");
	return;
    }
}

void
cmd_DEV_suspend(object user, object body, varargs string what, string name,
		mapping reason)
{
    string current, reason_text, verb;
    object udat;

    if (is_root(user->query_name())) {
	verb = "+suspend";
    } else {
	verb = "!suspend";
    }
    if (!what) {
        user->message("Usage: " + verb + " add <username> [\"...reason...\"]\n" +
		      "       " + verb + " remove <username>\n" +
		      "       " + verb + " status <username>\n");
	return;
    }
    udat = UDATD->query_udat(name);
    if (!udat) {
        user->message("Could not find a user named \"" + name + "\".\n");
	return;
    }
    current = udat->query_suspended();
    switch (what) {
    case "add":
	if (reason && reason["evoke"]) {
	    reason_text = reason["evoke"];
	    reason_text = reason_text[1..strlen(reason_text) - 2];
	} else {
	    reason_text = "";
	}
	if (current) {
	    user->message("The user \"" + name +
			  "\" has already been suspended.\n" +
			  "Changing the reason to: " +
			  (strlen(reason_text) ? reason_text : "None") + "\n");
	    if (SYS_MERRY->query_script_space("assist")) {
	       /* Hook into Aziel's TAS, if it's there. */
	       run_merry(SYS_MERRY->query_script_space("assist"),
			 "dofileassist",
			 "lib",
			 ([ "actor":      body,
			    "imperative": "+suspend",
			    "data":       find_object("Data:DB:assist:AssistGeneralDB"),
			    "char":       name(body),
			    "queue":      "suspended",
			    "assisttext": "Updated the reason for suspending the \"" + name + "\" account to: " + (strlen(reason_text) ? reason_text : "None"),
			    "log":        "[TAS]",
			    "sysid":      "tas",
			    "syslvl":     "7" ]));
	    }
	    LOGD->add_entry("Suspend Log",
			    describe_one(body) + "[" +
			    user->query_name() + "] changed reason for " +
			    name + "'s suspension",
			    name +
			    "'s account has been suspended for this game.\n" +
			    "\n" +
			    "Reason was: " +
			    (strlen(current) ? current : "None") + "\n" +
			    "Reason changed to: " +
			    (strlen(reason_text) ? reason_text : "None") +
			    "\n");
	} else {
	    user->message("The user \"" + name +
			  "\" has now been suspended.\n" +
			  "The reason given was: " +
			  (strlen(reason_text) ? reason_text : "None") + "\n");
	    if (SYS_MERRY->query_script_space("assist")) {
	       /* Hook into Aziel's TAS, if it's there. */
	       run_merry(SYS_MERRY->query_script_space("assist"),
			 "dofileassist",
			 "lib",
			 ([ "actor":      body,
			    "imperative": "+suspend",
			    "data":       find_object("Data:DB:assist:AssistGeneralDB"),
			    "char":       name(body),
			    "queue":      "suspended",
			    "assisttext": "The account \"" + name + "\" has been suspended; Reason: " + (strlen(reason_text) ? reason_text : "None"),
			    "log":        "[TAS]",
			    "sysid":      "tas",
			    "syslvl":     "7" ]));
	    }
	    LOGD->add_entry("Suspend Log",
			    describe_one(body) + "[" +
			    user->query_name() + "] suspended " +
			    name + "'s account",
			    name +
			    "'s account has been suspended for this game.\n" +
			    "\n" +
			    "Reason given: " +
			    (strlen(reason_text) ? reason_text : "None") +
			    "\n");
	}
	udat->set_suspended(reason_text);
        break;
    case "remove":
        if (current) {
	    user->message("The user \"" + name +
			  "\" is now no longer suspended.\n");
	    udat->set_suspended(nil);
	    if (SYS_MERRY->query_script_space("assist")) {
	       /* Hook into Aziel's TAS, if it's there. */
	       run_merry(SYS_MERRY->query_script_space("assist"),
			 "dofileassist",
			 "lib",
			 ([ "actor":      body,
			    "imperative": "+suspend",
			    "data":       find_object("Data:DB:assist:AssistGeneralDB"),
			    "char":       name(body),
			    "queue":      "suspended",
			    "assisttext": "The account \"" + name + "\" is no longer suspended.",
			    "log":        "[TAS]",
			    "sysid":      "tas",
			    "syslvl":     "7" ]));
	    }
	    LOGD->add_entry("Suspend Log",
			    describe_one(body) + "[" +
			    user->query_name() + "] unsuspended " +
			    name + "'s account",
			    name +
			    "'s account is no longer suspended for this game.\n");
	} else {
	    user->message("The user \"" + name +
			  "\" had not been suspended.\n");
        }
        break;
    case "status":
    default:
	if (current) {
	    user->message("Suspend comment for \"" + name + "\": " +
			  (strlen(current) ? current : "None") + "\n");
	} else {
	    user->message("The user \"" + name +
			  "\" has not been suspended.\n");
	}
	break;
    }
}

void cmd_DEV_storypoints_help(object user, object body)
{
    user->html_message("Usage:<PRE>\n" +
		       replace_html(
			   "+storypoints [help]\n" +
			   "        This text.\n" +
			   "+storypoints info <account>\n"+
			   "        Verify somene's SPS total.\n" +
			   "+storypoints take <account> \"<reason>\n" +
			   "        Take 50 SPS from someone for the reason given.\n" +
			   "+storypoints take <account> <amount> \"<reason>\n" +
			   "        Take SPS from someone for the reason given, not necessarily 50.\n" +
			   "+storypoints give <account> \"<reason>\n" +
			   "        Give 50 SPS to someone for the reason given.\n" +
			   "+storypoints give <account> <amount> \"<reason>\n" +
			   "        Give SPS to someone for the reason given, not necessarily 50.\n" +
			   "+storypoints log [<days>]\n" +
			   "        Review the +storypoint activity in the last X days (default 7 days).\n" +
			   "+storypoints log <account> [<days>]\n" +
			   "        Review the +storypoint activity by the given account (default 7 days).\n") +
		       "</PRE>\n");
}

void cmd_DEV_storypoints_info(object user, object body, string name)
{
    start_getprop("delayed_storypoints_info", name, "storypoints:available",
		  user, body, name);
}

static void
delayed_storypoints_info(int success, string val, object user, object body, string name)
{
    int total, i, sz;
    mixed **result;

    if (!user || !body) {
	return;
    }
    if (success) {
	total = (int)val;
    } else {
	total = 0;
    }
    if (total) {
	user->message("The account \"" + name + "\" has " + total + " StoryPoint" + (total == 1 ? "" : "s") + " available.\n");
    } else {
	user->message("The account \"" + name + "\" has no StoryPoints or does not exist.\n");
    }
    result = STORYPOINTS->query_per_account(name, 0, time());
    sz = sizeof(result);
    if (sz > 0) {
       user->message("StoryPoints activity for this account:\n");
       for (i = 0; i < sz; i++) {
	  if (result[i][3] >= 0) {
	     user->message(ctime(result[i][0]) + " - " + capitalize(result[i][1]) + " took " + result[i][3] + " StoryPoint" +
			   (result[i][3] == 1 ? "" : "s") + " from " + capitalize(result[i][2]) + ": \"" +
			   result[i][4] + "\".\n");
	  } else {
	     user->message(ctime(result[i][0]) + " - " + capitalize(result[i][1]) + " gave " + (-result[i][3]) + " StoryPoint" +
			   (result[i][3] == -1 ? "" : "s") + " to " + capitalize(result[i][2]) + ": \"" +
			   result[i][4] + "\".\n");
	  }
       }
    } else {
       user->message("No known StoryPoints activity for this account.\n");
    }
}

void cmd_DEV_storypoints_take(object user, object body, string name, int amount, mapping evoke)
{
    string reason;

    if (sizeof(STORYPOINTS->query_accounts() & ({ user->query_name() })) == 0) {
	user->message("Alas, this function is not available for general use.\n");
	return;
    }
    if (amount <= 0) {
	user->message("You need to provide a positive number.\n");
	return;
    }
    reason = evoke["evoke"];
    reason = reason[1..strlen(reason) - 2];
    if (!strlen(reason)) {
	user->message("You need to provide a meaningful reason for this action.\n");
	return;
    }
    start_sps_use("delayed_storypoints_take", name, amount, reason, body->query_property("theatre:id"), user, body, name, amount, reason);
}

static void
delayed_storypoints_take(int success, string val, object user, object body, string name, int amount, string reason)
{
    if (success) {
	STORYPOINTS->add_usage(user->query_name(), name, amount, reason);
	user->message("You've successfully taken " + amount + " StoryPoint" + (amount == 1 ? "" : "s") + " from the account \"" +
		      name + "\" because: \"" + reason + "\".\n");
    } else {
	user->message("No success, because: \"" + val + "\".\n");
    }
}

void cmd_DEV_storypoints_give(object user, object body, string name, int amount, mapping evoke)
{
    string reason;

    if (sizeof(STORYPOINTS->query_accounts() & ({ user->query_name() })) == 0) {
	user->message("Alas, this function is not available for general use.\n");
	return;
    }
    if (amount <= 0) {
	user->message("You need to provide a positive number.\n");
	return;
    }
    reason = evoke["evoke"];
    reason = reason[1..strlen(reason) - 2];
    if (!strlen(reason)) {
	user->message("You need to provide a meaningful reason for this action.\n");
	return;
    }
    start_sps_add("delayed_storypoints_give", name, amount, reason, body->query_property("theatre:id"), user, body, name, amount, reason);
}

static void
delayed_storypoints_give(int success, string val, object user, object body, string name, int amount, string reason)
{
    if (success) {
	STORYPOINTS->add_usage(user->query_name(), name, -amount, reason);
	user->message("You've successfully given " + amount + " StoryPoint" + (amount == 1 ? "" : "s") + " to the account \"" +
		      name + "\" because: \"" + reason + "\".\n");
    } else {
	user->message("No success, because: \"" + val + "\".\n");
    }
}

void cmd_DEV_storypoints_log(object user, object body, int days)
{
    int   i, sz;
    mixed **result;

    result = STORYPOINTS->query_per_stamp(time() - days * 86400, time());
    sz = sizeof(result);
    if (!sz) {
	user->message("No StoryPoints activity in the past " + days + " day" + (days == 1 ? "" : "s") + ".\n");
	return;
    }
    user->message("StoryPoints activity in the past " + days + " day" + (days == 1 ? "" : "s") + ":\n");
    for (i = 0; i < sz; i++) {
	if (result[i][3] >= 0) {
	    user->message(ctime(result[i][0]) + " - " + capitalize(result[i][1]) + " took " + result[i][3] + " StoryPoint" +
			  (result[i][3] == 1 ? "" : "s") + " from " + capitalize(result[i][2]) + ": \"" +
			  result[i][4] + "\".\n");
	} else {
	    user->message(ctime(result[i][0]) + " - " + capitalize(result[i][1]) + " gave " + (-result[i][3]) + " StoryPoint" +
			  (result[i][3] == -1 ? "" : "s") + " to " + capitalize(result[i][2]) + ": \"" +
			  result[i][4] + "\".\n");
	}
    }
}

void cmd_DEV_storypoints_log_user(object user, object body, string name, int days)
{
    int i, sz;
    mixed **result;

    result = STORYPOINTS->query_per_user(name, time() - days * 86400, time());
    sz = sizeof(result);
    if (!sz) {
	user->message("No StoryPoints activity in the past " + days + " day" + (days == 1 ? "" : "s") + " by " +
		      capitalize(name) + ".\n");
	return;
    }
    user->message("StoryPoints activity in the past " + days + " day" + (days == 1 ? "" : "s") + " by " +
		  capitalize(name) + ":\n");
    for (i = 0; i < sz; i++) {
	if (result[i][2] >= 0) {
	    user->message(ctime(result[i][0]) + " - " + result[i][2] + " StoryPoint" +
			  (result[i][2] == 1 ? "" : "s") + " from " + capitalize(result[i][1]) + ": \"" +
			  result[i][3] + "\".\n");
	} else {
	    user->message(ctime(result[i][0]) + " - " + (-result[i][2]) + " StoryPoint" +
			  (result[i][2] == -1 ? "" : "s") + " to " + capitalize(result[i][1]) + ": \"" +
			  result[i][3] + "\".\n");
	}
    }
}


/*
 * Several commands are available to both StoryGuides and StoryHosts &
 * StoryPlotters, this section is dedicated to relaying the SG version
 * to the SH version.
 */

void cmd_GUIDE_finger(object user, object body, mixed args...) {
    this_object()->cmd_DEV_finger(user, body, args...);
}

void cmd_GUIDE_badname(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_badname(user, body, args...);
}

void cmd_GUIDE_badnames(object user, object body, mixed args...) {
    this_object()->cmd_DEV_badnames(user, body, args...);
}

void cmd_GUIDE_goodname(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_goodname(user, body, args...);
}

void cmd_GUIDE_drag(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_drag(user, body, args...);
}

void cmd_GUIDE_infoline(object user, object body, mixed args...) {
    this_object()->cmd_DEV_infoline(user, body, args...);
}

void cmd_GUIDE_suspend(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_suspend(user, body, args...);
}

void cmd_GUIDE_rename(object user, object body, mixed args...) {
    this_object()->cmd_DEV_rename(user, body, args...);
}
