/*
**	Provide a clonable version of /lib/n_to_one.c
**
**	Copyright Skotos Tech Inc 2000
**
**	Zell 000826
*/

inherit "/lib/n_to_one";

static
void create(int clone) {
   if (clone) {
      ::create();
   }
}

void die() { destruct_object(); }
