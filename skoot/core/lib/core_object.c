/*
 *	/core/lib/core_object.c
 *
 *	This is the base class of any object that wants to have ur-parents,
 *	properties, derived properties, execute scripts, handle delayed
 *	execution, etc. Known subclassers include
 *
 *		/base/lib/thing
 *		~SkotOS/obj/verb
 *		~SkotOS/obj/meracha.c
 *		/obj/properties.c
 *
 *	Copyright Skotos Tech Inc 2001
 */


inherit dp "./core_dp";
inherit scripts "./core_scripts";

private int creation_stamp;

static
void create() {
   dp :: create();
   scripts :: create();
   creation_stamp = time();
}

int query_creation_stamp() {
   return creation_stamp;
}
