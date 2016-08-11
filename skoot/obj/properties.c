/*
**	/obj/properties.c
**
**	Inherit /lib/properties.c and be clonable.
**
**	Copyright Skotos Tech Inc 1999
**
*/

# include <base.h>
# include <core.h>
# include <SkotOS.h>

inherit notes      "/lib/notes";
inherit properties "/core/lib/core_object";

string query_state_root() { return "Core:PropertyContainer"; }

static void run_start_scripts();
static void run_stop_scripts();

string query_script_registry() {
   return BASE_INITD;
}

string query_dp_registry() {
   return CORE_DP_REGISTRY;
}

static
void create(int clone) {
   notes      :: create();
   properties :: create();
   if (!clone) {
      set_object_name("Core:PropertyContainer");
   } else {
      call_out("run_start_scripts", 0);
   }
}

void suicide() {
   call_out("die", 0);
}

void die() {
   destruct_object();
}

static
void destructive_desire() {
   properties :: destructive_desire();

   catch {
      run_stop_scripts();
   }
}

private
int run_signal(string signal, varargs NRef target, mapping args) {
   return call_runners("run_signal", signal, target, args, ({ }));
}

static
void run_start_scripts() {
   run_signal("start", this_object());
}

static
void run_stop_scripts() {
   run_signal("stop", this_object());
}

void patch_notes() {
   notes :: create();
}
