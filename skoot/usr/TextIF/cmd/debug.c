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

# define CHUNK	20


private int emergency_break;

string apply_emergency_break() {
   emergency_break = TRUE;
   return "OK";
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

