/*
**	/base/initd.c
**
**	Compile the base object.
*/

# include <base.h>

inherit vault "~SID/lib/vaultnode";

mapping runners;

static
void create() {
   vault::create("/base/data/vault");

   SysLog("Compiling the Ur Thing...");

   compile_object(OBJ_THING);
   compile_object(OBJ_ERROR);
   compile_object(OBJ_OUTPUT);
   compile_object(ACTIONS);
   compile_object(MATERIALS);
   compile_object(BASE_PROPERTIES);
   compile_object(DETAILS);
   compile_object(CRAFTD);
   compile_object(CRAWLERS);

   runners = ([ ]);
   set_object_name("Base:Init");

   claim_node("Base");
}

void patch_runners() {
   runners = ([ ]);
}

#if 0
void patch() {
   vault::create("/base/data/vault");   
   claim_node("Base");
}
#endif

/* generic runner support */
mapping query_runners() {
   return runners;
}

void set_runner(string type, object handler) {
   SysLog("Base: Setting the " + type + " handler to " + name(handler));
   runners[type] = handler;
}

object query_runner(string type) {
   return runners[type];
}

