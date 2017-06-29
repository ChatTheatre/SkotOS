/*
 * ~SkotOS/obj/tip.c
 *
 * Copyright Skotos Tech Inc 2004.
 */

# include <core.h>
# include <base.h>
# include <SkotOS.h>
# include <SAM.h>

inherit "/lib/womble";
inherit core "/core/lib/core_object";

private string topic;
private mixed *descriptions;

private int	dirty_callout;

void womble_descriptions() {
   descriptions = womble(descriptions);
}

# define Dirty() { \
  if (dirty_callout == 0) { \
    dirty_callout = call_out("signal_update", 0); \
  } \
}

string query_state_root()
{
   return "Tip:Node";
}

string query_script_registry()
{
   return BASE_INITD;
}

string query_dp_registry()
{
   return CORE_DP_REGISTRY;
}

static void signal_update()
{
   dirty_callout = 0;
   TIPD->update_tip(this_object());
}

static void create(int clone)
{
   core::create();

   descriptions = ({ });
   if (!clone) {
      set_object_name("SkotOS:Tips:UrTip");
   }
}

void set_topic(string t)
{
   if (topic == t) {
      return;
   }
   topic = t;
   Dirty();
}

string query_topic()
{
   return topic;
}

SAM *query_descriptions()
{
   return descriptions[..];
}

int *query_description_indices()
{
   int i, sz, *ind;

   sz = sizeof(descriptions);
   ind = allocate_int(sz);
   for (i = 0; i < sz; i++) {
      ind[i] = i + 1;
   }
   return ind;
}

SAM query_description(int ix)
{
   return descriptions[ix - 1];
}

void clear_descriptions()
{
   descriptions = ({ });
   Dirty();
}

void add_description(mixed d)
{
   descriptions += ({ d });
}

void set_description(int ix, mixed d)
{
   while (sizeof(descriptions) < ix) {
      descriptions += ({ "" });
   }
   descriptions[ix - 1] = d;
}

/*
 * Caution: This simplified implementation does not handle recursive data-types!
 */
private int
equal(mixed a, mixed b)
{
   int type_a, type_b;

   type_a = typeof(a);
   type_b = typeof(b);
   if (type_a != type_b) {
      return FALSE;
   }
   switch (type_a) {
   case T_NIL:
      return TRUE;
   case T_INT:
   case T_FLOAT:
   case T_STRING:
   case T_OBJECT:
      return a == b;
   case T_ARRAY: {
      int i, sz;

      sz = sizeof(a);
      if (sz != sizeof(b)) {
	 return FALSE;
      }
      for (i = 0; i < sz; i++) {
	 if (!equal(a[i], b[i])) {
	    return FALSE;
	 }
      }
      return TRUE;
   }
   case T_MAPPING: {
      int sz;

      sz = map_sizeof(a);
      if (sz != map_sizeof(b)) {
	 return FALSE;
      }
      if (!equal(map_indices(a), map_indices(b))) {
	 return FALSE;
      }
      return equal(map_values(a), map_values(b));
	 
   }
   default:
      /* Should never happen, all types are accounted for. */
      error("Unknown value type: " + type_a);
   }
}

void del_description(mixed d)
{
   int i, sz;

   sz = sizeof(descriptions);
   for (i = 0; i < sz; i++) {
      if (equal(descriptions[i], d)) {
	 descriptions[i] = nil;
      }
   }
   descriptions -= ({ nil });
}

void del_description_ix(int ix)
{
   ix--;
   descriptions = descriptions[.. ix - 1] + descriptions[ix + 1 ..];
}
