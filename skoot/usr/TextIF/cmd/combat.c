/*
**	/usr/TextIF/lib/cmds/combat.c
**
**	
**
**	Copyright Skotos Tech Inc 1999
*/

# include <System.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <base/act.h>
# include <base.h>

private inherit "/lib/string";

private inherit "/base/lib/urcalls";
inherit "~TextIF/lib/misc";

# include "/usr/SAM/include/sam.h"
# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"

string query_fatigue_adverb(int a, int b) {
   float res;

   if ((a == 0) || (b == 0))
      return "";

   res = (float)(a / b);

   XDebug("RES: " + res);

   if ((res > 2.1) && (res < 3.1))
      return "masterfully ";
   if ((res > 1.0) && (res < 2.1))
      return "strongly ";

   /* 0 */

   if (res < 1.0)
      return "weakly ";
   if (res <= 0.5)
      return "poorly ";

   return "";
}

void cmd_duel(object user, object body, varargs Ob *what) {
   object who_dueling, me_dueling;
   object who, *clothes_list;
   NRef what_ref;
   int i, clothes_size, found;

   if (!what) {
      if (me_dueling = body->query_opponent()) {
	 user->paragraph("You are dueling " +
		       describe_one(me_dueling, body, body) + ".");
	 return;
      }
      user->paragraph("You are not dueling.\n");
      user->paragraph("usage: duel <person>\n");
      return;
   }

   what_ref = locate_one(user, FALSE, what, body, body->query_environment());
   if (!what_ref) {
      return;
   }
   who = NRefOb(what_ref);

   if (who == body) {
      if (who->query_opponent()) {
	 user->paragraph("You courageously stop fighting.");
	 body->action("end_duel",
		      ([ "who" : ({ who, who->query_opponent() }),
		         "loser" : who ]) );
	 return;
      }
      else {
	 user->paragraph("You cannot duel yourself.");
	 return;
      }
   }

   if (me_dueling = body->query_opponent()) {
      user->paragraph("You are already dueling " 
		    + describe_one(me_dueling, body, body) + ".");
      return;
   }
   
   if (who_dueling = who->query_opponent()) {
      user->paragraph(describe_one(who) + " is already dueling " + 
		    describe_one(who_dueling, who) + ".");
      return;
   }

   clothes_list = body->list_clothes();
   clothes_size = sizeof(clothes_list);
   found = FALSE;
   for(i = 0; i < clothes_size; i++) {
      if (ur_weapon(clothes_list[i]))
	 found = TRUE;
   }

   if (!found) {
      user->paragraph("You are not wielding a weapon.");
      return;
   }

   if (body->query_challenger() == who) {
      body->action("duel",
		   ([ "who" : who ]) );
      return;
   }

   body->action("challenge",
		([ "who" : who ]) );
}

void cmd_challenge(object user, object body, varargs Ob *what) {
   cmd_duel(user, body, what);
}  

void cmd_combat(object user, object body, string action, mixed adverb) {
   object *souls, opponent;
   int i, souls_size, handler;

   /* TODO: adverb is currently not used */

   if (opponent = body->query_opponent()) {
      body->set_curr_combat_action(action);
      body->action("combat_" + action, ([ "opponent" : opponent ]));
   }
   else {
      user->paragraph("You are not dueling.");
   }

   return;
}

object
find_target(object user, object body, Ob who)
{
    NRef who_ref;

    who_ref = locate_one(user, LF_HUSH_ALL, ({ who }), body,
			 body->query_environment());
    if (IsNRef(who_ref)) {
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

    list = users();
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

	    if (DEV_USERD->query_wiztool(list[i][0]->query_name())) {
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
