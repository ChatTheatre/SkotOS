/*
**	~System/lib/sys_auto.c
**
**	This program is automatically inherited by all programs that
**	are neither in /kernel nor /usr/System. It's the equivalent
**	of /kernel/lib/auto, but on the ~System level. Its only duty
**	at present is to ensure a clonable program with dependent
**	clones absolutely cannot be destructed.
**
**	TODO: Add permissions checks.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <System.h>
# include <System/log.h>
# include <XML.h>
# include <status.h>

private string name;

nomask void _S_next_clone(object obj);
nomask void _S_prev_clone(object obj);
nomask void _F_add_to_clone_count(int n);
nomask object query_next_clone();
nomask object query_previous_clone();
nomask int query_clone_count();

# define CHECKARG(arg, n, func)	if (!(arg)) badarg((n), (func))

private void badarg(int n, string func)
{
   ::error("Bad argument " + n + " for function " + func);
}

static nomask
void _F_sys_create(int clone) {
    string clonable;

    if (::function_object("query_clone_count", ::this_object()) == CLONABLE) {
	if (::sscanf(::object_name(::this_object()), "%s#", clonable)) {
	    object master, head;
	    /* we're a clone, so link us at the head of the clone chain */
	    master = ::find_object(clonable);
	    head = ::call_other(master, "query_next_clone");

	    _S_next_clone(head);
	    _S_prev_clone(master);

	    ::call_other(head, "_S_prev_clone", ::this_object());
	    ::call_other(master, "_S_next_clone", ::this_object());
	    ::call_other(master, "_F_add_to_clone_count", 1);
	    return;
	}
	/* we're a clonable program */
	_S_next_clone(::this_object());
	_S_prev_clone(::this_object());
    }
}

static
int destruct_program(string path) {
    return ::destruct_object(path);
}

static atomic
void destruct_object(varargs int force) {
   string clonable;

   if (!::this_object()) {
      return;
   }
   if (!force) {
      /* this one is NOT caught */
      ::call_other(::this_object(), "destructive_desire");
   }
   /* if we get here, it's happening */
   catch {
      ::call_other(find_object(PROGDB), "destructing");
      ::call_other(::this_object(), "destructing");
   }
   if (::function_object("query_clone_count", ::this_object()) == CLONABLE) {
      if (::sscanf(::object_name(this_object()), "%s#", clonable)) {
	 /* obj is a clone, it must be unlinked */
	 object master, next, prev;

	 next = query_next_clone();
	 prev = query_previous_clone();
	 master = ::find_object(clonable);

	 ::call_other(master, "_F_add_to_clone_count", -1);
	 ::call_other(prev, "_S_next_clone", next);
	 ::call_other(next, "_S_prev_clone", prev);
      } else if (query_clone_count()) {
	 /* clonable program with clones! */
	 ::error("Cannot destruct: clones exist");
      }
   }
   if (name) {
      ::call_other(IDD, "clear_name", name);
   }
   :: destruct_object(::this_object());
}

/* DO NOT USE THIS */
static
void unatomic_destruct_object(varargs int force) {
   string clonable;

   if (!::this_object()) {
      return;
   }
   if (!force) {
      /* this one is NOT caught */
      ::call_other(::this_object(), "destructive_desire");
   }
   rlimits(0; -1) {
      /* if we get here, it's happening */
      catch {
	 ::call_other(find_object(PROGDB), "destructing");
	 call_limited("destructing");
      }
      if (::function_object("query_clone_count", ::this_object()) == CLONABLE) {
	 if (::sscanf(::object_name(this_object()), "%s#", clonable)) {
	    /* obj is a clone, it must be unlinked */
	    object master, next, prev;

	    next = query_next_clone();
	    prev = query_previous_clone();
	    master = ::find_object(clonable);

	    ::call_other(master, "_F_add_to_clone_count", -1);
	    ::call_other(prev, "_S_next_clone", next);
	    ::call_other(next, "_S_prev_clone", prev);
	 } else if (query_clone_count()) {
	    /* clonable program with clones! */
	    ::error("Cannot destruct: clones exist");
	 }
      }
      if (name) {
	 ::call_other(IDD, "clear_name", name);
      }
      :: destruct_object(::this_object());
   }
}

nomask
void destruct() {
   /* we must have some security check here, but what? */
   destruct_object();
}

static
object new_object(mixed obj, mixed args...) {
   object ob;

   ob = ::new_object(obj);
   :: call_other(ob, "_F_configure", args...);

   return ob;
}

static
void configure(mixed args...) {}

nomask
void _F_configure(mixed args...) {
   if (sscanf(previous_program(), "/usr/System/%*s")) {
      configure(args...);
   }
}


/*
 *	This is entirely unrestricted for now, but should probably
 *	be secured when ~System/sys/progdb.c does something lenient
 *	with it. -- Zell 040105
 */

/* unmasked because crawlers got broken - Shentino, Oct 7, 2016 */

void call_touch(object obj) {
   if (previous_program() == TOUCHD) {
      ::call_touch(obj);
   } else {
      TOUCHD->call_touch(obj);
   }
}

nomask
string query_object_name() {
   return name;
}

static object query_originator();

atomic
void set_object_name(string lname) {
   int i;

   if (lname) {
     for (i = 0; i < ::strlen(lname); i ++) {
       switch(lname[i]) {
       case 0..31:
       case 127:	
	 /* unsafe */
       case '<': case '>': case '\"' : case '#': case '%':
       case '{': case '}': case '|': case '\\': case '^':
       case '~': case '[': case ']': case '`': case '\'':
	  ::error("bad character in set_object_name");
       }
     }
   }
   if (name && name != lname) {
      ::call_other(IDD, "clear_name", name);
   }
   if (lname) {
      ::call_other(IDD, "set_name", ::this_object(), lname);
   }
   catch {
      if (name && lname && name != lname) {
	 mixed log;

	 log = ::call_other(::this_object(), "query_property", "revisions");
	 if (typeof(log) != T_ARRAY) {
	    log = ({ });
	 }
	 log += ({
	    time(),
	    query_originator() ? query_originator()->query_name() : "???",
	    "R"
	 });
	 ::call_other(::this_object(), "set_property", "revisions", log);
      }
   }
   name = lname;
}

static
object find_object(string str) {
   object parent, res;
   int num;

   if (res = ::find_object(str)) {
      return res;
   }
   if (::sscanf(str, "[%s]#%d", str, num) == 2) {
      /* split into ur-parent name and clone number */
      if (parent = ::call_other(IDD, "query_object", str)) {
	 /* found the ur parent */
	 string base;

	 /* extract the base name */
	 base = ::object_name(parent);
	 sscanf(base, "%s#", base);

	 if (res = ::find_object(base + "#" + num)) {
	    if (::call_other(res, "query_ur_object") == parent) {
	       /* found it! */
	       return res;
	    }
	 }
      }
      return nil;
   }
   return ::call_other(IDD, "query_object", str);
}

static
string ur_name(object ob) {
   return ::object_name(ob);
}


static
string object_name(object ob) {
   ::error("object_name() is replaced by name() and ur_name()");
}


static
void set_originator(object ob) {
   ::call_other(SYSLOGD, "set_originator", ob);
}


static
object query_originator() {
   return ::call_other(SYSLOGD, "query_originator");
}


static
int call_out(string function, mixed delay, mixed args...) {
   object user;
   int handle;

   CHECKARG(function, 1, "call_out");
   handle = ::typeof(delay);
   CHECKARG(handle == T_INT || handle == T_FLOAT, 2, "call_out");
   if (!::this_object()) {
      return 0;
   }
   CHECKARG(::function_object(function, ::this_object()) != AUTO,
	    1, "call_out");

   return ::call_out("_F_skotos_callout", delay, function,
		     ::call_other(SYSLOGD, "query_originator"),
		     args);
}

nomask void _F_skotos_callout(string function, object originator,
			      mixed *args) {
   if (::previous_program() == AUTO) {
      ::call_other(SYSLOGD, "set_originator", originator);
      ::call_other(this_object(), function, args...);
   }
}


static
object compile_object(string name, string source...) {
   string creator, oname, err;
   object obj;

   if (sizeof(source)) {
      oname = ::object_name(::this_object());
      creator = ::call_other(DRIVER, "creator", oname);
      name = ::call_other(DRIVER, "normalize_path", name,
			  oname + "/..", creator);

      /* the directory must exist */
      if (!::file_info(name + "/..")) {
	 ::error("bad object name: " + name);
      }

      catch {
	 obj = ::compile_object(name, source...);
      } : {
	 ::call_other(SYSLOGD, "rethrow_atomically");
      }

      ::call_other(PROGDB, "write_source", name, source);
      return obj;
   }
   catch {
      return ::compile_object(name);
   } : {
      ::call_other(SYSLOGD, "rethrow_atomically");
   }
}

# include "./textdata.c"
# include "./find_or_load.c"

static string dump_value(mixed value, varargs mixed blah) {
   return mixed_to_ascii(value);
}

/*
 * Assisting the upcoming conversion away from colours of any sort.
 */

static mixed query_colour_value(mixed value) {
   if (typeof(value) == T_OBJECT) {
      switch (::object_name(value)) {
      case "/base/data/nref#-1":
      case "/usr/XML/data/element#-1":
      case "/usr/XML/data/pcdata#-1":
      case "/usr/XML/data/samref#-1":
	 return value->query_data();
      default:
	 break;
      }
   }
   return value;
}

static int query_colour(mixed value) {
   if (typeof(value) == T_OBJECT) {
      switch (::object_name(value)) {
      case "/base/data/nref#-1":       return COL_NREF;
      case "/usr/XML/data/element#-1": return COL_ELEMENT;
      case "/usr/XML/data/pcdata#-1":  return COL_PCDATA;
      case "/usr/XML/data/samref#-1":  return COL_SAMREF;
      default:
	 break;
      }
   }
   return 0;
}

void womble_auto() {
   int i, sz;
   string *wombles;

   wombles = ({
      "details", "properties", "notes", "requests", "xml", "bulk", "consent",
      "crafting", "die_messages", "initials", "description", "descriptions",
      "innards", "errors", "messages", "banner", "look_and_feel", "core"
   });
   sz = sizeof(wombles);
   for (i = 0; i < sz; i++) {
      if (function_object("womble_" + wombles[i], this_object())) {
	 catch {
	    call_other(this_object(), "womble_" + wombles[i]);
	    ::call_other(SYSLOGD, "syslogd", LOG_DEBUG,
			 "[womble_auto] Called womble_" + wombles[i] + "() in " + ::object_name(this_object()));
	 }
      }
   }
}

static
string hash_md5(string str, string extra...)
{
    return hash_string("MD5", str, extra...);
}

static
string hash_sha1(string str, string extra...)
{
    return hash_string("SHA1", str, extra...);
}

string query_state_root() { return "System:Empty"; }


/***********************/
/* debugging functions */
/***********************/

private mapping system_query_callouts()
{
	mixed **callouts;
	mapping ret;
	int *handles;
	int sz;
	ret = ([ ]);

	callouts = status(this_object())[O_CALLOUTS];

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_FUNCTION] == "_F_skotos_callout") {
			callout = ({
				callout[CO_HANDLE],
				callout[CO_FIRSTXARG],
				callout[CO_DELAY] })
				+ callout[CO_FIRSTXARG + 2];
		}

		ret[callouts[sz][CO_HANDLE]] = callout;
	}

	return ret;
}

int *system_query_callout_handles()
{
	return map_indices(system_query_callouts());
}

string system_query_callout_func(int handle)
{
	return system_query_callouts()[handle][CO_FUNCTION];
}

mixed system_query_callout_delay(int handle)
{
	return system_query_callouts()[handle][CO_DELAY];
}

mixed *system_query_callout_args(int handle)
{
	return system_query_callouts()[handle][CO_FIRSTXARG ..];
}

int *system_query_merry_continuation_handles()
{
	mapping callouts;
	int sz;
	int *handles;

	callouts = system_query_callouts();

	handles = map_indices(callouts);

	for (sz = sizeof(handles); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[handles[sz]];

		if (callout[CO_FUNCTION] != "perform_delayed_call") {
			handles[sz] = 0;
		}
	}

	return handles - ({ 0 });
}

string system_query_merry_continuation_context(int handle)
{
	mapping callouts;
	mixed *callout;

	callouts = system_query_callouts();
	callout = callouts[handle];

	if (!callout) {
		error("No such callout");
	}

	if (callout[CO_FUNCTION] != "perform_delayed_call") {
		error("Not a merry continuation");
	}

	return callout[CO_FIRSTXARG]->describe_delayed_call("foo", ({ this_object() }), callout[CO_DELAY]);
}
