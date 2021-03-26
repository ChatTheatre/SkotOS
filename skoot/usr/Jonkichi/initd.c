/*
 * ~Jonkichi/initd.c
 *
 * Copyright Skotos Tech Int 2006
 */
# include <System.h>
# include <SkotOS.h>
# include <UserAPI.h>

private inherit        "/lib/loader";
        inherit module "/lib/module";
        inherit        "/usr/SkotOS/lib/merryapi";
        inherit        "/usr/SMTP/lib/smtp_api";

static void create() {
    set_object_name("Jonkichi:Init");
    module::create("SkotOS");
}

void patch() {
    set_object_name("Jonkichi:Init");
}

static void SkotOS_loaded() {
}

int query_method(string method) {
    return !!function_object("merry_" + method, this_object());
}

mixed call_method(string method, mapping args) {
    return call_other(this_object(), "merry_" + method, args);
}

static mixed merry_sendmail(mapping args) {
    send_message(args["envelope"],
		 args["from_name"],
		 args["from_address"],
		 ([ args["to_address"]: args["to_name"] ]),
		 ([ ]),
		 args["subject"],
		 ({ }),
		 args["message"]);
    return TRUE;
}
