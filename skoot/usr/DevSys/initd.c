/*
**	~DevSys/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <status.h>
# include <System.h>
# include <SkotOS.h>
# include <mapargs.h>
# include <SAM.h>
# include <SID.h>
# include <HTTP.h>
# include <base.h>

inherit "/lib/womble";

private inherit "/lib/string";
private inherit "/lib/file";
private inherit "/lib/date";

private inherit "/lib/url";
private inherit "/lib/mapargs";
private inherit "/lib/loader";

private inherit "/base/lib/urcalls";

private inherit tool  "/usr/System/lib/toolkit";

private inherit xmd	"/usr/XML/lib/xmd";

inherit gen	"/usr/XML/lib/xmlgen";
inherit parse	"/usr/XML/lib/xmlparse";

private inherit "/usr/DevSys/lib/formstate";
private inherit "/usr/DevSys/lib/karmod";

inherit "/usr/SID/lib/stateimpex";
inherit vault "/usr/SID/lib/vaultnode";

inherit module "/lib/module";

inherit "/usr/SAM/lib/sam";
inherit "/lib/data";

private mapping look_and_feel;

void womble_look_and_feel() {
   look_and_feel = womble(look_and_feel);
}

string query_state_root() { return "DevSys:Init"; }

static
void create() {
   tool::create();
   parse::create();
   module::create("HTTP", "SAM");
   vault::create("/usr/DevSys/data/vault");
   sequencer::create("/usr/DevSys/sys/woed");

   look_and_feel = ([ ]);

   set_object_name("DevSys:Init");

   claim_node("DevSys");

   find_or_load("/usr/DevSys/sys/avchat");
}

nomask
void patch() {
   int port;

   port = SYS_INITD->query_portbase() + 80;

   if (!HTTPD->query_port_master(port)) {
      HTTPD->claim_port(port);
   }
}

static
void HTTP_loaded() {
   int port;

   HTTPD->register_root("Dev");
   HTTPD->register_root("Admin");
   HTTPD->register_root("LF");

   port = SYS_INITD->query_portbase() + 89;

   if (!HTTPD->query_port_base(port)) {
      HTTPD->claim_port(port);
   }
}

string query_http_root_url_document() {
   return "/Dev/TreeOfWoe.sam";
}   

static
void SAM_loaded() {
   load_programs("/usr/DevSys/samtags");

   SAMD->register_root("Dev");
   SAMD->register_root("Status");
   SAMD->register_root("ProgDB");
   SAMD->register_root("System");
   SAMD->register_root("LF");
   SAMD->register_root("ConfigD");
}

mixed handle_http_request(string method, object conn, string base,
			  string url, mapping args) {
   string user;
   object wiztool, ob;

   user = conn->query_name();

   switch(base) {
   case "dev":
      if (user) {
	 wiztool = DEV_USERD->query_wiztool(user);
      }

      if (!wiztool) {
	 error("User " + (user ? user : "nobody") + " has insufficient privilege for /dev/" + lower_case(url));
      }
      switch(lower_case(url)) {
      case "/rename":
	 ob = ObArg("obj", "/Dev/Rename");
	 ob->set_object_name(StrArg("name", "/Dev/Rename"));

	 conn->redirect_to(make_url("/Dev/View.sam", ([
	    "masterpage": "/Dev/Object.sam", "obj": name(ob) ])));
	 return TRUE;
      case "/destroy":
	 wiztool->destroy(ObArg("obj", "/Dev/Destruct"));
	 conn->redirect_to("/Dev/blank.html");
	 return TRUE;
      case "/duplicate": {
	 string clonable;
	 object orig, clone;
	 mixed state;

	 orig = ObArg("obj", "/Dev/Duplicate");
	 if (sscanf(ur_name(orig), "%s#%*d", clonable)) {
	    state = export_state(orig);
	    clone = wiztool->clone(clonable);
	    import_state(clone, state);
	    args["obj"] = name(clone);
	    conn->redirect_to(args["redirect"], args - ({ "redirect" }));
	    return TRUE;
	 }
	 error("object is not a clone");
      }
      case "/spawn": {
	 string clonable;
	 object orig, clone;

	 orig = ObArg("obj", "/Dev/Spawn");

	 if (sscanf(ur_name(orig), "%s#%*d", clonable)) {
	    clone = wiztool->clone(clonable);
	    clone->set_ur_object(orig);
	    args["obj"] = name(clone);
	    conn->redirect_to(args["redirect"], args - ({ "redirect" }));
	    return TRUE;
	 }
	 error("object is not a clone");
      }
      case "/clone": {
	 object obj, clone;

	 obj = ObArg("obj", "/Dev/Clone");

	 if (function_object("query_clone_count", obj) == CLONABLE) {
	    clone = wiztool->clone(ur_name(obj));
	    args["obj"] = name(clone);
	    conn->redirect_to(args["redirect"], args - ({ "redirect" }));
	    return TRUE;
	 }
	 error("object is not clonable");
      }
      case "/newitem": {
	 mapping itargs;
	 object node;
	 string *call;
	 int i;

	 ob = ObArg("obj", "/Dev/NewItem");

	 if (args["ptamap"] && args["pttmap"]) {
	    itargs = parse_context_mappings(args["ptamap"], args["pttmap"]);
	    XDebug("ptargs are " + dump_value(itargs));
	 } else {
	    itargs = ([ ]);
	 }
	 args += itargs;

	 node = SID->get_node(StrArg("element", "/Dev/NewItem"));
	 if (call = node->query_newitem()) {
	    catch {
	       execute_call(call, ob, args);
	    }
	    catch {
	       mixed log;

	       log = ob->query_property("revisions");
	       if (typeof(log) != T_ARRAY) {
		  log = ({ });
	       }
	       log += ({ time(), user, "E" });
	       ob->set_property("revisions", log);
	    }
	 }
	 if (args["uramap"] && args["urtmap"]) {
	    itargs = parse_context_mappings(args["uramap"], args["urtmap"]);
	 } else {
	    itargs = ([ ]);
	 }
	 XDebug("urargs are " + dump_value(itargs));
	 XDebug("UREL is " + dump_value(args["urel"]));
	 conn->redirect_to(make_url("/Dev/View.sam", itargs + ([
	    "obj": args["obj"],
	    "element": args["urel"],
	    "homeurl": args["homeurl"],
	    "hometitle": args["hometitle"],
	    "homedisplay": "1"
	    ])));
	 return TRUE;
      }
      case "/delitem": {
	 mapping itargs;
	 object node;
	 string *call;
	 int i;

	 ob = ObArg("obj", "/Dev/DelItem");

	 if (args["ptamap"] && args["pttmap"]) {
	    itargs = parse_context_mappings(args["ptamap"], args["pttmap"]);
	 } else {
	    itargs = ([ ]);
	 }
	 args += itargs;

	 node = SID->get_node(StrArg("element", "/Dev/DelItem"));
	 if (call = node->query_delitem()) {
	    catch {
	       execute_call(call, ob, args);
	    }
	    catch {
	       mixed log;

	       log = ob->query_property("revisions");
	       if (typeof(log) != T_ARRAY) {
		  log = ({ });
	       }
	       log += ({ time(), user, "E" });
	       ob->set_property("revisions", log);
	    }
	 }
	 if (args["uramap"] && args["urtmap"]) {
	    itargs = parse_context_mappings(args["uramap"], args["urtmap"]);
	 } else {
	    itargs = ([ ]);
	 }
	 XDebug("urargs are " + dump_value(itargs));
	 XDebug("UREL is " + dump_value(args["urel"]));
	 conn->redirect_to(make_url("/Dev/View.sam", itargs + ([
	    "obj": args["obj"],
	    "element": args["urel"],
	    "homeurl": args["homeurl"],
	    "hometitle": args["hometitle"],
	    "homedisplay": "1"
	    ])));
	 return TRUE;
      }
      }
   }
}

/* we're guaranteed to be authenticated when we get here */
string remap_http_request(string base, string url, mapping args) {
   return "/usr/DevSys/data/www" + url;
}

mixed eval_sam_ref(string service, string ref, object context, mapping args) {
   string user, str;
   mixed *status;
   object ob;

   switch(service) {
   case "lf": {
     return look_and_feel[lower_case(ref)];
   }
   case "dev":
      switch(ref) {
      case "wiztool":
	 return DEV_USERD->query_wiztool(context->query_user()->query_name());
      case "file":
	 return replace_html(read_file(StrArg("file", "/Dev/File")));
      case "rawtree": {
	 mapping res;

	 res = ([
	    "subdirs":	IDD->query_subdirs(args["path"]),
	    "objects":	IDD->query_objects(args["path"]),
	    ]);
	 return res;
      }
      case "rawstate": {
	 ob = ObArg("obj", "Dev.RawState");

	 if (args["element"]) {
	    return export_state(ob,
				SID->get_node(args["element"]),
				args - ({ "element" }));
	 }
	 return export_state(ob);
      }
      case "formstate": {
	 mapping cmap, bump, *context;
	 mixed state;

	 ob = ObArg("obj", "Dev.FormState");

	 if (args["detail"]) {
	    /* Shortcut for viewing a Base:Thing detail: */
	    string id;

	    id = args["detail"];
	    args -= ({ "detail" });
	    args += ([
	        "element": "Base:Detail",
		"itamap": ([ "id": id ]),
		"ittmap": ([ "id": "lpc_str" ])
            ]);
	 } else if (args["property"]) {
	 /* Shortcut for viewing a specific property in most objects: */
	    string id;

	    id = args["property"];
	    args -= ({ "property" });
	    args += ([
	        "element": "Core:Property",
		"itamap": ([ "property": id ]),
		"ittmap": ([ "property": "lpc_str" ])
            ]);
	 }
	 if (args["itamap"] && args["ittmap"]) {
	    context = make_form_context(args["itamap"], args["ittmap"]);
	    cmap = parse_context_mappings(args["itamap"], args["ittmap"]);
	    XDebug("CMAP: " + dump_value(cmap));
	 } else {
	    cmap = ([ ]);
	 }
	 cmap = args + cmap;
	 if (args["element"]) {
	    state = export_state(ob,
				 SID->get_node(args["element"]),
				 args - ({ "element" }),
				 cmap);
	 } else {
	    state = export_state(ob, nil, nil, cmap);
	 }

	 bump = state_to_form(state, ([ ]),
			      args["itamap"], args["ittmap"]);

	 return ([
	    "context": context,
	    "bump":   bump,
	    ]);
      }
      case "karstate": {
	 return genkar(ObArg("obj", "Dev.KarState"));
      }
      case "xmlstate": {
	 mapping cmap;
	 mixed state, context;

	 ob = ObArg("obj", "Dev.XMLState");

	 if (args["itamap"] && args["ittmap"]) {
	    context = make_xml_context(args["itamap"], args["ittmap"]);
	    cmap = parse_context_mappings(args["itamap"], args["ittmap"]);
	 } else {
	    cmap = ([ ]);
	 }
	 cmap = args + cmap;
	 XDebug("CMAP IS " + dump_value(cmap));
	 if (args["element"]) {
	    state = export_state(ob,
				 SID->get_node(args["element"]),
				 args - ({ "element" }),
				 cmap);
	 } else {
	    state = export_state(ob, nil, nil, cmap);
	 }
	 if (!state) {
	    error("object does not export state");
	 }
	 state = xmd_elts("object", ({ "id",  ob }), context, state);

	 clear();
	 generate_xml(state, this_object());
	 
	 return query_chunks(); /* TODO */
      }
      case "editable":
	 return ObArg("obj", "Dev.Editable")->query_state_root();
      }
      return nil;
   case "system":
      switch(ref) {
      case "day":
	 switch(ctime(time())[.. 2]) {
	 case "Mon":	return 1;
	 case "Tue":	return 2;
	 case "Wed":	return 3;
	 case "Thu":	return 4;
	 case "Fri":	return 5;
	 case "Sat":	return 6;
	 case "Sun":	return 7;
	 default:	error("internal error in ctime");
	 }
      case "hour":
	 return (int) ctime(time())[11..12];
      case "minute":
	 return (int) ctime(time())[14..15];
      case "hostname":
	 return SYS_INITD->query_hostname();
      case "portbase":
	 return SYS_INITD->query_portbase();
      case "webport":
	 return SYS_INITD->query_portbase() + 80;
      case "woeport":
	 return SYS_INITD->query_portbase() + 90;
      case "textport":
	 return SYS_INITD->query_textport();
      case "version":
	 return "alpha-0.9";
      case "users": {
	 object *usr;
	 string *names;
	 int i;

	 usr = users();
	 names = allocate(sizeof(usr));
	 for (i = 0; i < sizeof(usr); i ++) {
	    names[i] = capitalize(usr[i]->query_name());

	 }
	 return names;
      }
      }
      return nil;
   case "configd":
      return CONFIGD->query(ref);
   }
   return nil;
}

string *query_lf_properties() {
   return map_indices(look_and_feel);
}

mixed query_lf_property(string id) {
   return look_and_feel[id];
}

void clear_lf_properties() {
   look_and_feel = ([ ]);
}

void set_lf_property(string id, mixed val) {
   if (typeof(id) == T_STRING) {
      look_and_feel[lower_case(id)] = val;
   }
}
