/*
**	/base/lib/living.c
**
**	Contain the code relevant for living objects.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

inherit gender		"./living/gender";
inherit combat		"./living/combat";
inherit properties	"/lib/properties";

static
void create() {
   combat::create();
}

int query_volition() {
   mixed tmp;
   tmp = query_property("volition");

   return typeof(tmp) == T_INT ? tmp : (!!tmp);
}

void set_volition(int i) {
   /* volition = i; */
   set_property( "volition", i );

   if (i) {
      set_property("Skill:ThrustOffensive", 3);
      set_property("Skill:CutOffensive", 3);
      set_property("Skill:FeintOffensive", 0);
      set_property("Skill:GuardDefensive", 3);
      set_property("Skill:SaluteDefensive", 0);
      set_property("Skill:RecoverDefensive", 2);
      set_property("Skill:FumbleDefensive", 0);
      set_property("Skill:DodgeDefensive", 1);
      set_property("Skill:AdvanceDefensive", 2);
      set_property("Skill:RetireDefensive", 2);
      set_property("Skill:RestDefensive", 1);
      set_property("Skill:AttackingDefensive", 0);
      set_property("Skill:IdleDefensive", 0);
   } else {
      clear_property("Skill:ThrustOffensive");
      clear_property("Skill:CutOffensive");
      clear_property("Skill:FeintOffensive");
      clear_property("Skill:GuardDefensive");
      clear_property("Skill:SaluteDefensive");
      clear_property("Skill:RecoverDefensive");
      clear_property("Skill:FumbleDefensive");
      clear_property("Skill:DodgeDefensive");
      clear_property("Skill:AdvanceDefensive");
      clear_property("Skill:RetireDefensive");
      clear_property("Skill:RestDefensive");
      clear_property("Skill:AttackingDefensive");
      clear_property("Skill:IdleDefensive");
   }
}

