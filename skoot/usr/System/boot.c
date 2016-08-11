/*
**	~System/boot.c
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <System.h>
# include <System/log.h>

inherit access API_ACCESS;
inherit rsrc API_RSRC;
inherit sequencer "/lib/sequencer";

private inherit "/lib/string";

static
void create() {
   string *mods;
   int i;

   rlimits(0; -1) {
      access::create();
      rsrc::create();

      if (mods = SYS_INITD->query_boot_modules()) {
	 for (i = 0; i < sizeof(mods); i ++) {
	    MODULED->load_module(strip(mods[i]));
	 }
      }
      sequencer::create(DEV_USERD, MODULED);
   }
}

void patch() {
   sequencer::create(DEV_USERD, MODULED);
}

void boot() {
   if (previous_program() == SYS_INITD) {
      INFO("Starting system boot...");
      call_daemons("boot", TRUE);
   }
}   

void prepare_reboot() {
   if (previous_program() == SYS_INITD) {
      INFO("Preparing system for statedump...");
      call_daemons("prepare_reboot", FALSE);
   }
}

void reboot() {
   if (previous_program() == SYS_INITD) {
      INFO("Rebooting system...");
      call_daemons("reboot", TRUE);
   }
}

static
void boot_sequence_done() {
   INFO("Boot completed.");
# ifdef __EPP__
   shutdown();
# endif
}

static
void reboot_sequence_done() {
   INFO("Reboot completed.");
}

static
void prepare_reboot_sequence_done() {
   INFO("Prepared for statedump.");
}

