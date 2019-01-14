/*
**	/usr/CSD/lib/stepped-skills.c
**
**	The stepped skill checking stuff.
**
**	Copyright Skotos Tech Inc 2001
*/

# include <nref.h>
# include <base.h>
# include "/usr/CSD/include/skills.h"

private inherit "/lib/properties";
private inherit "/lib/string";
private inherit "/lib/random";

mapping steps;

static
void create() {
   steps = ([ ]);
}

/* steps mapping manipulation */
mapping query_skill_steps() {
   return steps;
}

float *query_skill_steps_values() {
   return map_indices(steps);
}

void clear_skill_steps() {
   steps = ([ ]);
}

void new_skill_step() {
   steps[0.0] = "NEW";
}

void set_skill_step(float val, string name) {
   steps[val] = name;
}

void remove_skill_step(float val) {
   steps[val] = nil;
}

string query_skill_step_name(float val) {
   return steps[val];
}

/* stepped */
int skill_check_stepped(object actor, string skill_name) {
   float res;
   mixed skill;

   res = (float)roll(1,sizeof(map_indices(steps)));
   skill = actor->query_property("base:skill:" + skill_name);

   if (skill) {
      if (res <= skill) {
	 return CHECK_SUCCESS;
      } else {
	 return CHECK_FAILURE;
      }
   }

   return CHECK_NOTHING;
}



















