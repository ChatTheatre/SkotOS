/*
**	/usr/CSD/obj/weapon.c
**
**	Every weapon type will have one of these objects. All info about
**	that specific weapon type will be stored in here.
**
**	Skotos Tech Inc. 2001
*/

# include "/usr/CSD/include/csd.h"

string name;				/* weapon name */
float die_multiplier;			/* damage die multiplier */
float die_size;				/* damage die size */
mapping stance_to_defense;		/* stance to defensive modifier */
mapping stat_to_defense;		/* defensive skill table */

mapping attack_list;	/* these are the various attacks used and their 
			   related info */

nomask string query_state_root() { return "CSD:Weapon"; }

void patch() {
   CSD->set_weapon_object(name, this_object());
}

static
void create(int clone) {
   ::create();

   if (!clone) {
      set_object_name("CSD:Weapon");
   }

   stance_to_defense = ([ ]);
   stat_to_defense = ([ ]);
   attack_list = ([ ]);
}

nomask
void die() {
   object *attacks;
   int i;

   /* first we need to kill all the attack objects I have */
   attacks = map_values(attack_list);
   
   for (i = 0; i < sizeof(attacks); i++) {
      attacks[i]->suicide();
   }

   /* now lets kill ourself */
   destruct_object();
}   

nomask
void suicide() {
   call_out("die", 0);
}

/* lists */
string *query_attacks() { return CSD->query_attack_list(); }
string *query_stances() { return CSD->query_combat_stance_strings(); }
string *query_stats()   { return CSD->query_stat_list(); }

/* name */
void set_name(string val) {
   name = val;
   set_object_name("CSD:Weapons:" + val);
   CSD->set_weapon_object(val, this_object());
}

string query_name() {
   return name;
}

/* die multiplier */
void set_die_multiplier(float val) {
   die_multiplier = val;
}

float query_die_multiplier() {
   return die_multiplier;
}

/* die size */
void set_die_size(float val) {
   die_size = val;
}

float query_die_size() {
   return die_size;
}

/* stance to defense */
mapping query_stance_to_defense() {
   return stance_to_defense;
}

float query_stance_to_defense_value(string stance) {
   if (!stance_to_defense[stance])
      return 0.0;

   return stance_to_defense[stance];
}

void set_stance_to_defense_value(string stance, float val) {
   stance_to_defense[stance] = val;
}
   
/* attacks */
mapping query_attack_list() {
   return attack_list;
}

object query_attack(string attack_name) {
   return attack_list[attack_name];
}

void set_attack(string attack_name, object attack) {
   attack_list[attack_name] = attack;
}

void change_attack_name(string from, string to) {
   if (!attack_list[from]) 
      error("former attack does not exist");

   if (attack_list[to])
      error("new attack already exists");

   attack_list[to] = attack_list[from];
   attack_list[from] = nil;
}

void remove_attack(string attack_name) {
   if (!attack_list[attack_name])
      error("attack does not exist");

   attack_list[attack_name]->suicide();
}

void new_attack(string attack_name) {
   object ob;

   if (attack_list[attack_name])
      error("attack already exists");

   ob = clone_object(ATTACK);
   
   if (!ob) 
      error("could not clone attack");

   attack_list[attack_name] = ob;
   ob->set_name(attack_name);
}

/* defense skill table */
mapping query_stat_to_defense() {
   return stat_to_defense;
}

float query_stat_to_defense_value(string stat) {
   if (!stat_to_defense[stat])
      return 0.0;

   return stat_to_defense[stat];
}

void set_stat_to_defense_value(string stat, float val) {
   stat_to_defense[stat] = val;
}
