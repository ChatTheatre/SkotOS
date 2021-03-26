/*
 *	~SkotOS/sys/api.c
 *
 *	A collection of Merry calls into LPC/UserAPI land that don't
 *	seem to fit anywhere else.
 *
 *	Copyright Skotos Tech Inc 2005-2006
 */

# include <type.h>
# include <SkotOS.h>
# include <UserAPI.h>

inherit authapi "/usr/UserAPI/lib/authapi";
inherit ctrlapi "/usr/UserAPI/lib/ctlapi";
inherit         "/usr/SkotOS/lib/merryapi";

private mapping notifiers;

static void create() {
   set_object_name("SkotOS:API");

   authapi::create();
   ctrlapi::create();

   notifiers = ([ ]);
}

atomic void patch() {
    notifiers = ([ ]);
}

void register_notifier(object obj, string func)
{
    notifiers[obj] = func;
}

void unregister_notifier(object obj)
{
    notifiers[obj] = nil;
}

int query_method(string method) {
    return !!function_object("merry_" + method, this_object());
}

mixed call_method(string method, mapping args) {
    return call_other(this_object(), "merry_" + method, args);
}

static mixed merry_query_storypoints(mapping args) {
    if (typeof(args["name"]) != T_STRING) {
	error("query_storypoints expects $name");
    }
    if (typeof(args["call_func"]) != T_STRING) {
	error("query_storypoints expects $call_func");
    }
    if (typeof(args["call_obj"]) != T_OBJECT) {
	error("query_storypoints expects $call_obj");
    }
    start_getprop("delayed_query_storypoints", args["name"], "storypoints:available",
		  args["call_func"], args["call_obj"], args[..]);
    return nil;
}

static void delayed_query_storypoints(int success, string val, string call_func, object call_obj, mapping args) {
    run_merry(call_obj, call_func, "callback", args + ([ "success": success, "value": success ? (int)val : 0 ]));
}

static mixed merry_apply_storypoints(mapping args) {
    if (typeof(args["name"]) != T_STRING) {
	error("apply_storypoints expects $name");
    }
    if (typeof(args["amount"]) != T_INT) {
	error("apply_storypoints expects $amount");
    }
    if (args["amount"] < 1) {
	error("apply_storypoints expects $amount to be larger than zero");
    }
    if (typeof(args["reason"]) != T_STRING) {
	error("apply_storypoints expects $reason");
    }
    if (args["theatre"] && typeof(args["theatre"]) != T_STRING) {
	error("apply_storypoints $theatre to be nil or a string");
    }
    if (typeof(args["call_func"]) != T_STRING) {
	error("apply_storypoints expects $call_func");
    }
    if (typeof(args["call_obj"]) != T_OBJECT) {
	error("apply_storypoints expects $call_obj");
    }
    start_sps_use("delayed_apply_storypoints", args["name"], args["amount"], args["reason"], args["theatre"],
		  args["call_func"], args["call_obj"], args[..]);
    return nil;
}

static void delayed_apply_storypoints(int success, string val, string call_func, object call_obj, mapping args) {
    int i, sz;
    object *list;
    string *funcs;

    if (success) {
	sz = map_sizeof(notifiers);
	list = map_indices(notifiers);
	funcs = map_values(notifiers);
	for (i = 0; i < sz; i++) {
	    call_other(list[i], funcs[i], "<" + name(args["this"]) + ">", args["name"], args["amount"], args["reason"]);
	}
    }
    run_merry(call_obj, call_func, "callback", args + ([ "success": success, "value": val ]));
}

mapping
query_notifiers()
{
    return notifiers[..];
}

int merry_query_badname(mapping args)
{
    if (typeof(args["name"]) != T_STRING) {
	error("query_badname expects $name");
    }
    if (find_object(STARTSTORY)) {
	return !!STARTSTORY->query_badname("\"" + args["name"] + "\"");
    }
    return FALSE;
}

object *merry_help_match_keyword(mapping args)
{
    string id, key;
    object *obarr;

    if (typeof(args["keyword"]) != T_STRING) {
	return ({ });
    }
    if (args["id"] && typeof(args["id"]) != T_STRING) {
	return ({ });
    }
    key = args["keyword"];
    id = args["id"];
    obarr = HELPD->match_keyword(key);
    if (!obarr || !sizeof(obarr)) {
	return ({ });
    }
    if (id) {
	int i, sz;
	object *tarr, *garr;

	sz = sizeof(obarr);
	tarr = allocate(sz);
	garr = allocate(sz);
	for (i = 0; i < sz; i++) {
	    if (obarr[i]->query_property("theatre:id") == id) {
		tarr[i] = obarr[i];
	    } else if (!obarr[i]->query_property("theatre:id")) {
		garr[i] = obarr[i];
	    }
	}
	tarr -= ({ nil });
	garr -= ({ nil });
	if (sizeof(tarr) > 0) {
	    obarr = tarr;
	} else {
	    obarr = garr;
	}
    } else {
	int i, sz;

	sz = sizeof(obarr);
	for (i = 0; i < sz; i++) {
	    if (obarr[i]->query_property("theatre:id")) {
		obarr[i] = nil;
	    }
	}
	obarr -= ({ nil });
    }
    return obarr;
}

static mixed merry_accountping(mapping args) {
    if (typeof(args["name"]) != T_STRING) {
	error("query_storypoints expects $name");
    }
    if (typeof(args["call_func"]) != T_STRING) {
	error("query_storypoints expects $call_func");
    }
    if (typeof(args["call_obj"]) != T_OBJECT) {
	error("query_storypoints expects $call_obj");
    }
    start_accountping("delayed_accountping", args["name"], args[..]);
    return nil;
}

static void delayed_accountping(int success, string val, mapping args)
{
    int    next_stamp;
    string account_type, account_flags;

    if (success) {
	object udat;

	udat = UDATD->get_udat(args["name"]);
	sscanf(val, "%d (%s;%s)", next_stamp, account_type, account_flags);
	udat->update_account(account_type, explode(account_flags, " "));
    }
    run_merry(args["call_obj"], args["call_func"], "callback",
	      args + ([ "success": success,
			"next_stamp": success ? next_stamp :
			              (val == "TIMEOUT" ? -1 : 0) ]));
}
