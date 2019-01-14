/*
**	/usr/CSD/lib/boolean-skills.c
**
**	The boolean skill stuff. Pretty simple.
**
**	Copyright Skotos Tech Inc 2001
*/

# include <nref.h>
# include <base.h>
# include "/usr/CSD/include/skills.h"

private inherit "/lib/properties";
private inherit "/lib/string";
private inherit "/lib/random";

static
void create() {
   return;
}

/* boolean */
int skill_check_boolean(object actor, string skill_name) {
   if (actor->query_property("base:skill:" + skill_name)) {
      return CHECK_SUCCESS;
   } 

   return CHECK_FAILURE;
}


















