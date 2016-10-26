/*
**	~HTTP/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <System.h>
# include <base.h>

private inherit "/lib/loader";
inherit sequencer "/lib/sequencer";
inherit module "/lib/module";


static
void create() {
   sequencer::create("~HTTP/sys/httpd");
   module::create("SAM");

   set_object_name("HTTP:Init");
}

static
void SAM_loaded() {
   load_programs("/usr/HTTP/samtags");
}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      INFO("Booting HTTP...");
      call_daemons("boot", FALSE);
   }
}

static
void boot_sequence_done() {
}

void prepare_reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("prepare_reboot", FALSE);
   }
}

void reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("reboot", FALSE);
   }
}
