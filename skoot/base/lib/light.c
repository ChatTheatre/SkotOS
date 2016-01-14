/*
**	/base/lib/light.c
**
**
**	Copyright Skotos Tech Inc 1999
**
**
*/

# include <base.h>
# include <base/light.h>

private inherit "/lib/array";
private inherit "/lib/string";
private inherit "/base/lib/urcalls";

inherit "/lib/ur";

# include "/base/include/thing.h"
# include "/base/include/motion.h"
# include "/base/include/environment.h"
# include "/base/include/events.h"
# include "/base/include/light.h"

private float	cached_light;
private int	light_is_cached;

private float	cached_luminosity;
private int	luminosity_is_cached;

private int	call;


float query_light();

atomic
void cached_light_enter(object ob) {
   object env;

   /* allow an object to enter us without dirtying our cache */

   if (light_is_cached) {
      if (!call) {
	 call = call_out("check_light_levels", 0, query_light(), ob);
      }
      cached_light += ob->query_local_light();
   }
   if (env = query_environment()) {
      env->cached_light_enter(ob);
   }
}

atomic
void cached_light_leave(object ob) {
   object env;

   /* allow an object to leave us without dirtying our cache */

   if (light_is_cached) {
      if (!call) {
	 call = call_out("check_light_levels", 0, query_light(), ob);
      }
      cached_light -= ob->query_local_light();
   }
   if (env = query_environment()) {
      env->cached_light_leave(ob);
   }
}

atomic
void dirty_light_cache(varargs int flag) {
   object env;

   if (light_is_cached || luminosity_is_cached) {
      if (flag && !call) {
	call = call_out("check_light_levels", 0, query_light());
      }
      if (env = query_environment()) {
	env->dirty_light_cache();
      }
      light_is_cached = luminosity_is_cached = FALSE;
   }
}


void dirty_local_light_cache() {
   light_is_cached = luminosity_is_cached = FALSE;
}   

private
string categorize_light(float val) {
   if (val < 1.0) {
      return "dark";
   }
   if (val < 10.0) {
      return "dim";
   }
   if (val < 300000.0) {
      return "bright";
   }
   return "superbright";
}

static
void check_light_levels(float old, varargs object actor) {
   mapping args;
   float new_light;

   call = 0;

   new_light = query_light();

   args = ([
	      "old-light": old,
	      "old-light-category": categorize_light(old),
	      "new-light": new_light,
	      "new-light-category": categorize_light(new_light),
	      "light-bringer": actor,
	      ]);

   if (new_light > old) {
      this_object()->execute_action("base/lightens", args);
   } else if (new_light < old) {
      this_object()->execute_action("base/darkens", args);
   }
}

/* debug */
float query_cached_light() {
   return cached_light;
}

/*
 * This function should perhaps be better named as query_total_luminosity(), while
 * query_total_luminosity() should be renamed to query_intrinsic_luminosity()!
 * --Erwin.
 */
float query_local_light() {
   if (!light_is_cached) {
      object *inv;
      int i, sz;

      cached_light = query_total_luminosity();
      inv = query_inventory();
      sz = sizeof(inv);
      for (i = 0; i < sz; i ++) {
	 object ob;

	 ob = inv[i];
	 /* only get light from open containers */
	 if (!sizeof(ob->query_doors()) ||
	     sizeof(ob->query_open_doors()) ||
	     ur_transparent_container(ob)) {
	    cached_light += ob->query_local_light();
	 }
      }
      light_is_cached = TRUE;
   }
   return cached_light;
}

/* trivial debug function with messed-up indentation */
void debug_local_light(varargs int indent) {
   object *inv;
   float light;
   int i, sz;

   light = query_total_luminosity();
   inv = query_inventory();
   sz = sizeof(inv);
   for (i = 0; i < sz; i ++) {
      object ob;

      ob = inv[i];
      /* only get light from open containers */
      if (!sizeof(ob->query_doors()) ||
	  sizeof(ob->query_open_doors()) ||
	  ur_transparent_container(ob)) {
	 light += ob->query_local_light();
	 ob->debug_local_light(indent + 2);
      }
   }
   SysLog(pad_left(light + " <-- " + name(this_object()), indent));

   if (light_is_cached && light != cached_light) {
      SysLog(pad_left(cached_light + " <-- CACHED LIGHT - ERROR!", indent));
   }
}

float query_light() {
   float here, there, volume, area, relative_area;

   here = query_local_light();

   volume = ur_intrinsic_capacity(this_object());

   /* area is vol^(2/3); compare to normal room of 10 m^2 */

   relative_area = pow(volume, 2.0/3.0) / 10.0;

   if (relative_area < 0.1) {
      relative_area = 0.1;
   } else if (relative_area > 5.0) {
      relative_area = 5.0;
   }

   here /= relative_area;

   if (query_environment()) {
      there = query_environment()->query_light();
      if (there > here) {
	 return there;
      }
   }
   return here;
}

float query_total_luminosity() {
   if (!luminosity_is_cached) {
      string *arr;
      int i, sz;

      arr = ur_details(this_object());
      cached_luminosity = 0.0;
      sz = sizeof(arr);
      for (i = 0; i < sz; i ++) {
	 cached_luminosity += this_object()->get_luminosity(arr[i]);
      }
      luminosity_is_cached = TRUE;
   }
   return cached_luminosity;
}

int query_light_category() {
   float l;

   l = query_light();
   if (l < 1.0) {
      return LIGHT_DARK;
   }
   if (l < 10.0) {
      return LIGHT_DIM;
   }
   if (l < 300000.0) {
      return LIGHT_BRIGHT;
   }
   return LIGHT_SUPERBRIGHT;
}

