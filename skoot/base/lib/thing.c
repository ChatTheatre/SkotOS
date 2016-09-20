/*
**	/base/lib/thing.c
**
**	THE physical object.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>
# include <base/thing.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/mapping";

private inherit "/base/lib/toolbox";

inherit "/base/lib/urcalls";

inherit notes "/lib/notes";
inherit ur    "/lib/ur";
inherit "/lib/random";

inherit properties      "./properties";

inherit events		"./events";

inherit motion		"./motion";
inherit detail		"./detail";
inherit stance		"./stance";
inherit bulk		"./bulk";
inherit light		"./light";
inherit ownership	"./ownership";
inherit clothing	"./clothing";
inherit consent		"./consent";

inherit environment	"./environment";
inherit prox		"./prox";

inherit crafting	"./crafting";

inherit memories        "./memories";
inherit living		"./living";
inherit skills		"./skills";

inherit scheduler	"./scheduler";

inherit study_teach     "./study_teach";

inherit act_basic	"./act/basic";
inherit act_social	"./act/social";
inherit act_movement	"./act/movement";
inherit act_clothing	"./act/clothing";
inherit act_consent	"./act/consent";

inherit act_study_teach "./act/study_teach";

inherit action		"./act/actions";
inherit "/lib/womble";

# include "/base/include/events.h"


/* transient state */
private object disconnect_room;	/* where did we linkdie? */
private int last_logout;		/* time() of last logout */
private object *souls;		/* who sees through our eyes? */
private int weapon;		/* can I be used as a weapon? */
private int current_handler;	/* what action I am performing */
private int debugmode;		/* am I in debugmode? */
private string game;		/* what game do I belong to? */

/* static state */

private mixed die_message_first; /* what I say when I die */
private mixed die_message_third; 

private mapping initial_prop_set;	/* property -> TRUE/FALSE */
private mapping initial_properties;	/* property -> actual value */

private mapping contents;

private object possessor;		/* what other body possesses us? */
private object *possessor_souls;	/* which souls possess us? */
private object possessee;		/* what other body do we possess? */

private string integration;		/* room prop saying we're integral */

void run_auto_stop( void );
void die();

nomask int is_thing() { return TRUE; }
nomask int is_nref() { return TRUE; }

nomask string query_state_root() { return "Base:Thing"; }

mapping braindata;

/* experimental combinables support, version 1.0 */

private int combinable;

void womble_die_messages() {
  die_message_first = womble(die_message_first);
  die_message_third = womble(die_message_third);
}

void womble_initials() {
  initial_properties = womble(initial_properties);
  braindata = womble(braindata);
}

static atomic
void create() {
   properties::create();
   notes::create();
   ur::create();

   events::create();
   environment::create();
   prox::create();
   motion::create();
   detail::create();
   bulk::create();
   ownership::create();
   clothing::create();
   consent::create();
   crafting::create();
   living::create();
   memories::create();
   study_teach::create();

   add_event("destructed");

   souls = ({ });
   braindata = ([ ]);

   initial_properties = ([ ]);
   initial_prop_set = ([ ]);

   contents = ([ ]);

   catch {
      this_object()->create_thing();
      call_out("run_auto_start", 0);
   }
}

void dirty_caches();

atomic
void touch() {
   /* put in patch functionality here later */

   /* put in general pendant functionality here later */

/*
   SysLog("[" + name(this_object()) + "]: Dirtying caches through touch()");
*/
   dirty_caches();
}

nomask
void die() {
   object env, holder, holder_env;
   object offered_to;

   holder = query_holder();
   env = query_environment();

   if (holder)
      holder_env = holder->query_environment();

   /* send out the die message */
   if (ur_die_message_first(this_object()) ||
       ur_die_message_third(this_object())) {
      if (holder && holder_env)
	 holder_env->broadcast_event("die", this_object(), holder_env);
      else if (!holder && env)
	 env->broadcast_event("die", this_object(), env);
   }

   destruct_object();
}

nomask
void suicide() {
   call_out("die", 0);
}

nomask 
void painful_death(mixed pause) {
   call_out("destruct_object", pause);
}

static
void destructive_desire() {
   ur::destructive_desire();

   catch {
      escape_offer();
   }
   catch {
      run_auto_stop();
   }
}


static nomask atomic
void destructing() {
   object *arr;
   int i;

   XDebug("XX - destructing() object: " + dump_value(this_object()));

   if (previous_program() != SYS_AUTO) {
      error("illegal call");
   }

   catch {
      this_object()->destruct_thing();
   }

   arr = map_values(prefixed_map(query_properties(), "virtualhome:"));
   for (i = 0; i < sizeof(arr); i ++) {
      object ur_parent;
      mixed owner;

      if (!arr[i]) {
	 continue;
      }
      if (arr[i]->query_property("virtual:noslay")) {
	 continue;
      }
      /* Check if the "homeowner" property matches */
      owner = arr[i]->query_property("HomeOwner");
      if (typeof(owner) != T_OBJECT) {
	 continue;
      }
      if (owner != this_object()) {
	 continue;
      }

      /* Check if the room's ur-parent has an "isvirtual" property. */
      ur_parent = arr[i]->query_ur_object();
      if (!ur_parent) {
	 continue;
      }
      if (!ur_parent->query_property("IsVirtual")) {
	 continue;
      }

      /* Destroy virtual rooms in separate threads */
      arr[i]->suicide();
   }

   ur::destructing();
   motion::destructing();
   environment::destructing();
   prox::destructing();
   clothing::destructing();

   /* inform everybody who sees through our eyes */

   arr = souls[..]; /* copy */
   for (i = 0; i < sizeof(arr); i ++) {
      if (arr[i]) {
	 catch {
	    arr[i]->body_destructing();
	 }
      }
   }
   event("destructed", this_object());
}

void dirty_caches() {
   catch {
      dirty_light_cache();
   }
   catch {
      dirty_bulk_cache();
   }
   catch {
      if (query_environment()) {
	 if (ur_cached_motion(this_object())) {
	    query_environment()->enter_cache(this_object());
	 } else {
	    query_environment()->leave_cache(this_object());
	 }
      }
   }   
}

float query_amount() {
   if (ur_combinable(this_object())) {
      if (ur_discrete(this_object())) {
	 /* for e.g. pieces of wood, use the bulk mass multipler */
	 return query_intrinsic_mass();
      }
      if (ur_by_weight(this_object())) {
	 return query_actual_mass();
      }
      return query_actual_volume();
   }
   return 1.0;
}

void set_amount(float num) {
   if (ur_combinable(this_object())) {
      if (ur_discrete(this_object())) {
	 /* set intrinsic mass as a multiplier */
	 if (num <= 0.0) {
	    set_mass(0.0);
	    suicide(); return;
	 }
	 set_mass(num);
	 return;
      }
      if (num <= 0.0) {
	 set_mass(0.0);
	 suicide(); return;
      }
      if (ur_by_weight(this_object())) {
	 /* just bloody well make the total mass what it should be */
	 set_mass(query_intrinsic_mass() * num / query_actual_mass());
	 return;
      }
      /* or the volume! */
      set_mass(query_intrinsic_mass() * num / query_actual_volume());
      return;
   }
   if (num <= 0.0) {
      set_mass(0.0);
      suicide(); return;
   }
   error("not a combinable");
}

string query_combinable_noun() {
   return "heap";
}

string query_combinable_plural_noun() {
   return "heaps";
}

atomic
object chip_combinable(float num) {
   if (ur_combinable(this_object())) {
      object ur;
      if (ur = query_ur_object()) {
	 object new_obj;
	 float amount;

	 amount = query_amount();
	 if (num > amount) {
	    num = amount;
	 }
	 new_obj = spawn_thing(ur);
	 new_obj->set_amount(num);
	 set_amount(amount - num);
	 if (query_environment()) {
	    int res;

	    res = new_obj->move(query_environment());
	    if (res < 0) {
	       error("chip failed: res = " + res);
	    }
	 }
	 return new_obj;
      }
      error("no ur parent");
   }
   error("not combinable");
}

void increase_amount(float num) {
   set_amount(query_amount() + num);
}

void decrease_amount(float num) {
   set_amount(query_amount() - num);
}


# define TestFlag(mask, bit) \
  !!(mask & bit)
# define SetFlag(mask, bit, val) \
  { if (val) (mask) |= (bit); else (mask) &= ~(bit); }

int query_combinable() {
   return TestFlag(combinable, COMBINABLE_ENABLED);
}

void set_combinable(int val) {
   SetFlag(combinable, COMBINABLE_ENABLED, val);
}

int query_discrete() {
   return TestFlag(combinable, COMBINABLE_DISCRETE);
}

void set_discrete(int val) {
   SetFlag(combinable, COMBINABLE_DISCRETE, val);
}

int query_by_weight() {
   return TestFlag(combinable, COMBINABLE_BY_WEIGHT);
}

void set_by_weight(int val) {
   SetFlag(combinable, COMBINABLE_BY_WEIGHT, val);
}

/* this is basically a callback from motion.c, in its own thread */

static atomic
void combinable_check() {
   object *obj, env, ur;
   NRef prox;
   int i;

   /* look for objects that are in our environment, share our prox,
      and are combinables with the same ur objects */

   if (ur = query_ur_object()) {
      /* the UR OBJECT has to be combinable, not just us */
      if (!ur_combinable(ur)) {
	 return;
      }
      if (env = query_environment()) {
	 prox = query_proximity();
	 if (prox == nil || NRefOb(prox) == env) {
	    obj = env->query_near(prox) - ({ this_object() });

	    for (i = 0; i < sizeof(obj); i ++) {
	       if (ur != obj[i]->query_ur_object()) {
		  obj[i] = nil;
	       }
	    }
	    obj -= ({ nil });

	    execute_action("combinables/merge",
			   ([ "roles": ({ "mergee" }),
			      "mergee": obj ]));
	 }
      }
   }
}

private object last_known_environment;

void initialize_last_known_environment() {
   last_known_environment = query_environment();
}

static atomic
void post_move_effects() {
   combinable_check();

   if (query_environment() != last_known_environment) {
      this_object()->occur("object_moved", last_known_environment,
			   query_environment());
      if (last_known_environment) {
	 last_known_environment->broadcast_locally("object_left",
						   this_object());
	 last_known_environment->occur("inv_left", this_object());
      }
      if (query_environment()) {
	 query_environment()->broadcast_locally("object_entered",
						this_object());
	 query_environment()->occur("inv_entered", this_object());
      }
      last_known_environment = query_environment();
   }

# if 0
   if (ur_combinable(this_object()) && !ur_discrete(this_object()) &&
       query_environment() && !ur_tight(query_environment())) {
      broadcast("spill");
      die();
   }
# endif
}

/* the nref interface implementation */

object query_object() {
   return this_object();
}

string query_detail(varargs object looker) {
   return ur_prime_id(this_object(), figure_side(looker));
}

int is_prime() { return TRUE; }

object chip() { return this_object(); }

int equals(NRef b) {
   return NRefOb(b) == this_object() &&
      ur_prime(this_object(), NRefDetail(b)) &&
      query_amount() == NRefAmount(b);
}

/* other stuff */
mixed query_die_message_first() {
   return die_message_first;
}

void set_die_message_first(mixed msg) {
   die_message_first = msg;
}

mixed query_die_message_third() {
   return die_message_third;
}

void set_die_message_third(mixed msg) {
   die_message_third = msg;
}

string *query_initial_properties() {
   return map_indices(initial_prop_set);
}

mapping query_all_initial_properties() {
   return initial_properties[..];
}

mixed query_initial_property(string property) {
   return initial_properties[property];
}

void set_integration_property(string str) {
   integration = str;
}

string query_integration_property() {
   return integration;
}

string query_ascii_initial_property(string property) {
   mixed val;

   val = query_initial_property(property);
   if (val != nil) {
      return mixed_to_ascii(val, TRUE);
   }
   return nil;
}

atomic
void set_initial_property(string property, mixed value) {
   if (property) {
      property = strip(lower_case(property));
      if (typeof(value) == T_STRING) {
	 value = strip(value);
      }
      initial_prop_set[property] = TRUE;
      initial_properties[property] = value;
      set_property(property, value);
   }
}

void set_ascii_initial_property(string property, string value) {
   string err;

   err = catch(set_initial_property(property, ascii_to_mixed(value)));
   if (err) {
      error("Error setting initial property \"" + property + "\": " + err);
   }
}


atomic
void clear_initial_property(string property) {
   if (property) {
      property = strip(lower_case(property));
      initial_prop_set[property] = nil;
      initial_properties[property] = nil;
      clear_property(property);
   }
}

atomic
void clear_initial_properties() {
   initial_prop_set = ([ ]);
   initial_properties = ([ ]);
}

int write_initial_properties(object into) {
   object ur;
   string *props;
   int i, tot;

   if (ur = query_ur_object()) {
      tot += ur->write_initial_properties(into);
   }

   props = map_indices(initial_prop_set);
   for (i = 0; i < sizeof(props); i ++) {
      into->set_property(props[i], initial_properties[props[i]]);
   }
   return tot + sizeof(props);
}

int copy_initial_properties() {
   return write_initial_properties(this_object());
}

static
void ur_object_set(object new_obj, object old) {
   /* we have a new ur object! */
   dirty_caches();
   /* copy initial properties */
   copy_initial_properties();
}

int query_debugmode() {
   return debugmode;
}

void set_debugmode(int mode) {
   debugmode = mode;
}

object *query_souls() {
    souls -= ({ nil });
    return souls[..]; /* copy */
}

object *query_host_souls() {
   int i;
   object *hosts;

   hosts = ({ });
   souls -= ({ nil });

   for (i = 0; i < sizeof(souls); i++) {
      if (DEV_USERD->query_wiztool(souls[i]->query_name())) {
	 hosts += ({ souls[i] });
      }
   }

   return hosts;
}

atomic
void disconnect() {
   object *soularr;
   int i;

   soularr = souls[..];
   for (i = 0; i < sizeof(soularr); i ++) {
      if (soularr[i]) {
	 catch {
	    soularr[i]->disconnect();
	 }
      }
   }
}

void possess(object user) {
   souls |= ({ user });
}

void possess_nocheck( object user ) {
   souls |= ({ user });  /* used for brain */
}

void depossess(object user) {
   souls -= ({ user });
}

/* disconnect */

object query_disconnect_room() {
   object virthome;

   /* this must at the very least query CONFIGD -- TODO -- Zell */
   if (sizeof(query_host_souls())) {
      return disconnect_room;
   }

   if (!ur_safe(disconnect_room) && time() - last_logout > 7200) {
      if (virthome = this_object()->query_property("virtualhome:home")) {
	 return virthome;
      } else {
	 return find_object(DEFAULT_DISCONNECT_ROOM);
      }
   }

   return disconnect_room;
}

void set_disconnect_room(object ob) {
   object virthome;

   last_logout = time();

   /* this must at the very least query CONFIGD -- TODO -- Zell */
   if (sizeof(query_host_souls())) {
      disconnect_room = ob;
      return;
   }

   if (ur_unsafe(ob)) {
      if (virthome = this_object()->query_property("virtualhome:home")) {
	 disconnect_room = virthome;
	 return;
      } else {
	 disconnect_room = find_object(DEFAULT_DISCONNECT_ROOM);
	 return;
      }
   } 
      
   disconnect_room = ob;
}

void clear_disconnect_room() {
   disconnect_room = nil;
}

/* public API */


/* create an array of NRefs from an array of details */
private
NRef *mkrefs(string *idarr) {
   NRef *res;
   int i;

   res = allocate(sizeof(idarr));
   for (i = 0; i < sizeof(res); i ++) {
      res[i] = NewNRef(this_object(), idarr[i]);
   }
   return res;
}

nomask
NRef *single_match(string name, varargs object looker, string *adj, int ignore_inv) {
   return
      mkrefs(match_single_detail(name, figure_side(looker), adj)) +
      (ignore_inv ? ({ }) : environment::single_match(name, adj));
}

nomask
NRef *plural_match(string name, varargs object looker, string *adj, int ignore_inv) {
   return
      mkrefs(match_plural_detail(name, figure_side(looker), adj)) +
      (ignore_inv ? ({ }) : environment::plural_match(name, adj));
}

nomask
NRef prime_reference(varargs object looker) {
   string id, *arr;

   if (id = ur_prime_id(this_object(), figure_side(looker))) {
      return NewNRef(this_object(), id);
   }
   return nil;
}

void set_brain ( string idx, mixed data ) {
   braindata[ idx ] = data;
   /* SysLog( "obj: "+ur_name(this_object())+" idx: "+idx+" data: "+dump_value(data) ); */
}

void set_brainX ( string idx, mixed data ) {
   int cnt;

   if ( braindata[ "num"+idx ]==nil )
      braindata[ "num"+idx ] = 0;
   cnt = braindata[ "num"+idx ];
   /* SysLog( "cnt for "+idx+": "+cnt ); */

   braindata[ idx+(cnt+1) ] = data;
   braindata[ "num"+idx ]++;
}

mixed query_brain ( string idx ) {
   return braindata[idx];
}

mapping query_braindata( void ) {
   return braindata;
}

void erase_braindata ( void ) {
   braindata = ([ ]);
}

void dump_brain ( object user ) {
   int i;
   string *bd;

   bd = map_indices( braindata );
   for ( i=0; i<sizeof(bd); i++ ) {
      user->message( bd[i] +" = "+dump_value(braindata[ bd[i] ])+"\n" );
   }
}

void run_auto_start ( void ) {
   run_signal("start", this_object());
}

void run_auto_stop ( void ) {
   run_signal("stop", this_object());
}

/* weapon stuff */
int query_weapon() {
   return weapon;
}

void set_weapon(int value) {
   weapon = value;
}

/* Allow anyone to subscribe to events we send out. */
int
allow_subscribe(object ob, string event)
{
    switch (event) {
    case "moved":
	/* Need this for study/teach. */
	return 1;
    default:
	return 0;
    }
}

/* these should be LWO's later */

/* This Patch and calls to it can be removed in some future date
   when we've cleaned up Marrach and patched it for all objects */

# define Patch() { if (!contents) contents = ([ ]); }

void clear_contents() {
   contents = ([ ]);
}

void add_content_group(string group) {
   Patch();
   contents[group] = ([ ]);
}

void clear_content_group(string group) {
   Patch();
   contents[group] = nil;
}

void add_content_choice(string group, string ix,
			varargs object obj, int wear,
			string prox, int stance, int prep) {
   int flag;

   if (!obj) {
      obj = find_object(OBJ_THING);
   }
   if (wear) {
      flag |= CONTENT_FLAG_WEAR;
   }
   Patch();
   if (!contents[group]) {
      contents[group] = ([ ]);
   }
   contents[group][ix] = ({ obj, prox, stance, prep, flag });
}

string *query_content_groups() {
   Patch();
   return map_indices(contents);
}

string *query_content_group_indices(string group) {
   Patch();
   if (contents[group]) {
      return map_indices(contents[group]);
   }
   return nil;
}

mapping query_content_group(string group) {
   Patch();
   return contents[group] ? contents[group] : ([ ]);
}

private
string *pick_complex_subset(string *ix) {
   mapping sort;
   string tmp, *nix;
   int i;

   if (!sizeof(ix)) {
      return ({ });
   }

   sort = ([ ]);

   for (i = 0; i < sizeof(ix); i ++) {
      if (sscanf(ix[i], "%s-", tmp)) {
	 sort[tmp] = TRUE;
      } else {
	 sort[ix[i]] = TRUE;
      }
   }
   nix = map_indices(sort);
   return map_indices(prefixed_map(arr_to_set(ix), nix[random(sizeof(nix))]));
}

string *pick_content_groups() {
   return pick_complex_subset(ur_content_groups(this_object()));
}

mixed **pick_from_content_group(string group) {
   mapping map;
   string *ix;
   mixed **out;
   int i;

   if (map = ur_content_group(this_object(), group)) {
      ix = pick_complex_subset(map_indices(map));

      XDebug("out of " + dump_value(map_indices(map)) + " i picked " + dump_value(ix));

      out = allocate(sizeof(ix));

      for (i = 0; i < sizeof(ix); i ++) {
	 out[i] = map[ix[i]];
      }
      return out;
   }
   return nil;
}

object query_indexed_content_object(string group, string ix) {
   mapping map;

   if (map = ur_content_group(this_object(), group)) {
      return map[ix][CONTENT_GROUP_OBJ];
   }
   return nil;
}

int query_indexed_content_wear(string group, string ix) {
   mapping map;

   if (map = ur_content_group(this_object(), group)) {
      return !!(map[ix][CONTENT_GROUP_FLAGS] & CONTENT_FLAG_WEAR);
   }
   return FALSE;
}


int query_perception_class() {
   return 
      (query_property("see-invisible") ? 1 : 0) |
      (query_property("infravision") ? 2 : 0) |
      (query_property("superb-sense-of-smell") ? 4 : 0);
}


int can_identify(mixed what) {
   mixed peer;
   object ob;

   peer = find_peer(what, this_object(), FALSE);

   if (peer == this_object()) {
      return TRUE;
   }
   if (typeof(peer) == T_STRING && query_proximity() &&
       NRefDetail(query_proximity()) == peer) {
      return TRUE;
   }

   ob = NRefOb(what);

   if (ob->query_property("invisible") && !query_property("see-invisible")) {
      return FALSE;
   }
   if (query_light() >= 1.0 || query_property("infravision")) {
      /* can see */
      return TRUE;
   }
   if (ur_targetable_in_dark(ob, NRefDetail(what))) {
      return TRUE;
   }
   if (query_property("superb-sense-of-smell") &&
       !ob->query_property("odorless")) {
      return TRUE;
   }
   if (typeof(peer) != T_OBJECT) {
      /* either our prime environment, or someplace totally off */
      return FALSE;
   }

   return NRefsEqual(query_proximity(), peer) ||
      NRefsEqual(peer->query_proximity(), this_object()) ||
      NRefsEqual(peer->query_proximity(), this_object()->query_proximity());
}


object query_possessor() {
   return possessor;
}

object *query_possessor_souls() {
   return possessor_souls;
}

object query_possessee() {
   return possessee;
}

object get_ultimate_possessee() {
   if (possessee) {
#ifdef WE_WANTS_THIS_BACK
      return possessee->get_ultimate_possessee();
#else
      return possessee;
#endif
   }
   return this_object();
}

/* this is called in the possessing body */

atomic
void possess_body(object body) {
   if (!body) {
      error("first argument to possess_body must not be nil");
   }
   if (possessor) {
      error("cannot possess possessing body");
   }
   if (possessee) {
      if (possessee == body) {
	 return;
      }
      error("already possessing another body");
   }
   if (body->query_possessor()) {
      error("cannot possess already possessed body");
   }
   if (body->query_possessee()) {
      error("cannot possess possessing body");
   }
   body->set_possessor(this_object(), query_souls());
   possessee = body;
   occur("possess", possessee);
}

/* this is called in either body */
atomic
void break_possession() {
   if (possessee) {
      if (possessor) {
	 error("internal inconsistency");
      }
      possessee->break_possession();
      return;
   }
   if (!possessor) {
      error("body is not possessed");
   }
   occur("unpossess", possessor);
   possessor->set_possessee(nil);
   possessor = nil;
}

void clear_possess_garbage() {
   if (possessor == this_object() && possessee == this_object()) {
      possessor = nil;
      possessee = nil;
   }
}


void set_possessor(object ob, object *souls) {
   if (previous_program() == "/base/lib/thing") {
      possessor = ob;
      possessor_souls = souls;
   }
}

void set_possessee(object ob) {
   if (previous_program() == "/base/lib/thing") {
      possessee = ob;
   }
}
