/*
**	/lib/module.c
**
**	Assist modules in doing something if/when a module is
**	loaded. Handy. Check /usr/SAM/initd.c for an example.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/kernel.h>
# include <System/log.h>

static
void create(string mods...) {
   mapping loaded;
   int i, sub;

   loaded = MODULED->query_modules();
   sub = FALSE;

   for (i = 0; i < sizeof(mods); i ++) {
      if (loaded[mods[i]]) {
	 call_out("callback", 0, mods[i]);
      } else {
	 sub = TRUE;
      }
   }
   if (sub) {
      subscribe_event(find_object(MODULED), "module_loaded");
   }
}

static
void callback(string name) {
   call_other(this_object(), name + "_loaded");
}

void evt_module_loaded(object ob, string name) {
   if (previous_program() == AUTO) {
      call_other(this_object(), name + "_loaded");
   }
}
