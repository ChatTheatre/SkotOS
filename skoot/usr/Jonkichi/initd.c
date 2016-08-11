/*
 * ~Jonkichi/initd.c
 *
 * Copyright Skotos Tech Int 2006
 */

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
    AUTHD->set_auth_hook(this_object());
}

mixed hook_passlogin(string user, string pass) {
    int result;
    mapping args;

    args = ([ "user": user, "pass": pass ]);
    result = run_merry(find_object("Jonkichi:Lib:UserDB"), "passlogin", "lib", args);

    return ({ result, args["info"] });
}

mixed hook_keycodeauth(string user, string code) {
    int result;
    mapping args;

    args = ([ "user": user, "code": code ]);
    result = run_merry(find_object("Jonkichi:Lib:UserDB"), "keycodeauth", "lib", args);

    DEBUG("hook_keycodeauth(" + dump_value(user) + ", " + dump_value(code) + ") => " + dump_value(args));

    return ({ result, args["info"] });
}

mixed hook_md5login(string user, string hash) {
    int result;
    mapping args;

    args = ([ "user": user, "hash": hash ]);
    result = run_merry(find_object("Jonkichi:Lib:UserDB"), "md5login", "lib", args);

    DEBUG("hook_md5login(" + dump_value(user) + ", " + dump_value(hash) + ") => " + dump_value(args));

    return ({ result, args["info"] });
}

string hook_login_document() {
    return "http://gables.skotos.net/open/Jonkichi/Login.sam";
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
