/*
**	/base/lib/living/combat.c
**
**	Combat stuff
**
**	Copyright Skotos Tech Inc 2000
**
**	Initial Revision by Wes Connell
*/

# include <nref.h>

inherit "/lib/properties";

private int strength;		/* does this really need a comment? */
private float flt_str;		/* converting strength to float */
private int max_fatigue;	/* maximum fatigue level */

nomask NRef query_proximity();
string get_prime_id(string side);

static
void create() {
   flt_str = 1.0;
   max_fatigue = 1;
}

/* strength functions */
void set_strength(float value) {
   flt_str = value;
}

float query_strength() {
   if (!flt_str) {
      flt_str = (float) strength;
   }
   return flt_str;
}

void set_max_fatigue(mixed value) {
   set_property("skill:fatigue", (float)value);
}

float query_max_fatigue() {
   mixed value;

   value = query_property("skill:fatigue");
   if (value != nil) {
       return (float)value;
   }

   /* Conversion code */
   set_max_fatigue(max_fatigue);
   return (float)max_fatigue;
}
