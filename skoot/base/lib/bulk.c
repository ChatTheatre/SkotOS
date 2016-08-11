/*
**	/base/lib/bulk.c
**
**	This code handles the bulk, weight, volume, etc of things.
**
**	Copyright Skotos Tech Inc 1999
**
**	Initial Revision - Wes Connell
**
*/

# include <base.h>

private inherit "/lib/array";
private inherit "/base/lib/urcalls";

inherit "/lib/ur";
inherit "/lib/womble";

# include "/base/include/thing.h"
# include "/base/include/bulk.h"
# include "/base/include/motion.h"

private float mass;
private float original_mass;
private float density;
private float capacity;
private float largest_dim;		/* save for now */
private float original_largest_dim;	/* save for now */

private int	flexible;		/* does the container collapse? */

private float	cached_actual_mass;	/* sum of actual mass of inventory */
private	int	actual_mass_is_cached;

private float	cached_minimum_volume;	/* sum of minimum volume of inv */
private	int	minimum_volume_is_cached;

private float	cached_actual_volume;	/* sum of actual volume of inv */
private	int	actual_volume_is_cached;

/* i'm not sure where to put these -- wes */
private int edible;
private int potable;

private mixed drink_first;
private mixed drink_third;

void womble_bulk() {
    drink_first = womble(drink_first);
    drink_third = womble(drink_third);
}

nomask object *query_inventory();

void clear_bulk();

static
void create( ) {
   clear_bulk();
}

atomic
void clear_bulk() {
  mass = 1.0;
  original_mass = mass;
  density = 1.0;
  capacity = 1.0;
  largest_dim = 1.0;
  original_largest_dim = largest_dim;
}

atomic
void cached_bulk_enter(object ob) {
   object env;

   /* allow an object to enter us without dirtying our cache */

   if (actual_mass_is_cached) {
      cached_actual_mass += ob->query_actual_mass();
   }
   if (minimum_volume_is_cached) {
      cached_minimum_volume += ob->query_minimum_volume();
   }
   if (actual_volume_is_cached) {
      cached_actual_volume += ob->query_actual_volume();
   }
   if (env = query_environment()) {
      env->cached_bulk_enter(ob);
   }
}

atomic
void cached_bulk_leave(object ob) {
   object env;

   /* allow an object to leave us without dirtying our cache */

   if (actual_mass_is_cached) {
      cached_actual_mass -= ob->query_actual_mass();
   }
   if (minimum_volume_is_cached) {
      cached_minimum_volume -= ob->query_minimum_volume();
   }
   if (actual_volume_is_cached) {
      cached_actual_volume -= ob->query_actual_volume();
   }
   if (env = query_environment()) {
      env->cached_bulk_leave(ob);
   }
}

atomic
void dirty_bulk_cache() {
   object env;

   if (actual_mass_is_cached ||
       minimum_volume_is_cached ||
       actual_volume_is_cached) {
      if (env = query_environment()) {
	 env->dirty_bulk_cache();
      }
      actual_mass_is_cached =
	 minimum_volume_is_cached =
	 actual_volume_is_cached = FALSE;
   }
}

/* edible and potable object functions */
void set_edible(int value) { 
   edible = value;
}

string query_eaten_term() {
   return "TEST";
}

int query_edible() {
   return edible;
}

void be_eaten() {
   destruct();
}

void set_potable(int value) {
   potable = value;
}

int query_potable() {
   return potable;
}

void be_drank() {
   destruct();
}

mixed query_drink_message_first() {
   return drink_first;
}

mixed query_drink_message_third() {
   return drink_third;
}

void set_drink_message_first(mixed val) {
   drink_first = val;
}

void set_drink_message_third(mixed val) {
   drink_third = val;
}

void set_flexible(int i) {
   if (i != flexible) {
      CRAWLERS->start_crawler(this_object());
      dirty_bulk_cache();
   }
   flexible = i;
}

int query_flexible() {
   return flexible;
}

void set_mass(float i) {
   if (i != mass) {
      CRAWLERS->start_crawler(this_object());
      dirty_bulk_cache();
   }
   mass = i;
}

float query_intrinsic_mass() {
   return mass;
}

/* mass - kg */
void set_original_mass(float i) {
   original_mass = i;
   set_mass(i);
}

float query_original_mass() {
   return original_mass;
}

float query_actual_mass() {
   if (!actual_mass_is_cached) {
      cached_actual_mass =
	 add(rmap(query_inventory(), "query_actual_mass"), 0.0);
      actual_mass_is_cached = TRUE;
   }
   return cached_actual_mass + ur_intrinsic_mass(this_object());
}

/* density - kg/dm^3 */
void set_density(float i) {
   if (i != density) {
      CRAWLERS->start_crawler(this_object());
      dirty_bulk_cache();
   }
   density = i;
}

float query_intrinsic_density() {
   return density;
}

float query_actual_density() { 
   float m, v;

   m = query_actual_mass();
   v = query_minimum_volume() * 1000.0;

   if (!v) {
      if (!m) {
	 return 1.0;
      }
      error("infinite density");
   }
   return m/v;
}

/* capacity - m^3 */
void set_capacity(float i) {
   if (i != capacity) {
      CRAWLERS->start_crawler(this_object());
      dirty_bulk_cache();
   }
   capacity = i;
}

float query_intrinsic_capacity() { return capacity; }

private
float query_actual_volume_of_inventory() {
   if (!actual_volume_is_cached) {
      cached_actual_volume =
	 add(rmap(query_inventory(), "query_actual_volume"), 0.0);
      actual_volume_is_cached = TRUE;
   }
   return cached_actual_volume;
}

private
float query_minimum_volume_of_inventory() {
   if (!minimum_volume_is_cached) {
      cached_minimum_volume =
	 add(rmap(query_inventory(), "query_minimum_volume"), 0.0);
      minimum_volume_is_cached = TRUE;
   }
   return cached_minimum_volume;
}

float query_actual_capacity() {
   return ur_intrinsic_capacity(this_object()) -
      query_actual_volume_of_inventory();
}

float query_intrinsic_volume() {
   float m, d;

   m = ur_intrinsic_mass(this_object());
   d = ur_intrinsic_density(this_object());

   if (!d) {
      if (!m) {
	 return 1.0;
      }
      error("infinite volume");
   }
   return m / d / 1000.0;
}

/* minimum volume - m^3 */
float query_minimum_volume() {
   return query_minimum_volume_of_inventory() +
      query_intrinsic_volume();
}

/* actual volume - m^3 */
float query_actual_volume( ) {
   if (ur_flexible(this_object())) {
      /* collapses: own intrinsic volume plus actual volume of contents */
      return query_intrinsic_volume() + query_actual_volume_of_inventory();
   }
   /* rigid: own intrinsic volume plus entire own capacity */
   return query_intrinsic_volume() + ur_intrinsic_capacity(this_object());
}


string explain_density() {
   string **bracket;
   string *lo, *hi, los, his;

   if (bracket = MATERIALS->find_material_bracket(query_actual_density())) {
      lo = last_elements(bracket[0], 4);
      hi = first_elements(bracket[1], 4);

      return "The density of this object in relatoin to some common materials, listed from less dense to more dense: " + implode(lo, " < ") + " < THIS OBJECT < " + implode(hi, " < ");
   }
}
