/*
**	~CSD/initd.c
**
**	This module handles (C)ombat, (S)kills, and (D)eath
**
**	Copyright Skotos Tech Inc 2001
*/

# include <base.h>
# include <type.h>
# include <kernel/kernel.h>
# include <System.h>
# include <SkotOS.h>

private inherit "/lib/string";
private inherit "/lib/loader";

inherit sequencer "/lib/sequencer";
inherit vaultnode "/usr/SID/lib/vaultnode";

string query_state_root() { return "CSD:Init"; }

static
void create() {
   vaultnode::create("/usr/CSD/data/vault");

   set_object_name("CSD:Init");

   sequencer::create("~CSD/sys/csd",
		     "~CSD/sys/cmds/csd");
   compile_object("~CSD/obj/weapon");
   compile_object("~CSD/obj/attack");

   load_programs("/usr/CSD/samtags");

   claim_node("CSD");

   BASE_INITD->set_runner("combat", find_object("~CSD/sys/csd"));
}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      SysLog("Booting CSD...");
      call_daemons("boot", FALSE);
   }
}

static
void boot_sequence_done() {
}

void prepare_reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      SysLog("Preparing CSD for statedump...");
      call_daemons("prepare_reboot", FALSE);
   }
}

void reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      SysLog("Rebooting CSD...");
      call_daemons("reboot", FALSE);
   }
}
