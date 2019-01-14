/*
**	/usr/CSD/sys/cmds/combat.c
**
**	Copyright Skotos Tech Inc 2001
*/

# include <System.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <base/act.h>
# include <base.h>

private inherit "/lib/string";
private inherit "/base/lib/urcalls";

inherit "/usr/SkotOS/lib/noun";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"
# include "/usr/SkotOS/include/describe.h"
# include "/usr/SkotOS/include/noun.h"
# include "/usr/CSD/include/csd.h"

/* toggle auto combat */
void cmd_autocombat(object user, object body) {
   if (body->query_property("base:combat:auto-combat")) {
      body->set_property("base:combat:auto-combat", nil);
      user->message("Auto combat toggled OFF.\n");
      return;
   } 

   body->set_property("base:combat:auto-combat", TRUE);
   user->message("Auto combat toggled ON.\n");
}

/* display the values of my stats */
/* for now it will show the numerical value */
void cmd_stats(object user, object body) {
   string *stats;
   int i;

   /* this is ugly, I hate doing this */
   /* I would like to popup a cute window to display all this crap -- Wes */

   stats = CSD->query_stat_list();
   user->message("Character statistics for " + describe_one(body) +
		 ".\n");

   for (i = 0; i < sizeof(stats); i++)
      user->message(stats[i] + ": " + 
		    CSD->query_stat(body, stats[i]) + "\n");

}

/* display the values of my skills */
/* for now it will show the numerical value */
void cmd_skills(object user, object body) {
   mapping skills;
   string *indices;
   int i, found;

   skills = body->query_skills();
   indices = map_indices(skills);

   user->message("-- Skills\n");

   for (i = 0; i < sizeof(indices); i++) {
      user->message(indices[i] + ": " + skills[indices[i]] + "\n");
      found = TRUE;
   }

   if (!found) {
      user->message("You are a pathetic worm with no skills.\n");
   }

   return;
}

/* surrender to the foe */
void cmd_surrender(object user, object body, varargs Ob *what) {
   object *opponents, who;
   NRef what_ref;

   if (!what) {
      user->paragraph("usage: surrender <person>\n");
      return;
   }

   what_ref = locate_one(user, FALSE, what, body, body->query_environment());
   if (!what_ref) {
      return;
   }
   who = NRefOb(what_ref);

   if (who == body) {
      user->paragraph("You cannot surrender to yourself.");
      return;
   }

   if (!CSD->is_fighting(body, who)) {
      user->paragraph("You are not fighting " + 
		      describe_one(who, body, body) + ".\n");
      return;
   }

   body->action("surrender",
		([ "who" : who ]) );

}

/* new general combat command -- fight! */
void cmd_fight(object user, object body, varargs Ob *what) {
   object who;
   mapping me_dueling, who_dueling;
   NRef what_ref;
   int i, clothes_size, found;

   me_dueling = body->query_property("base:combat:opponents");

   if (!what) {
      if (me_dueling && sizeof(map_indices(me_dueling))) {
	 user->paragraph("You are fighting " +
		       describe_many(map_indices(me_dueling), 
				     body, body) + ".");
	 return;
      }
      user->paragraph("You are not in combat.\n");
      user->paragraph("usage: fight <person>\n");
      return;
   }

   what_ref = locate_one(user, FALSE, what, body, body->query_environment());
   if (!what_ref) {
      return;
   }
   who = NRefOb(what_ref);

   if (who == body) {
      /* we should enventually make this possible */
      user->paragraph("You cannot fight yourself.");
      return;
   }

   if (me_dueling && me_dueling[who]) {
      user->paragraph("You are already fighting " 
		    + describe_one(who, body, body) + ".");
      return;
   }

   if (!ur_volition(who)) {
      user->paragraph("You cannot fight that.");
      return;
   }

   body->action("fight",
		([ "who" : who ]) );
}
