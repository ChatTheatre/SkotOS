/*
**	~UserDB/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <System.h>
# include <SkotOS.h>

private inherit "/lib/string";
private inherit "/lib/loader";

inherit sequencer "/lib/sequencer";

inherit vaultnode "/usr/SID/lib/vaultnode";

string query_state_root() { return "UserDB:Init"; }

static
void create() {
   vaultnode::create("/usr/UserDB/data/vault");

   set_object_name("UserDB:Init");

   sequencer::create("~UserDB/sys/userd",
		     "~UserDB/obj/bill",
		     "~UserDB/sys/ctld",
		     "~UserDB/sys/authd",
		     "~UserDB/sys/broadcast");

   load_programs("/usr/UserDB/samtags");

   claim_node("UserDB");
}

void patch() {
  add_daemon("~UserDB/sys/broadcast");
  "/usr/UserDB/sys/broadcast"->boot(FALSE);
}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      SysLog("Booting UserDB...");
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
