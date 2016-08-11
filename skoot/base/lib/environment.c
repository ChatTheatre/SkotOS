/*
**	/base/lib/environment.c
**
**	This is the fundamental code inherited by all objects that
**	can be the environment of other objects. Environments hold
**	inventories, and we keep those in an array -- except for a
**	one-object buffer so we do not need to modify the array in
**	case somebody quickly runs through the room.
**
**	In the long run, our inventory data structure will need to
**	be faster, probably.
**
**	This object also contains the functionality for broadcasting
**	actions in the virtual world to all the interface handlers
**	that describe such events to their users.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/urcalls";

# include "/base/include/bulk.h"
# include "/base/include/light.h"
# include "/base/include/environment.h"
# include "/base/include/ownership.h"

# include <base.h>

private object *inv;	/* array of objects constituting our inventory */
private object buf;	/* most recent object to enter inventory */
private float max_weight; /* maximum weight this container can hold. */
private float max_frontal_area; /* maximum frontal area that can pass */
private float max_depth; /* maximum largest dim that will fit */
private string container_verb_second; /* are carrying */
private string container_verb_third; /* is carrying */
private int clothes_expected; /* am I expected to wear clothes */
private int unsafe;  /* unsafe to quit inside of? */
private int safe;    /* am I safe to quit inside of? */
private int tight;	/* can we put powders and liquids into it? */

private int public_container; /* are contents visible to other players */
private int transparent_container; /* are contents visible even when closed */

private mapping cache;

object *list_clothes();

nomask int is_environment() { return 1; }

/* called by thing.c when the object has just been created */
static
void create() {
   buf = nil;
   inv = ({ });

   cache = ([ ]);

   container_verb_second = nil;
   container_verb_third = nil;

   /* these are temp */
   max_weight = 1.0;		/* kg */
   max_frontal_area = 1.0;	/* m^2 */
   max_depth = 1.0;		/* m */

   add_event("entered");
   add_event("left");
}

/* called by thing.c when the object is about to be destroyed */
static
void destructing() {
   object *obarr;
   int i;

   obarr = query_inventory();

   for (i = 0; i < sizeof(obarr); i ++) {
      catch {
	 obarr[i]->die();
      }
   }
}


/* public API */

int query_unsafe() {
   return unsafe;
}

void set_unsafe(int value) {
   if (!value)
      unsafe = FALSE;
   else {
      unsafe = TRUE;
      safe = FALSE;
   }
}

int query_safe() {
   return safe;
}

void set_safe(int value) {
   if (!value)
      safe = FALSE;
   else {
      safe = TRUE;
      unsafe = FALSE;
   }
}

/* container_verb functions */
/* the container verb is what determines the type of description given
** when this object is viewed. 
** 'Wes is <carrying> the sword.'  'The bag <contains> the sword'.
** if nil then the prox description will be used.
*/
string query_container_verb_second() {
   return container_verb_second;
}

void set_container_verb_second(string verb) {
   container_verb_second = verb;
}

string query_container_verb_third() {
   return container_verb_third;
}

void set_container_verb_third(string verb) {
   container_verb_third = verb;
}

/* clothes_expected */
/* If clothes are expected on an object then a naked state will be revealed
** if clothes are NOT present on the object
*/

int query_clothes_expected() {
   return clothes_expected;
}

void set_clothes_expected(int value) {
   clothes_expected = value;
}

int query_tight() {
   return tight;
}

void set_tight(int value) {
   tight = value;
}

int query_transparent_container() {
   return transparent_container;
}

void set_transparent_container(int value) {
   if (transparent_container != value) {
      transparent_container = value;
      CRAWLERS->start_crawler(this_object()); 
      this_object()->dirty_light_cache();
   }
}

int query_public_container() {
   return public_container;
}

void set_public_container(int value) {
   public_container = value;
}

nomask
object *query_near(mixed what) {
   object *out;
   string id;
   NRef prox;
   int i, j;

   out = query_inventory();

   if (what == nil) {
      for (i = 0; i < sizeof(out); i ++) {
	 if (!out[i]->query_proximity()) {
	    out[j ++] = out[i];
	 }
      }
      return out[.. j-1];
   }
   if (typeof(what) == T_STRING) {
      id = what;
   } else if (IsNRef(what)) {
      if (NRefOb(what) != this_object()) {
	 error("non-local nref to query_near");
      }
      id = NRefDetail(what);
   } else {
      error("bad type argument to query_near()");
   }

   for (i = 0; i < sizeof(out); i ++) {
      if (prox = out[i]->query_proximity()) {
	 if (NRefDetail(prox) == id) {
	    out[j ++] = out[i];
	 }
      }
   }
   return out[.. j-1];
}


/* return an array of this objects inventory */
nomask
object *query_inventory() {
    /* Hack hack. --Erwin. */
    inv -= ({ nil });
    if (buf) {
	return ({ buf }) + inv;
    }
    return inv[..]; /* copy */
}

nomask
mapping deep_inventory_map() {
   mapping out;
   object *arr;
   int i;

   out = ([ ]);

   if (buf) {
      out[buf] = buf;
      out += buf->deep_inventory_map();
   }
   for (i = 0; i < sizeof(inv); i ++) {
      out[inv[i]] = inv[i];
      out += inv[i]->deep_inventory_map();
   }
   return out;
}

nomask
object *deep_inventory() {
   return map_indices(deep_inventory_map());
}



void fix_inventory() {
   inv -= ({ nil });
}

/* max weight allowed to enter */
float query_max_weight() { return max_weight; }
void set_max_weight( float i ) { max_weight = i; }

int check_max_weight( object ob ) {
   if (sizeof(inv) > 100) {
      SysLog("WARNING:: check_max_weight() ran for object " + name(this_object()) + " with inventory size: " + sizeof(inv));
   }

   if (ob->contained_by(this_object())) {
      return 0;
   }
   if ((query_actual_mass() - ur_intrinsic_mass(this_object())) +
       ob->query_actual_mass() > ur_max_weight(this_object())) {
      return -1;
   }
   return 0;
}

/* check object actual volume against dest capacity */
int check_capacity( object ob ) {
   if (ob->contained_by(this_object())) {
      return 0;
   }
   if ( ob->query_actual_volume() > query_actual_capacity() ) {
      return -1;
   }
   return 0;
}

/* create an array of NRefs from an array of details */
private
NRef *mkrefs(object ob, string *idarr) {
   NRef *res;
   int i;

   res = allocate(sizeof(idarr));
   for (i = 0; i < sizeof(res); i ++) {
      res[i] = NewNRef(ob, idarr[i]);
   }
   return res;
}

/* return NRefs based on their singular name */
static
NRef *single_match(string name, varargs string *adj) {
   string *tmp, id;
   object *inv;
   NRef *res;
   int i;

   res = ({ });
   inv = query_inventory();

   for (i = 0; i < sizeof(inv); i ++) {
      tmp = inv[i]->match_single_detail(name, "outside", adj);
      if (sizeof(tmp) > 0) {
	 res += mkrefs(inv[i], tmp);
      }
      if (name == inv[i]->query_combinable_noun() &&
	  inv[i]->query_amount() > 1.0) {
	 if (id = ur_prime_id(inv[i], "outside")) {
	    res += ({ NewNRef(inv[i], id) });
	 }
      }
   }
   return res;
}

/* return NRefs based on their plural name */
static
NRef *plural_match(string name, varargs string *adj) {
   string *tmp, id;
   object *inv;
   NRef *res;
   int i;

   res = ({ });
   inv = query_inventory();

   for (i = 0; i < sizeof(inv); i ++) {
      tmp = inv[i]->match_plural_detail(name, "outside", adj);
      if (sizeof(tmp) > 0) {
	 res += mkrefs(inv[i], tmp);
      }
      if (name == inv[i]->query_combinable_plural_noun() &&
	  inv[i]->query_amount() > 1.0) {
	 if (id = ur_prime_id(inv[i], "outside")) {
	    res += ({ NewNRef(inv[i], id) });
	 }
      }
   }

   return res;
}

/* base-internal API */

void enter_cache(object ob) {
   if (!cache) {
      cache = ([ ]);
   }
   cache[ob] = ob;
}

void leave_cache(object ob) {
   if (!cache) {
      cache = ([ ]);
   }
   cache[ob] = nil;
}

object *query_cached_inventory() {
   if (!cache) {
      cache = ([ ]);
   }
   return map_indices(cache);
}

/* another object has just entered; called from move() */
nomask atomic
void _F_entered(object ob, object old) {
   if (sscanf(previous_program(), "/base/lib/%*s") == 0) {
      error("bad caller");
   }

   if (ur_cached_motion(ob)) {
      enter_cache(ob);
   }

   if (buf) {
      inv = ({ buf }) + inv;
   }
   if (sizeof(inv & ({ ob }))) {
      error("object already present");
   }
   buf = ob;

   cached_bulk_enter(ob);
   cached_light_enter(ob);

   event("entered", ob, old);
}

/* another object is just leaving; called from move() and destructing() */
nomask atomic
void _F_leaving(object ob, varargs object new) {
   int size;

   if (sscanf(previous_program(), "/base/lib/%*s") == 0) {
      error("bad caller");
   }

   cached_bulk_leave(ob);
   cached_light_leave(ob);

   leave_cache(ob);

   if (buf == ob) {
      buf = nil;
      event("left", ob, new);
      return;
   }
   size = sizeof(inv);
   inv -= ({ ob });
   if (size == sizeof(inv)) {
      error("object not present");
   }

   event("left", ob, new);
}

nomask void cleanclean() { inv -= ({ nil }); }
