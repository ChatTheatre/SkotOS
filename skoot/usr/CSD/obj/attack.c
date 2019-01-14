/*
**	/usr/CSD/obj/attack.c
**
**	Each weapon will have its own version of each attack type (one of
**	these objects) inserted into it.
**
**	Skotos Tech Inc. 2001
*/

# include "/usr/CSD/include/csd.h"

string name;				/* attack name */
float armor_mod;			/* how well I do with this weapon */
mapping stance_to_diff;			/* stance to skill difficulty */
mapping stat_to_offense;		/* offensive skill table */
int prime;				/* prime attack */

nomask string query_state_root() { return "CSD:Attack"; }

static
void create(int clone) {
   ::create();

   if (!clone) {
      set_object_name("CSD:Attack");
   }

   stance_to_diff = ([ ]);
   stat_to_offense = ([ ]);
}

nomask
void die() {
   destruct_object();
}

nomask
void suicide() {
   call_out("die", 0);
}

/* lists */
string *query_stances() { return CSD->query_combat_stance_strings(); }
string *query_stats()   { return CSD->query_stat_list(); }

/* name */
void set_name(string val) {
   if (name)
      CSD->change_attack(name, val);
   name = val;
}

string query_name() {
   return name;
}

/* prime attack */
void set_prime(int val) {
   if (val) {
      prime = TRUE;
      return;
   }
   
   prime = FALSE;
}

int query_prime() {
   return prime;
}

/* armor mod */
void set_armor_mod(float val) {
   armor_mod = val;
}

float query_armor_mod() {
   return armor_mod;
}

/* stance to difficulty */
mapping query_stance_to_diff() {
   return stance_to_diff;
}

float query_stance_to_diff_value(string stance) {
   if (!stance_to_diff[stance])
      return 0.0;

   return stance_to_diff[stance];
}

void set_stance_to_diff_value(string stance, float val) {
   stance_to_diff[stance] = val;
}

/* offensive skill table */
mapping query_stat_to_offense() {
   return stat_to_offense;
}

float query_stat_to_offense_value(string stat) {
   if (!stat_to_offense[stat]) 
      return 0.0;

   return stat_to_offense[stat];
}

void set_stat_to_offense_value(string stat, float val) {
   stat_to_offense[stat] = val;
}


