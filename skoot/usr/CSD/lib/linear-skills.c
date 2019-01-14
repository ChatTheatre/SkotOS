/*
**	/usr/CSD/lib/linear-skills.c
**
**	The linear skill checking stuff.
**
**	Copyright Skotos Tech Inc 2001
*/

# include <nref.h>
# include <base.h>
# include "/usr/CSD/include/skills.h"

private inherit "/lib/properties";
private inherit "/lib/string";
private inherit "/lib/random";

int max_linear_skill;

static
void create() {
   return;
}

/* max amount a linear skill can be */
void set_max_linear_skill(int val) {
   max_linear_skill = val;
}

int query_max_linear_skill() {
   return max_linear_skill;
}

/* linear */
int skill_check_linear(object actor, string skill_name) {
   float res;
   mixed skill;

   res = (float)roll(1, max_linear_skill);
   skill = actor->query_property("base:skill:" + skill_name);

   if (skill) {
      if (res <= skill) {
	 return CHECK_SUCCESS;
      }	else {
	 return CHECK_FAILURE;
      }
   }
   
   return CHECK_NOTHING;
}


















