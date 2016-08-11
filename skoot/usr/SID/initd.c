# include <SID.h>

private inherit "/lib/loader";
inherit sequencer "/lib/sequencer";
inherit module "/lib/module";

static
void create() {
   sequencer::create("/usr/SID/sys/sid", "/usr/SID/sys/vault");
   module::create("SAM");
}

static
void SAM_loaded() {
   load_programs("/usr/SID/samtags");
}

int boot(int block) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("boot", TRUE);
      return 120;
   }
}

void boot_sequence_done() {
   MODULED->daemon_done();
}
