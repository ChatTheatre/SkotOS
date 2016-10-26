/*
**	~SkotOS/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <System.h>
# include <SkotOS.h>
# include <SAM.h>
# include <base.h>

private inherit "/lib/string";
private inherit "/lib/loader";

private inherit "/base/lib/toolbox";

inherit sequencer "/lib/sequencer";

inherit vaultnode "/usr/SID/lib/vaultnode";

mapping tick_average;
mapping tick_count;
mapping tick_max;
mapping tick_min;

static void setup_sequencer();

string query_state_root() { return "SkotOS:Init"; }

static
void create() {
   vaultnode::create("/usr/SkotOS/data/vault");

   set_object_name("SkotOS:Init");
   compile_object("/data/data");

   sequencer::create("~SkotOS/sys/logd",
		     "~SkotOS/sys/bilbo",
		     "~SkotOS/sys/tip",
		     "~SkotOS/sys/npc-bodies",  
		     "~SkotOS/sys/npc-brains",
		     "~SkotOS/sys/lang",
		     "~SkotOS/sys/cmemories",
		     "~SkotOS/sys/helpd",
		     "~SkotOS/sys/merry",
		     "~SkotOS/obj/canode",
		     "~SkotOS/sys/assistd",
		     "~SkotOS/sys/startstory",
		     "~SkotOS/sys/socials",
		     "~SkotOS/sys/infoline",
		     "~SkotOS/sys/dtd",
		     "~SkotOS/sys/status",
		     "~SkotOS/sys/profiler");

   load_programs("/usr/SkotOS/samtags");
   load_programs("/usr/SkotOS/data");
   load_programs("/usr/SkotOS/obj");
   load_programs("/usr/SkotOS/sys");

   claim_node("SkotOS");
}

void patch()
{
   add_daemon(MPROFILER);
}

static
void setup_sequencer() {
}

int boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      INFO("Booting SkotOS...");
      call_daemons("boot", FALSE);
      spawn_now(0);
      return 600;	/* allow 10 minutes */
   }
}

void archives_parsed() {
   MODULED->daemon_done();
}

static
void boot_sequence_done() {
   BASE_INITD->set_runner("event", find_object(SAMD));
}

void prepare_reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      setup_sequencer();
      call_daemons("prepare_reboot", FALSE);
   }
}

void reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      setup_sequencer();
      call_daemons("reboot", FALSE);
   }
}

void clear_averages() {
   tick_average = ([ ]);
   tick_count = ([ ]);
   tick_max = ([ ]);
   tick_min = ([ ]);
}

void collect_tick_stats(int size, int ticks) {
   if (!tick_max[size] || ticks > tick_max[size]) {
      tick_max[size] = ticks;
   }
   if (!tick_min[size] || ticks < tick_min[size]) {
      tick_min[size] = ticks;
   }
   if (!tick_average[size] || !tick_count[size]) {
      tick_average[size] = ticks;
      tick_count[size] = 1;
   }
   if (tick_count[size] > 500) {
      /* enough! */
      return;
   }
   tick_average[size] =
      ((tick_average[size] * tick_count[size]) + ticks) /
      (tick_count[size] + 1);
   tick_count[size] ++;
}


mapping query_averages() {
   return tick_average;
}

mapping query_counts() {
   return tick_count;
}

mapping query_maxes() {
   return tick_max;
}

mapping query_mins() {
   return tick_min;
}
