/*
**	/usr/SkotOS/obj/verb.c
**
**	Maintain the data for one verb.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <core.h>
# include <base.h>
# include <base/prox.h>
# include <SkotOS.h>
# include <SAM.h>

/* nasty dependency breaker -- time to split verb.c into ~TextIF vs /base */
# define FILTERFARM	"/usr/TextIF/data/filterfarm"

private inherit "/lib/string";
private inherit "/lib/array";

inherit "/usr/SkotOS/lib/ursocials";

inherit core "/core/lib/core_object";

inherit notes "/lib/notes";
inherit ur "/lib/ur";

private mapping handlers;

private string imp;
private string second;
private string third;

private int evoke;	/* 1, 2, 3 */

private mapping role_to_requirement;
private mapping role_to_distance;
private mapping role_to_prepset;
private mapping role_to_direct;
private mapping role_to_raw;
private mapping role_to_single;
private mapping role_to_filters;

private string default_role;

private string *actions;	/* now an array! */

private int privacy;
private int secrecy;
private int obscurity;

private int priviliged;

private int target_abstracts;

private int	audible;

private string *prepositions;

private string	log_evokes;
private int	crafting;
private int	always_make;
private int	connects;

private int dirty_callout;
private int disabled;
private int is_ooc;
private int is_raw;

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

/* creator */

static atomic
void create(int clone) {
   if (!clone) {
      set_object_name("SkotOS:Socials:UrVerb");
   }
   core::create();
   notes::create();
   ur::create();

   role_to_requirement = ([ ]);
   role_to_distance = ([ ]);
   role_to_prepset = ([ ]);
   role_to_direct = ([ ]);
   role_to_raw = ([ ]);
   role_to_single = ([ ]);
   role_to_filters = ([ ]);

   handlers = ([ ]);

   evoke = VERB_OPTIONAL;
}

int query_evoke() {
   return evoke;
}

void set_evoke(int n) {
   evoke = n;
}

void clear_roles() {
   role_to_requirement = ([ ]);
   role_to_distance = ([ ]);
   role_to_prepset = ([ ]);
   role_to_direct = ([ ]);
   role_to_raw = ([ ]);
   role_to_single = ([ ]);
   role_to_filters = ([ ]);
}

void clear_prepositions(string role) {
   role_to_prepset[role] = ({ });
}

atomic
void delete_object(string role) {
   role_to_requirement[role] = nil;
   role_to_distance[role] = nil;
   role_to_prepset[role] = nil;
   role_to_direct[role] = nil;
   role_to_raw[role] = nil;
   role_to_single[role] = nil;
   role_to_filters[role] = nil;
}

atomic
void configure_object(string role, int required, int dist) {
   role_to_requirement[role] = required;
   role_to_distance[role] = dist;
   if (!role_to_prepset[role]) {
      role_to_prepset[role] = ({ });
   }
}

int query_requirement(string role) {
   return role_to_requirement[role] ? role_to_requirement[role] : 0;
}

int query_required(string role) {
   if (role_to_requirement[role]) {
      return role_to_requirement[role] == VERB_REQUIRED;
   }
}

int query_distance(string role) {
   return role_to_distance[role] ? role_to_distance[role] : 0;
}

string *query_roles() {
   return map_indices(role_to_requirement);
}

string *query_required_roles() {
   string *ix;
   int i;

   ix = ur_roles(this_object());
   for (i = 0; i < sizeof(ix); i ++) {
      if (ur_requirement(this_object(), ix[i]) != VERB_REQUIRED) {
	 ix[i] = nil;
      }
   }
   return ix - ({ nil });
}

string *query_owned_roles() {
   string *ix;
   int i;

   ix = ur_roles(this_object());
   for (i = 0; i < sizeof(ix); i ++) {
      if (ur_distance(this_object(), ix[i]) != SA_OWNED) {
	 ix[i] = nil;
      }
   }
   return ix - ({ nil });
}

string *query_direct_roles() {
   string *ix;
   int i;

   ix = ur_roles(this_object());
   for (i = 0; i < sizeof(ix); i ++) {
      if (!ur_direct(this_object(), ix[i])) {
	 ix[i] = nil;
      }
   }
   return ix - ({ nil });
}



string *query_prepositions(string role) {
   if (role_to_prepset[role]) {
      return role_to_prepset[role];
   }
   return ({ });
}

atomic
void set_preposition(string role, string prep) {
   role_to_prepset[role] |= ({ prep });
}

atomic
void set_direct(string role, int bool) {
   role_to_direct[role] = bool ? TRUE : nil;
}

int query_direct(string role) {
   return role_to_direct[role] ? TRUE : FALSE;
}

atomic
void set_raw(string role, int bool) {
   role_to_raw[role] = bool ? TRUE : nil;
}

int query_raw(string role) {
   return role_to_raw[role] ? TRUE : FALSE;
}

atomic
void set_single(string role, int bool) {
   role_to_single[role] = bool ? TRUE : nil;
}

int query_single(string role) {
   return role_to_single[role] ? TRUE : FALSE;
}

atomic
void clear_filters(string role) {
   role_to_filters[role] = nil;
}

atomic
void add_filter(string role, string filter) {
   if (!role_to_filters[role]) {
      role_to_filters[role] = new_object(FILTERFARM, ({ }));
   }
   role_to_filters[role]->add_filter(filter);
}

string *query_filters(string role) {
   if (role_to_filters[role]) {
      return role_to_filters[role]->query_filters();
   }
   return ({ });
}

atomic
void delete_filter(string role, string filter) {
   if (role_to_filters[role]) {
      role_to_filters[role]->delete_filter(filter);
   }
}

object query_filter_object(string role) {
   return role_to_filters[role];
}

atomic
void set_prepositions(string role, string *preps) {
   role_to_prepset[role] = preps[..];
}

void delete_preposition(string role, string prep) {
   role_to_prepset[role] -= ({ prep });
}

string get_role_by_preposition(string prep) {
   string *ix;
   int i;

   ix = ur_roles(this_object());

   for (i = 0; i < sizeof(ix); i ++) {
      if (member(prep, ur_prepositions(this_object(), ix[i]))) {
	 return ix[i];
      }
   }
   return nil;
}


static atomic
void destructing() {
   if (imp) {
      SOCIALS->deregister_verb(imp, this_object());
   }
   ur::destructing();
}

/* TODO: Debug */
void die() { destruct_object(); }

void suicide() { call_out("die", 0); }

nomask
string query_state_root() {
   return "Socials:Verb";
}

/* public API */

string query_imperative() {
   return imp;
}

atomic
void set_imperative(string l_imp) {
   if (l_imp && sscanf(l_imp, "%*s ")) {
      error("no spaces allowed in verbs");
   }
   if (imp) {
      SOCIALS->deregister_verb(imp, this_object());
   }
   imp = l_imp;
   if (imp) {
      if (disabled) {
	 SOCIALS->deregister_verb(imp, this_object());
      } else {
	 SOCIALS->register_verb(imp, this_object());
      }
   }
   Dirty();
}

atomic
void set_second(string l_sec) {
   second = l_sec;
   Dirty();
}

string query_second() {
   return second;
}

atomic
void set_third(string l_third) {
   third = l_third;
   Dirty();
}

string query_third() {
   return third;
}

void set_priviliged(int value) {
   priviliged = !!value;
}

int query_priviliged() {
   return priviliged;
}

void set_privacy(int value) {
   privacy = value;
}

int query_privacy() {
   return privacy;
}

void set_secrecy(int value) {
   secrecy = value;
}

int query_secrecy() {
   return secrecy;
}

void set_obscurity(int value) {
   obscurity = value;
}

int query_obscurity() {
   return obscurity;
}

void set_target_abstracts(int value) {
   target_abstracts = value;
}

int query_target_abstracts() {
   return target_abstracts;
}

atomic
void set_default_role(string str) {
   default_role = str;
}

string query_default_role() {
   return default_role;
}

atomic
void set_log_evokes(string str) {
   log_evokes = str;
   Dirty();
}

string query_log_evokes() {
   return log_evokes;
}

atomic
void set_audible(int truth) {
   audible = truth;
   Dirty();
}

int query_audible() {
   return audible;
}

void clear_actions() {
   actions = ({ });
}

void add_action(string str) {
   actions |= ({ str });
}

void remove_action(string str) {
   actions -= ({ str });
}

void patch_actions() {
   if (actions) {
      actions -= ({ imp });
   }
}

string *get_actions() {
   /* at some point we can hopefully remove the ', imp' part */
   return ur_actions(this_object()) | ({ "verb/" + imp, imp });
}

string get_action() {
   return ur_action(this_object());
}

string *query_actions() {
   return actions;
}

atomic
void set_crafting(int truth) {
   crafting = truth;
   Dirty();
}

int query_crafting() {
   return crafting;
}

atomic
void set_always_make(int truth) {
   always_make = truth;
   Dirty();
}

int query_always_make() {
   return always_make;
}

atomic
void set_connects(int truth) {
   connects = truth;
   Dirty();
}

int query_connects() {
   return connects;
}

int query_ooc() {
   return is_ooc;
}

void set_ooc(int truth) {
   is_ooc = truth;
}

int query_disabled() {
   return disabled;
}

atomic
void set_disabled(int i) {
   i = !!i; /* Just in case. */
   if (disabled == i) {
      return;
   }
   DEBUG(name(this_object()) + " :: set_disabled(" + i + ")");
   disabled = i;
   if (imp) {
      if (disabled) {
	 SOCIALS->deregister_verb(imp, this_object());
      } else {
	 SOCIALS->register_verb(imp, this_object());
      }
   } else {
      DEBUG("[set_disabled] Called in " + name(this_object()) + " while no imp set?");
   }
   Dirty();
}

int query_raw_verb() {
   return is_raw;
}

void set_raw_verb(int truth)
{
   is_raw = truth;
   Dirty();
}

/* internals */

static atomic
void signal_update() {
   dirty_callout = 0;
   SOCIALS->update_verb(this_object());
}

mixed
query_downcased_property(string prop) {
   string role;

   if (sscanf(prop, "verb:%s", prop)) {
      if (sscanf(prop, "roles:%s:%s", role, prop)) {
	 switch(prop) {
	 case "allowed:local":
	    return (query_requirement(role) == VERB_OPTIONAL ||
		    query_requirement(role) == VERB_REQUIRED);
	 case "allowed":
	    return (ur_requirement(this_object(), role) == VERB_OPTIONAL ||
		    ur_requirement(this_object(), role) == VERB_REQUIRED);
	 case "required:local":
	    return query_requirement(role) == VERB_REQUIRED;
	 case "required":
	    return ur_requirement(this_object(), role) == VERB_REQUIRED;
	 case "far:local":
	    return query_distance(role) == SA_FAR;
	 case "far":
	    return ur_distance(this_object(), role) == SA_FAR;
	 case "armslength:local":
	    return query_distance(role) == SA_ARMSLENGTH;
	 case "armslength":
	    return ur_distance(this_object(), role) == SA_ARMSLENGTH;
	 case "close:local":
	    return query_distance(role) == SA_CLOSE;
	 case "close":
	    return ur_distance(this_object(), role) == SA_CLOSE;
	 case "owned:local":
	    return query_distance(role) == SA_OWNED;
	 case "owned":
	    return ur_distance(this_object(), role) == SA_OWNED;
	 case "prepositions:local":
	    return query_prepositions(role);
	 case "prepositions":
	    return ur_prepositions(this_object(), role);
	 case "direct:local":
	    return query_direct(role);
	 case "direct":
	    return ur_direct(this_object(), role);
	 case "raw:local":
	    return query_raw(role);
	 case "raw":
	    return ur_raw(this_object(), role);
	 case "single:local":
	    return query_single(role);
	 case "single":
	    return ur_single(this_object(), role);
	 }
	 error("unknown verb:roles property type: " + prop);
      }
      switch(prop) {
      case "imperative:local":
      case "imperative":
	 return query_imperative();
      case "second:local":
      case "second":
	 return query_second();
      case "third:local":
      case "third":
	 return query_third();
      case "priviliged:local":
      case "priviliged":
	 return query_priviliged();
      case "privacy:local":
	 return query_privacy();
      case "privacy":
	 return ur_privacy(this_object());
      case "secrecy:local":
	 return query_secrecy();
      case "secrecy":
	 return ur_secrecy(this_object());
      case "obscurity:local":
	 return query_obscurity();
      case "obscurity":
	 return ur_obscurity(this_object());
      case "target-abstracts:local":
	 return query_target_abstracts();
      case "target-abstracts":
	 return ur_target_abstracts(this_object());
      case "log-evokes:local":
	 return query_log_evokes();
      case "log-evokes":
	 return ur_log_evokes(this_object());
      case "audible:local":
	 return query_audible();
      case "audible":
	 return ur_audible(this_object());
      case "crafting:local":
      case "crafting":
	 return query_crafting();
      case "always-make:local":
      case "always-make":
	 return query_always_make();
      case "connects:local":
      case "connects":
	 return query_connects();
      case "evoke-allowed:local":
	 return (query_evoke() == VERB_OPTIONAL ||
		 query_evoke() == VERB_REQUIRED);
      case "evoke-allowed":
	 return (ur_evoke(this_object()) == VERB_OPTIONAL ||
		 ur_evoke(this_object()) == VERB_REQUIRED);
      case "evoke-required:local":
	 return query_evoke() == VERB_REQUIRED;
      case "evoke-required":
	 return ur_evoke(this_object()) == VERB_REQUIRED;
      case "roles:local":
	 return query_roles();
      case "roles":
	 return ur_roles(this_object());
      case "disabled":
	 return ur_disabled(this_object());
      case "disabled:local":
	 return query_disabled();
      case "ooc":
	 return ur_ooc(this_object());
      case "ooc:local":
	 return query_ooc();
      case "raw":
	 return ur_raw_verb(this_object());
      case "raw:local":
	 return query_raw_verb();
      case "actions":
	 return ur_actions(this_object());
      case "actions:local":
	 return query_actions();
      }
      error("unknown verb: property type: " + prop);
   }
   return ::query_downcased_property(prop);
}

void set_downcased_property(string prop, mixed val, varargs int opaque) {
   if (sscanf(prop, "verb:%s", prop)) {
      /* ignore attempts to write into derived properties */
      sscanf(prop, "%s:local", prop);
      switch (prop) {
      case "imperative":
	 set_imperative(val);
	 break;
      case "second":
	 set_second(val);
	 break;
      case "third":
	 set_third(val);
	 break;
      case "priviliged":
	 set_priviliged(!!val);
	 break;
      case "privacy":
	 set_privacy(!!val);
	 break;
      case "secrecy":
	 set_secrecy(!!val);
	 break;
      case "obscurity":
	 set_obscurity(!!val);
	 break;
      case "target-abstracts":
	 set_target_abstracts(!!val);
	 break;
      case "log-evokes":
	 /* Not listed in SID */
	 break;
      case "audible":
	 /* Not listed in SID */
	 set_audible(!!val);
	 break;
      case "crafting":
	 /* Not listed in SID */
	 set_crafting(!!val);
	 break;
      case "always-make":
	 /* Not listed in SID */
	 set_always_make(!!val);
	 break;
      case "connects":
	 set_connects(!!val);
	 break;
      case "roles":
	 break;
      case "disabled":
	 set_disabled(!!val);
	 break;
      case "ooc":
	 set_ooc(!!val);
	 break;
      case "raw":
	 set_raw_verb(!!val);
	 break;
      default: {
	 string action;

	 if (sscanf(prop, "action:%s", action) || sscanf(prop, "actions:%s", action)) {
	    if (val) {
	       add_action(action);
	    } else {
	       remove_action(action);
	    }
	 }
	 break;
      }
      }
      return;
   }
   :: set_downcased_property(prop, val, opaque);
}

mapping query_properties(varargs int derived) {
   string *arr, *roles;
   mapping map;
   int i, j;

   map = ::query_properties(derived);

   if (derived) {
      arr = ({
	 "imperative",
	    "second",
	    "third",
	    "priviliged",
	    "privacy",
	    "secrecy",
	    "obscurity",
	    "target-abstracts",
	    "log-evokes",
	    "audible",
	    "crafting",
	    "always-make",
	    "connects",
	    "evoke-allowed",
	    "evoke-required",
	    "roles",
	    "disabled",
	    "ooc",
	    "raw",
	    "actions",
	 "imperative:local",
	    "second:local",
	    "third:local",
	    "priviliged:local",
	    "privacy:local",
	    "secrecy:local",
	    "obscurity:local",
	    "target-abstracts:local",
	    "log-evokes:local",
	    "audible:local",
	    "crafting:local",
	    "always-make:local",
	    "connects:local",
	    "evoke-allowed:local",
	    "evoke-required:local",
	    "roles:local",
	    "disabled:local",
	    "ooc:local",
	    "raw:local",
	    "actions:local"
	    });
      for (i = 0; i < sizeof(arr); i ++) {
	 map["verb:" + arr[i]] = query_property("verb:" + arr[i]);
      }
      roles = query_roles();
      arr = ({
	 "allowed",
	    "required",
	    "far",
	    "armslength",
	    "close",
	    "owned",
	    "prepositions",
	    "single",
	 "allowed:local",
	    "required:local",
	    "far:local",
	    "armslength:local",
	    "close:local",
	    "owned:local",
	    "prepositions:local",
	    "single:local",
      });
      for (i = 0; i < sizeof(roles); i ++) {
	 for (j = 0; j < sizeof(arr); j ++) {
	    map["verb:roles:" + roles[i] + ":" + arr[j]] =
	       query_property("verb:roles:" + roles[i] + ":" + arr[j]);
	 }
      }
   }
   return map;
}

void patch_duelling() {
   if (imp && strlen(imp) > 2 && imp[.. 1] == "+z") {
      set_imperative(imp[2 ..]);
   }
}


/*
**	Code for the registration of action handler dependencies. Please
**	note that while the OO relationships are nowhere made explicit,
**	we follow an essentially module-respecting delinatiation of
**	interfaces and subclassing or whatnot
*/



void patch_handlers() {
   if (!handlers) {
      handlers = ([ ]);
   }
}

void patch_raw() {
   if (!role_to_raw) {
      role_to_raw = ([ ]);
   }
}

void add_handler_dependency(object handler) {
   handlers[handler] = TRUE;
}

void remove_handler_dependency(object handler) {
   handlers[handler] = nil;
}

static
void destructive_desire() {
   if (map_sizeof(handlers)) {
      error("action handlers depend on this verb");
   }
}

object *query_handler_dependencies() {
   return map_indices(handlers);
}
