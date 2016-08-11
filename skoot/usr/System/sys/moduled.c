/*
**	~System/sys/module.c
**
**	Handle the on-demand loading of modules.
**
**	There is as yet no unload_module(), which would at the very
**	least require in-memory dependency information and a bunch of
**	checks before the unload is allowed -- e.g. no clones and no
**	objects must be owned by the module user...
**
**	We can add this functionality later; since we have the .mod
**	files, we can always reconstruct the information of a running
**	system later.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <System/log.h>

inherit rsrc	API_RSRC;
inherit access	API_ACCESS;

inherit seq	"/lib/sequencer";
inherit "/lib/string";

mapping modules;
mapping loading;

static
void create() {
   rsrc::create();
   access::create();
   seq::create();

   modules = ([ ]);
   loading = ([ ]);

   add_event("module_loaded");
}

int allow_subscribe(object ob, string name) {
   return TRUE;
}

void patch() {   add_event("module_loaded");
}


mapping query_modules() {
   return modules[..];
}

/* make sure the module pointed at by base_path is loaded */

atomic
void load_module(string name) {
   string text, *lines;
   string init;
   int i;

   if (modules[name]) {
      /* already initialized */
      return;
   }
   if (loading[name]) {
      error("MODULE ERROR: Dependency cycle found loading " + name);
   }
   loading[name] = TRUE;

   text = read_file("/data/modules/" + name + "/" + name + ".mod");
   if (!text) {
      error("cannot find required module " + name);
   }
   lines = explode(text, "\n");
   INFO("Loading module " + name);
   for (i = 0; i < sizeof(lines); i ++) {
      string var, val;

      if (!strlen(lines[i]) || lines[i][0] == '#') {
	 continue;
      }
      if (sscanf(lines[i], "%s:%s", var, val)) {
	 switch(lower_case(strip(var))) {
	 case "dep": {
	    string *mods;
	    int j;

	    /* ensure module dependencies satisifed */
	    mods = explode(val, " ");
	    for (j = 0; j < sizeof(mods); j ++) {
	       mods[j] = strip(mods[j]);
	       INFO("Attempting to satisfy dependency of " + name + " on " + mods[j]);
	       load_module(mods[j]);
	    }
	 }
	 break;
	 case "init":
	    init = strip(val);
	    break;
	 }
      }
   }
   modules[name] = TRUE;

   if (init) {
      string user;

      if (sscanf(init, "/usr/%s/", user)) {
	 add_owner(user);
	 add_user(user);
	 /* TODO: TEMPORARY */
	 set_access(user, "/", FULL_ACCESS);
	 set_global_access(user, TRUE);
      }
      find_or_load(init);

      modules[name] = init;
      add_daemon(init);
   }
   event("module_loaded", name);
   INFO(name + " loaded successfully.");
   loading[name] = nil;
}

static
mixed call_daemon(object obj, string fun, int blocking) {
   rlimits(0; -1) {
      return call_other(obj, fun, blocking);
   }
}

void boot_module(string module) {
   if (modules[module]) {
      rlimits(0; -1) {
	 modules[module]->boot(FALSE);
      }
   }
}

atomic
int boot(int block) {
# ifndef __EPP__
   INFO("Booting modules...");
   call_daemons("boot", block);
# endif
   return 3600;	/* give modules an entire hour to start up */
}

static
void boot_sequence_done() {
   SYS_BOOT->daemon_done();
}

atomic
int reboot(int block) {
   INFO("Rebooting modules...");
   call_daemons("reboot", block);
   return 1;
}

static
void reboot_sequence_done() {
   SYS_BOOT->daemon_done();
}

atomic
int prepare_reboot(int block) {
   INFO("Preparing modules for statedump...");
   call_daemons("prepare_reboot", block);
   return 1;
}

static
void prepare_reboot_sequence_done() {
   SYS_BOOT->daemon_done();
}
