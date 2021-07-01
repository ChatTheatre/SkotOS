/*
 * /base/sys/base.c
 *
 * Take care of the 'base:*' properties.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <type.h>
# include <base.h>
# include <base/light.h>
# include <base/living.h>
# include <base/preps.h>

# include <dp_handler.h>

# include <mapargs.h>

private inherit "/lib/mapargs";
private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";

inherit "/core/lib/dp_handler";

string query_derived_root() {
   return "base";
}

static
void create() {
   BASE_PROPERTIES->register_root_handler("base", this_object());
   set_object_name("Base:DerivedBase");
}

int r_query_method(string root, string method) {
   if (root != "base") {
      error("unexpected root: " + dump_value(root));
   }
   return !!function_object("method_" + method, this_object());
}

mixed r_call_method(object ob, string root, string method, mapping args) {
   if (root != "base") {
      error("unexpected root: " + dump_value(root));
   }
   if (function_object("method_" + method, this_object())) {
      return call_other(this_object(), "method_" + method, ob, args);
   }
   error("unknown method: " + dump_value(method));
}


mapping get_configuration() {
   return ([
	      "actualcapacity":
	      ({ C_F_READ, T_FLOAT,
		    "object's actual current capacity" }),
	      "actualdensity":
	      ({ C_F_READ, T_FLOAT,
		    "object's actual current density" }),
	      "actualmass":
	      ({ C_F_READ, T_FLOAT,
		    "object's actual current capacity" }),
	      "actualvolume":
	      ({ C_F_READ, T_FLOAT,
		    "object's actual current capacity" }),
	      "amount":
	      ({ C_F_RW, 0,
		    "the actual amount of this (combinable) object" }),
	      "bright":
	      ({ C_F_READ, T_INT,
		    "is the object's light level bright?" }),
	      "by-weight":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this (non-discrete combinable) object measured in weight rather than volume?" }),
	      "clothesexpected":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this object expected to be dressed?" }),
	      "clothes-worn":
	      ({ C_F_RW, T_OBJECT | C_T_ARRFLAG,
		    "clothes that this object is currently wearing" }),
	      "closeddoors":
	      ({ C_F_READ, T_STRING | C_T_ARRFLAG,
		    "details on this object that are closed doors" }),
	      "combinable":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this object combinable?" }),
	      "dark":
	      ({ C_F_READ, T_INT,
		    "is the object's light level dark?" }),
	      "delays":
	      ({ C_F_READ, C_F_READ | C_T_ARRFLAG,
		    "complex datastructure describing current delayed calls in object" }),
	      "deep-inventory":
	      ({ C_F_READ, T_OBJECT | C_T_ARRFLAG,
		    "all objects recursively contained in object" }),
	      "defaultstance":
	      ({ C_F_RW, T_INT,
		    "stance object takes when dropped" }),
	      "details":
	      ({ C_F_READ | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "this object's details" }),
	      "dim":
	      ({ C_F_READ, T_INT,
		    "is the object's light level dim?" }),
	      "dirs":
	      ({ C_F_READ | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "the directions of the object's exits" }),
	      "disconnectroom":
	      ({ C_F_RW, T_OBJECT,
		    "Where the body was last seen" }),
	      "discrete":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this (combinable) object discrete (three eggs) rather than non-discrete (some beer)?" }),
	      "doors":
	      ({ C_F_READ, T_STRING | C_T_ARRFLAG,
		    "details on this object that are doors" }),
	      "edible":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "can this object be eaten?" }),
	      "environment":
	      ({ C_F_RW, T_OBJECT,
		    "this object's environment" }),
	      "female":
	      ({ C_F_RW, T_INT,
		    "is this object female?" }),
	      "firsturchild":
	      ({ C_F_READ, T_OBJECT,
		    "first entry in this object's list of ur-children" }),
              "flexible":
              ({ C_F_RW | C_F_LOCAL, T_INT,
                         "whether this object is a flexible container" }),
	      "gait":
	      ({ C_F_RW, T_STRING,
		    "this object's gait" }),
	      "gender":
	      ({ C_F_RW, T_INT,
		    "this object's gender" }),
	      "genderstring":
	      ({ C_F_RW, T_STRING,
		    "human-readable description of this object's gender" }),
	      "holder":
	      ({ C_F_READ, T_OBJECT,
		    "first volitional recursive environment; i.e. who holds the pouch containing this object" }),
	      "immobile":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this object unable to move?" }),
	      "integration":
	      ({ C_F_RW | C_F_LOCAL, T_STRING,
		    "a string describing which integration cluster this object belongs to" }),
	      "inventory":
	      ({ C_F_READ, T_OBJECT | C_T_ARRFLAG,
		    "objects contained inside this object" }),
	      "incoming-offers":
	      ({ C_F_READ, T_OBJECT | C_T_ARRFLAG,
		    "objects currently offering us something" }),
	      "initprop-indices":
	      ({ C_F_READ, T_STRING | C_T_ARRFLAG,
		    "list of object's initial properties" }),
	      "intrinsiccapacity":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "maximum volume of this object's contents" }),
	      "intrinsicdensity":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "average density of materials constituting this object" }),
	      "intrinsicmass":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "total mass of materials constituting this object" }),
	      "light":
	      ({ C_F_READ, T_FLOAT,
		    "the perceived amount of light of this object" }),
	      "totalluminosity":
	      ({ C_F_READ, T_FLOAT,
		    "total luminosity of this object" }),
	      "intrinsicluminosity":
	      ({ C_F_READ, T_FLOAT,
		    "intrinsic luminosity of this object" }),
	      "light-level":
	      ({ C_F_READ, T_INT,
		    "the categorized light level of this object; LIGHT_DIM, LIGHT_DARK, etc" }),
	      "light-category":
	      ({ C_F_READ, T_STRING,
		    "human-readable description of light level; 'dim', 'dark', etc" }),
	      "male":
	      ({ C_F_RW, T_INT,
		    "is this object male?" }),
	      "maxfatigue":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "this object's maximum fatigue [largely obsolete]" }),
	      "maxweight":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "total allowed mass of this object's contents" }),
	      "minimumvolume":
	      ({ C_F_RW, T_FLOAT,
		    "calculated volume of this object when empty" }),
	      "neuter":
	      ({ C_F_RW, T_INT,
		    "is this object neuter?" }),
	      "nextursibling":
	      ({ C_F_READ, T_OBJECT,
		    "next sibling in this object's ur-parent's ur-child list" }),
	      "objective":
	      ({ C_F_RW, T_STRING,
		    "objective description of this object's gender, e.g. 'him'" }),
	      "objectname":
	      ({ C_F_RW, T_STRING,
		    "this object's woe name" }),
	      "offered-by":
	      ({ C_F_RW, T_OBJECT,
		    "the object currently offering this object to someone" }),
	      "offered-to":
	      ({ C_F_RW, T_OBJECT,
		    "the object we're currently being offered to" }),
	      "opendoors":
	      ({ C_F_READ, T_STRING | C_T_ARRFLAG,
		    "details on this object that are open doors" }),
	      "outgoing-offers":
	      ({ C_F_READ, T_OBJECT | C_T_ARRFLAG,
		    "objects we're currently offering something" }),
	      "parts-covered":
	      ({ C_F_RW | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "body parts this object covers when worn" }),
	      "possessee":
	      ({ C_F_RW, T_OBJECT,
		    "object we're currently possessing" }),
	      "possessive":
	      ({ C_F_READ, T_STRING,
		    "possessive description of this object's gender, e.g. 'his'" }),
	      "possessor":
	      ({ C_F_RW, T_OBJECT,
		    "object currently possessing us" }),
	      "potable":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "can this object be drunk?" }),
	      "preposition":
	      ({ C_F_RW, T_INT,
		    "this object's prepositional relationship to its prox; PREP_ON, PREP_BEHIND, etc" }),
	      "prepositionstring":
	      ({ C_F_RW, T_STRING,
		    "human-readable description of preposition; 'on', 'behind', etc" }),
	      "previousursibling":
	      ({ C_F_READ, T_OBJECT,
		    "previous sibling in this object's ur-parent's ur-child list" }),
	      "pronoun":
	      ({ C_F_READ, T_STRING,
		    "pronoun description of this object's gender, e.g. 'he'" }),
	      "proximates":
	      ({ C_F_READ, T_OBJECT | C_T_ARRFLAG,
		    "objects currently proximate to this object" }),
	      "proximity":
	      ({ C_F_RW, T_OBJECT,
		    "NRef this object is proximate to" }),
	      "public-container":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "do we display our contents to the public whenever we display them to our holder?" }),
	      "safe":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does logging out of this environment guarantee we'll be here when log in?" }),
	      "scriptrunner":
	      ({ C_F_RW, T_INT,
		    "does this object receieve witness signals?" }),
	      "stance":
	      ({ C_F_RW, T_INT,
		    "this object's current stance; STANCE_KNEEL, STANCE_STAND, etc" }),
	      "stancestring":
	      ({ C_F_RW, T_STRING,
		    "human-readable description of stance; 'kneel', 'stand', etc" }),
	      "strength":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "this object's muscular strength [largely obsolete]" }),
	      "superbright":
	      ({ C_F_READ, T_INT,
		    "is this object's light level superbright?" }),
	      "tight":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "can this object contain non-discrete combinables, e.g. liquids?" }),
	      "transparency":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does this piece of clothing allow other clothes on the same body-parts to show through?" }),
	      "transparent-container":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "can we see through this container even when it's closed?" }),
	      "unsafe":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does logging out of this room always return you to your home room?" }),
	      "urparent":
	      ({ C_F_RW, T_OBJECT,
		    "this object's ur-parent" }),
              "verb2":
              ({ C_F_RW | C_F_LOCAL, T_STRING,
                         "how the describe (second person verb) the contents of this object" }),
              "verb3":
              ({ C_F_RW | C_F_LOCAL, T_STRING,
                         "how the describe (third person verb) the contents of this object" }),
	      "volition":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does this object have volition?" }),
	      "weapon":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "can this object be used as a weapon?" }),
	      "weapons":
	      ({ C_F_READ, T_OBJECT | C_T_ARRFLAG,
		    "objects currently wielded by this object" }),
	      "wielded-by":
	      ({ C_F_RW, T_OBJECT,
		    "object is currently wielding this object" }),
	      "worn-by":
	      ({ C_F_RW, T_OBJECT,
		    "object currently wearing this object" })
	      ]);
}


mixed r_query_property(object ob, string root, string property) {
   string str;

   if (root != "base") {
      return nil;
   }
   if (property == "infravision") {
      return FALSE;
   }

   if (sscanf(property, "prime-detail:%s:local", str)) {
      return ob->get_prime_id(str);
   }
   if (sscanf(property, "prime-detail:%s", str)) {
      return ur_prime_id(ob, str);
   }      
   if (sscanf(property, "parts-covered:%s:local", str)) {
      return member(str, ob->query_parts_covered());
   }
   if (sscanf(property, "parts-covered:%s", str) && str != "local") {
      return member(str, ur_parts_covered(ob));
   }

   if (sscanf(property, "clothes-worn-on-part:%s:local", str) ||
       sscanf(property, "clothes-worn-on-part:%s", str)) {
      return ob->query_worn_on_location(str);
   }

   if (sscanf(property, "initprop:%s", property)) {
      sscanf(property, "%s:local", property);
      return ob->query_initial_property(property);
   }

   if (sscanf(property, "consent:%s", str)) {
      mapping map;

      map = ob->query_consent();
      switch (str) {
      case "list":
	 return map ? map_indices(map) - ({ "all" }) : ({ });
      case "all":
	 return map ? map["all"] : nil;
      case "map":
	 return map ? map - ({ "all" }) : ([ ]);
      case "requests":
	 return ob->query_requests()[..];
      }
   }

   switch(property) {
   case "actualmass":
   case "actualmass:local":
      return ob->query_actual_mass();
   case "actualdensity":
   case "actualdensity:local":
      return ob->query_actual_density();
   case "actualcapacity":
   case "actualcapacity:local":
      return ob->query_actual_capacity();
   case "actualvolume":
   case "actualvolume:local":
      return ob->query_actual_volume();
   case "amount":
   case "amount:local":
      return ob->query_amount();
   case "bright":
   case "bright:local":
      return ob->query_light_category() == LIGHT_BRIGHT;
   case "by-weight":
      return ur_by_weight(ob);
   case "by-weight:local":
      return ob->query_by_weight();
   case "clothesexpected":
      return ur_clothes_expected(ob);
   case "clothesexpected:local":
      return ob->query_clothes_expected();
   case "closeddoors":
   case "closeddoors:local":
      return ob->query_closed_doors();
   case "combinable":
      return ur_combinable(ob);
   case "combinable:local":
      return ob->query_combinable();
   case "clothes-worn":
   case "clothes-worn:local":
      return ob->query_clothes_worn();
   case "dark":
   case "dark:local":
      return ob->query_light_category() == LIGHT_DARK;
   case "deep-inventory":
   case "deep-inventory:local":
      return ob->deep_inventory();
   case "defaultstance":
      return ur_default_stance(ob);
   case "defaultstance:local":
      return ob->query_stance();
   case "delays":
   case "delays:local":
      return ob->query_delayed_calls();
   case "details":
      return ur_details(ob);
   case "details:local":
      return ob->query_details();
   case "dim":
   case "dim:local":
      return ob->query_light_category() == LIGHT_DIM;
   case "dirs":
      return ur_dirs(ob);
   case "dirs:local":
      return ob->query_dirs();
   case "disconnectroom":
      return ob->query_disconnect_room();
   case "discrete":
      return ur_discrete(ob);
   case "discrete:local":
      return ob->query_discrete();
   case "doors":
   case "doors:local":
      return ob->query_doors();
   case "edible":
      return ur_edible(ob);
   case "edible:local":
      return ob->query_edible();
   case "environment":
   case "environment:local":
      return ob->query_environment();
   case "female":
   case "female:local":
      return ob->query_female();
   case "firsturchild":
   case "firsturchild:local":
      return ob->query_first_child();
   case "flexible":
      return ur_flexible(ob);
   case "flexible:local":
      return ob->query_flexible();
   case "gait":
      return ur_gait(ob);
   case "gait:local":
      return ob->query_gait();
   case "gender":
   case "gender:local":
      return ob->query_gender();
   case "genderstring":
   case "genderstring:local":
      return ob->query_gender_string();
   case "holder":
   case "holder:local":
      return ob->query_holder();
   case "immobile":
      return ur_immobile(ob);
   case "immobile:local":
      return ob->query_immobile();
   case "incoming-offers":
   case "incoming-offers:local":
      return map_indices(ob->query_incoming_offers());
   case "integration":
      return ur_integration_property(ob);
   case "integration:local":
      return ob->query_integration_property();
   case "initprop-indices":
   case "initprop-indices:local":
      return ob->query_initial_properties();
   case "intrinsiccapacity":
      return ur_intrinsic_capacity(ob);
   case "intrinsiccapacity:local":
      return ob->query_intrinsic_capacity();
   case "intrinsicdensity":
      return ur_intrinsic_density(ob);
   case "intrinsicdensity:local":
      return ob->query_intrinsic_density();
   case "intrinsicmass":
      return ur_intrinsic_mass(ob);
   case "intrinsicmass:local":
      return ob->query_intrinsic_mass();
   case "inventory":
   case "inventory:local":
      return ob->query_inventory();
   case "light":
   case "light:local":
      return ob->query_light();
   case "intrinsicluminosity":
   case "intrinsicluminosity:local":
      return ob->query_total_luminosity();
   case "totalluminosity":
   case "totalluminosity:local":
      return ob->query_local_light();
   case "light-level":
   case "light-level:local":
      return ob->query_light_category();
   case "light-category":
   case "light-category:local":
      switch(ob->query_light_category()) {
      case LIGHT_DARK:
	 return "dark";
      case LIGHT_DIM:
	 return "dim";
      case LIGHT_BRIGHT:
	 return "bright";
      case LIGHT_SUPERBRIGHT:
	 return "superbright";
      }
      return "error";
   case "male":
   case "male:local":
      return ob->query_male();
   case "maxfatigue":
      return ur_max_fatigue(ob);
   case "maxfatigue:local":
      return ob->query_max_fatigue();
   case "maxweight":
      return ur_max_weight(ob);
   case "maxweight:local":
      return ob->query_max_weight();
   case "minimumvolume":
   case "minimumvolume:local":
      return ob->query_minimum_volume();
   case "neuter":
   case "neuter:local":
      return ob->query_neuter();
   case "nextursibling":
   case "nextursibling:local":
      return ob->query_next_ur_sibling();
   case "objective":
   case "objective:local":
      return ob->query_objective();
   case "objectname":
   case "objectname:local":
      return name(ob);
   case "offered-by":
   case "offered-by:local":
      return ob->query_offered_by();
   case "offered-to":
   case "offered-to:local":
      return ob->query_offered_to();
   case "opendoors":
   case "opendoors:local":
      return ob->query_open_doors();
   case "outgoing-offers":
   case "outgoing-offers:local":
      return map_indices(ob->query_outgoing_offers());
   case "parts-covered":
      return ur_parts_covered(ob);
   case "parts-covered:local":
      return ob->query_parts_covered();
   case "possessee":
   case "possessee:local":
      return ob->query_possessee();
   case "possessor":
   case "possessor:local":
      return ob->query_possessor();
   case "possessive":
   case "possessive:local":
      return ob->query_possessive();
   case "potable":
      return ur_potable(ob);
   case "potable:local":
      return ob->query_potable();
   case "preposition":
   case "preposition:local":
      return ob->query_preposition();
   case "prepositionstring":
   case "prepositionstring:local":
      return ob->describe_preposition();
   case "previousursibling":
   case "previousursibling:local":
      return ob->query_previous_ur_sibling();
   case "pronoun":
   case "pronoun:local":
      return ob->query_pronoun();
   case "proximates":
   case "proximates:local":
      return ob->query_proximate();
   case "proximity":
   case "proximity:local":
      return ob->query_proximity();
   case "public-container":
      return ur_public_container(ob);
   case "public-container:local":
      return ob->query_public_container();
   case "safe":
      return ur_safe(ob);
   case "safe:local":
      return ob->query_safe();
   case "scriptrunner":
   case "scriptrunner:local":
      return ob->query_cached_motion();
   case "stance":
   case "stance:local":
      return ob->query_stance();
   case "stancestring":
   case "stancestring:local":
      return ob->describe_stance();
   case "strength":
      return ur_strength(ob);
   case "strength:local":
      return ob->query_strength();
   case "superbright":
   case "superbright:local":
      return ob->query_light_category() == LIGHT_SUPERBRIGHT;
   case "tight":
      return ur_tight(ob);
   case "tight:local":
      return ob->query_tight();
   case "transparency":
      return ur_transparency(ob);
   case "transparency:local":
      return ob->query_transparency();
   case "transparent-container":
      return ur_transparent_container(ob);
   case "transparent-container:local":
      return ob->query_transparent_container();
   case "unsafe":
      return ur_unsafe(ob);
   case "unsafe:local":
      return ob->query_unsafe();
   case "urparent":
   case "urparent:local":
      return ob->query_ur_object();
   case "verb2":
      return ur_container_verb_second(ob);
   case "verb2:local":
      return ob->query_container_verb_second();
   case "verb3":
      return ur_container_verb_third(ob);
   case "verb3:local":
      return ob->query_container_verb_third();
   case "volition":
      return ur_volition(ob);
   case "volition:local":
      return ob->query_volition();
   case "weapon":
      return ur_weapon(ob);
   case "weapon:local":
      return ob->query_weapon();
   case "weapons":
   case "weapons:local":
      return ob->query_weapons();
   case "wielded-by":
   case "wielded-by:local":
      return ob->query_worn_by();
   case "worn-by":
   case "worn-by:local":
      return ob->query_worn_by();
   }
}

mapping r_query_properties(object ob, string root, int derived) {
   mapping map, config;
   string *ix;
   int i;

   if (root != "base" || !derived) {
      return ([ ]);
   }
   
   config = get_configuration();

   ix = map_indices(config);

   map = ([ ]);

   for (i = 0; i < sizeof(ix); i ++) {
      map[ix[i]] = r_query_property(ob, root, ix[i]);
      if (config[ix[i]][CONFIG_FLAGS] & C_F_LOCAL) {
	 map[ix[i] + ":local"] = r_query_property(ob, root, ix[i] + ":local");
      }
   }

   /* not yet exported: prime-detail:<face> */

   ix = map_indices(ob->query_clothes_worn());
   for (i = 0; i < sizeof(ix); i ++) {
      map[ix[i]] = r_query_property(ob, root, "clothes-worn-on-part:" + ix[i]);
   }

   return map;
}

mixed r_set_property(object ob, string root, string property, mixed value,
		     mixed *retval) {
   string str;
   mixed tmp;

   if (root != "base") {
      return FALSE;
   }
   if (property == "infravision") {
      return FALSE;
   }
   sscanf(property, "%s:local", property);

   if (sscanf(property, "initprop:%s", property)) {
      sscanf(property, "%s:local", property);
      if (value != nil) {
	 ob->set_initial_property(value);
      } else {
	 ob->clear_initial_property(value);
      }
   }
   if (sscanf(property, "parts-covered:%s", str)) {
      string *parts;
      int sz;

      parts = ob->query_parts_covered();
      sz = sizeof(parts);

      if (value) {
	 parts |= ({ str });
      } else {
	 parts -= ({  str });
      }
      if (sz != sizeof(parts)) {
	 /* the array was modified */
	 ob->set_parts_covered(parts);
	 retval[0] = TRUE;
      } else {
	 retval[0] = FALSE;
      }
      return TRUE;
   }
   if (!get_configuration()[property]) {
      /* SysLog("Unknown property being set: " + property); */
      return FALSE;
   }
   switch(property) {
   case "amount":
      ob->set_amount(Flt(value));
      return TRUE;
   case "by-weight":
      ob->set_by_weight(value);
      return TRUE;
   case "clothes-worn":
      if (typeof(value) == T_NIL || (typeof(value) == T_MAPPING && !map_sizeof(value))) {
	 ob->clear_clothes_worn();
      } else {
	 error("Assigning to base:clothes-worn can only be used to clear it entirely.");
      }
      return TRUE;
   case "clothesexpected":
      ob->set_clothes_expected(value);
      return TRUE;
   case "combinable":
      ob->set_combinable(value);
      return TRUE;
   case "defaultstance":
   case "defaultstancestring":
      if (typeof(value) == T_STRING) {
	 switch(lower_case(value)) {
	 case "crouching":
	    value = STANCE_CROUCH;
	    break;
	 case "kneeling":
	    value = STANCE_KNEEL;
	    break;
	 case "lying":
	    value = STANCE_LIE;
	    break;
	 case "sitting":
	    value = STANCE_SIT;
	    break;
	 case "standing":
	    value = STANCE_STAND;
	    break;
	 case "hanging":
	    value = STANCE_HANG;
	    break;
	 case "floating":
	    value = STANCE_FLOAT;
	    break;
	 default:
	    error("stance string must be one of {crouching, kneeling, lying, sitting, standing, hanging, floating}");
	 }
      }
      ob->set_default_stance(value);
      return TRUE;
   case "disconnectroom":
      if (value) {
	 ob->set_disconnect_room(value);
      } else {
	 ob->clear_disconnect_room();
      }
      return TRUE;
   case "discrete":
      ob->set_discrete(value);
      return TRUE;
   case "edible":
      ob->set_edible(value);
      return TRUE;
   case "environment":
      retval[0] = ob->move(value);
      return TRUE;
   case "female":
      ob->set_female();
      return TRUE;
   case "flexible":
      ob->set_flexible(!!value);
      return TRUE;
   case "flushresult":
      ob->flush_result_object();
      return TRUE;
   case "gait":
      ob->set_gait(value);
      return TRUE;
   case "gender":
      ob->set_gender(value);
      return TRUE;
   case "immobile":
      ob->set_immobile(!!value);
      return TRUE;
   case "integration":
      ob->set_integration_property(value);
      return TRUE;
   case "intrinsiccapacity":
      ob->set_capacity(value);
      return TRUE;
   case "intrinsicdensity":
      ob->set_density(value);
      return TRUE;
   case "intrinsicmass":
      ob->set_mass(value);
      return TRUE;
   case "male":
      ob->set_male();	
      return TRUE;
   case "maxfatigue":
      ob->set_max_fatigue(value);
      return TRUE;
   case "maxweight":
      ob->set_max_weight(value);
      return TRUE;
   case "minimumvolume":
      ob->set_minimum_volume(value);
      return TRUE;
   case "neuter":
      ob->set_neuter();
      return TRUE;
   case "objectname":
      ob->set_object_name(value);
      return TRUE;
   case "potable":
      ob->set_potable(value);
      return TRUE;
   case "possessee":
      if (value) {
	 if (ob->query_possessee() != value) {
	    if (ob->query_possessee()) {
	       ob->break_possession();
	    }
	    ob->possess_body(value);
	 }
      } else if (ob->query_possessee()) {
	 ob->break_possession();
      }
      return TRUE;
   case "possessor":
      if (value) {
	 if (ob->query_possessor() != value) {
	    if (ob->query_possessor()) {
	       ob->break_possession();
	    }
	    value->possess_body(ob);
	 }
      } else if (ob->query_possessor()) {
	 ob->break_possession();
      }
      return TRUE;
   case "preposition":
   case "prepositionstring":
      if (typeof(value) == T_STRING) {
	 value = prep_atoi(value);
	 if (value == -1) {
	    error("preposition string must be one of {close to, against, beneath, on, in, near, above, behind, in front of, next to}");
	 }
      }
      ob->set_preposition(value);
      return TRUE;
   case "proximity":
      retval[0] = ob->move(ob->query_environment(), value);
      return TRUE;
   case "public-container":
      ob->set_public_container(value);
      return TRUE;
   case "safe":
      ob->set_safe(value);
      return TRUE;
   case "scriptrunner":
      ob->set_cached_motion(value);
      return TRUE;
   case "stance":
   case "stancestring":
      if (typeof(value) == T_STRING) {
	 switch(lower_case(value)) {
	 case "crouching":
	    value = STANCE_CROUCH;
	    break;
	 case "kneeling":
	    value = STANCE_KNEEL;
	    break;
	 case "lying":
	    value = STANCE_LIE;
	    break;
	 case "sitting":
	    value = STANCE_SIT;
	    break;
	 case "standing":
	    value = STANCE_STAND;
	    break;
	 case "hanging":
	    value = STANCE_HANG;
	    break;
	 case "floating":
	    value = STANCE_FLOAT;
	    break;
	 default:
	    error("stance string must be one of {crouching, kneeling, lying, sitting, standing, hanging, floating}");
	 }
      }
      ob->set_stance(value);
      return TRUE;
   case "strength":
      ob->set_strength(value);
      return TRUE;
   case "tight":
      ob->set_safe(value);
      return TRUE;
   case "transparency":
      ob->set_transparency(value);
      return TRUE;
   case "transparent-container":
      ob->set_transparent_container(value);
      return TRUE;
   case "unsafe":
      ob->set_unsafe(value);
      return TRUE;
   case "urparent":
      ob->set_ur_object(value);
      return TRUE;
   case "verb2":
      ob->set_container_verb_second(value);
      return TRUE;
   case "verb3":
      ob->set_container_verb_third(value);
      return TRUE;
   case "volition":
      ob->set_volition(value);
      return TRUE;
   case "weapon":
      ob->set_weapon(value);
      return TRUE;
   case "worn-by":
   case "wielded-by":
      if (tmp = ob->query_worn_by()) {
	 if (tmp == value) {
	    /* already worn by intended wearer */
	    return TRUE;
	 }
	 /* else unwear */
	 tmp->unwear_article(ob);
      }
      if (typeof(value) == T_OBJECT) {
	 value->add_article(ob);
      }
      return TRUE;
   case "parts-covered": {
      string *parts;
      object parent;
 
       /*
        * Figure out what parts we inherit, so we don't accidentally redundantly
        * add those to the local object, as that is all you can here anyway.
        */
      if (parent = ob->query_ur_object()) {
	 parts = ur_parts_covered(parent);
      } else {
	 parts = ({ });
      }
      ob->set_parts_covered(value - parts);
      return TRUE;
   }
   }
   if (!(get_configuration()[property][CONFIG_FLAGS] & C_F_WRITE)) {
      DEBUG("/base/sys/base: Writing to readonly property \"base:" + property + "\" in " + name(ob) + " from " +
	    name(previous_object(1)));
      return TRUE;
   }
   return FALSE;
}

void
r_clear_all_properties(object ob, string root)
{
    if (root == "base") {
	/* Ain't touching this one, no sir. */
    }
}

/* method implementations */

static
void method_addoffer(object ob, mapping args) {
   ob->add_offer(Arg("what", "base:addoffer"),
		 ObArg("from", "base:addoffer"));
}

static
void method_removeoffer(object ob, mapping args) {
   ob->remove_offer(ObArg("from", "base:removeoffer"));
}

static
void method_removefromoffer(object ob, mapping args) {
   ob->remove_from_offer(ObArg("from", "base:removefromoffer"),
			 ObArg("what", "base:removefromoffer"));
}

static
object *method_getoffersfrom(object ob, mapping args) {
   object *res;

   res = ob->query_incoming_offers()[ObArg("from", "base:getoffersfrom")];

   return res ? copy(res) : ({ });
}

static
object *method_getoffersto(object ob, mapping args) {
   object *res;

   res = ob->query_outgoing_offers()[ObArg("to", "base:getoffersfrom")];

   return res ? copy(res) : ({ });
}

static
void method_spawn_initial_contents(object ob, mapping args) {
   spawn_initial_contents(ob);
}
