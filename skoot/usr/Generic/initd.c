/*
**	~Generic/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <System.h>

inherit vault "/usr/SID/lib/vaultnode";

static
void create() {
   vault::create("/usr/Generic/data/vault");

   set_object_name("Generic:Init");

   claim_node("Generic");
}


int boot(int block) {
   if (ur_name(previous_object()) == MODULED) {
      "~System/sys/tool/vault"->spawn_subdir(nil);
      "~System/sys/tool/vault"->load_spawned_subdir("DevSys");
      "~System/sys/tool/vault"->load_spawned_subdir("Shared:sys");
      "~System/sys/tool/vault"->load_spawned_subdir("SkotOS");
      "~System/sys/tool/vault"->load_spawned_subdir(nil);
      "~System/sys/tool/vault"->queue_signal("archives_parsed");
      return 600;	/* this could easily take 10 minutes */
   }
}

void archives_parsed() {
   MODULED->daemon_done();
}
