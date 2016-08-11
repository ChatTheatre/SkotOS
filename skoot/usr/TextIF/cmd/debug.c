/*
**	/usr/TextIF/cmds/debug.c
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";
private inherit "/lib/url";
private inherit "/lib/array";

private inherit "/base/lib/toolbox";
private inherit "/base/lib/urcalls";

inherit "/usr/SID/lib/stateimpex";
inherit "/usr/SkotOS/lib/auth";
inherit "/usr/SkotOS/lib/basecontext";

inherit authapi	"/usr/UserAPI/lib/authapi";
inherit ctlapi	"/usr/UserAPI/lib/ctlapi";

inherit "/usr/TextIF/lib/debug";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";
inherit "/usr/SkotOS/lib/bilbo";

# include <nref.h>
# include <TextIF.h>
# include <UserAPI.h>
# include <SkotOS.h>
# include <base.h>
# include <base/act.h>

# include "/usr/SAM/include/sam.h"
# include "/usr/SkotOS/include/describe.h"
# include "/usr/SkotOS/include/noun.h"
# include "/usr/SkotOS/include/inglish.h"

static
void create() {
   authapi::create();
   ctlapi::create();
}

static
mixed run_bilbo ( object obj, mixed other, string verb, string mode, string detail, varargs mapping params );

/* stat command prototypes */
static void stat_name( object user, object ob );
static void stat_detail( object user, object ob );
static void stat_environment( object user, object ob );
static void stat_bulk( object user, object ob );
static void stat_living( object user, object ob );
static void stat_motion( object user, object ob );
static void stat_stance( object user, object ob );
static void stat_thing( object user, object ob );
static void stat_clothing( object user, object ob );
static void stat_property( object user, object ob, varargs string pattern );
static void stat_brain( object user, object ob );

static int check_storyguide_body(object user, object body);

/* tmp command to heal a player to 100.0 durability */

void cmd_DEV_heal(object user, object body) {
   body->set_property("base:stat:durability", 100.0);
   user->message("Your durability has been reset to 100.\n");
}

/*
 * Prototype of funtion which is actually in ~/lib/cmds/misc.c
 */
void generic_finger(object user, object body, object udat, int restricted);

/*
 * Same idea as +finger, just find the account directly instead of
 * through the character name.
 */
void
cmd_DEV_accfinger(object user, object body, varargs string *bits)
{
    string uname;
    object udat;

    uname = implode(bits, " ");
    udat = UDATD->query_udat(uname);
    if (!udat) {
	user->paragraph("Account not found.\n");
	return;
    }
    start_staff("accfinger_callback", user->query_name(), user, body, udat);
}

static void
accfinger_callback(int success, string result, object user, object body,
		   object udat)
{
   "~/cmd/misc"->generic_finger(user, body, udat,
				(success && result == "1") ? 2 : 1);
}

# define CHUNK	20


private int emergency_break;

string apply_emergency_break() {
   emergency_break = TRUE;
   return "OK";
}

void cmd_DEV_refresh(object user, object body, string woe) {
   object ob, firstborn;
   mixed *list;
   int count;

   if (!woe) {
      user->paragraph("Syntax: +refresh [woe name]");
      return;
   }
   if (lower_case(woe) == "abort") {
      user->message("Aborting a running +refresh: " +
		    apply_emergency_break() + "\n");
      return;
   }
   ob = find_object(woe);
   if (!ob) {
      user->paragraph("I cannot find an object of that name.");
      return;
   }
   firstborn = ob->query_first_child();
   if (!firstborn) {
      user->paragraph("This object has no ur-children.");
      return;
   }
   user->paragraph("OK, building refresh list for ur-children...");
   call_out("construct_refresh_list", 0, user, firstborn, firstborn, ({ }));
}


static
void construct_refresh_list(object user, object firstborn,
			    object child, mixed *list, varargs int count) {
   int i, stamp;

   stamp = time();

   for (i = 0; i < CHUNK; i ++) {
      list = ({ child, list });
      count ++;

      if ((count % 100) == 0) {
	 user->paragraph("REFRESH: List size is now " + count + "; still building...");
      }

      if (child->query_next_ur_sibling() == nil) {
	 SysLog("NIL link from ur-child " + dump_value(child));
	 error("nil in ur-child link");
      }
      child = child->query_next_ur_sibling();

      if (child == firstborn) {
	 user->paragraph("REFRESH: Starting BILBO calls; list size is " + count);
	 call_out("call_bilbo_scripts", 1, user, list, count);
	 return;
      }
      if (time() != stamp) {
	 /* urk, sizeable fraction of second, skip out */
	 user->paragraph("REFRESH: Brief hiccup due to sluggish scripts...");
	 call_out("construct_refresh_list", 4, user, firstborn, child, list, count);
	 return;
      }
   }

   if (emergency_break) {
      emergency_break = FALSE;
      user->paragraph("REFRESH: EMERGENCY BREAK APPLIED. Stopping.");
      return;
   }
   call_out("construct_refresh_list", 1, user, firstborn, child, list, count);
}

static
void call_bilbo_scripts(object user, mixed *list, int count) {
   object ob;
   int i, stamp;

   stamp = time();

   for (i = 0; i < CHUNK/2; i ++) {
      if (!sizeof(list)) {
	 user->paragraph("REFRESH: Done!\n");
	 return;
      }
      if ((count % 100) == 0) {
	 user->paragraph("REFRESH: Calling BILBO scripts; " + count + " left to do...");
      }
      count --;
      ob = list[0];
      if (ob) {
	 mixed refresh_old;

	 refresh_old = ob->query_property("bilbo:solo:refresh");
	 if (refresh_old == "run") {
	    user->paragraph("Warning: " + name(ob) + "'s own refresh script temporarily disabled.");
	    ob->set_property("bilbo:solo:refresh", nil);
	 }
	 ob->set_property("bilbo:inherit:solo:refresh",
			  ob->query_ur_object());
	 catch {
	    run_bilbo(ob, ob->query_ur_object(), "refresh", "solo", nil);
	 }
	 ob->clear_property("bilbo:inherit:solo:refresh");
	 if (refresh_old != nil) {
	    ob->set_property("bilbo:solo:refresh", refresh_old);
	 }
      } else {
	 user->paragraph("Beware: destructed ur-sibling encountered.");
      }
      list = list[1];
      if (time() != stamp) {
	 /* urk, sizeable fraction of second, skip out */
	 user->paragraph("REFRESH: Brief hiccup due to sluggish scripts...");
	 call_out("call_bilbo_scripts", 4, user, list, count);
	 return;
      }
   }
   if (emergency_break) {
      emergency_break = FALSE;
      user->paragraph("REFRESH: EMERGENCY BREAK APPLIED. Stopping.");
      return;
   }
   call_out("call_bilbo_scripts", 1, user, list, count);
}


/* destructs a wonderful object of your choice */

void cmd_DEV_slay(object user, object body, string flag, Ob **what) {
   NRef *what_refs;
   object ob;
   int slain, i, override;

   override = !!flag;

   if (what_refs = locate_many(user, FALSE, what, body,
			       body->query_environment(), body)) {
      for (i = 0; i < sizeof(what_refs); i++) {
	 if (IsPrime(what_refs[i])) {
	    ob = NRefOb(what_refs[i]);

	    if (!override && body->contained_by(ob)) {
	       user->message("If you're sure you want to destruct your environment (and thus be destructed yourself), use +slay force.\n");
	       return;
	    }

	    if (override || safe_to_slay(ob)) {
	       catch {
		  ob->die();
		  slain++;
		  what_refs[i] = nil;
	       }
	    }
	 }
      }
      what_refs -= ({ nil });

      user->message("You slay " + slain);
      if (slain > 1)
	 user->message(" objects.\n");
      else
	 user->message(" object.\n");
      if (sizeof(what_refs) > 0) {
	 user->message("You fail to slay " + describe_many(what_refs, body, body) + ". You may have better luck with +slay override [objects]\n");
      }
   }
}

/* sets a property on an object */

void cmd_DEV_setprop(object user, object body, string str) {
   mixed val;
   string bstr, pstr, vstr, ostr;
   object ob;
   NRef ref;

   if (!str || sscanf(str, "%s %s %s", bstr, pstr, vstr) != 3) {
      user->message("Usage: setprop <body> <prop> <value>\n");
      return;
   }
   ref = locate_one(user, LF_HUSH_ALL, ({ NewOb(nil, 0, 0, ({ bstr })) }),
		    body, body, body->query_environment());
   if (ref) {
      if (!IsPrime(ref)) {
	 user->message("Details don't have properties.\n");
	 return;
      }
      ob = NRefOb(ref);
   } else {
      ob = find_object(bstr);
      if (!ob) {
	 user->message("Could not find " + bstr + ".\n");
	 return;
      }
      if (!function_object("set_property", ob)) {
	 user->message("You cannot use +setprop on that object.\n");
	 return;
      }
   }
   if (vstr == "nil") {
       val = nil;
   } else if (sscanf(vstr, "OBJ(%s)", ostr)) {
       val = find_object(ostr);
   } else {
      val = ascii_to_mixed(vstr);
   }
   ob->set_property(pstr, val);
   user->paragraph("Property '" + pstr + "' on " + name(ob) + " set to: " + mixed_to_ascii(val, TRUE));
}

/* sets a property on an object */

void cmd_DEV_clearprop(object user, object body, string str) {
   mixed val;
   string bstr, pstr;
   object ob;
   NRef ref;

   if (!str || sscanf(str, "%s %s", bstr, pstr) != 2) {
      user->message("Usage: clearprop <body> <prop> <value>\n");
      return;
   }
   ref = locate_one(user, LF_HUSH_ALL, ({ NewOb(nil, 0, 0, ({ bstr })) }),
		    body, body, body->query_environment());
   if (ref) {
      if (!IsPrime(ref)) {
	 user->message("Details don't have properties.\n");
	 return;
      }
      ob = NRefOb(ref);
   } else {
      ob = find_object(bstr);
      if (!ob) {
	 user->message("Could not find " + bstr + ".\n");
	 return;
      }
      if (sscanf(ur_name(ob), "/base/obj/thing#%*d") < 1) {
	 user->message("You cannot use +clearprop on that object.\n");
	 return;
      }
   }
   ob->clear_property(pstr);
   user->message("Done.\n");
}

/* stat an object by its base object name */

void cmd_DEV_obstat(object user, object body, string str) {
   object ob, urparent;
   object temp_ob;
   NRef nref_ob;
   mixed *exploded_what;
   string stat_type;
   string what;

   if (!str || sscanf(str,"%s %s", what, stat_type) < 2) {
      user->message("Bad arguments!!\n");
      user->message("Usage: +obstat <object name> <stat type>\n");
      user->message("Types: name, details, environment, bulk, living, motion\n");
      user->message("       stance, thing, clothing, property, brain, all\n");
      return;
   }

   if (what) {
      ob = find_object( what );

      if (!ob) {
	 user->message("+obstat: Could not find '" + what + "', check spelling and/or capitalization?\n");
      } else {
	 string pattern;

	 if (stat_type == "name" || stat_type == "all")
	    stat_name( user, ob );		
	 if (stat_type == "details" || stat_type == "all")
	    stat_detail( user, ob );		
	 if (stat_type == "environment" || stat_type == "all")
	    stat_environment( user, ob );
	 if (stat_type == "bulk" || stat_type == "all")
	    stat_bulk( user, ob );
	 if (stat_type == "living" || stat_type == "all")
	    stat_living( user, ob );
	 if (stat_type == "motion" || stat_type == "all")
	    stat_motion( user, ob );
	 if (stat_type == "stance" || stat_type == "all")
	    stat_stance( user, ob );
	 if (stat_type == "thing" || stat_type == "all")
	    stat_thing( user, ob );
	 if (stat_type == "clothing" || stat_type == "all")
	    stat_clothing( user, ob );
	 if (stat_type == "property" || stat_type == "all")
	    stat_property( user, ob ); 
	 if (stat_type == "brain" || stat_type == "all")
	    stat_brain( user, ob );
	 if (sscanf(stat_type, "property;%s", pattern) ||
	     sscanf(stat_type, "property:%s", pattern)) {
	    stat_property(user, ob, pattern);
	 }
      }
   }
}

/* stat an object by its normal in game name 'sword' */

void cmd_DEV_stat(object user, object body, Ob *what, mixed args) {
   object ob, urparent;
   object temp_ob;
   NRef nref_ob;
   string stat_type;

   if (what && args["evoke"]) {
      
      if (nref_ob = locate_one(user, FALSE, what, body, 
			       body, body->query_environment())) {
	 ob = NRefOb(nref_ob);	
      }	

      if (ob) {
	 string pattern;

	 sscanf(args["evoke"], "\"%s\"", stat_type);
	 XDebug("stat() type: " + stat_type);

	 if (stat_type == "name" || stat_type == "all")
	    stat_name(user, ob);
	 if (stat_type == "details" || stat_type == "all")
	    stat_detail( user, ob );		
	 if (stat_type == "environment" || stat_type == "all")
	    stat_environment( user, ob );
	 if (stat_type == "bulk" || stat_type == "all")
	    stat_bulk( user, ob );
	 if (stat_type == "living" || stat_type == "all")
	    stat_living( user, ob );
	 if (stat_type == "motion" || stat_type == "all")
	    stat_motion( user, ob );
	 if (stat_type == "stance" || stat_type == "all")
	    stat_stance( user, ob );
	 if (stat_type == "thing" || stat_type == "all")
	    stat_thing( user, ob );
	 if (stat_type == "clothing" || stat_type == "all")
	    stat_clothing( user, ob );
	 if (stat_type == "property" || stat_type == "all")
	    stat_property( user, ob ); 
	 if (stat_type == "brain" || stat_type == "all")
	    stat_brain( user, ob );
	 if (sscanf(stat_type, "property;%s", pattern) ||
	     sscanf(stat_type, "property:%s", pattern)) {
	    stat_property(user, ob, pattern);
	 }
	 return;
      }
   }

   user->message("usage: stat <object name> \"stat type\"\n");
   user->message("types: details, environment, bulk, living, motion\n");
   user->message("       stance, thing, clothing, property, brain, all\n");
   return;
}

static
void stat_name(object user, object ob) {
   string desc, pdesc;
   
   if (ob->query_object_name()) {
      desc = ob->query_object_name() + " [" + ur_name(ob) + "]";
   } else {
      desc = "[" + ur_name(ob) + "]";
   }
   if (ob = ob->query_ur_object()) {
      if (ob->query_object_name()) {
	 pdesc = ob->query_object_name() + " [" + ur_name(ob) + "]";
      } else {
	 pdesc = "[" + ur_name(ob) + "]";
      }
   } else {
      pdesc = "None.";
   }
   user->message("Object: " + desc + "\nParent: " + pdesc + "\n");
}

/* display clothing information */
static
void stat_clothing(object user, object ob) {
   mapping clothes_worn;

   user->message("-- Clothes --\n");
   clothes_worn = ob->query_clothes_worn();
   user->message("Clothes: " + 
		 describe_clothes(clothes_worn, nil, STYLE_NONPOSS,TRUE));
   user->message("Weapons: " +
		 describe_clothes(clothes_worn, nil, STYLE_NONPOSS,FALSE));
   user->message("\n");
}

/* display clothing information */
static
void stat_property(object user, object ob, varargs string pattern) {
   mapping map;
   string *props, *bits;
   mixed *vals;
   int i, match;

   map = ob->query_properties(TRUE);

   props = map_indices(map);
   vals  = map_values(map);

   if (pattern) {
      string *bits, orig;

      orig = pattern = lower_case(pattern);
      user->message("-- Properties (" + pattern + ")--\n");
      pattern = implode(explode("%" + pattern + "%", "%"), "%%");
      bits = explode("*" + pattern + "*", "*");
      match = sizeof(bits) - 1;
      if (match == 0) {
	 if (map[orig]) {
	    user->message("Property: " + orig + " = " +
			  dump_value(map[orig]) + "\n");
	 } else {
	    user->message("No such property found.\n");
	 }
	 return;
      }
      pattern = implode(bits, "%*s");
   } else {
      user->message("-- Properties --\n");
   }
   
   for (i = 0; i < sizeof(props); i ++) {
      if (pattern && sscanf(props[i], pattern) != match) {
	 continue;	
      }
      user->message("Property: " + props[i] + " = " +
		    dump_value(vals[i]) + "\n");
   }
}

static
void stat_brain( object user, object ob ) {
   mapping map;
   string *props;
   mixed *vals;
   int i;

   map = ob->query_braindata();

   props = map_indices(map);
   vals  = map_values(map);

   user->message("-- Brain --\n");

   for (i = 0; i < sizeof(props); i ++) {
      user->message("Brain trait: " + props[i] + " = " +
      dump_value(vals[i]) + "\n");
   }
}


/* display thing information */
static
void stat_thing( object user, object ob ) {
   object *soul_list;

   user->message( "-- Thing --\n" );

   user->message( "Souls: " );
   soul_list = ob->query_souls();
   if ( !sizeof(soul_list) )
      user->message( "none." );
   else
      user->message( dump_value(soul_list) );
   user->message( "\n" );

   user->message( "\n" );
}

/* display stance information */
static
void stat_stance( object user, object ob ) {
   user->message( "-- Stance --\n" );
   user->message( "Stance: " + ob->query_stance() + " | " + ob->describe_stance() +  "\n" );
   user->message( "Stance Prep: " );
   if (ob->query_preposition()) {
      user->message(ob->describe_preposition());
   } else {
      user->message( "none." );
   }
   user->message( "\n" );
   user->message( "\n" );
}

/* display motion information */
static
void stat_motion( object user, object ob ) {
	object env_ob;
	object holder_ob;	
	NRef prox_ref;
	object prox_ob;

	user->message( "-- Motion --\n" );

	user->message( "Environment: " );
	if ( (env_ob = ob->query_environment()) == nil )
		user->message( "none." );
	else
		user->message( describe_one(env_ob) );
	user->message( "\n" );

	user->message( "Holder: " );
	if ( (holder_ob = ob->query_holder()) == nil )
		user->message( "none." );
	else
		user->message( describe_one(holder_ob) );
	user->message( "\n" );	

	user->message( "Proximity: " );
	if ( (prox_ref = ob->query_proximity()) == nil )
		user->message( "none." );
	else {
		if ( (prox_ob = NRefOb(prox_ref)) == nil )
			user->message( "none." );
		else
			user->message( describe_one(prox_ob) );
	}
	user->message( "\n" );

	user->message( "\n" );
}

/* display living information */
static
void stat_living( object user, object ob ) {

	user->message( "-- Living --\n" );
	user->message( "Volition: " + ur_volition(ob) + "\n" );
	user->message( "Sex: " + ob->query_gender_string() + "\n" );
	user->message( "\n" );
}

/* display bulk information */
static
void stat_bulk( object user, object ob ) {

	user->message( "-- Bulk --\n");
	user->message( "Intrinsic Mass: " + ur_intrinsic_mass(ob) + " kg\n" );
	user->message( "Actual Mass: " + ob->query_actual_mass() + " kg\n" );
	user->message( "Intrinsic Density: " + ur_intrinsic_density(ob) + "\n" );
	user->message( "Actual Density: " + ob->query_actual_density() + "\n" );
	user->message( "Intrinsic Capacity: " + ur_intrinsic_capacity(ob) + " m^3\n" );
	user->message( "Actual Capacity: " + ob->query_actual_capacity() + " m^3\n" );
	user->message( "Minimum Volume: " + ob->query_minimum_volume() + " m^3\n" );
	user->message( "Actual Volume: " + ob->query_actual_volume() + " m^3\n" );
	user->message( "\n" );
}

/* display environment information */
static
void stat_environment( object user, object ob ) {
	string many;
	string one;
	int loop;

	user->message( "-- Environment --\n" );

	/* step into the inventory and display its guts */
	user->message( "Inventory: " );
	many = describe_many( ob->query_inventory() );
	if ( !strlen(many) )
		user->message( "none." );
	else
		user->message( many );
	user->message( "\n" );

	user->message( "Max Weight: " + ur_max_weight(ob) + " kg\n" );
	user->message( "\n" );
}

/* display detail information */
static
void stat_detail( object user, object ob ) {
	string *details;
	string detail;
	string *desc_types;
	string desc_type;
	int desc_types_size;
	int details_size;
	int loop, inner_loop;

	details = ur_details(ob);
	details_size = sizeof( details ); 
	user->message( "-- Details --\n " );

	/* step in each detail and display its guts */ 
	for( loop = 0; loop < details_size; loop++ ) {
		detail = details[loop];      
		user->message("Detail ID:: " + detail + "\n" );
            
		desc_types = ur_description_types(ob, detail );
		desc_types_size = sizeof( desc_types );
		/* display the guts of this detail */
		for( inner_loop = 0; inner_loop < desc_types_size; inner_loop++ ) {
			desc_type = desc_types[inner_loop];
			user->message( "* " + desc_type + ": "  );
			user->message( unsam(ur_description(ob, detail,desc_type)) );
			user->message( "\n" );
		}
	}
}


void cmd_DEV_spawn(object user, object body, string what) {
   object ob, clone;
   string clone_string;

   if (what) {
      int    silently;

      if (sscanf(what, "silently %s", what)) {
	 silently = TRUE;
      } else {
	 silently = FALSE;
      }
      if (ob = find_object(what)) {
	 string desc;

	 clone = spawn_thing(ob);

	 if (ur_combinable(clone) && !ur_discrete(clone) &&
	     find_object("Generic:clothing:pouch")) {
	    object container;
	    float amount, vol, weight, max;
	    int ret;

	    container = spawn_thing(find_object("Generic:clothing:pouch"));

	    vol = 0.90 * container->query_actual_capacity();
	    SysLog("vol: " + dump_value(vol));
	    weight = vol * clone->query_actual_density() * 1E3;
	    SysLog("weight: " + dump_value(weight));
	    max = ur_max_weight(container);
	    SysLog("max: " + dump_value(max));
	    if (max < weight) {
	       vol *= (0.99 * max / weight);
	    }
	    weight = vol * clone->query_actual_density() * 1E3;

	    if (ur_by_weight(clone)) {
	       amount = weight;
	    } else {
	       amount = vol;
	    }
	    clone->set_amount(amount);

	    ret = clone->move(container);
	    if (ret) {
	       error("failure: " + ret);
	    }

	    desc = "A pouch full of " + describe_one(clone, body, body, STYLE_NONPOSS);
	    clone = container;
	 } else {
	    desc = "A spawn of " + describe_one(clone, body, body, STYLE_NONPOSS);
	 }

	 clone->action("teleport", ([ "dest": body ]),
		       silently ? ACTION_SILENT : 0);

	 if (clone->query_environment() == body) {
	    user->message(desc + " has been created and teleported into your inventory.\n");
	    return;	
	 }
	 clone->action("teleport", ([ "dest" : body->query_environment() ]),
		       silently ? ACTION_SILENT : 0);
	 if (clone->query_environment() == body->query_environment()) {
	    user->message(desc + " has been created and teleported into your environment, because the move into your inventory failed.\n");
	    return;
	 }
	 user->message(desc + " has been duplicated, under the name " + name(clone) + " but left in limbo because it could neither move into your inventory nor your environment.\n");
	 return;
      }
      user->message("There is no such object.\n");
      return;
   }
   user->message("Usage example: +spawn /base/obj/thing#802\n");
}


void cmd_DEV_goto(object user, object body, string where) {
   string detail;
   object ob;
   NRef target;

   if (where) {
      sscanf(where, "%s|%s", where, detail);
      ob = find_object(where);
      if (ob) {
	 if (detail) {
	    target = NewNRef(ob, detail);
	 } else {
	    target = ob->prime_reference(body);
	 }
	 body->action("teleport",
		      ([ "dest" : ob ]) );
	 return;
      }
      user->message("There is no such object.\n");
      return;
   }
   user->message("Usage example: goto /base/obj/room#355\n");
}


void cmd_DEV_join ( object user, object body, string what ) {
    object ob;

    if (!what) {
	user->message("Usage: +join objectname\n");
	return;
    }

    /* Backward compatibility CM crap */
    ob = find_object(what);
    if (!ob) {
	ob = find_object("Marrach:players:" + capitalize(what[0..0]) +  ":" +
			 lower_case(what));
    }
    if (!ob) {
	ob = find_object("Marrach:NPCs:"+proper(what));
    }
    if (!ob) {
	ob = determine_user(what, TRUE);
	if (ob) {
	    ob = ob->query_body();
	}
    }

    if (ob) {
	if (ob->query_environment()) {
	    body->act_teleport(ob->query_environment());
	} else {
	    user->message("Uhh, no. That object is in the NIL.\n");
	}
    } else {
	user->message("That is not a valid object.\n");
    }
}


void cmd_GUIDE_join (object user, object body, string what ) {
    object ob;

    if (check_storyguide_body(user, body)) {
	return;
    }
    if (!what) {
	user->message("Usage: !join objectname\n");
	return;
    }

    ob = determine_user(what, TRUE);
    if (ob) {
	ob = ob->query_body();
    }
    if (ob) {
	if (ob->query_environment()) {
	    body->act_teleport(ob->query_environment());
	} else {
	    user->message("That player is not logged on.\n");
	}
	return;
    }
    user->message("There is no such player.\n");
}

 
void cmd_DEV_whereis ( object user, object body, string what ) {
   object o, *children;
   int i;
   string s;

   if ( !what ) {
      user->message("Usage: +whereis objectname\n");
	  return;
   }

   o = find_object(what);
   if ( !o )
      o = find_object( "Marrach:players:" + capitalize(what[0..0]) + 
		       ":" + lower_case(what) );
   if ( !o )
      o = find_object( "Marrach:NPCs:"+proper(what) );
   if ( !o ) {
      o = determine_user(what, TRUE);
      if (o) {
	 o = o->query_body();
      }
   }
   if ( !o ) {
      o = UDATD->query_body(what);
   }
   
   if ( o  ) {
      user->clear_more();
		   
      if ( o->query_environment() )
         user->add_more( name(o->query_environment())+"\n");
	  else
         user->add_more( "In the NIL\n" );

      children = o->query_ur_children();
	  if ( sizeof(children) ) {
         user->add_more( "" );      
         user->add_more( sizeof(children)+" ur-children:\n" );
	  }

	  for( i=0; i<sizeof(children); i++ ) {
	     s = name(children[i]) + " is at ";
		 if ( children[i]->query_environment() ) {
            user->add_more( s + name( children[i]->query_environment() )+"\n" );
		 } else {
            user->add_more( s + "the NIL\n");
		 }
	  }
	  user->show_more();

   } else {
      user->message("That is not a valid object name.\n");
   }
}

 
void cmd_GUIDE_whereis ( object user, object body, string what ) {
   object o, *children;
   int i;
   string s;

   if (!what) {
      user->paragraph("Usage: !whereis player");
      return;
   }

   if (o = UDATD->query_body(what)) {
      if (o->query_environment()) {
	 user->paragraph(describe_one(o) + " is in " +
			 describe_one(o->query_environment()) + ".");
	 return;
      }
      user->paragraph(describe_one(o) + " is not logged in.");
      return;
   }
   user->paragraph("There is no such player.");
}


void cmd_DEV_summon(object user, object body, string what) {
   object ob, error, *obs;
   string *subs;
   int i;

   if (what) {
      int silently;

      if (sscanf(what, "silently %s", what)) {
	 silently = TRUE;
      } else {
	 silently = FALSE;
      }
      if (ob = find_object(what)) {
	 obs = ({ ob });
      } else if (subs = IDD->query_objects(what)) {
	 obs = allocate(sizeof(subs));
	 for (i = 0; i < sizeof(obs); i ++) {
	    obs[i] = find_object(what + ":" + subs[i]);
	 }
      }
      obs -= ({ nil });
      if (!sizeof(obs)) {
	 user->message("There is no such object.\n");
	 return;
      }
      if (sizeof(obs) > 1) {
	 string msg;

	 msg = "You found more than one, choose from these objects:\n";
	 for (i = 0; i < sizeof(obs); i++) {
	    msg += "- " + name(obs[i]) + "\n";
	 }
	 user->message(msg);
	 return;
      }
      obs[0]->action("teleport", ([ "dest" : body->query_environment() ]),
		     silently ? ACTION_SILENT : 0);
      if (obs[0]->query_environment() == body->query_environment()) {
	 if (silently) {
	    user->message("Object successfully summoned silently.\n");
	 }
      } else {
	 user->message("Object was not successfully summoned!\n");
      }
      return;
   }
   user->message("Usage example: summon /base/obj/thing#471\n" +
		 "           or: summon ur:objects:clothing\n" +
		 "           or: summon silently ur:objects:clothing\n");
}


void cmd_DEV_unsummon(object user, object body, string what) {
   object ob;

   if (what) {
      ob = find_object(what);
      if (ob) {
	 ob->action("teleport", nil);
	 user->message("Done!\n");
	 return;
      }
      user->message("There is no such object.\n");
      return;
   }
   user->message("Usage example: summon /base/obj/thing#471\n");
}

/*
 * !grab/+grab and !ungrab/+ungrab commands.
 */
atomic void
cmd_DEV_grab(object user, object body, varargs string *who, int override)
{
    string  verb;
    object  ob, env;
    mapping map;

    if (is_root(user->query_name())) {
	verb = "+grab";
    } else {
	verb = "!grab";
    }
    if (!who) {
        user->message("Usage: " + verb + " <playername>\n");
        return;
    }

    ob = determine_user(implode(who, " "), FALSE);
    if (!ob) {
        user->message("Did not find anyone by that name.\n");
        return;
    }
    ob = ob->query_body();
    env = ob->query_environment();
    if (!env) {
        user->message(capitalize(describe_one(ob)) + " has no environment.\n");
	return;
    }
    map = ob->query_property("skotos:grabbed");
    if (!map) {
        map = ([ ]);
    }
    if (map[body] && !override) {
        user->message("Warning: You grabbed this player before, remembering " +
		      "original location!\n");
    } else {
      map[body] = env;
    }
    ob->set_property("skotos:grabbed", map);
    map = body->query_property("skotos:grab");
    if (!map) {
        map = ([ ]);
    }
    map[ob] = env;
    body->set_property("skotos:grab", map);

    ob->action("teleport", ([ "dest": body->query_environment() ]));
}

atomic void
cmd_DEV_ungrab(object user, object body, varargs string *who)
{
    string  verb;
    object  ob;
    mapping map;
    if (is_root(user->query_name())) {
	verb = "+ungrab";
    } else {
	verb = "!ungrab";
    }
    if (!who) {
        /*
	 * Check who we grabbed, if there's only one option, see if we can
	 * ungrab that one player.
	 */
        map = body->query_property("skotos:grab");
	if (map && map_sizeof(map) == 1) {
	    ob = map_indices(map)[0];
	} else {
	    user->message("Usage: " + verb + " <playername>\n");
	    if (map && map_indices(map)) {
	        user->message("Characters you've grabbed: " +
			      describe_many(map_indices(map)) + "\n");
	    }
	    return;
	}
    }
    if (!ob) {
	ob = determine_user(implode(who, " "), FALSE);
	if (!ob) {
	    user->message("Did not find anyone by that name.\n");
	    return;
	}
	ob = ob->query_body();
    }
    if (!ob->query_environment()) {
        user->message(capitalize(describe_one(ob)) + " has no environment.\n");
        return;
    }
    map = ob->query_property("skotos:grabbed");
    if (!map || !map[body]) {
        user->message("You did not grab this player, cannot return " +
		      describe_one(ob) + ".\n");
	return;
    }
    ob->action("teleport", ([ "dest": map[body] ]));

    /* Update the map of people that the target was grabbed by. */
    map[body] = nil;
    if (!map_sizeof(map)) {
        map = nil;
    }
    ob->set_property("skotos:grabbed", map);

    /* Update the map of people we grabbed. */
    map = body->query_property("skotos:grab");
    map[ob] = nil;
    if (!map_sizeof(map)) {
        map = nil;
    }
    body->set_property("skotos:grab", map);
}

void
cmd_GUIDE_grab(object user, object body, varargs string *who, int override)
{
    if (check_storyguide_body(user, body)) {
	return;
    }
    cmd_DEV_grab(user, body, who, override);
}
void
cmd_GUIDE_ungrab(object user, object body, varargs string *who)
{
    /* Just for completeness, it won't really have any impact. */
    if (check_storyguide_body(user, body)) {
	return;
    }
    cmd_DEV_ungrab(user, body, who);
}

private string
inventory_object(object ob)
{
    string name, ur_name;

    name    = name(ob);
    ur_name = ur_name(ob);

    if (name == ur_name) {
        if (sscanf(ur_name, "/base/obj/thing#%s", ur_name)) {
	    return "#" + ur_name;
	}
	return ur_name;
    }
    if (sscanf(ur_name, "/base/obj/thing#%s", ur_name)) {
        ur_name = "#" + ur_name;
    }
    return name + " (" + ur_name + ")";
}

/* Round float to 3 decimals max. */

private string
inventory_float(float f)
{
   int i, r;

   return (string) f;

   /* this doesn't handle large numbers well */

    i = (int) (f * 1000.0);
    r = i % 1000;
    i = i / 1000;
    switch (r) {
    case   0..  9: return i + ".00" + r;
    case  10.. 99: return i + ".0" + r;
    case 100..999: return i + "." + r;
    }
}
void
cmd_DEV_inventory(object user, object body, varargs string arg) {
    int    i, sz;
    string what, result;
    object ob, *worn, *inv, env;

    if (arg == "help") {
	user->message("S = Single wear\n" +
		      "T = Transparency\n" +
		      "W = Worn\n" +
		      "E = Edible\n" +
		      "P = Potable\n" +
		      "M = Mass in kg\n" +
		      "V = Volume in liters\n");
	return;
    }
    if (!arg || !strlen(arg)) {
        ob = body;
    } else {
	mixed data;

	if (sscanf(arg, "$%s", arg)) {
	    if (arg[0] == '{') {
		if (arg[strlen(arg) - 1] != '}') {
		    user->message("+inventory: Missing '}' at end of parameter.\n");
		    return;
		}
		arg = arg[1..strlen(arg) - 2];
	    }
	} else {
	    user->message("+inventory: Badly formatted parameter.\n");
	    return;
	}
	data = locate_objects(user, body, arg);
	switch (typeof(data)) {
	case T_OBJECT:
	    ob = data;
	    break;
	case T_ARRAY:
	    ob = data[0];
	    break;
	case T_NIL:
	    user->message("+inventory: Could not find \"" + arg +
			  "\".\n");
	    return;
	default:
	    user->message("+inventory: Unexpected results for \"" + arg +
			  "\".\n");
	    return;
	}
    }
    result = "";

    /* Object itself: */
    result += "Object:       \"" + capitalize(describe_one(ob, body, body)) + "\" " + inventory_object(ob) + "\n";
    if (!ob->query_neuter()) {
        result += "Gender:       " + capitalize(ob->query_gender_string()) + "\n";
    }

    /* Environment: */
    env = ob->query_environment();
    if (env) {
	result += "Environment:  \"" +
	          capitalize(describe_one(env, body, body)) + "\" " +
	          inventory_object(env) + "\n";
    }

    /* Remaining inventory bits: */
    inv = ob->query_inventory();
    sz = sizeof(inv);
    if (sz) {
        mixed **arr;

        result += "Inventory:\n";
	arr = ({
	    ({ "#" })           + allocate(sz),
	    ({ "S" })           + allocate(sz),
	    ({ "T" })           + allocate(sz),
	    ({ "W" })           + allocate(sz),
	    ({ "E" })           + allocate(sz),
	    ({ "P" })           + allocate(sz),
	    ({ "M" })           + allocate(sz),
	    ({ "V" })           + allocate(sz),
	    ({ "Description" }) + allocate(sz),
	    ({ "Name" })        + allocate(sz) });
        for (i = 0; i < sz; i++) {
	    object ob;

	    ob = inv[i];
	    arr[0][i + 1] = i + 1;

	    arr[1][i + 1] = ur_single_wear(ob)  ? "x" : "";
	    arr[2][i + 1] = ur_transparency(ob) ? "x" : "";
	    arr[3][i + 1] = ob->query_worn_by() ? "x" : "";
	    arr[4][i + 1] = ur_edible(ob)  ? "x" : "";
	    arr[5][i + 1] = ur_potable(ob) ? "x" : "";
	    arr[6][i + 1] = inventory_float(ob->query_actual_mass());
	    arr[7][i + 1] = inventory_float(ob->query_actual_volume() * 1000.0);
	    arr[8][i + 1] = capitalize(describe_one(ob, body, body));
	    arr[9][i + 1] = inventory_object(ob);
	}
	/*
	 * Check for empty columns:
	 */
	if (sizeof(arr[1][1..] - ({ "" })) == 0) {
	    arr[1] = nil;
	}
	if (sizeof(arr[2][1..] - ({ "" })) == 0) {
	    arr[2] = nil;
	}
	if (sizeof(arr[3][1..] - ({ "" })) == 0) {
	    arr[3] = nil;
	}
	if (sizeof(arr[4][1..] - ({ "" })) == 0) {
	    arr[4] = nil;
	}
	if (sizeof(arr[5][1..] - ({ "" })) == 0) {
	    arr[5] = nil;
	}
	arr -= ({ nil });
	result += tabulate(arr...);
    }
    user->html_message("<PRE>" + replace_html(result) + "</PRE>\n");
}

static atomic void
start_statistics(string callback, string user, string type, mixed args...);
void
cmd_DEV_statistics(object user, object body, varargs string type)
{
    switch (type) {
    case "subscriptions":
    case "referrals":
    case "games":
    case "games-all":
    case "games-paid":
    case "players":
    case "players-all":
    case "players-paid":
    case "tec-conversion":
    case "tec-users":
    case "distribution":
    case "affiliates":
        break;
    default:
	user->message("Usage:\n" +
		      "  +statistics subscriptions | referrals\n" +
		      "  +statistics affiliates | distribution\n" +
		      "  +statistics games-all | games-paid\n" +
		      "  +statistics players-all | players-paid\n" +
		      "  +statistics tec-conversion | tec-users\n");
	return;
    }
    start_statistics("delayed_statistics", user->query_name(), type, user, body);
}

static void
delayed_statistics(int success, string reply, object user, object body)
{
    if (success) {
        user->html_message("<PRE>" + replace_html(url_decode(reply)) + "</PRE>\n");
    } else {
        user->message("Failed: " + reply + "\n");
    }
}
void
cmd_DEV_log(object user, object body)
{
    user->message("Usage:\n" +
		  "+log assign <entry>... \"assignee\"\n" +
		  "+log assignees\n" +
		  "+log grep <words>\n" +
		  "+log grep <words> \"matching text\"\n" +
		  "+log grep \"matching text\"\n" +
		  "+log show <entry>\n");
}

private string loggrep_prepare(string str);
void
cmd_DEV_loggrep(object user, object body, varargs string *words, mapping evoke)
{
    string name, match;
    object master;

    if ((!words || !sizeof(words)) &&
	(!evoke || !evoke["evoke"] || strlen(evoke["evoke"]) <= 2)) {
	user->message("Usage:\n" +
		      "+log grep <words>\n" +
		      "+log grep <words> \"matching text\"\n" +
		      "+log grep \"matching text\"\n");
	return;
    }
    if (words) {
	name = lower_case(implode(words, "."));
    }
    if (evoke && (match = evoke["evoke"])) {
	match = match[1..strlen(match) - 2];
    }
    user->message("Starting log-search for" +
		  (name ? " assignees matching \"" + name + "\"" : "") +
		  (match ? " headers/descriptions matching \"" + match + "\"" : "") +
		  "...\n");
    if (match) {
	match = loggrep_prepare(match);
    }
    master = find_object("/usr/SkotOS/obj/lognode");
    call_out("delayed_loggrep", 1, user, body, name,
	     (match ? explode(match, " ") - ({ "" }) : nil),
	     time(), 0, 0, master, master->query_next_clone(), ([ ]));
}

private string
loggrep_prepare(string str)
{
    int i, sz;

    sz = strlen(str);
    for (i = 0; i < sz; i++) {
	switch (str[i]) {
	case 'A'..'Z':
	    str[i] += 'a' - 'A';
	    break;
	case 'a'..'z':
	case '0'..'9':
	    break;
	default:
	    str[i] = ' ';
	    break;
	}
    }
    return " " + str + " ";
}

private int
loggrep_match(string *words, string text)
{
    int i, sz;

    if (!words || !sizeof(words)) {
	return TRUE;
    }
    if (!text) {
       return FALSE;
    }
    text = loggrep_prepare(text);
    sz = sizeof(words);
    for (i = 0; i < sz; i++) {
	if (!sscanf(text, "%*s " + words[i] + " ")) {
	    return FALSE;
	}
    }
    return TRUE;
}

static void
delayed_loggrep(object user, object body, string name, string *match,
		int start_ts, int total, int found, object master,
		object current, mapping result)
{
    int count;

    if (!user || !body) {
	return;
    }
    count = 0;
    while (count < 16 && current && current != master) {
	int    *indices, a_match;
	string assignee, tag;

	assignee = current->query_assignee();
	tag      = current->query_tag();
	indices  = current->get_ixset();

	a_match = FALSE;
	if (name) {
	    string str;

	    str = assignee ? lower_case(assignee) : "";
	    if (str == name || sscanf(str, "%*s" + name)) {
		a_match = TRUE;
	    }
	} else {
	    a_match = TRUE;
	}
	if (a_match) {
	    int i, sz;

	    sz = sizeof(indices);
	    for (i = 0; i < sz; i++) {
		int stamp, ix;
		string header, description;

		ix = indices[i];
		stamp = current->query_stamp(ix);
		header = current->query_header(ix);
		description = current->query_description(ix);
		if (!loggrep_match(match, header) && !loggrep_match(match, description)) {
		    continue;
		}
		result[ix] = ([
		    "assignee": assignee,
		    "tag": tag,
		    "timestamp": stamp,
		    "header": header ]);
		found++;
	    }
	}
	count++;
	total += sizeof(indices);
	current = current->query_next_clone();
    }
    if (current && current != master) {
	call_out("delayed_loggrep", 1, user, body, name, match,
		 start_ts, total, found, master, current, result);
    } else {
	int sz;

	sz = map_sizeof(result);
	if (sz > 0) {
	    int i, *indices;
	    mapping *results;

	    indices = map_indices(result);
	    results = map_values(result);

	    user->clear_more();
	    user->add_more("Done (" + total + " checked, " + sz + " found; search took " +
			   (time() - start_ts) + " seconds):\n");
	    for (i = 0; i < sz; i++) {
		user->add_more("[" + pad_left((string)indices[i], 5) + "] " +
			       (results[i]["timestamp"] ? ctime(results[i]["timestamp"])[4..15] : "Unknown     ") +
			       ": " +
			       results[i]["assignee"] + " / " +
			       results[i]["header"] + "\n");
	    }
	    user->add_more("*** End of log search reached.\n");
	    user->show_more();
	} else {
	    user->message("Done (" + total + " checked, 0 found, " + (time() - start_ts) + " seconds).\n");
	}
    }
}
void
cmd_DEV_logshow(object user, object body, string number)
{
    int    ix;
    object master, current;

    ix = (int)number;
    master = find_object("/usr/SkotOS/obj/lognode");
    current = master->query_next_clone();
    call_out("delayed_logshow", 1, user, body, ix, master, current);
}

static void
delayed_logshow(object user, object body, int ix, object master,
		object current)
{
    int count;

    if (!user || !body) {
	return;
    }
    count = 0;
    while (count < 32 && current && current != master) {
	if (current->query_header(ix)) {
	    int    stamp;
	    string header, description;

	    stamp       = current->query_stamp(ix);
	    header      = current->query_header(ix);
	    description = current->query_description(ix);
	    user->html_message(
		"Found entry  " + ix + " in " + name(current) + ":\n" +
		"<B>Tag</B>:         " + replace_html(current->query_tag()) + "\n" +
		"<B>Assignee</B>:    " + replace_html(current->query_assignee()) + "\n" +
		"<B>Logged at</B>:   " + replace_html(stamp ? ctime(stamp) : "Unknown") + "\n" +
		"<B>Header</B>:      " + replace_html(header) + "\n" +
		"<B>Description</B>: " + replace_html(strip(description)) + "\n");
	    /* No sense in searching any further, "there can be only one." */
	    return;
	}
	count++;
	current = current->query_next_clone();
    }
    if (current && current != master) {
	call_out("delayed_logshow", 1, user, body, ix, master, current);
	return;
    }
    user->message("Unable to find entry " + ix + ".\n");
}
void
cmd_DEV_logassign(object user, object body, string *numbers, mapping evoke)
{
    int     i, sz, *entries, *unneeded;
    string  assigned;
    object  master, current;
    mapping entry_map;

    sz = sizeof(numbers);
    entry_map = ([ ]);
    for (i = 0; i < sz; i++) {
	entry_map[(int)numbers[i]] = TRUE;
    }
    entries = map_indices(entry_map);

    if (!evoke || !evoke["evoke"] || strlen(evoke["evoke"]) <= 2) {
	user->message("Usage: +log assign <entries> \"assignee\"\n");
	return;
    }

    assigned = evoke["evoke"];
    assigned = assigned[1..strlen(assigned) - 2];

    master = find_object("/usr/SkotOS/obj/lognode");
    current = master->query_next_clone();

    call_out("delayed_logassign", 1, user, body, entries, assigned,
	     master, current, ({ }), ({ }));
}

static void
delayed_logassign(object user, object body, int *entries, string assigned,
		  object master, object current, int *unneeded, string *result)
{
    int count;

    if (!user || !body) {
	return;
    }
    count = 0;
    while (count < 32 && current && current != master && sizeof(entries)) {
	int *indices, sz;

	indices = current->get_ixset() & entries;
	sz = sizeof(indices);
	if (sz) {
	    string assignee;

	    assignee = current->query_assignee();
	    if (assignee == assigned) {
		/* Uhm, it's already set at that. */
		unneeded |= indices;
	    } else {
		int    i;
		string tag;

		tag  = current->query_tag();

		for (i = 0; i < sz; i++) {
		    int    stamp;
		    string header, description;

		    stamp       = current->query_stamp(indices[i]);
		    header      = current->query_header(indices[i]);
		    description = current->query_description(indices[i]);
		    result += ({
			"Reassigned " + indices[i] + " to " +
			assigned +" (original: " + assignee + ")\n" +
			"Logged at:   " + (stamp ? ctime(stamp) : "unknown") + "\n",
			"Tag:         " + tag + "\n",
			"Header:      " + header + "\n",
			"Description: " + strip(description) + "\n" });
		    current->set_entry(tag, assigned, indices[i], stamp, header, description);
		}
	    }
	    entries -= indices;
	}
	current = current->query_next_clone();
	count++;
    }
    if (current && current != master && sizeof(entries)) {
	call_out("delayed_logassign", 1, user, body, entries, assigned,
		 master, current, unneeded, result);
	return;
    }

    user->clear_more();

    if (sizeof(unneeded)) {
	int i, sz;
	string *numbers;

	sz = sizeof(unneeded);
	numbers = allocate(sz);
	for (i = 0; i < sz; i++) {
	    numbers[i] = (string)unneeded[i];
	}
	if (sz == 1) {
	    user->add_more("This entry is already assigned to " + assigned + ": " +
			   numbers[0] + ".\n");
	} else {
	    user->add_more("These entries are already assigned to " + assigned + ": " +
			   implode(numbers[..sz - 2], ", ") + " and " + numbers[sz - 1] + ".\n");
	}
    }
    if (sizeof(result)) {
	int i, sz;

	sz = sizeof(result);
	for (i = 0; i < sz; i++) {
	    user->add_more(result[i]);
	}
    }
    if (sizeof(entries)) {
	int i, sz;
	string *numbers;

	sz = sizeof(entries);
	numbers = allocate(sz);
	for (i = 0; i < sz; i++) {
	    numbers[i] = (string)entries[i];
	}
	if (sz == 1) {
	    user->add_more("Cannot find this entry: " + numbers[0] + ".\n");
	} else {
	    user->add_more("Cannot find these entries: " +
			   implode(numbers[..sz - 2], ", ") + " and " + numbers[sz - 1] + ".\n");
	}
    }
    user->show_more();
}
void
cmd_DEV_logassignees(object user, object body)
{
    int     i, sz;
    string  *list, **lists;
    mapping assignees;

    list = LOGD->query_assignees();
    sz = sizeof(list);
    assignees = ([ ]);
    for (i = 0; i < sz; i++) {
	string entry;

	entry = list[i];
	if (sscanf(entry, "unassigned-%*d")) {
	    continue;
	}
	if (sscanf(entry, "pre-beta unassigned-%*d")) {
	    continue;
	}
	entry = loggrep_prepare(list[i]);
	entry = implode(explode(entry, " ") - ({ }), "");
	if (assignees[entry]) {
	    assignees[entry] |= ({ list[i] });
	} else {
	    assignees[entry] = ({ list[i] });
	}
    }
    sz = map_sizeof(assignees);
    lists = map_values(assignees);
    for (i = 0; i < sz; i++) {
	user->message("* \"" + implode(lists[i], "\", \"") + "\"\n");
    }
}
void
cmd_DEV_ipnames(object user, object body)
{
    int     i, sz, *ip_ranges;
    string  *names, result;
    object  *users;
    mapping ipnames;
    mapping *data;

    users = INFOD->query_current_user_objects();
    sz = sizeof(users);
    ipnames = ([ ]);

    for (i = 0; i < sz; i++) {
	int     d1, d2, d3, ip_sort;
	string  ipname, ipnr, range;
	mapping map, nap;

	if (!users[i]->query_body()) {
	    continue;
	}
	ipnr   = query_ip_number(users[i]->query_conn());
	ipname = query_ip_name(users[i]->query_conn());

	sscanf(ipnr, "%d.%d.%d.%*d", d1, d2, d3);
	switch (d1) {
	case   1..127:
	    ip_sort = (d1 << 16);
	    break;
	case 128..171:
	    ip_sort = (d1 << 16) | (d2 << 8);
	    break;
	case 172..254:
	    ip_sort = (d1 << 16) | (d2 << 8) | d3;
	    break;
	}

	map = ipnames[ip_sort];
	if (!map) {
	    map = ipnames[ip_sort] = ([ ]);
	}
	nap = ipnames[ipname];
	if (!nap) {
	    nap = map[ipname] = ([ ]);
	}
	nap[users[i]->query_name()] = TRUE;
    }

    result =
	"IP range:      Names:\n" +
	"--------       -----\n";

    sz        = map_sizeof(ipnames);
    ip_ranges = map_indices(ipnames);
    data      = map_values(ipnames);

    for (i = 0; i < sz; i++) {
	int     j, sz_j;
	string  ip_range, *ip_names, *str;
	mapping *name_maps;

	sz_j = map_sizeof(data[i]);
	ip_names = map_indices(data[i]);
	name_maps = map_values(data[i]);

	switch (ip_ranges[i] >> 16) {
	case 1..127:
	    ip_range = (ip_ranges[i] >> 16) + ".*.*.*";
	    break;
	case 128..171:
	    ip_range =
		(ip_ranges[i] >> 16) + "." + ((ip_ranges[i] >> 8) & 0xff) +
		".*.*";
	    break;
	case 172..254:
	    ip_range =
		(ip_ranges[i] >> 16) + "." + ((ip_ranges[i] >> 8) & 0xff) +
		"." + (ip_ranges[i] & 0xff) + ".*";
	    break;
	}
	str = allocate(sz_j);
	for (j = 0; j < sz_j; j++) {
	    str[j] = implode(map_indices(name_maps[j]), " ") + " (" + ip_names[j] + ")";
	}
	result +=
	    pad_left(ip_range, 13) + "  " + implode(str, " ") + "\n";
    }
    user->html_message("<PRE>\n" + result + "</PRE>\n");
}
void
cmd_DEV_status(object user, object body, varargs string what)
{
   switch (what) {
   case "heartbeat":
      user->html_message("<PRE>\n" +
			 "/usr/SkotOS/sys/status"->dump_heartbeat_avg() +
			 "</PRE>\n");
      break;
   case "swapping":
      user->html_message("<PRE>\n" +
			 "/usr/SkotOS/sys/status"->dump_swapavg1() +
			 "\n" +
			 "/usr/SkotOS/sys/status"->dump_swapavg5() +
			 "</PRE>\n");
      break;
   default:
      user->message("Usage: +status heartbeat|swapping\n");
      break;
   }
}


void cmd_GUIDE_accfinger(object user, object body, mixed args...) {
    cmd_DEV_accfinger(user, body, args...);
}
