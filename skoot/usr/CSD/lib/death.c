/*
**	/usr/CSD/lib/death.c
**
**     	This code handles the death of an object.
**
**	Copyright Skotos Tech Inc 2001
*/

# include <nref.h>
# include <base/living.h>

private inherit "/lib/properties";

static
void create() {
   return;
}

/* trigger death script */
void perform_death(object ob) {
   string type;

   if(!ob->query_property("base:death-type")) {
      type = "default-death";
   } else {
      type = ob->query_property("base:death-type");
   }

   SysLog(dump_value(ob) + " is dying a " + type);
   ob->run_signal(type);
}
