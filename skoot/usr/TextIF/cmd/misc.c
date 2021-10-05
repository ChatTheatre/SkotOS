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
