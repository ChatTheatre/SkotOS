/*
**	/usr/SkotOS/socials/obj/advcat.c
**
**	Maintain the data for one adverb category.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <core.h>
# include <base.h>
# include <SkotOS.h>

private inherit "/lib/array";

inherit core "/core/lib/core_object";

string *adverbs;

private int dirty_callout;

# define Dirty() { \
  if (dirty_callout != 0) { \
    remove_call_out(dirty_callout); \
  }\
  dirty_callout = call_out("signal_update", 1); \
}

string query_script_registry() {
   return BASE_INITD;
}

string query_dp_registry() {
   return CORE_DP_REGISTRY;
}

atomic  void clear_category();

/* creator */

void patch() {
#if 0
   Dirty();
#endif
   core::create();
}

static
void create(int clone) {
   core::create();

   if (!clone) {
      set_object_name("SkotOS:Socials:UrAdvCat");
   }
   clear_category();
}

static
void destructing() {
   /* we're dying -- empty ourselves and update DB */
   adverbs = nil;
   SOCIALS->update_advcat(this_object());
}

/* TODO: Debug */
void die()
{
   SOCIALS->remove_advcat(this_object());
   destruct_object();
}

void suicide() { call_out("die", 0); }

string query_state_root() { return "Socials:AdvCat"; }

/* public API */

string *query_adverbs() {
  int i, sz;
  mapping map;

  if (!adverbs) {
    return ({ });
  }
  map = ([ ]);
  sz = sizeof(adverbs);
  for (i = 0; i < sz; i++) {
    map[adverbs[i]] = TRUE;
  }
  return map_indices(map);
}

int query_adverb(string adv) {
   return member(adv, adverbs);
}

atomic
void clear_category() {
   adverbs = ({ });
}



atomic
void set_adverb(string adv) {
   if (adv && strlen(adv)) {
      adverbs |= ({ adv });
   }
   Dirty();
}

/* atomic TODO */
object make_duplicate(string new_cat) {
   object obj;
   int i;

   obj = clone_object("/usr/SkotOS/socials/obj/advcat");

   for (i = 0; i < sizeof(adverbs); i ++) {
      obj->set_adverb(adverbs[i]);
   }
   obj->set_object_name("SkotOS:SocialAdvCats:" + new_cat);

   return obj;
}

/* internals */

static atomic
void signal_update() {
   dirty_callout = 0;
   SOCIALS->update_advcat(this_object());
}

mixed query_downcased_property(string prop) {
   if (sscanf(prop, "adverb:%s", prop)) {
      int i, sz, len;
      string *list;

      if (prop == "*") {
	 return adverbs[..];
      }
      len = strlen(prop);
      if (len > 0) {
	 list = adverbs[..];
	 sz = sizeof(list);
	 for (i = 0; i < sz; i++) {
	    if (strlen(list[i]) < len || list[i][..len - 1] != prop) {
	       list[i] = nil;
	    }
	 }
	 return list - ({ nil });
      }
      return ({ });
   }
   return ::query_downcased_property(prop);
}

void set_downcased_property(string prop, mixed val, varargs int opaque) {
   if (sscanf(prop, "adverb:%s", prop)) {
      if (prop == "*") {
	 if (typeof(val) == T_ARRAY) { 
	    int i, sz;

	    sz = sizeof(val);
	    for (i = 0; i < sz; i++) {
	       if (typeof(val[i]) != T_STRING) {
		  error("Bad property value at index " + i + " for \"adverb:*\"");
	       }
	    }
	    adverbs = val[..];
	    Dirty();
	    return;
	 } else {
	    error("Bad property value for \"adverb:*\"");
	 }
      }
      return;
   }
   ::set_downcased_property(prop, val, opaque);
}

mapping query_properties(varargs int derived) {
   mapping map;

   map = ::query_properties(derived);

   if (derived) {
      map["adverb:*"] = query_property("adverb:*");
   }

   return map;
}
