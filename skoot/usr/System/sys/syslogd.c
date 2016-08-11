/*
**	~System/sys/syslogd.c
**
**	This object handles error reporting. At its current state it
**	duplicates the code in /kernel/sys/driver apart from the HTTP
**	awareness; this may change.
**
**	It also supports the system-wide debug-message handling.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <trace.h>
# include <kernel/kernel.h>
# include <kernel/tls.h>
# include <System.h>
# include <System/log.h>

private inherit "/kernel/lib/api/tls";

# include "../lib/textdata.c"

mapping debug_level;

object identify_object;

private int atomic_looping;
private int runtime_looping;

static
void create() {
   ::create();

   debug_level = ([
      "/usr/System/sys/progdb": 0
      ]);

   DRIVER->set_error_manager(this_object());
}


atomic
void set_originator(object ob) {
   if (previous_program() == SYSLOGD || previous_program() == SYS_AUTO) {
      set_tlvar(0, ob);
   }
}

object query_originator() {
   object user;

   if (previous_program() == SYSLOGD || previous_program() == SYS_AUTO) {
      if (user = get_tlvar(0)) {
	 /* log */
      } else if (user = ::this_user()) {
	 /* log */
      } else {
	 /* log */
      }
   }
   return user;
}

string query_last_error() {
   if (get_tlvar(1)) {
      return get_tlvar(1)[0];
   }
}

mixed **query_last_trace() {
   if (get_tlvar(1)) {
      mixed **trace;
      int i;

      trace = get_tlvar(1)[1];
      /* remove arguments */
      for (i = 0; i < sizeof(trace); i ++) {
	 trace[i] = trace[i][.. TRACE_FIRSTARG - 1];
      }
      return trace;
   }
}

mixed query_last_compile_errors() {
   if (get_tlvar(2)) {
      return get_tlvar(2);
   }
}

void
set_identify_object(object ob)
{
    identify_object = ob;
}

void rethrow_atomically() {
   error("PACKET: " + query_last_error() + "|" + mixed_to_ascii(get_tlvar(2)));
}

/* Usually called via the SYSLOG() macro */
void
syslogd(int priority, string text)
{
    string label;

    switch (priority) {
    default:
	/* Fallback in case someone tries to be 'funny'. */
    case LOG_DEBUG:
	label = "debug";
	break;
    case LOG_INFO:
	label = "info";
	break;
    case LOG_WARNING:
	label = "warning";
	break;
    case LOG_ERROR:
	label = "error";
	break;
    case LOG_CRITICAL:
	label = "critical";
	break;
    }
    label += ":";
    DRIVER->message(label + implode(explode(text, "\n"), "\n" + label) + "\n");
}

/* Usually called from the Debug() and and XDebug() macros */
void
syslogd_debug(string name, string text)
{
    if (!name) {
	if (previous_object()) {
	    name = name(previous_object());
	} else {
	    name = "UNKNOWN";
	}
    }
    syslogd(LOG_DEBUG, "[ " + name + "] " + text + "\n");
}

/*
 * Backward compatibility, should be removed as soon as all objects
 * have been upgraded.
 */

void syslog(string text)                    { syslogd(LOG_DEBUG, text); }
void log(string name, string str, int dlev) { syslogd_debug(name, str); }

void
set_debug_level(string p, int i)
{
    if (i == 0) {
	debug_level[p] = nil;
    } else {
	debug_level[p] = i;
    }
}

mixed
query_my_debug_level()
{
    mixed d;

    if (d = debug_level[previous_program()]) {
	return d;
    }
    return 0;
}

mapping
query_debug_level()
{
    return debug_level[..];
}


/* these two are called from /kernel/sys/driver */

/*
 * NAME:	atomic_error()
 * DESCRIPTION:	log a runtime error in atomic code
 */
void atomic_error(string str, int atom, mixed **trace) {
   string line, function, progname, objname, last_prog, last_obj;
   mixed *arr;
   object obj;
   int i, sz, len;

   if (atomic_looping) {
      return;
   }
   atomic_looping = TRUE;

   sz = sizeof(trace) - 1;

   str += " [atomic]\n";

   for (i = atom; i < sz; i++) {
      progname = trace[i][TRACE_PROGNAME];
      len = trace[i][TRACE_LINE];
      if (len == 0) {
	 line = "    ";
      } else {
	 line = "    " + len;
	 line = line[strlen(line) - 4 ..];
      }

      function = trace[i][TRACE_FUNCTION];
      len = strlen(function);
      if ((progname == AUTO || progname == SYS_AUTO) &&
	  i != sz - 1 && len > 3) {
	 switch (function[.. 2]) {
	 case "bad":
	 case "_F_":
	 case "_Q_":
	    continue;
	 }
      }
      objname = trace[i][TRACE_OBJNAME];

      if (objname != last_obj) {
	  string obj_id;

	  if (identify_object) {
	      catch {
		  obj_id = identify_object->syslogd_identify(objname);
	      }
	  }
	  last_obj  = objname;
	  last_prog = objname;
	  if (obj_id) {
	      if (strlen(objname) < 30) {
		  objname = (objname + "                           ")[..29];
	      }
	      objname += " " + obj_id;
	  }
	  str += "    " + " " + objname + "\n";
      }
      if (progname == last_prog) {
	  progname = "-";
      } else {
	  last_prog = progname;
      }
      if (strlen(progname) < 30) {
	  progname = (progname + "                              ")[..29];
      }
      str += line + "     " + progname + " " + function + "\n";
   }

   if (arr = get_tlvar(2)) {
      for (i = 0; i < sizeof(arr); i ++) {
	 str += arr[i][0] + ", " + arr[i][1] + ": " + arr[i][2] + "\n";
      }
      set_tlvar(2, nil);
   }

   syslogd(LOG_ERROR, str);

   atomic_looping = FALSE;
}


void
runtime_error(string error, int caught, mixed **trace) {
   mapping *wtrace, *cline;
   string str, line, function, progname, objname, last_prog, last_obj;
   string rawprog;
   mixed *arr;
   int i, sz, len;
   object obj;

   if (previous_program() != DRIVER) {
      return;
   }

   if (runtime_looping) {
      return;
   }
   runtime_looping = TRUE;

   if (error == "*") {
      mixed *tmp;

      if (tmp = get_tlvar(1)) {
	 error = tmp[0];
	 trace = tmp[1];
      } else {
	 error = "[rethrow of nonexistent error]";
      }
   } else if (sscanf(error, "PACKET: %s|%s", error, str)) {
      arr = ascii_to_mixed(str);
      set_tlvar(1, ({ error, trace, nil }));
      set_tlvar(2, arr);
   } else {
      set_tlvar(1, ({ error, trace, nil }));
   }

   str = error;
   if (caught != 0) {
      str += " [caught]";
   }
   str += "\n";
   wtrace = ({ });
   sz = sizeof(trace) - 1;
   for (i = 0; i < sz; i++) {
      progname = trace[i][TRACE_PROGNAME];
      len = trace[i][TRACE_LINE];
      if (len == 0) {
	 line = "    ";
      } else {
	 line = "    " + len;
	 line = line[strlen(line) - 4 ..];
      }

      function = trace[i][TRACE_FUNCTION];
      len = strlen(function);
      if ((progname == AUTO || progname == SYS_AUTO) &&
	  i != sz - 1 && len > 3) {
	 switch (function[.. 2]) {
	 case "bad":
	 case "_F_":
	 case "_Q_":
	    continue;
	 }
      }
      objname = trace[i][TRACE_OBJNAME];

      if (objname != last_obj) {
	  string obj_id;

	  if (identify_object) {
	      catch {
		  obj_id = identify_object->syslogd_identify(objname);
	      }
	  }
	  last_obj  = objname;
	  last_prog = objname;
	  if (obj_id) {
	      if (strlen(objname) < 30) {
		  objname = (objname + "                           ")[..29];
	      }
	      objname += " " + obj_id;
	  }
	  str += "    " + " " + objname + "\n";
      }
      rawprog = progname;

      if (progname == last_prog) {
	  progname = "-";
      } else {
	  last_prog = progname;
      }
      if (strlen(progname) < 30) {
	  progname = (progname + "                              ")[..29];
      }
      str += line + "     " + progname + " " + function + "\n";

      /* build a structured version for possible HTML display */
      wtrace += ({ ([
         "line": trace[i][TRACE_LINE],
	 "function": function,
	 "progname": progname,
	 "rawprog": rawprog,
	 "objname": objname
	 ]) });
   }

   if (arr = get_tlvar(2)) {
      /* compiler errors */
      cline = allocate(sizeof(arr));
      for (i = 0; i < sizeof(arr); i ++) {
	 str += arr[i][0] + ", " + arr[i][1] + ": " + arr[i][2] + "\n";
	 /* build a structured version for possible HTML display */
	 cline[i] = ([
	    "file": arr[i][0],
	    "line": arr[i][1],
	    "error": arr[i][2]
	    ]);
      }
   }

   syslogd(LOG_ERROR, str);

   if (caught == 0) {
      obj = query_originator();
      if (identify_object) {
	 catch {
	    if (obj) {
	       identify_object->syslogd_handle_error(obj, error, wtrace,
						     cline, str);
	    } else {
	       identify_object->syslogd_handle_anonymous_error(error, wtrace,
							       cline, str);
	    }
	 }

      } else if (obj) {
	   /* Fallback. */
	   if (obj->html_connection()) {
	       obj->handle_error(error, wtrace, cline);
	       runtime_looping = FALSE;
	       return;
	   }
	   /* or plaintext ... */
	   obj->message(str);
       }
   } else {
      if (identify_object) {
	 obj = query_originator();
	 catch {
	    if (obj) {
	       identify_object->syslogd_handle_error(obj, error, wtrace,
						     cline, str, TRUE);
	    } else {
	       identify_object->syslogd_handle_anonymous_error(error, wtrace,
							       cline, str, TRUE);
	    }
	 }
      }
   }

   runtime_looping = FALSE;
}

void
compile_error(string file, int line, string err) {
   mixed *arr;

   if (previous_program() != DRIVER) {
      return;
   }
   arr = get_tlvar(2);
   if (!arr) {
      arr = ({ });
   }
   arr += ({ ({ file, line, err }) });
   set_tlvar(2, arr);
}
