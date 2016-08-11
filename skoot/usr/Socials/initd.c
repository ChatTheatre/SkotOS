# include <SkotOS.h>

inherit "/usr/SID/lib/vaultnode";
inherit "/usr/Socials/lib/verb_loader";

# define SOC_STARTUP	"/usr/Socials/data"

static
void create() {
   ::create("/usr/Socials/data/vault");

   claim_node("Socials");
}

int boot(int boot) {
   spawn_now(0);
   return 600;	/* allow 10 minutes */
}

void archives_parsed() {
   MODULED->daemon_done();
}
