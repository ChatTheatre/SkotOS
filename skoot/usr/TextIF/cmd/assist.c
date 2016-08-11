/*
**	/usr/TextIF/lib/cmds/assist.c
**
**********************************************************************
**	00070x Wes	Added assist commands
**	010112 Zell	Moved assist stuff out of misc.c into assist.c
**
**	Copyright Skotos Tech Inc 1999-2001
*/

# include <XML.h>
# include <SkotOS.h>
# include <base.h>
# include <System.h>
# include <TextIF.h>

private inherit "/lib/string";
private inherit "/lib/array";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";

private inherit "/usr/SkotOS/lib/auth";

# include "/usr/SkotOS/include/noun.h"
# include "/usr/TextIF/include/misc.h"
# include <UserAPI.h>

static int check_storyguide_body(object user, object body);

private int
valid_user(string username)
{
    int i, sz;
    object *users, user;

    username = lower_case(username);

    if (UDATD->query_udat(username)) {
        return TRUE;
    }
    users = INFOD->query_current_user_objects();
    sz = sizeof(users);
    for (i = 0; i < sz; i++) {
	if (users[i]->query_name() && lower_case(users[i]->query_name()) == username) {
	    return TRUE;
	}
    }
    return FALSE; 
}

private object *
find_users(string username)
{
   int    i, sz;
   object *list;

   username = lower_case(username);
   list = INFOD->query_current_user_objects();
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      if (list[i]->query_name() != username || !list[i]->query_body()) {
	 list[i] = nil;
      }
   }
   return list - ({ nil });
}

void cmd_assist ( object user, object body, mixed str ) {
    int len;

    if (!str["evoke"] || (len = strlen(str["evoke"])) <= 2) {
	user->paragraph("Use the assist command to receive help from our Customer Experience department.  Use it as follows: assist \"I need help with whatever.\" You will be sent a message when the next CE specialist can help you.\n");
	return;
    }
    ASSISTD->add_assist( user, body, str["evoke"][1..len - 2]);
}

private void
assist_goto(object user, object body)
{
    int    i, sz;
    string helped, test;
    object *users, target;

    helped = ASSISTD->query_currently_helping(user);
    if (!helped) {
        user->paragraph("You aren't helping anyone right now.\n");
	return;
    }

    users = INFOD->query_current_user_objects();
    sz = sizeof(users);
    for (i = 0; i < sz; i++) {
        test = describe_one(users[i]->query_body());
	if (pad_right(lower_case(test),80) == pad_right(lower_case(helped),80)) {
	    target = users[i]->query_body();
	    break;
	}
    }
    if (target) {
        body->action("teleport", ([ "dest" : target->query_environment() ]) );
    } else {
        user->message("Could not find the character you are helping.\n");
    }
}

void
cmd_DEV_assist(object user, object body, varargs string *str, mapping evoke)
{
   int    i, sg_check;
   string verb;

   if (is_root(user->query_name())) {
      verb = "+assist";
      sg_check = FALSE;
   } else {
      verb = "!assist";
      sg_check = TRUE;
   }
   if (!str || !sizeof(str)) {
      ASSISTD->list_assists( user );
      return;
   }
   sscanf( str[0], "%d", i );
   if ( i > 0 ) {
      if (sg_check && check_storyguide_body(user, body)) {
	 return;
      }
      ASSISTD->do_assist( user, i );
      return;		
   }
   switch( str[0] ) {
   case "cur":
      ASSISTD->list_open( user );
      return;
   case "clear":
      if (sg_check && check_storyguide_body(user, body)) {
	 return;
      }
      ASSISTD->clear_assists( user );
      return;
   case "done":
      if (sg_check && check_storyguide_body(user, body)) {
	 return;
      }
      if (!evoke["evoke"]) {
	  user->message("Please provide a brief description of what the problem was and what you did.\n");
	  return;
      }
      ASSISTD->assist_done( user, evoke["evoke"]);
      return;
   case "ugly":
      ASSISTD->list_assists(user, TRUE);
      return;
   case "curugly":
      ASSISTD->list_open(user, TRUE);
      return;
   case "alert": {
      object *alerts;

      alerts = ASSISTD->query_alerts();
      if (member(body, alerts)) {
	 ASSISTD->remove_from_alerts(body);
	 user->message("You will no longer receive assist alerts.\n");
      } else {
	 ASSISTD->add_to_alerts(body);
	 user->message("You will now receive assist alerts.\n");
      }
      return;
   }
   case "reboot":
      if (sg_check) {
	 break;
      }
      ASSISTD->clear();
      user->paragraph("Assists cleared.\n");
      return;
   case "goto":
      if (sg_check && check_storyguide_body(user, body)) {
	 return;
      }
      assist_goto( user, body );
      return;
   case "status": {
      object *alerts, *souls, obj;
      int i, j;

      alerts = ASSISTD->query_alerts();

      user->message("\nThe following logged-in staff have alerts enabled:\n" );

      for (i = 0; i < sizeof(alerts); i++) {
	 souls = alerts[i]->query_souls();
	 for (j = 0; j < sizeof(souls); j ++) {
	    obj = alerts[i]->query_souls()[j];
	    if (obj->query_name()) {
	       user->message( "   " + obj->query_name() + " (as "+describe_one(alerts[i])+")\n");
	       j = sizeof(souls);
	    }
	 }
      }
      user->message( "\nThere are "+map_sizeof(ASSISTD->query_wait_queue()) +" assists waiting to be helped.\n");
      user->message( "There are "+map_sizeof(ASSISTD->query_work_queue()) +" assists currently being helped.\n\n");
   }
   }
   user->paragraph(verb + "  (open) (done) (alert) (1-XX)\n");
}


void cmd_GUIDE_assist(object user, object body, varargs string *str, mapping evoke)
{
    cmd_DEV_assist(user, body, str, evoke);
}

private string
pad_char(string s, string char, int max)
{
    while (strlen(s) < max) {
        s += char;
    }
    return s;
}

void
cmd_DEV_notes(object user, object body, mixed person)
{
    int    i;
    string author, stime, msg;
    mixed  notes;

    if (!valid_user(person)) {
        user->paragraph("That is not a valid user name.\n");
	return;
    }
    notes = ASSISTD->query_user_notes(person);

    if (!notes) {
        user->message("No notes have been entered for that user.\n");
	return;
    }

    user->clear_more();
    user->add_more("\n");
    for (i = sizeof(notes); i > 0; i--) {
        sscanf( notes[i-1], "%s|%s|\"%s\"", author, stime, msg );

	user->add_more("** "+pad_char(capitalize(author)+" wrote at "+stime+" ","*",57));
	user->add_more("\n");
	user->add_more(msg);
	user->add_more("\n");
    }
    user->add_more( "** "+pad_char("End of notes ","*",57) );
    user->add_more("\n");
    user->show_more();
}

void
cmd_DEV_addnote(object user, object body, mixed person, mapping msg)
{
    string s;

    if (msg["evoke"] == nil) {
        user->paragraph("Invalid note text.\n");
	return;
    }
    if (!valid_user(person)) {
        user->paragraph("That is not a valid user name.\n" );
	return;
    }
    ASSISTD->add_user_note(person, user->query_name()+ "|" +
			   ctime(time())+"|"+msg["evoke"]);
    user->message("Note added.\n");  
}

void
cmd_DEV_killnote(object user, object body, mixed person)
{
    if (!valid_user(person)) {
        user->paragraph("That is not a valid user name.\n");
	return;
    }
    if (ASSISTD->kill_last_note(person)) {
        user->message("Last note removed.\n");
	return;
    }
    user->message("No notes entered for that user.\n");
}

void
cmd_DEV_info(object user, object body, mixed person)
{
    int    i, sz;
    object *list;

    if (!person) {
       list = ({ user });
    } else {
       list = find_users(person);
       if (!sizeof(list)) {
	  user->message("Noone with that account name is playing right now.\n");
	  return;
       }
    }
    user->message("Information for " + capitalize(lower_case(person)) + ":\n");
    sz = sizeof(list);
    for (i = 0; i < sz; i++) {
       object body;

       body = list[i]->query_body();
       if (!body) {
	  continue;
       }
       user->message("\n" +
		     "  Character: " + describe_one(body)+ "\n" +
		     "  Location:  " + (body->query_environment() ? describe_one(body->query_environment()) : "Nowhere.") + "\n" +
		     "  Woe name:  " + (body->query_environment() ? name(body->query_environment()) : "<NIL>") + "\n");
    }
}

void cmd_GUIDE_notes(object user, object body, mixed args...) {
    this_object()->cmd_DEV_notes(user, body, args...);
}

void cmd_GUIDE_addnote(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_addnote(user, body, args...);
}

void cmd_GUIDE_killnote(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_killnote(user, body, args...);
}
