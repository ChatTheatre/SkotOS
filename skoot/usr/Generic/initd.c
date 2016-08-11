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
      spawn_now(0);
      return 300;	/* this could easily take 5 minutes */
   }
}

void archives_parsed() {
   MODULED->daemon_done();
}
