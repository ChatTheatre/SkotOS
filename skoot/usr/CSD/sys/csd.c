/*
**	/usr/CSD/sys/csd.c
**
**	This is the mighty front-end for the CSD module. All requests
**	for Combat, Skills, and Death will be made through here.
**
**	Copyright 2001, Skotos Tech Inc.
*/

# include <nref.h>

inherit "/lib/string";

inherit combat 			"/usr/CSD/lib/combat";
inherit stats			"/usr/CSD/lib/stats";
inherit death			"/usr/CSD/lib/death";
inherit boolean_skills		"/usr/CSD/lib/boolean-skills";
inherit stepped_skills		"/usr/CSD/lib/stepped-skills";
inherit linear_skills		"/usr/CSD/lib/linear-skills";
inherit non_linear_skills	"/usr/CSD/lib/non-linear-skills";

# define PARSER "/usr/TextIF/sys/parser/basic"

mapping weapon_registry;	/* listing of weapons */

string query_state_root() { return "CSD:Main"; }

void create() {
   stepped_skills::create();
   non_linear_skills::create();
   combat::create();
   stats::create();

   PARSER->register_additional("~CSD/grammar/csd.y");
   PARSER->read_grammar();

   weapon_registry = ([ ]);

   set_object_name("CSD:Main");
}

void patch() {
   weapon_registry = ([ ]);
}

/* base-act death performance front end */
/* death is pretty basic, just a call to bilbo */
void death(object actor) {
   XDebug("Entering death() ----");

   perform_death(actor);
}

/* base-act skill check front end */
/* this will route the info to the appropriate skill check function */
mixed skill(string type, object actor, object target, string skill_name) {

   XDebug("Entering skill_check() ----");

   switch(type) {
   case "stepped": return skill_check_stepped(actor, skill_name);
   case "linear": return skill_check_linear(actor, skill_name);
   case "non-linear": return skill_check_non_linear(actor, target, skill_name);
   }

   return skill_check_boolean(actor, skill_name);
}


/* base-act front end filter */
/* this bad boy filters all the crap sent in from the base-act layer */
/* we will pick out what we like and push it into another mapping */
/* and pass that to the combat sequence */
void combat(mapping args, string act_type) {
   object actor;
   mapping info;

   XDebug("Entering combat() ----");
   XDebug("combat() args: " + dump_value(args));

   info = ([ ]);
   actor = args["actor"];

   /* SOCIAL ACTIONS */
   if (act_type == "social") {
      
      XDebug("combat() act_type is SOCIAL");

      /* extract the actor */
      if (actor) {
	 info["actor"] = actor;
      } else {
	 error("combat: no actor");
      }

      /* extract the actor's target */
      if (args["obj"]) {
	 mixed *obs;

	 if (sizeof(args["obj"]) > 1) {
	    error("too many targets");
	 }

	 obs = args["obj"];
	 
	 if (IsNRef(obs[0])) {
	    info["target"] = NRefOb(obs[0]);
	    info["target-stance"] = query_combat_stance(
	       NRefOb(obs[0])->query_stance());
	 } else {
	    info["target"] = obs[0];
	    info["target-stance"] = query_combat_stance(
	       obs[0]->query_stance());
	 }
      } else {
	 error("combat: no target");
      }

      /* extract the attack and the system */
      if (args["imp"]) {
	 info["system"] = CONFIGD->query("Skill:" + 
					 capitalize(args["imp"]) + 
					 ":System");
      }
      
      /* extract the damage type */
      if (args["damage"]) {
	 info["damage-type"] = args["damage"];
      }

      /* extract the weapon requirement */
      if (args["weapon-required"]) {
	 info["weapon-required"] = args["weapon-required"];
	 info["attack"] = info["damage-type"];
      } else {
	 info["attack"] = args["imp"];
      }
 
      /* insert the action type */
      info["action-type"] = act_type;

      /* determine the weapon type */
      info["weapon-type"] = check_weapon_type(info);
      info["weapon"] = actor->query_property("base:combat:weapon");

      /* store the info for later if in auto mode */
      if (actor->query_property("base:combat:auto-combat")) {
	 actor->set_property("base:combat:next-auto-action", info);

	 if (!is_fighting(actor, info["target"])) {
	    actor->act_fight(info["target"]);
	 }

	 return;
      } else if (!is_fighting(actor, info["target"])){
	 actor->act_fight(info["target"]);
      }

   }

   /* auto combat filtering */
   if (act_type == "auto") {
      object *opponents, weapon;
      mapping tmp, info_tmp;
      
      if (actor) {
	 info_tmp = actor->query_property("base:combat:next-auto-action");
	 if (info_tmp) {
	    info_tmp["action-type"] = "auto";
	    actor->set_property("base:combat:next-auto-action", nil);
	    combat_sequence(info_tmp);
	    return;
	 }

	 info["actor"] = actor;
      } else {
	 error("combat: no actor");
      }

      tmp = info["actor"]->query_property("base:combat:opponents");
      
      if (tmp) {
	 opponents = map_indices(tmp);
      } else {
	 return;
      }

      if (opponents && sizeof(opponents)) {
	 info["target"] = opponents[0];
	 info["target-stance"] = query_combat_stance(
	    opponents[0]->query_stance());
      }

      weapon = info["actor"]->query_property("base:combat:weapon");

      if (weapon) {
	 info["attack"] = query_prime_attack(weapon);
	 info["system"] = "non-linear";
	 info["weapon-required"] = TRUE;
	 info["damage-type"] = info["attack"];
      } else {
	 /* default to punch for now */
	 info["damage-type"] = "bludgeon";
	 info["attack"] = "punch";
	 info["system"] = "non-linear";
	 info["weapon-required"] = FALSE;
      }
      info["weapon-type"] = check_weapon_type(info);
      info["weapon"] = weapon;

      info["action-type"] = act_type;
   }

   XDebug("Calling combat_sequence() with: " + dump_value(info));
   combat_sequence(info);
}
 
mapping query_weapon_registry() {
   return weapon_registry;
}
     
object query_weapon_object(string name) {
   return weapon_registry[name];
}

void set_weapon_object(string name, object weapon) {
   weapon_registry[name] = weapon;
}
