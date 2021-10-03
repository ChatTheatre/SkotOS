/*
**	/usr/TextIF/cmds/basic.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <status.h>
# include <base.h>
# include <SAM.h>
# include <XML.h>
# include <base/living.h>
# include <base/bulk.h>
# include <base/preps.h>
# include <base/act.h>
# include <TextIF.h>
# include <SkotOS.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";

inherit "/usr/TextIF/lib/misc";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";
inherit "/usr/TextIF/lib/tell";

# include "/usr/SkotOS/include/noun.h"
# include "/usr/TextIF/include/tell.h"

# define INVENTORY_CAP	30
# define OBJECT_INV_CAP	30
# define PLAYER_INV_CAP	30

private void show_item(object *users, object who, NRef what, string types...);

private
int object_number(object item) {
   int num;

   if (sscanf(ur_name(item), "%*s#%d", num)) {
      return num;
   }
   return status(item)[O_INDEX];
}

void illustrate_environment(object *users, object item, object env);

void desc_login(object *users, object who) {
   object *inv;
   int cut, i;

   general_sam(users, who, nil,
	       XML->parse("<describe what=$(Actor)/> enters the world.<br/>"),
	       nil, TRUE);

   inv = who->query_inventory();

   if (sizeof(inv) > INVENTORY_CAP) {
      inv = inv[.. INVENTORY_CAP - 1];
      cut = TRUE;
   } else {
      cut = FALSE;
   }

   for (i = 0; i < sizeof(inv); i ++) {
      transmit_to_client(nil, who, object_number(inv[i]) + " " +
			 capitalize(describe_one(inv[i], nil, nil,
						 STYLE_NONPOSS)),
			 INVENTORY_ENTER);
   }
   if (cut) {
      transmit_to_client(users, who, "1 (truncated)", INVENTORY_ENTER);
   }
   illustrate_environment(users, who, who->query_environment());
}

void desc_quit(object *users, object who) {
   if ( who->query_brain("npc")!=nil )
      return;

   if ( ur_volition(who)==FALSE )
      return; 

   general_sam(users, who, nil,
	       XML->parse("<describe what=$(Actor)/> leaves the world.<br/>"),
	       ([ ]), TRUE);
}

void desc_linkdie(object *users, object who) {
   if (who->query_brain("npc")!=nil )
      return;

   if (ur_volition(who)==FALSE )
      return;

   general_sam(users, who, nil,
	       XML->parse("OOC - <describe what=$(Actor)/> loses <possessive what=$(Actor)/> connection.<br/>"),
	       ([ ]), TRUE);
}

static
void do_emote ( object user, object body, string what, string mode ) {
   object o;
   mixed *who;
   int i;
   string s, s1, s2, name, tmp;

   if ( !what ) {
      user->message("Usage: +emote string without quotes, and use +Personname for substitutions\n");
      return;
   }

   if ( what[0] == '"' )
       what = what[ 1..strlen(what)-1 ];
   if ( what[strlen(what)-1] == '"' )  
       what = what[ 0..strlen(what)-2 ];

   if ( mode=="emitall" ) {
      mapping obset, list, ip, charnames;
      object master, ob, whom;
      mixed *obs;
      object *users, *tmp, *bodies;
      int users_size;

      users = INFOD->query_current_user_objects();
      users_size = sizeof(users);

      who = ({ });

      for(i = 0; i < users_size; i++) {
          if (users[i]->query_conn() && (name = users[i]->query_name())) {
             if (users[i]->query_body()) {
                who += ({ users[i]->query_body() });
             }
          }
      }
	  
   } else {
      who = body->query_environment()->query_inventory();
   }

   who -= ({ nil });

   if ( mode=="emote" ) {
      what = describe_one(body) + " " + what;
   }
   what = strip(what);

   switch(what[strlen(what)-1]) {
   case '.':
   case '!':
   case '?':
   case ')':
      break;
   default:
      what += ".";
      break;
   }

   for ( i=0; i<sizeof(who); i++ ) {
      if ( typeof(who[i]) == T_STRING ) {
	 tmp = who[i];
         who[i] = find_object("Marrach:players:" + capitalize(tmp[0..0]) +
	    ":" + who[i]);
      }

      if ( who[i] == nil )
	 continue;

      if ( who[i]->query_environment() == nil )
         continue;
		   
      s = what;
	  name = describe_one( who[i] );
      while( sscanf( s, "%s+" + name + "'s%s", s1, s2 ) )
         s = s1 + "your" + s2;
	  while( sscanf( s, "%s+" + name + "%s", s1, s2 ) )
         s = s1 + "you" + s2;

	  name = lower_case(name);
      while( sscanf( s, "%s+" + name + "'s%s", s1, s2 ) )
         s = s1 + "your" + s2;
      while( sscanf( s, "%s+" + name + "%s", s1, s2 ) )
         s = s1 + "you" + s2;
	  
	  while( sscanf( s, "%s+%s", s1, s2 ) )
         s = s1 + s2;
      who[i]->act_emote( s );
   }
}


 cmd_DEV_emit ( object user, object body, string what ) {
   do_emote( user, body, what, "emit" );
}

 cmd_DEV_emote ( object user, object body, string what ) {
   do_emote( user, body, what, "emote" );
}

 cmd_DEV_emitall ( object user, object body, string what ) {
   do_emote( user, body, what, "emitall" );
}

desc_emote ( object *users, object body, string text ) {
   tell_to( users, capitalize(text), body );
}

void desc_client_control(object *users, object body, int id, string val) {
   transmit_to_client(users, body, val, id);
}

void cmd_open(object user, object body, Ob *what) {
   NRef *what_refs;

   if (what_refs = locate_best(user, FALSE, what,
			       new_object("~TextIF/data/filterfarm",
					  ({ ">open" })),
			       body,
			       body->query_environment(), body)) {
      body->action("open", ([ "what" : what_refs ]));
   }
}

void desc_opens(object *users, object who, string id) {
   tell_all_but(users, P(describe_one(NewNRef(who, id)) + " opens."));
}


void cmd_close(object user, object body, Ob *what) {
   NRef *what_refs;

   if (what_refs = locate_best(user, FALSE, what,
			       new_object("~TextIF/data/filterfarm",
					  ({ "<open" })),
			       body,
			       body->query_environment(), body)) {
      body->action("close", ([ "what": what_refs ]));
   }
}


void desc_closes(object *users, object who, string id) {
   tell_all_but(users, P(describe_one(NewNRef(who, id)) + " closes."));
}

private object *get_keys(object who) {
   object *inv;
   string code;
   int i;

   inv = who->query_inventory();
   for (i = 0; i < sizeof(inv); i ++) {
      code = inv[i]->query_property("Base:KeyCode");
      if (!code || !strlen(code)) {
	 inv[i] = nil;
      }
   }
   return inv - ({ nil });
}



void cmd_unlock(object user, object body, Ob *what, Ob *with) {
   object *keys;
   NRef *what_refs, *key_refs;
   int explicit, i;

   if (what_refs = locate_many(user, FALSE, what, body,
			       body->query_environment(), body)) {
      if (!with) {
	 keys = get_keys(body);
	 explicit = FALSE;
      } else {
	 key_refs = locate_many(user, FALSE, with, body,
			       body->query_environment(), body);
	 if (!key_refs) {
	    return;
	 }
	 keys = allocate(sizeof(key_refs));
	 for (i = 0; i < sizeof(key_refs); i ++) {
	    keys[i] = NRefOb(key_refs[i]);
	 }
	 keys -= ({ nil });
	 explicit = TRUE;
      }
      body->act_unlock(what_refs, keys, explicit);
   }
}


void lock_description(object *users, object who, mixed *success,
		      string verb) {
   mapping per_key;
   object *keys, **locks, key;
   object *others, *arr, **classes;
   string *first, *third;
   int i;

   /* we don't do second person output here yet, not for locks nor keys */

   if (!success || !sizeof(success)) {
      return;
   }

   per_key = ([ ]);
   for (i = 0; i < sizeof(success); i += 2) {
      key = success[i+1];
      if (!per_key[key]) {
	 per_key[key] = ({ });
      }
      per_key[key] += ({ success[i] });
   }
   keys = map_indices(per_key);
   locks = map_values(per_key);

   /* process the first person output */
   first = allocate(sizeof(keys));

   for (i = 0; i < sizeof(keys); i++ ) {
      if (keys[i]) {
	 first[i] = describe_many(locks[i], who, who) + " using " +
	    describe_one(keys[i], who, who);
      } else {
	 first[i] = describe_many(locks[i], who, who);
      }
   }

   if (sizeof(first) == 1) {
      tell_to(users, IP("You " + verb + " ", first[0] + "."), 
	      who);
   } else {
      tell_to(users, IP("You " + verb + " ", 
			implode(first[.. sizeof(first)-2], ", ") +
			" and " + first[sizeof(first)-1] + "."), 
	      who);
   }

   others = who->query_environment()->query_inventory() -
      ({ who }) -
      keys -
      locks;
      
   classes = classify_bodies(others);

   third = allocate(sizeof(keys));

   for (i = 0; i < sizeof(classes); i ++) {
      if (sizeof(arr = classes[i])) {
	 for (i = 0; i < sizeof(keys); i ++) {
	    if (keys[i]) {
	       third[i] = describe_many(locks[i], who, arr[0]) +
		  " using " + describe_one(keys[i], who, arr[0]);
	    } else {
	       third[i] = describe_many(locks[i], who, arr[0]);
	    }
	 }

	 if (sizeof(first) == 1) {
	    tell_to(users,
		    IP(describe_one(who, nil, arr[0]) + " " + verb + "s ", 
		       third[0] + "."),
		    arr...);
	 } else {
	    tell_to(users,
		    IP(describe_one(who, nil, arr[0]) + " " + verb + "s ",
		       implode(third[.. sizeof(third)-2], ", ") + " and " +
		       third[sizeof(third)-1] + "."),
		    arr...);
	 }
      }
   }
}   

void desc_unlock(object *users, object who, object *success) {
   lock_description(users, who, success, "unlock");
}

void desc_unlocks(object *users, object who, string id) {
   tell_all_but(users, P("There is a click from " +
			 describe_one(NewNRef(who, id)) + "."));
}

void cmd_lock(object user, object body, Ob *what, Ob *with) {
   object *keys;
   NRef *what_refs, *key_refs;
   int explicit, i;

   if (what_refs = locate_many(user, FALSE, what, body,
			       body->query_environment(), body)) {
      if (!with) {
	 keys = get_keys(body);
	 explicit = FALSE;
      } else {
	 key_refs = locate_many(user, FALSE, with, body,
			       body->query_environment(), body);
	 if (!key_refs) {
	    return;
	 }
	 keys = allocate(sizeof(key_refs));
	 for (i = 0; i < sizeof(key_refs); i ++) {
	    keys[i] = NRefOb(key_refs[i]);
	 }
	 keys -= ({ nil });
	 explicit = TRUE;
      }
      body->action("lock", ([
	 "what": what_refs,
	 "keys": keys,
	 "explicit": explicit
	 ]));
   }
}

void desc_lock(object *users, object who, object *success) {
   lock_description(users, who, success, "lock");
}

void desc_locks(object *users, object who, string id) {
   tell_all_but(users, P("There is a click from " + describe_one(NewNRef(who, id)) + "."));
}


void cmd_DEV_lock(object user, object body, Ob *what) {
   object ob;
   NRef ref;

   if (ref = locate_one(user, FALSE, what, body,
			body->query_environment(), body)) {
      ob = NRefOb(ref);
      if (!ur_is_door(ob, NRefDetail(ref))) {
	 user->message("That is not a door.\n");
	 return;
      }
      if (!ob->query_closed(NRefDetail(ref))) {
	 user->message("Close it first!\n");
	 return;
      }
      ob->set_locked(NRefDetail(ref), TRUE);
      if (ref = ur_exit_dest(ob, NRefDetail(ref))) {
	 NRefOb(ref)->set_locked(NRefDetail(ref), TRUE);
	 user->message("Both sides of the door have been locked.\n");
	 return;
      }
      user->message("This side of the door has been locked.\n");
   }
}



void cmd_DEV_unlock(object user, object body, Ob *what) {
   object ob;
   NRef ref;

   if (ref = locate_one(user, FALSE, what, body,
			body->query_environment(), body)) {
      ob = NRefOb(ref);
      if (!ur_is_door(ob, NRefDetail(ref))) {
	 user->message("That is not a door.\n");
	 return;
      }
      ob->set_locked(NRefDetail(ref), FALSE);
      if (ref = ur_exit_dest(ob, NRefDetail(ref))) {
	 NRefOb(ref)->set_locked(NRefDetail(ref), FALSE);
	 user->message("Both sides of the door have been unlocked.\n");
	 return;
      }
      user->message("This side of the door has been unlocked.\n");
   }
}

private
string categorize_light(float val) {
   if (val < 1.0) {
      return "dark";
   }
   if (val < 20.0) {
      return "dim";
   }
   return "bright";
}

void desc_lighten(object *users, object who, float old, float new_val,
		  varargs object actor) {
   string ncat, ocat;

   if (actor) {
      users -= actor->query_souls();
   }

   ocat = categorize_light(old);
   ncat = categorize_light(new_val);

   if (ocat != ncat) {
      tell_all_but(users, P("It's now " + ncat + " here, rather than " + ocat + "."), who);
   }
}

void desc_darken(object *users, object who, float old, float new_val,
		 varargs object actor) {
   desc_lighten(users, who, old, new_val);
}


void cmd_pour(object user, object body, mixed *amarr,
	      Ob what, Ob from, Ob into) {
   object from_ob, what_ob, into_ob;
   float amount;
   NRef ref;

   if (ref = locate_one(user, FALSE, ({ from }), body,
			body->query_environment(), body)) {
      from_ob = NRefOb(ref);
      DEBUG("Performing locate inside " + dump_value(from_ob));
      if (ref = locate_one(user, FALSE, ({ what }), body, from_ob)) {
	 what_ob = NRefOb(ref);
	 if (ref = locate_one(user, FALSE, ({ into }), body,
			      body->query_environment(), body)) {
	    if (!ur_combinable(what_ob) || ur_discrete(what_ob)) {
	       user->paragraph("You can't pour that.");
	       return;
	    }
	    into_ob = NRefOb(ref);
	    if (!ur_tight(into_ob)) {
	       user->paragraph("You can't do that, it would spill!\n");
	       return;
	    }
	    if (!amarr) {
	       amount = what_ob->query_amount();
	    } else if (amarr[0] == "V" && ur_by_weight(what_ob)) {
	       amount = amarr[1] * what_ob->query_actual_density() * 1E3;
	    } else if (amarr[0] == "W" && !ur_by_weight(what_ob)) {
	       amount = amarr[1] / what_ob->query_actual_density() * 1E-3;
	    } else {
	       amount = amarr[1];
	    }
	    if (what_ob->query_amount() < amount) {
	       user->paragraph("You don't have that much, you only have " + describe_one(what_ob, nil, body, STYLE_NONPOSS) + ".");
	       return;
	    }
	    body->action("pour",
			 ([ "what": ({ (amarr ?
					MutatedNRef(what_ob, amount) :
					what_ob) }),
			    "where": ({ into_ob }) ]));
	 }
      }
   }
}


void cmd_fill(object user, object body, Ob into,
	      mixed *amarr, Ob what, Ob from) {
   object from_ob, what_ob, into_ob;
   float amount;
   NRef from_ref, what_ref, into_ref;

   from_ref = locate_one(user, FALSE, ({ from }), body,
			 body->query_environment(), body);
   if (!from_ref) {
      return;
   }
   from_ob = NRefOb(from_ref);

   into_ref = locate_one(user, FALSE, ({ into }), body,
			 body->query_environment(), body);
   if (!into_ref) {
      return;
   }
   into_ob = NRefOb(into_ref);
   if (!ur_tight(into_ob)) {
      user->paragraph("You can't do that, it would spill!\n");
      return;
   }

   if (what) {
      /* user specified what to fill with, check that it's there */
      what_ref = locate_one(user, FALSE, ({ what }), body, from_ob);
      if (!what_ref) {
	 return;
      }
      what_ob = NRefOb(what_ref);
      if (!ur_combinable(what_ob) || ur_discrete(what_ob)) {
	 user->paragraph("You can't pour that.");
	 return;
      }
   } else {
      object *inv;
      int i;

      /* user said 'fill bottle from barrel', grab first likely object */
      inv = from_ob->query_inventory();

      i = 0;
      do {
	 if (i == sizeof(inv)) {
	    user->paragraph("There's nothing in " + describe_one(from_ref, body, body) + " suitable for filling " + describe_one(what_ref, body, body) + " with.");
	    return;
	 }
	 if (ur_combinable(inv[i]) && !ur_discrete(inv[i])) {
	    what_ob = inv[i];
	 }
	 i ++;
      } while (!what_ob);
   }

   if (amarr) {
      float vol;

      if (amarr[0] == "V") {
	 /* volume specified; do we need to convert? */
	 vol = amarr[1];
	 if (ur_by_weight(what_ob)) {
	    amount = vol * what_ob->query_actual_density() * 1E3;
	 } else {
	    amount = vol;
	 }
      } else {
	 /* weight specified; do we need to convert? */
	 vol = amarr[1] / what_ob->query_actual_density() * 1E-3;
	 if (!ur_by_weight(what_ob)) {
	    amount = vol;
	 } else {
	    amount = amarr[1];
	 }
      }
      DEBUG("Volume supplied: " + dump_value(vol));
      if (what_ob->query_amount() < amount) {
	 user->paragraph("There isn't that much, there's only " + describe_one(what_ob, body, body) + ".");
	 return;
      }
      if (into_ob->query_actual_capacity() < vol) {
	 user->paragraph("Alas, " + describe_one(into_ob, body, body) + " can't hold that much.");
	 return;
      }
   } else {
      float vol, maybe_vol;

      /* user did not supply an amount */

      /* how much can the destination container hold? */
      vol = into_ob->query_actual_capacity() * 0.9;	/* not to the rim */

      /* how much is there in the source container? */
      maybe_vol = what_ob->query_amount();

      DEBUG("DestHold: " + dump_value(vol));
      DEBUG("Source:   " + dump_value(maybe_vol));

      /* use whichever is smaller */
      if (maybe_vol < vol) {
	 vol = maybe_vol;
      }
      if (ur_by_weight(what_ob)) {
	 amount = vol * what_ob->query_actual_density() * 1E3;
      } else {
	 amount = vol;
      }
      DEBUG("Resulting amount: " + dump_value(amount));
   }
   body->action("fill", ([ "what": ({ MutatedNRef(what_ob, amount) }),
			   "where": ({ into_ob }) ]));
}


void cmd_take(object user, object body, Ob *what, varargs Ob *from) {
   NRef from_ref, *what_refs;
   object *obarr;
   int *error_list;
   int i, error_size;
   string prep;

   if (from) {
      from_ref = locate_first(user, FALSE, from,
			      new_object("~TextIF/data/filterfarm",
					 ({ "<carried" })),
			      body,
			      body->query_environment(), body);
      if (!from_ref) {
	 return;
      }

      if (!IsPrime(from_ref)) {
	 obarr = NRefOb(from_ref)->query_near(from_ref);
	 what_refs = search_best(user, FALSE, what,
				 new_object("~TextIF/data/filterfarm",
					    ({ })),
				 body,
				 obarr...);
      } else {
	 what_refs = locate_best(user, FALSE, what,
				 new_object("~TextIF/data/filterfarm",
					    ({
					       })),
				 body,
				 NRefOb(from_ref));
      }
   } else {
      XDebug("XX - what: " + dump_value(what));
      what_refs = locate_best(user, FALSE, what,
			      new_object("~TextIF/data/filterfarm",
					 ({ ">carried" })),
			      body,
			      body->query_environment(), body);
   }
   if (!what_refs) {
      return;
   }
   body->action("take", ([ "what": what_refs ]));
}

void cmd_wear(object user, object body, Ob *what) {
   NRef *what_refs;
   object *obarr;
   int i;

   if (what_refs = locate_best(user, FALSE, what,
			       new_object("~TextIF/data/filterfarm",
					  ({ ">worn", "<carried" })),
			       body,
			       body->query_environment(), body)) {
      obarr = allocate(sizeof(what_refs));

      for (i = 0; i < sizeof(what_refs); i ++) {
	 if (IsPrime(what_refs[i])) {
	     obarr[i] = NRefOb(what_refs[i]);
	 }
      }	
      body->action("wear", ([ "articles" : obarr - ({ nil }) ]) );
   }
}

void cmd_wield(object user, object body, varargs Ob *what) {
   NRef what_ref;
   object weapon;

   what_ref = locate_first(user, FALSE, what,
			   new_object("~TextIF/data/filterfarm",
				      ({ ">worn", "<carried" })),
			   body,
			   body->query_environment(), body);

   if (!what_ref) {
      return;
   }

   weapon = NRefOb(what_ref);

   if (body->query_property("base:combat:weapon")) {
      user->paragraph("You are already wielding a weapon.");
      return;
   }

   body->action("wield", ([ "article" : weapon ]));
}


void cmd_unwear(object user, object body, Ob *what) {
   NRef *what_refs;
   object *obarr;
   int i, what_size;

   if (what_refs = locate_best(user, FALSE, what,
			       new_object("~TextIF/data/filterfarm",
					  ({ "<worn" })),
			       body,
			       body->query_environment(), body)) {
      obarr = allocate(sizeof(what_refs));
      what_size = sizeof(what_refs);
      for(i = 0; i < what_size; i++) {
	 if (IsPrime(what_refs[i])) {
	    obarr[i] = NRefOb(what_refs[i]);
	 }
      }
      body->action("unwear", ([ "articles" : obarr - ({ nil }) ]) );
   }
}

void cmd_unwield(object user, object body, Ob *what) {
   NRef what_ref;
   object weapon;

   what_ref = locate_first(user, FALSE, what,
			   new_object("~TextIF/data/filterfarm",
				      ({ "<worn" })),
			   body,
			   body->query_environment(), body);

   if (!what_ref) {
      return;
   }

   weapon = NRefOb(what_ref);

   if (!body->query_property("base:combat:weapon")) {
      user->paragraph("You are not wielding a weapon.");
      return;
   }

   body->action("unwield", ([ "article" : weapon ]));
}


/* alias to lots of things */
void cmd_remove(object user, object body, Ob *what, Ob *from) {
   NRef *what_refs;
   object *obarr;
   int i, what_size;

   if (from) {
      cmd_take(user, body, what, from);
      return;
   }

   if (what_refs = locate_best(user, FALSE, what,
			       new_object("~TextIF/data/filterfarm",
					  ({ "<worn" })),
			       body,
			       body->query_environment(), body)) {
      obarr = allocate(sizeof(what_refs));
      what_size = sizeof(what_refs);
      for(i = 0; i < what_size; i++) {
	 if (IsPrime(what_refs[i])) {
	    obarr[i] = NRefOb(what_refs[i]);
	 }
      }
      body->action("remove", ([ "articles" : obarr - ({ nil }) ]) );
   }
}


/* examine command and desc function */

void cmd_examine(object user, object body, varargs Ob *obs) {
   NRef *obarr;

   XDebug("XX - cmd_examine()");

   if (!obs) {
      body->action("examine", nil);
      return;
   }

   obarr = locate_many(user, FALSE, obs, body,
		       body->query_environment(), body);
   if (obarr) {
      if (sizeof(obarr) == 1) {
	 body->action("examine", ([ "nref" : obarr[0] ]) );
	 return;
      }
      user->paragraph("You see " +
		      describe_many(obarr, body, body,
				    STYLE_REDUCE_CHUNK | STYLE_NOCLUSTER) +
		      ".");
   }
}

void desc_finish_examine(object *users, object who, NRef what) {
   if (what) {
      transmit_description_to_client(users, who, what, "arturl", ARTURL);
      transmit_description_to_client(users, who, what, "anyurl", ANYURL);
      show_item(users, who, what, "examine", "look");
   }
   else {
      show_item(users, who, who->query_environment(), "examine", "look");
   }
}   

/* look command and desc functions */

void cmd_look(object user, object body, varargs string prep, Ob *obs) {
   NRef *obarr;

   if (prep) {
      obarr = locate_many(user, FALSE, obs, body,
			  body->query_environment(), body);
      if (obarr) {
	 if (sizeof(obarr) == 1) {
	    body->action("look", ([ "nref" : obarr[0] ]));
	    return;
	 }
	 user->paragraph("You see " +
			 describe_many(obarr, body, body, STYLE_REDUCE_CHUNK) +
			 ".");
      }
      return;
   }
   body->action("look");
}

void desc_look(object *users, object who, NRef what, int silently) {
   if (what) {
      transmit_description_to_client(users, who, what, "arturl", ARTURL);
      transmit_description_to_client(users, who, what, "anyurl", ANYURL);
      show_item(users, who, what, "look");
   }
   else if (what = who->query_environment()) {
      show_item(users, who, what, "look");
   } else {
      tell_all_but(users, "You are in an object WITHOUT A PRIME DETAIL.");
   }
}

void illustrate_environment(object *users, object item, object env) {
   object *inv;
   string *dirs, *details;
   int out, i;

   string bigmap_url, realm_url, theatre_name;
   object theatre;

   transmit_description_to_client(users, item, env, "mapurl", MAPURL);

   if (theatre_name = item->query_property("Theatre:ID")) {
      if (theatre = find_object("Theatre:Theatres:" + theatre_name)) {
	 bigmap_url = theatre->query_bigmap_url();
	 realm_url = theatre->query_realm_url();
      }
   } 
   transmit_description_to_client(users, item, env,
				  "realmurl", REALMURL,
				  realm_url);
   transmit_description_to_client(users, item, env,
				  "bigmapurl", BIGMAPURL,
				  bigmap_url);

   catch {
# if 0
      details = ur_details(env);
# else
      details = ur_match_exit_details(env, nil, TRUE);
# endif

      for (i = 0; i < sizeof(details); i ++) {
	 if (ur_exit_dest(env, details[i])) {
	    int j;

	    dirs = ur_exit_dirs(env, details[i]);

	    if (dirs) {
	       for (j = 0; j < sizeof(dirs); j++) {
		  switch (dirs[j]) {
		  case "north":		out |= 1;	break;
		  case "northeast":	out |= 2;	break;
		  case "east":		out |= 4;	break;
		  case "southeast":	out |= 8;	break;
		  case "south":		out |= 16;	break;
		  case "southwest":	out |= 32;	break;
		  case "west":		out |= 64;	break;
		  case "northwest":	out |= 128;	break;
		  }
	       }
	    }
	 }
      }
   }

   transmit_to_client(users, item, (string) out, COMPASS);

   /* also let the item's possessor know who we see now */

   transmit_to_client(nil, item, "foo", PLAYERS_CLEAR);
   transmit_to_client(nil, item, "foo", OBJECTS_CLEAR);

   inv = env->query_inventory();

   for (i = 0; i < sizeof(inv); i ++) {
      int p, o;

      if (ur_volition(inv[i])) {
	 p ++;
	 if (p < PLAYER_INV_CAP) {
	    transmit_to_client(users, item, object_number(inv[i]) + " " +
			       capitalize(describe_one(inv[i], nil, nil,
						       STYLE_NONPOSS)),
			       PLAYERS_ENTER);
	 } else if (p == PLAYER_INV_CAP) {
	    transmit_to_client(users, item, "1 (truncated)",
			       PLAYERS_ENTER);
	 }
      } else {
	 o ++;
	 if (o < OBJECT_INV_CAP) {
	    transmit_to_client(users, item, object_number(inv[i]) + " " +
			       capitalize(describe_one(inv[i], nil, nil,
						       STYLE_NONPOSS)),
			       OBJECTS_ENTER);
	 } else if (o == OBJECT_INV_CAP) {
	    transmit_to_client(users, item, "1 (truncated)",
			       OBJECTS_ENTER);
	 }
      }
   }

   /* finally let the item's possessor know what exits there are */

   transmit_to_client(nil, item, "foo", EXITS_CLEAR);

   /* get obvious exits visible to looker 'item' */
# if 0
   details = env->match_exit_details(env->figure_side(item), TRUE);
# else
   details = ur_match_exit_details(env, env->figure_side(item), TRUE);
# endif


   for (i = 0; i < sizeof(details); i ++) {
      dirs = ur_exit_dirs(env, details[i]);
      if (!dirs || !sizeof(dirs)) {
	 dirs = ({ "NONE" });
      }
      transmit_to_client(users, item, dirs[0] + " " +
			 describe_one(NewNRef(env, details[i])),
			 EXITS_ADD);
   }
}


void desc_object_moved(object *users, object item, object from, object to) {
   illustrate_environment(users, item, to);
}

void desc_illustrate(object *users, object item) {
   if (item->query_environment()) {
      illustrate_environment(users, item, item->query_environment());
   }
}

void desc_object_left(object *users, object item) {
   users -= item->query_souls();
   transmit_to_client(users, nil, (string) object_number(item),
		      ur_volition(item) ? PLAYERS_LEAVE : OBJECTS_LEAVE);
}

void desc_object_entered(object *users, object item) {
   users -= item->query_souls();
   transmit_to_client(users, nil, object_number(item) + " " +
		      capitalize(describe_one(item, nil, nil,
					      STYLE_NONPOSS)),
		      ur_volition(item) ? PLAYERS_ENTER : OBJECTS_ENTER);
}

void desc_inv_left(object *users, object env, object item) {
   transmit_to_client(users, env, (string) object_number(item),
		      INVENTORY_LEAVE);
}

void desc_inv_entered(object *users, object env, object item) {
   transmit_to_client(users, env, (string) object_number(item) + " " +
		      capitalize(describe_one(item, nil, nil, STYLE_NONPOSS)),
		      INVENTORY_ENTER);
}


void desc_glance(object *users, object who) {
   show_item(users, who, who->query_environment(), "glance");
}


void cmd_inventory(object user, object body) {
   body->action("inv", nil);
}

void desc_inv(object *users, object who) {
   mapping clothes_worn;
   object *inv, *list;
   string verb, clothes, weapons;
   mixed desc;
   int i;

   verb = ur_container_verb_second(who);

   if (!verb || !strlen(verb)) {
      verb = "contain";
   }
   inv = who->query_inventory() - who->list_clothes();
   if (!sizeof(inv)) {
      desc = "You " + verb + " nothing.";
   } else {
      inv -= who->list_clothes();
      desc = "";
      if (sizeof(inv)) {
	 desc += "You " + verb + " " + describe_many(inv, who, who) + ". ";
      }
   }

   clothes_worn = who->query_clothes_worn();
   clothes = describe_clothes(clothes_worn, who, STYLE_REL_LOC, TRUE);
   weapons = describe_clothes(clothes_worn, who, STYLE_FIRST_PERSON, FALSE);

   XDebug("XX - cmd clothes: " + dump_value(clothes));
   XDebug("XX - cmd weapons: " + dump_value(weapons));

   if (weapons)
      desc += "You are wielding " + weapons + ". ";

   if (clothes) {
      desc += "You are wearing " + clothes + ".\n\n";

      list = who->list_totally_covered_clothes();

      if (sizeof(list)) {
	 desc += capitalize(describe_many(list, who, who, 
					  STYLE_NONPOSS)) + 
	    " " + determine_prep(list) +
	    " totally covered by other clothes. ";
      }

      list = who->list_partially_covered_clothes();

      if (sizeof(list)) {
	 desc += capitalize(describe_many(list, who, who, 
					  STYLE_NONPOSS)) +
	    " " + determine_prep(list) + 
	    " partially covered by other clothes. ";
      }

   }
   else {
      if (ur_clothes_expected(who)) {
	 desc += "You are nude. ";
      }
   }

   tell_to(users, desc, who);

}


private
void show_item(object *users, object who, NRef what, string types...) {
   tell_to(users, describe_item(what, who, types...),  who);
}

void cmd_DEV_make(object user, object body, Ob *what) {
   NRef what_ref;

   if (what_ref = search_one(user, FALSE, what, body,
			     CRAFTD->query_objects()...)) {
      body->act_make(NRefOb(what_ref), "make", "makes", "");
   }
}


void desc_possess(object *users, object possessor, object possessee) {
   int i;

   for (i = 0; i < sizeof(users); i ++) {
      users[i]->possess_body(possessee);
   }
}

void desc_unpossess(object *users, object possessee, object possessor) {
   int i;

   /* only unpossess people actually possessing */
   users &= possessee->query_possessor_souls();

   for (i = 0; i < sizeof(users); i ++) {
      users[i]->possess_body(possessor);
   }
}
