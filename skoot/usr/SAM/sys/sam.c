/*
**	~SAM/sys/sam.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <mapargs.h>
# include <SkotOS.h>
# include <HTTP.h>

inherit "/lib/womble";

private inherit "/lib/string";
private inherit "/lib/mapargs";
private inherit "/lib/date";
private inherit "/lib/file";
private inherit "/lib/url";
private inherit "/lib/type";

inherit module "/lib/module";

inherit parse  "/usr/XML/lib/xmlparse";
private inherit "/usr/XML/lib/xmd";
inherit sam "/usr/SAM/lib/sam";

mapping file_contents;
mapping file_stamps;

mapping servers;

void womble_sam() {
   file_contents = womble(file_contents);
}

mixed resolve_unknown_suffix(object ob, string bit);
static mixed resolve_complex_reference(mixed var, string *bits);
void clear_sam_cache();

static
void create() {
   parse::create();
   module::create("HTTP");

   servers = ([
      "sam": this_object()
      ]);

   clear_sam_cache();
}

static
void HTTP_loaded() {
   HTTPD->register_suffix("sam");
   HTTPD->register_root("SAM");
}

void patch() {
   HTTPD->register_root("SAM");
}

void clear_sam_cache() {
   file_contents = ([ ]);
   file_stamps = ([ ]);
}

mixed get_sam_file(string file) {
   string txt;
   int stamp;

   stamp = file_stamp(file);

   if (!file_contents[file] || stamp > file_stamps[file]) {
      /* file is unparsed or recently modified */
      if (txt = read_file(file)) {
	 mixed res;

	 res = parse_xml(txt, file);
	 file_stamps[file] = stamp;
	 return file_contents[file] = res;
      }
   }
   return file_contents[file];
}

void handle_http_file(string method, object conn, string file,
		      string suffix, mapping args) {
   mixed xmd;
   object obj;

   xmd = get_sam_file(file);
   if (!xmd) {
      error("file not found");
   }
   obj = new_object("/usr/SAM/data/unsam");
   obj->set_targets(obj);
   obj->set_user(conn);

   call_limited("transform_to_html", xmd, obj, args);
   conn->send_html(200, obj);
}

mixed handle_http_request(string method, object conn, string base,
			  string url, mapping args) {
   string str;

   str = lower_case(url);
   if (str == "/post") {
      mixed action;

      action = Arg("action", "/SAM/Post");
      args["action"] = nil;
      sam(action, conn, args, conn, args["this"]);
      return TRUE;
   }
   if (strlen(str) > 6 && str[0 .. 5] == "/prop/") {
      string prop, *bits;
      object unsam, obj;

      bits = explode(url[6 ..], "/");
      str = url_decode(bits[0]);
      if (sizeof(bits) == 1) {
	 /* do HTTP directory style redirect to the 'index' property */
	 conn->redirect_to("/SAM/Prop/" + implode(bits, "/") + "/Index");
	 return TRUE;
      }
      prop = url_decode(bits[1]);
      if (!sscanf(lower_case(prop), "html:%*s")) {
	 /* this is the shorthand form so be nice */
	 prop = "html:" + prop;
      }
      if (sizeof(bits) > 2) {
	 error("URL syntax: /SAM/Prop/obName/propName");
      }
      obj = find_object(str);
      if (!obj) {
	 error("/SAM/Prop: cannot find object: " + str);
      }

      args["this"] = obj;

      unsam = new_object("/usr/SAM/data/unsam");
      unsam->set_targets(unsam);
      unsam->set_user(conn);
      unsam->set_this(obj);

      call_limited("transform_to_html",
		   obj->query_property(prop),
		   unsam, args);

      conn->send_html(200, unsam);

      return TRUE;
   }
   error("unknown request");
}


void register_root(string str) {
   if (str) {
      str = lower_case(str);
      servers[str] = previous_object();
   }
}

mapping query_servers() { return servers[..]; }

string unsam(mixed xmd, varargs mapping args) {
   return call_limited("do_unsam", xmd, args);
}

string do_unsam(mixed xmd, mapping args) {
   return ::unsam(xmd, args);
}


mixed resolve_reference(string refstr, object context,
			mapping args, mapping local) {
   object server;
   string *bits, bit;
   mixed res, tmp1, tmp2;

   bits = explode(refstr, ".");

   bit = lower_case(bits[0]);

   if (sizeof(bits) > 1 && (server = servers[bit])) {
      res = server->eval_sam_ref(bit, lower_case(bits[1]), context, args);
      bits = bits[2 ..];
   } else {
      switch(bit) {
      case "random":
	 return random(IntArg("n", "$Random"));
      case "break":
	 return break_string(unsam(StrArg("text", "$Break")),
			     IntArg("margin", "$Break"));
      case "mapref":
	 return Arg("map", "$MapRef")[Arg("index", "$MapRef")]; 
      case "lessthan":
	 return Flt(args["lhs"]) < Flt(args["rhs"]);
      case "greaterthan":
	 return Flt(args["lhs"]) > Flt(args["rhs"]);
      case "lessthanorequal":
	 return Flt(args["lhs"]) <= Flt(args["rhs"]);
      case "greaterthanorequal":
	 return Flt(args["lhs"]) >= Flt(args["rhs"]);
      case "range":
	 if (typeof(args["val"]) != T_NIL && 
	     typeof(args["start"]) != T_NIL &&
	     typeof(args["finish"]) != T_NIL) {
	    return (Flt(args["val"]) > Flt(args["start"]) &&
		    Flt(args["val"]) < Flt(args["finish"]));
	 }
	 return FALSE;
      case "mapmake":
	 return ([ ]);
      case "not":
	 switch(typeof(res = args["val"])) {
	 case T_STRING:
	    return !strlen(res);
	 case T_ARRAY:
	    return !sizeof(res);
	 case T_MAPPING:
	    return !map_sizeof(res);
	 }
	 return !res;
      case "equal":
	 return Str(args["left"]) == Str(args["right"]);
      case "inequal":
	 return Str(args["left"]) != Str(args["right"]);
      case "ctime":
	 res = ctime(Arg("time", "CTime"));
	 break;
      case "newnref":
	 res = NewNRef(Arg("object", "NewNRef"),
		       Arg("detail", "NewNRef"));
	 break;
      case "makeurl":
	 if (local["explicit"]) {
	    res = make_url(local["base"], local - ({ "base", "explicit" }));
	 } else {
	    res = UDat()->new_entry(({ local["base"], local - ({ "base" }) }));
	    res = "/Request?__id=" + res;
	 }
	 break;
      case "smalltime":
	 res = ctime(Arg("time", "CTime"))[4..18];
	 break;
      default:
	 if (args[bit] == nil && find_object(bits[0])) {
	    res = find_object(bits[0]);
	 } else {
	    res = args[bit];
	 }
	 break;
      }
      bits = bits[1 ..];
   }
   return resolve_complex_reference(res, bits);
}

static
mixed resolve_complex_reference(mixed var, string *bits) {
   object obj;
   mixed res;
   string bit;
   int num;

   if (sizeof(bits) == 0) {
      return var;
   }

   bit = lower_case(bits[0]);

   switch(bit) {
   case "even":
      if (typeof(var) == T_INT) {
	 var = (var % 2) == 0;
      }
      break;
   case "odd":
      if (typeof(var) == T_INT) {
	 var = (var % 2) != 0;
      }
      break;
   case "isfile":
      /*
       * Just in case someone somewhere plans on using this, however unlikely
       * that may be.
       */
      var = FALSE;
      break;
   case "isobject":
      var = typeof(var) == T_OBJECT;
      break;
   case "ismapping":
      var = typeof(var) == T_MAPPING;
      break;
   case "isnref":
      var = IsNRef(var);
      break;
   case "nrefob":
      var = NRefOb(var);
      break;
   case "nrefdetail":
      var = NRefDetail(var);
      break;
   case "mapindices":
      if (typeof(var) == T_MAPPING) {
	 var = map_indices(var);
      } else {
	 var = nil;
      }
      break;
   case "mapvalues":
      if (typeof(var) == T_MAPPING) {
	 var = map_values(var);
      } else {
	 var = nil;
      }
      break;
   case "size":
      switch(typeof(var)) {
      case T_ARRAY:
	 var = sizeof(var); break;
      case T_STRING:
	 var = strlen(var); break;
      case T_MAPPING:
	 var = map_sizeof(var); break;
      }
      break;
   default:
      switch(typeof(var)) {
      case T_ARRAY:
	 if (sscanf(bit, "%d", num)) {
	    if (num < 0 || num >= sizeof(var)) {
	       return "<blink>index " + num + " out of range</blink>";
	    }
	    var = var[num];
	 }
	 break;
      case T_STRING:
	 obj = find_object(var);

	 if (!obj) {
	    return "<blink>object " + var + " not found</blink>";
	 }
	 var = obj;
	 /* fall through */
      case T_OBJECT:
	 if (obj = var) {
	    switch(bit) {
	    case "name":
	       var = name(obj);
	       break;
	    case "clonable":
	       return
		  (function_object("query_clone_count", obj) == CLONABLE &&
		   !sscanf(ur_name(obj), "%*s#%*d"));
	    case "clone":
	       return !!sscanf(ur_name(obj), "%*s#%*d");
	    default:
	       var = resolve_unknown_suffix(obj, bit);
	       break;
	    }
	 }
	 break;
      case T_MAPPING:
	 var = var[bit];
	 break;
      }
   }
   return resolve_complex_reference(var, bits[1 ..]);
}

mixed resolve_unknown_suffix(object ob, string bit) {
   switch(bit) {
   case "environment":
   case "env":
   case "room":
      return ob->query_environment();
   case "inventory":
   case "inv":
      return ob->query_inventory();
   case "prox":
      return ob->query_proximity();
   case "prep":
      return ob->query_preposition();
   case "stance":
      return ob->query_stance();
   case "ur":
      return ob->query_ur_object();
   case "tname":
      return ob->query_object_name();
   default:
      return ob->query_property(bit);
   }

   /* should I error out here? -- Wes */
   return nil;
}

