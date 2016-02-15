# include <base/act.h>
# include <nref.h>
# include <type.h>
# include <SkotOS.h>
# include <TextIF.h>
# include <XML.h>
# include <SAM.h>

/* these can be called directly from Merry */
inherit "/lib/string";
inherit "/lib/array";
inherit "/lib/date";
inherit "/lib/mapping";
inherit "/lib/propmap";
inherit "/lib/url";
inherit "/lib/version";
inherit "/lib/type";
inherit "/lib/bigmap";

/* used by IsPrime */
inherit "/base/lib/urcalls";

/* just plain useful */
inherit "/base/lib/toolbox";

/* blech */
private inherit "/usr/SkotOS/lib/bilbo";

private inherit "/usr/SkotOS/lib/merryapi";
private inherit "/usr/SkotOS/lib/ursocials"; 

/* these are used by Merry-specific functions */
private inherit "/usr/SkotOS/lib/describe";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/SID/lib/stateimpex";

inherit "/lib/womble";

private int mru_stamp;
private object *obarr;
private SAM *samarr;

object this;	/* make this a TLS one day? */
mapping args;	/* make this a TLS one day? */

string stored_description;

mixed evaluate(object t, string signal, string mode, mapping a, string label,
	       varargs string description) {
   mapping oldargs;
   object oldthis;
   mixed res, oat;

   mru_stamp = time();

   oldthis = this;
   this = t;

   oldargs = args;
   args = a;

   oat = args["this"];
   args["this"] = this;

   stored_description = description;

   args["this"] = this;

   res = ::call_other(::this_object(), "merry", signal,
		      mode, label ? label : "virgin");

   stored_description = nil;

   args["this"] = oat;

   args = oldargs;
   this = oldthis;

   return res;
}

string query_description() {
   return stored_description;
}

int query_mru_stamp() {
   return mru_stamp;
}

void set_object_array(object *arr) {
   obarr = arr;
}

void set_sam_array(SAM *arr) {
   samarr = arr;
}

void womble_merry() {
   samarr = womble(samarr);
   args = womble(args);
}

static
object obref(int i) {
   return obarr[i];
}

static
SAM samref(int i) {
   return samarr[i];
}


object *query_obarr() { return obarr; }	/* for debugging */

nomask
void suicide() {
   :: call_out("do_suicide", 0);
}

static
void do_suicide() {
   catch {
      string path;

      path = ur_name(this_object());
      if (file_info(path + ".c")) {
	 string name;

	 sscanf(path, "/usr/SkotOS/merry/%s", name);
	 if (file_info("/usr/SkotOS/merry/cleaned/" + name + ".c")) {
	    remove_file("/usr/SkotOS/merry/cleaned/" + name + ".c");
	 }
	 rename_file(path + ".c", "/usr/SkotOS/merry/cleaned/" + name + ".c");
      }
   }
   :: destruct_object();
}

static
void do_delay(string mode, string signal, mixed delay, string label) {
   if (!this || !signal || !mode) {
      error("this merry node cannot perform delays");
   }
   :: call_other(this, "delayed_call",
		 ::new_object("/usr/SkotOS/data/mcontext",
			      signal,
			      mode,
			      label,
			      copy(args)),
		 "merry_continuation",
		 delay, this);
}

private atomic
void set_all(object o, mapping m) {
   string *ix;
   int i;

   :: call_other(o, "clear_all_properties");

   ix = map_indices(m);
   for (i = 0; i < sizeof(ix); i ++) {
      :: call_other(o, "set_property", ix[i], m[ix[i]]);
   }
}


nomask static
mixed Set(object o, string p, mixed v) {
   if (!o) {
      error("Missing object for Set of " + dump_value(p));
   }
   if (p == "*") {
      set_all(o, v);
      return nil;
   }
   return ::call_other(o, "set_property", p, v);
}

nomask static
mixed Get(object o, string p) {
   if (!o) {
      error("Missing object for Get of " + dump_value(p));
   }
   if (p == "*") {
      return ::call_other(o, "query_properties");
   }
   return ::call_other(o, "query_property", p);
}

nomask static
mixed SetVar(string n, mixed v) {
   if (n) {
      args[lower_case(n)] = v;
   }
}

nomask static
mixed GetVar(string n) {
   if (n) {
      return args[lower_case(n)];
   }
}

nomask static
void Popup(object who, object ob, string prop, varargs mixed *local) {
   int i, sz;
   object *souls;
   mapping largs;

   largs = ([ ]);

   if (local) {
      for (i = 0; i < sizeof(local); i += 2) {
	 largs[lower_case(local[i])] = local[i+1];
      }
   }

   souls = who->query_souls();
   sz = sizeof(souls);
   for (i = 0; i < sz; i++) {
      int     id;
      object  udat;
      mapping data;

      if (!souls[i]->query_skotos_client()) {
	 continue;
      }
      udat = souls[i]->query_udat();
      if (!udat) {
	 continue;
      }
      id = udat->new_entry(largs);
      if (largs["zid"] == -1) {
	 data = ([ "zid": id ]);
      } else {
	 /*
	  * Backwards compatible for the moment.
	  */
	 data = ([ "zid": id ]) + largs;
      }
      souls[i]->raw_line("SKOOT " + ANYURL + " " +
			 url_absolute(make_url("/SAM/Prop/" +
					       url_encode(name(ob)) + "/" +
					       prop, data, TRUE)));
   }
}

private object select_verb(object body, object *verbs) {
   int sz;
   string id;

   sz = sizeof(verbs);
   id = body->query_property("theatre:id");
   if (id) {
      int i;
      object *tverbs, *gverbs;

      tverbs = allocate(sz);
      gverbs = allocate(sz);
      for (i = 0; i < sz; i++) {
	 if (verbs[i]->query_property("theatre:id") == id) {
	    tverbs[i] = verbs[i];
	 } else if (!verbs[i]->query_property("theatre:id")) {
	    gverbs[i] = verbs[i];
	 }
      }
      tverbs -= ({ nil });
      gverbs -= ({ nil });
      if (sizeof(tverbs) > 0) {
	 verbs = tverbs;
	 DEBUG("[Social] Relevant Theatre verb object(s): " + dump_value(verbs));
      } else {
	 verbs = gverbs;
	 if (!sizeof(verbs)) {
	    return nil;
	 }
      }
   }
   if (sizeof(verbs) > 1) {
      DEBUG("[Social] More than one relevant verb object for " + verbs[0]->query_imperative() + ": " + dump_value(verbs));
   }
   return verbs[0];
}

nomask static
void Social(object ob, string verb, varargs string adverb,
	    string evoke, mixed targets...) {
   mapping largs;
   object *vobs, vob;
   string role, *roles;
   int i, j;

   vobs = SOCIALS->query_verb_obs(verb);
   if (!vobs || sizeof(vobs) == 0) {
      error("verb " + verb + " unknown");
   }
   vob = select_verb(ob, vobs);
   if (!vob) {
      error("No appropriate verb object for " + verb);
   }

   largs = args + ([ "imperative": vob->query_imperative() ]);

   largs["ldef"] = largs["ldef-actor"] = largs["ldef-witness"] =
      largs["ldef-target"] = largs["evoke"] = largs["adverb"] = nil;

   roles = ({ });

   for (i = 0; i < sizeof(targets); i += 2) {
      if (!targets[i]) {
	 string *lroles;

	 lroles = vob->query_direct_roles();
	 if (!sizeof(lroles)) {
	    error("verb '" + verb + "' has no direct roles");
	 }
	 role = lroles[0];
      }
      if (targets[i]) {
	 role = vob->get_role_by_preposition(targets[i]);
	 if (!role) {
	    error("verb '" + verb + "' does not accept preposition '" +
		  targets[i] + "'");
	 }
	 largs[role + ":preposition"] = targets[i];
      }
      if (typeof(targets[i+1]) == T_OBJECT) {
	 largs[role] = ({ targets[i+1] });
      } else if (typeof(targets[i+1]) == T_ARRAY) {
	 largs[role] = targets[i+1][..];
      } else {
	 error("bad targets for social object #" + (i/2+1));
      }
      roles += ({ role });
   }
   if (adverb) {
      largs["adverb"] = adverb;
   }
   if (evoke) {
      largs["evoke"] = "\"" + evoke + "\"";
   }
   largs["roles"]     = roles;
   largs["privacy"]   = ur_privacy(vob);
   largs["obscurity"] = ur_privacy(vob);
   largs["secrecy"]   = ur_privacy(vob);

   largs["vob"] = vob;

   if (!run_merry(vob, "command", "global", largs)) {
      return;
   }

   ob->action("social", largs);
}

nomask static
mixed Bilbo(mixed where, string type, string name, varargs mixed *local) {
   mapping largs;
   object ob;
   mixed det;
   int i;

   if (!local) {
      local = ({ });
   }
   if (sizeof(local) % 2) {
      error("uneven number of parameters to CallArg()");
   }
   largs = args[..];

   for (i = 0; i < sizeof(local); i += 2) {
      largs[lower_case(local[i])] = local[i+1];
   }

   if (IsNRef(where)) {
      ob = NRefOb(where); det = NRefDetail(where);
   } else {
      ob = where; det = nil;
   }

   return run_bilbo(ob, ob, name, type, det, largs);
}

nomask static
void Act(object ob, string action, varargs mixed *local) {
   string *save_ix;
   mixed *save_val;
   int i;

   if (!local) {
      local = ({ });
   }
   if (sizeof(local) % 2) {
      error("uneven number of parameters to CallArg()");
   }
   save_ix  = allocate(sizeof(local)/2);
   save_val = allocate(sizeof(local)/2);

   for (i = 0; i < sizeof(save_ix); i ++) {
      save_ix[i] = lower_case(local[i*2]);
      save_val[i] = args[save_ix[i]];
      args[save_ix[i]] = local[i*2+1];
   }
   :: call_other(ob, "action", action, args,
		 (args["silently"] ? ACTION_SILENT : 0));

   for (i = 0; i < sizeof(save_ix); i ++) {
      args[save_ix[i]] = save_val[i];
   }
}

nomask static
object FindMerry(mixed oref, string mode, string signal) {
   object obj;

   if (typeof(oref) == T_OBJECT) {
      obj = oref;
   } else if (typeof(oref) == T_STRING) {
      obj = ::find_object(oref);
      if (!obj) {
	 error("cannot find object: " + oref);
      }
   } else {
      error("bad first argument to FindMerry()");
   }
   return find_merry_location(obj, signal, mode);
}

nomask static
mixed Call(mixed oref, string name, varargs mixed *local) {
   string *save_ix;
   object obj;
   mixed *save_val;
   mixed code, res;
   int i, method;

   if (!local) {
      local = ({ });
   }
   if (sizeof(local) % 2) {
      error("uneven number of parameters to Call()");
   }
   if (typeof(oref) == T_OBJECT) {
      obj = oref;
   } else if (typeof(oref) == T_STRING) {
      obj = ::find_object(oref);
      if (!obj) {
	 error("cannot find object: " + oref);
      }
   } else {
      error("bad first argument to Call()");
   }

   method = !!obj->query_method(name);
   if (!method) {
      if (!find_merry(obj, name, "lib")) {
	 error("cannot find script [" + name + "] on " + name(obj));
      }
   }

   save_ix  = allocate(sizeof(local)/2);
   save_val = allocate(sizeof(local)/2);

   for (i = 0; i < sizeof(save_ix); i ++) {
      save_ix[i] = lower_case(local[i*2]);
      save_val[i] = args[save_ix[i]];
      args[save_ix[i]] = local[i*2+1];
   }

   if (method) {
      res = obj->call_method(name, args);
   } else {
      res = run_merry(obj, name, "lib", args, nil);
   }

   for (i = 0; i < sizeof(save_ix); i ++) {
      args[save_ix[i]] = save_val[i];
   }
   return res;
}

nomask static
mixed LabelCall(string space, string fun, varargs mixed *local) {   
   object handler;

   if (handler = SYS_MERRY->query_script_space(space)) {
      return Call(handler, fun, local);
   }
   error("cannot find script space: " + space);
}

nomask static
object LabelRef(string space) {
   return SYS_MERRY->query_script_space(space);
}

nomask static
object Spawn(object obj) {
   return spawn_thing(obj);
}

nomask static
void Slay(object obj) {
   if (!obj) {
      error("Missing object for Slay");
   }
   :: call_other(obj, "suicide");
}

private
void do_emit(object *targets, string str) {
   object *souls;
   int i, j;

   for (i = 0; i < sizeof(targets); i ++) {
      souls = ::call_other(targets[i], "query_souls");
      
      if (!souls) {
         SysLog("Strange, " + dump_value(targets[i]) + " has no souls.");
         continue;
      }
   
      for (j = 0; j < sizeof(souls); j ++) {
	 :: call_other(souls[j], "paragraph", str);
      }
   }
}

nomask static
void EmitTo(mixed targets, string str) {
   if (typeof(targets) == T_NIL) {
      return;
   }
   if (typeof(targets) == T_OBJECT) {
      targets = ({ targets });
   }
   do_emit(targets, str);
}

nomask static
void EmitIn(object room, string str, object except...) {
   if (room) {
      do_emit(room->query_inventory() - except, str);
   }
}

nomask static
string UnSAM(mixed sam, varargs mixed *local) {
   string *save_ix;
   mixed *save_val;
   string res;
   int i;

   if (!local) {
      local = ({ });
   }
   if (sizeof(local) % 2) {
      error("uneven number of parameters to UnSAM()");
   }
   save_ix  = allocate(sizeof(local)/2);
   save_val = allocate(sizeof(local)/2);

   for (i = 0; i < sizeof(save_ix); i ++) {
      save_ix[i] = lower_case(local[i*2]);
      save_val[i] = args[save_ix[i]];
      args[save_ix[i]] = local[i*2+1];
   }
   res = ::unsam(sam, args, nil, this);

   for (i = 0; i < sizeof(save_ix); i ++) {
      args[save_ix[i]] = save_val[i];
   }
   return res;
}

nomask static
mixed ParseXML(string xml) {
   return XML->parse(xml);
}

nomask static
string In(string signal, int seconds) {
   if (!this) {
      error("this merry node cannot perform delays");
   }
   return ::call_other(this, "schedule_entry",
		       signal, time() + seconds);
}

nomask static
string Every(string signal, int seconds) {
   if (!this) {
      error("this merry node cannot perform delays");
   }
   return ::call_other(this, "schedule_entry",
		       signal, time() + seconds, seconds);
}

nomask static
string Stop(string id) {
   if (!this) {
      error("this merry node cannot perform delays");
   }
   return ::call_other(this, "unschedule_entry", id);
}


nomask static atomic
object Duplicate(object ob) {
   string clonable;
   object clone;
   mixed state;

   if (!ob) {
      error("Missing object for Duplicate");
   }
   if (sscanf(ur_name(ob), "%s#%*d", clonable)) {
      state = export_state(ob);
      clone = clone_object(clonable);
      import_state(clone, state);
      return clone;
   }
   error("object is not a clone");
}

nomask static
string Describe(mixed what, varargs object actor, object looker, int style) {
   if (what == nil) {
      return "nothing";
   }
   if (::typeof(what) == T_OBJECT || IsNRef(what)) {
      return describe_one(what, actor, looker, style);
   }
   return describe_many(what, actor, looker, style);
}

nomask static
NRef *Match(object ob, string str, varargs object looker, string adj...) {
   return ::call_other(ob, "single_match", str, looker, adj);
}

nomask static
NRef *MatchPlural(object ob, string str, varargs object looker, string adj...) {
   return ::call_other(ob, "plural_match", str, looker, adj);
}

static
mixed call_other(mixed obj, string fun, mixed args...) {
   switch(typeof(obj)) {
   case T_NIL:
   case T_INT:
   case T_FLOAT:
   case T_ARRAY:
   case T_MAPPING:
      return ::call_other(obj, fun, args...);
   case T_OBJECT:
      switch(fun) {
      case "is_nref":
      case "query_object":
      case "query_detail":
      case "query_amount":
      case "equals":
      case "chip":
	 if ((::function_object("is_thing", obj) == "/base/lib/thing" ||
	      ::ur_name(obj) == "/base/data/nref#-1")) {
	    return ::call_other(obj, fun, args...);
	 }
      }
      break;
   }
   error("function 'call_other' not allowed in merry code");
}

static
object new_object(string obj, mixed args...) {
   if (obj == "/base/data/nref") {
      return ::new_object(obj, args...);
   }
   error("function 'new_object' not allowed in merry code");
}

# define SANDBOX(f) mixed f(mixed args...) { error("function '" + #f + "' not allowed in merry code"); }

SANDBOX(add_event)
SANDBOX(block_input)
SANDBOX(call_touch)
/* SANDBOX(call_out) */
SANDBOX(clone_object)
SANDBOX(compile_object)
SANDBOX(connect)
SANDBOX(destruct_object)
SANDBOX(destruct_program)
SANDBOX(dump_state)
SANDBOX(editor)
SANDBOX(event)
SANDBOX(event_except)
SANDBOX(execute_program)
SANDBOX(file_info)
SANDBOX(function_object)
SANDBOX(get_dir)
SANDBOX(make_dir)
SANDBOX(open_port)
SANDBOX(ports)
SANDBOX(query_editor)
SANDBOX(query_originator)
SANDBOX(read_file)
SANDBOX(remove_call_out)
SANDBOX(remove_dir)
SANDBOX(remove_event)
SANDBOX(remove_file)
SANDBOX(rename_file)
SANDBOX(send_datagram)
SANDBOX(send_message)
SANDBOX(set_object_name)
SANDBOX(set_originator)
SANDBOX(shutdown)
SANDBOX(subscribe_event)
SANDBOX(swapout)
SANDBOX(this_user)
SANDBOX(unsubscribe_event)
SANDBOX(users)
SANDBOX(write_file)
