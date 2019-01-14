/*
**	/usr/CSD/lib/non-linear-skills.c
**
**	The non-linear skill checking stuff. Non linear skills are pretty
**      specific to combat since they require so many detailed tables.
**
**	Copyright Skotos Tech Inc 2001
*/

# include <nref.h>
# include <base.h>
# include "/usr/CSD/include/skills.h"
# include "/usr/CSD/include/csd.h"

private inherit "/lib/properties";
private inherit "/lib/string";
inherit "/lib/random";

mapping stat_reqs;	/* what stats do what skills use? */

/* prototypes */
float query_stat(object ob, string stat);
object query_weapon_object(string name);

static
void create() {
   stat_reqs = ([ ]);
}

/* convert an unopposed contest value into a modifier */
private float convert_value(float val) {
   return (val - 100.0) * -1.0;
}

/* stat reqs manipulation */
mapping query_stat_reqs() {
   return stat_reqs;
}

string *query_stat_reqs_list() {
   return map_indices(stat_reqs);
}

void clear_stat_reqs() {
   stat_reqs = ([ ]);
}

void set_stat_reqs_entry(string skill_name, string *stats) {
   stat_reqs[skill_name] = stats;
}

string *query_stat_reqs_entry(string skill_name) {
   return stat_reqs[skill_name];
}

void remove_stat_reqs_entry(string skill_name) {
   stat_reqs[skill_name] = nil;
}

/* big ass formula */
private float big_ass_formula(float actor_skill, float target_skill) {
   float skill;

   skill = actor_skill - target_skill;

   return floor(((0.31831 * (atan(0.31831 * 0.1 * skill))) + 0.5) * 100.0);
}

/* non-linear */
int skill_check_non_linear(object actor, object target, 
				   string skill_name) {
   float target_skill, actor_skill, chance, res;
   string *sp, weapon, attack, stance, *stats;
   object weapon_ob, attack_ob;
   int i;

   /* BANE: this is just a tmp work around */
   XDebug("Entering skill_check_non_linear() ----");
   XDebug("skill_check_non_linear() target: " + dump_value(target) + 
	  " skill_name: " + dump_value(skill_name));

   sp = explode(skill_name, ":");
   
   if (sizeof(sp) != 3) {
      error("non linear skill check, bad skill name");
   }

   weapon = sp[0];
   attack = sp[1];
   stance = sp[2];

   weapon_ob = query_weapon_object(weapon);

   if (!weapon_ob)
      error("cannot find weapon: " + weapon + " in registry");

   attack_ob = weapon_ob->query_attack(attack);

   if (!attack_ob)
      error("cannot find attack: " + attack + " in weapon: " + weapon);

   stats = stat_reqs[skill_name];

   if (!stats) {
      error("no stat_reqs entry for " + skill_name);
   }

   /* first lets derive the actor's skill */
   for (i = 0; i < sizeof(stats); i++) {
      actor_skill += query_stat(actor, stats[i]) *
	 attack_ob->query_stat_to_offense_value(stats[i]);
   }

   XDebug("skill_check_non_linear() actor skill before difficulty modifier: " + 
	  dump_value(actor_skill));

   /* next we will subtract the difficulty modifer */
   /* from the current skill */
   actor_skill += convert_value(attack_ob->query_stance_to_diff_value(stance));

   XDebug("skill_check_non_linear() actor skill after difficulty modifier: " + 
	  dump_value(actor_skill));

   /*** calculate the target's defensive skill ***/

   /* first lets derive the target's skill */
   for (i = 0; i < sizeof(stats); i++) {
      target_skill += query_stat(actor, stats[i]) *
	 weapon_ob->query_stat_to_defense_value(stats[i]);
   }

   XDebug("skill_check_non_linear() target skill before defensive modifier: " + 
	  dump_value(target_skill));

   /* next we will subtract the defensive modifer */
   /* from the current skill */

   target_skill += convert_value(
      weapon_ob->query_stance_to_defense_value(stance));

   SysLog("skill_check_non_linear() target skill after defensive modifier: " + 
	  dump_value(target_skill));
      
   /*** push the skills into the BIG ASS FORMULA ***/
   chance = big_ass_formula(actor_skill, target_skill);

   SysLog("skill_check_non_linear() chance: " + dump_value(chance));

   res = (float)roll(1,100);

   SysLog("ROLL: " + res);

   /*** roll a 1d100 against the result ***/
   if (res <= chance) {
      return CHECK_SUCCESS;
   } else {
      return CHECK_FAILURE;
   }
   
   /* this should never happen */
   return CHECK_NOTHING;
}
















