/*
**	/usr/CSD/lib/combat.c
**
**	Combat stuff
**
**	Copyright Skotos Tech Inc 2000
**
**	Initial Revision by Wes Connell
*/

# include <base.h>
# include <nref.h>
# include <base/living.h>
# include "/usr/CSD/include/skills.h"
# include "/usr/CSD/include/csd.h"

private inherit "/lib/properties";
private inherit "/lib/string";
private inherit "/lib/random";

mapping stances;			/* list of stances */
mapping attacks;			/* list of attacks */

/* prototypes */
int skill_check_boolean(object actor, string skill_name);
int skill_check_stepped(object actor, string skill_name);
int skill_check_linear(object actor, string skill_name);
int skill_check_non_linear(object actor, object target, string skill_name);
void perform_death(object ob);
float query_stat(object ob, string stat);
object query_weapon_object(string name);

static
void create() {
   stances = ([ ]);
   return;
}

/* opponent functions */
void clear_opponents(object actor) {
   if (!actor->query_property("base:combat:opponents")) {
      return;
   }
  
   actor->set_property("base:combat:opponents", nil);
}

void set_opponents(object actor, mapping who) {
   actor->set_property("base:combat:opponents", who);
}

mapping query_opponents(object actor) {
   if (!actor->query_property("base:combat:opponents")) {
      return ([ ]);
   }

   return actor->query_property("base:combat:opponents");
}

void add_opponent(object actor, object who) {
   if (!query_opponents(actor)) {
      set_opponents(actor, ([ who : TRUE ]));
      return;
   }

   set_opponents(actor, query_opponents(actor) + ([ who : TRUE ]));
}

void remove_opponent(object actor, object who) {
   mapping tmp;

   if (!query_opponents(actor)) {
      return;
   }

   tmp = query_opponents(actor);
   tmp[who] = nil;

   set_opponents(actor, tmp);
}

/* attack stuff */
mapping query_attacks() {
   return attacks;
}

string *query_attack_list() {
   return map_indices(attacks);
}

/* sorta temp... might be useful later */
string query_first_available_attack() {
   string *list;
   int i;

   list = map_indices(attacks);

   for (i = 0; i < sizeof(list); i++) {
      if (attacks[list[i]] == "available")
	 return list[i];
   }

   /* this should never happen */
   error("no availble attacks");
}

string query_prime_attack(object weapon) {
   object ob, attack;
   string *list, type;
   int i;

   type = weapon->query_property("base:combat:weapon-type");

   if (ob = find_object("CSD:Weapons:" + type)) {
      list = query_attack_list();
      
      for (i = 0; i < sizeof(list); i++) {
	 attack = ob->query_attack(list[i]);
	 
	 if (attack->query_prime()) {
	    return list[i];
	 }
      }
   }
    
   /* try the first available attack */
   return query_first_available_attack();
}

atomic
void add_attack(string name) {
   object master, ob;

   if (master = find_object("~CSD/obj/weapon")) {
      if (function_object("query_clone_count", master) == CLONABLE) {
	 for (ob = master->query_next_clone();
	      ob != master;
	      ob = ob->query_next_clone()) {
	    ob->new_attack(name);
	 }
      }

      attacks[name] = "available";
   }
}

atomic
void remove_attack(string name) {
   object master, ob;

   if (master = find_object("~CSD/obj/weapon")) {
      if (function_object("query_clone_count", master) == CLONABLE) {
	 for (ob = master->query_next_clone();
	      ob != master;
	      ob = ob->query_next_clone()) {
	    ob->remove_attack(name);
	 }
      }

      attacks[name] = nil;
   }
}

atomic
void change_attack(string from, string to) {
   object master, ob;

   attacks[to] = attacks[from];
   attacks[from] = nil;

   if (master = find_object("~CSD/obj/weapon")) {
      if (function_object("query_clone_count", master) == CLONABLE) {
	 for (ob = master->query_next_clone();
	      ob != master;
	      ob = ob->query_next_clone()) {
	    ob->change_attack_name(from, to);
	 }
      }
   }
   
}

void new_attack() {
   add_attack("NEW ATTACK");
}

string query_attack_availability(string name) {
   return attacks[name];
}

void set_attack_availability(string name, string val) {
   if (!val || val == "")
      error("availability must not be empty");

   attacks[name] = val;
}

/* combat stance functions */
mapping query_combat_stances() {
   return stances;
}

string query_combat_stance(int stance) {
   return stances[stance];
}

int *query_combat_stance_list() {
   return map_indices(stances);
}

string *query_combat_stance_strings() {
   mapping tmp;
   string *list;
   int i;

   tmp = ([ ]);
   list = map_values(stances);

   for (i = 0; i < sizeof(list); i++)
      tmp[list[i]] = TRUE;

   return map_indices(tmp);
}

void set_combat_stance(int stance, string name) {
   stances[stance] = name;
}

void remove_combat_stance(int stance) {
   stances[stance] = nil;
}

void new_combat_stance() {
   stances[1000] = "NEW STANCE";
}

/* current combat action */
void set_curr_combat_action(object actor, string action) {
   actor->set_property("base:combat:action", action);
}

string query_curr_combat_action(object actor) {
   if (!actor->query_property("base:combat:action")) {
      return "idle";
   }

   return actor->query_property("base:combat:action");
}

/* is 'actor' fighting 'target'? */
int is_fighting(object actor, object target) {
   mapping opponents;
   int i;
   
   opponents = query_opponents(actor);

   if (opponents && opponents[target]) {
      return TRUE;
   }

   return FALSE;
}

/* calculates skills */
private int calculate_skills(mapping info) {
   object actor, target;

   XDebug("Entering calculate_skills() ----");
   XDebug("calculate_skills() info: " + dump_value(info));

   actor = info["actor"];
   target = info["target"];

   switch(info["system"]) {
   case "stepped":  
      return skill_check_stepped(actor, lower_case(info["attack"]));
   case "linear":     
      return skill_check_linear(actor, lower_case(info["attack"]));
   case "non-linear": 
      SysLog("calculate_skills() info: " + dump_value(info));
      return skill_check_non_linear(actor, target, 
				    lower_case(info["weapon-type"] + ":" +
					       info["damage-type"] + ":" +
					       info["target-stance"]));
   default: return skill_check_boolean(actor, info["attack"]);
   }

   /* shouldn't even get this far */
   return CHECK_NOTHING;
}

/* performs the specified combat action */
private float perform_action(mapping info, int result) {
   object target, actor, weapon_ob;
   float damage, die_mult, die_size;
   string weapon_type;

   XDebug("Entering perform_action() ----");
   
   target = info["target"];
   actor = info["actor"];
   weapon_type = info["weapon-type"];
   weapon_ob = query_weapon_object(weapon_type);

   XDebug("perform_action() result: " + dump_value(result));

   switch(result) {
   case CHECK_SUCCESS:

      XDebug("perform_action() damage type: " + 
	     dump_value(info["damage-type"]));

      switch(info["damage-type"]) {
      case "bludgeon":
	 damage += (query_stat(actor, "aim") - 100.0) / 20.0;
	 damage += (query_stat(actor, "control") - 100.0) / 10.0;
	 damage += (query_stat(actor, "power") - 100.0) / 5.0;
	 break;
      case "pierce":
	 damage += (query_stat(actor, "aim") - 100.0) / 10.0;
	 damage += (query_stat(actor, "control") - 100.0) / 5.0;
	 damage += (query_stat(actor, "power") - 100.0) / 20.0;
	 break;
      case "slash":
	 damage += (query_stat(actor, "aim") - 100.0) / 10.0;
	 damage += (query_stat(actor, "control") - 100.0) / 10.0;
	 damage += (query_stat(actor, "power") - 100.0) / 10.0;
	 break;
      case "restrain":
      default:
	 /* display error */
	 break;
      }

      XDebug("perform_action() damage before any mods: " + dump_value(damage));

      die_mult = weapon_ob->query_die_multiplier();
      die_size = weapon_ob->query_die_size();

      XDebug("perform_action() die_mult: " + dump_value(die_mult) + 
	     " die_size: " + dump_value(die_size));

      damage += (float)(roll((int)die_mult, (int)die_size) + 
	 roll((int)die_mult, (int)die_size));

      XDebug("perform_action() damage after rolls: " + dump_value(damage));

      XDebug("perform_action() opponents durability before hit: " + 
	     query_stat(target, "durability"));

      /* tmp until we decide how to do hit points */
      target->set_property("base:stat:durability",
			   query_stat(target, "durability") - damage);
      
      XDebug("perform_action() opponents durability after hit: " + 
	     query_stat(target, "durability"));

      return damage;	
   case CHECK_FAILURE:
      /* dont hit the fool */
      break;
   default:
      /* output some debug stuff */
      break;
   }
   return 0.0;
}

/* checks each party for death */
private void check_for_death(mapping info, mixed res) {
   object actor, target;
   int death;

   actor = info["actor"];
   target = info["target"];

   if (query_stat(actor, "durability") <= 0.0) {
      perform_death(actor);
      death = TRUE;
   }

   if (query_stat(target, "durability") <= 0.0) {
      perform_death(target);
      death = TRUE;
   }

   if (death) {
      SysLog("INSIDE DEATH");
      actor->set_stance(STANCE_STAND);
      target->set_stance(STANCE_STAND);
      remove_opponent(actor, target);
      remove_opponent(target, actor);
   }

   return;
}

string check_weapon_type(mapping info) {
   object actor, weapon;

   SysLog("CHECK WEAPON TYPE");

   actor = info["actor"];
   weapon = actor->query_property("base:combat:weapon");

   if (!weapon) {
      SysLog("Inside IF");
      return info["attack"];
   }

   return weapon->query_property("base:combat:weapon-type");
}

/* main combat sequence */
void combat_sequence(mapping info) {
   object output, error;
   object actor, target, env;
   int res;
   string output_type;

   XDebug("Entering combat_sequence() ----");

   actor = info["actor"];
   env = actor->query_environment();
   target = info["target"];
   New_Output_Object();

   /* stop automatic combat if we aren't fighting anymore */
   /* there is a wierd bug with actor being nil, it happens when */
   /* the fight is over... for now just end the fight */
   if (!actor || !target)
      return;

   if (!is_fighting(target, actor) || !is_fighting(actor, target)) {
      actor->set_stance(STANCE_STAND);
      target->set_stance(STANCE_STAND);
      actor->stop_busy();
      return;
   }

   /* calculate skills and perform the appropriate action */
   res = calculate_skills(info);
   info["damage-dealt"] = perform_action(info, res);

   /* build the output type */
   if (info["weapon-required"]) {
      output_type = info["damage-type"] + ":" + info["target-stance"];
   } else {
      output_type = info["attack"] + ":" + info["target-stance"];
   }

   switch(res) {
   case CHECK_CRIT_SUCCESS:	output_type += ":criticalsuccess"; break;
   case CHECK_SUCCESS:		output_type += ":success"; break;
   case CHECK_NOTHING:		output_type += ":nothing"; break;
   case CHECK_FAILURE:		output_type += ":failure"; break;
   case CHECK_CRIT_FAILURE:	output_type += ":criticalfailure"; break;
   }

   SysLog("INFO: " + dump_value(info));

   /* build output and broadcast it */
   Set_Output(output_type, "actor", actor);
   Set_Output(output_type, "who", actor);
   Set_Output(output_type, "opponent", target);
   Set_Output(output_type, "target", target);
   Set_Output(output_type, "result", res);
   Set_Output(output_type, "damage", info["damage-dealt"] + 
	      " points of damage");
   Set_Output(output_type, "weapon", info["weapon"]);
   Set_Output(output_type, "weapon-type", info["weapon-type"]);

   /* tmp stuff */
   Set_Output(output_type, "hit-loc", "HIT LOCATION");
   Set_Output(output_type, "hit-loc-poss", "HIT LOCATION POSSESSIVE");
   Set_Output(output_type, "hit-loc-pronoun", "HIT LOCATION PRONOUN");

   Set_Output_Actors(actor, target,
		     env->query_inventory() - ({ actor, target }));
   call_other(actor, "broadcast", "combat", output);

   /* reset the actor's stance */
   actor->set_stance(STANCE_RECOVERING);

   /* is anyone dead yet? */
   check_for_death(info, res);

   /* if auto then call next segment */
   if (info["action-type"] == "auto") {
      call_other(actor, "finish_round");
   }
}  
