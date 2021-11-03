/*
** ~AVChat/initd.c
**
** Copyright ChatTheatre, 2021
*/

# include <System.h>
# include <SkotOS.h>

inherit sequencer "/lib/sequencer";

static
void create() {
   set_object_name("AVChat:Init");

   sequencer::create("~AVChat/sys/avchat",
                     "~AVChat/sys/avchat_port");

}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      SysLog("Booting AVChat...");
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
