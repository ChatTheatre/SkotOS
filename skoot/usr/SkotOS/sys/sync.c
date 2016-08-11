/*
 * ~SkotOS/sys/sync.c
 *
 * Provides the means to automagically start archive-compilation and archive
 * parsing, depending on what flag-files are created externally, and report
 * back when it's all done.
 */

inherit "/lib/string";

/* For the save_object/restore_object requests */
private inherit "/lib/file";

inherit "/usr/SkotOS/lib/merryapi";
private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/XML/lib/xmd";
inherit "/usr/XML/lib/xmlgen";
inherit "/usr/XML/lib/xmlparse";

# include <type.h>
# include <System.h>

# define ST_INITIAL     nil
# define ST_PACK_WAIT   1
# define ST_PACK_RUN    2
# define ST_UNPACK_WAIT 3
# define ST_UNPACK_RUN  4

string  current;
string  *stack;
string  *groups;
mapping states;
mapping logs;

object  relay;

void
create()
{
    set_object_name("SkotOS:Sync");

    stack  = ({ });
    groups = ({ "Generic", "SkotOS", "Socials", "TextIF", "Pending", "Reverse", "Meta" });
    states = ([ ]);
    logs   = ([ ]);
    call_out("check", 1);
}

void
patch()
{
   groups |= ({ "SkotOS", "Socials", "Pending", "Reverse", "Meta" });

   set_object_name("SkotOS:Sync");
}

void clear_state() {
  stack = ({ });
  logs = ([ ]);
  states = ([ ]);
  current = nil;
}

string *
query_groups()
{
   return groups[..];
}

object
query_relay()
{
    return relay;
}

mixed apply_relay(string func, string mode, mapping args);

static void
check_stack()
{
    if (current) {
	return;
    }
    if (sizeof(stack) == 0) {
	return;
    }
    /* Pop first one off of the stack. */
    current = stack[0];
    stack = stack[1..];

    switch (states[current]) {
    case ST_PACK_WAIT:
	states[current] = ST_PACK_RUN;

	/* Alert Merry layer of a new sync about to begin. */
	apply_relay("initialize", "archive", ([ "group": current ]));

	call_other(find_object("/usr/" + current + "/initd"),
		   "compile_archives",
		   nil, 0, this_object(), "archive_done", current, "archive");
	break;
    case ST_UNPACK_WAIT:
	states[current] = ST_UNPACK_RUN;

	/* Alert Merry layer of a new sync about to begin. */
	apply_relay("initialize", "spawn", ([ "group": current ]));

	call_other(find_object("/usr/" + current + "/initd"),
		   "spawn_now",
		   1, FALSE, this_object(), "archive_done", current, "spawn");
	break;
    default:
	error("Unexpected state for " + current + ": " + dump_value(states[current]));
    }
}

void
check()
{
    int i, sz;
    sz = sizeof(groups);
    for (i = 0; i < sz; i++) {
	if (states[groups[i]] == ST_INITIAL) {
	    string filename, request, request_extra;

	    filename = "/usr/SkotOS/data/requests/" + groups[i];
	    request = read_file(filename);
	    if (request == nil) {
		continue;
	    }
	    sscanf(request, "%s\n%s", request, request_extra);
	    switch (request) {
	    case "archive":
		stack += ({ groups[i] });
		states[groups[i]] = ST_PACK_WAIT;
		logs[groups[i]] = ctime(time()) + ": Request processed, archiving pending.\n";
		remove_file(filename);
		break;
	    case "spawn":
		stack += ({ groups[i] });
		states[groups[i]] = ST_UNPACK_WAIT;
		logs[groups[i]] = ctime(time()) + ": Request processed, spawning pending.\n";
		remove_file(filename);
		break;
	    case "save_object": {
		/* This one is done instantly, no delay. */
		string extra_save, extra_where, program;
		object ob, result;
		mixed state;

		sscanf(request_extra, "%s\n%s\n", extra_save, extra_where);

		ob = find_object(extra_save);
		if (!ob) {
		    remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
		    remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result", "ERROR\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "Object not found.\n");
		    remove_file(filename);
		    break;
		}
		state = export_state(ob, nil, nil, nil, TRUE);
		program = ur_name(ob);
		sscanf(program, "%s#", program);
		state = xmd_elts("object",
				 ({ "program", program }),
				 state);
		result = new_object("/data/data");
		generate_xml(state, result);

		write_data_to_file(extra_where, result);

		remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
		remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");
		write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result", "OK\n"); /* or ERROR */
		write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			   "This space intentionally left blank.\n");
		remove_file(filename);
		break;
	    }
	    case "restore_object": {
		/* This one is done instantly, no delay. */
		string extra_restore, extra_where, text;
		object ob;
		mixed state;

		sscanf(request_extra, "%s\n%s\n", extra_restore, extra_where);
		ob = find_object(extra_restore);
		if (!ob) {
		    remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
		    remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result", "ERROR\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "Object not found.\n");
		    remove_file(filename);
		    break;
		}

		text = read_file(extra_where);
		if (!text) {
		    remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
		    remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result", "ERROR\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "File not found.\n");
		    remove_file(filename);
		}

		state = parse_xml(text, extra_where, FALSE, TRUE);
		state = query_colour_value(xmd_force_to_data(state))[0];
		state = query_colour_value(xmd_force_to_data(xmd_content(state)));
		import_state(ob, state[0]);

		remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
		remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");
		write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result", "OK\n"); /* or ERROR */
		write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			   "This space intentionally left blank.\n");
		remove_file(filename);
		break;
	    }
	    case "pending_initialize": {
	       apply_relay("initialize", "pending", ([ ]));
	       remove_file(filename);
	       remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
	       remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");

	       write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result",
			  "OK\n");
	       write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			  "This space intentionally left blank.\n");
	       
	       break;
	    }
	    case "pending_finalize": {
	       apply_relay("finalize", "pending", ([ ]));
	       remove_file(filename);
	       remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
	       remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");

	       write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result",
			  "OK\n");
	       write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			  "This space intentionally left blank.\n");
	       
	       break;
	    }
	    case "compile": {
		string extra_owner, extra_filename;
		object ob, wiztool;

		remove_file(filename);
		remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".result");
		remove_file("/usr/SkotOS/data/requests/" + groups[i] + ".log");


		sscanf(request_extra, "%s\n%s\n%s", extra_owner, extra_filename, request_extra);
		ob = find_object(extra_filename);
		if (ob) {
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result",
			       "OK\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "Object already exists.\n");
		    break;
		}
		wiztool = DEV_USERD->query_wiztool(extra_owner);
		if (!wiztool) {
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result",
			       "ERROR\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "Account does not exist or has insufficient access.\n");
		    break;
		}
		catch {
		    ob = wiztool->compile_object(extra_filename);
		    if (!sscanf(extra_filename, "%*s/lib/")) {
			call_other(ob, "???");
		    }
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result",
			       "OK\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "Object compiled successfully.\n");
		} : {
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".result",
			       "ERROR\n");
		    write_file("/usr/SkotOS/data/requests/" + groups[i] + ".log",
			       "Object failed to compile: \"" +
			       SYSLOGD->query_last_error() +
			       "\"\n");
		}
		break;
	    } 
	    case "patch": {
	    }
	    default:
		DEBUG("SYNC:Unrecognized request in ~SkotOS/data/requests/" + groups[i] + ": " + request);
		break;
	    }
	    
	}
    }
    check_stack();
    call_out("check", 1);
}

void
archive_done(int success, string log, string group, string type)
{
    current = nil;

    if (success) {
	write_file("/usr/SkotOS/data/requests/" + group + ".result", "OK\n");
    } else {
	write_file("/usr/SkotOS/data/requests/" + group + ".result", "ERROR\n");
    }
    write_file("/usr/SkotOS/data/requests/" + group + ".log", log);
    states[group] = ST_INITIAL;

    /* Notify Merry layer of a sync having finished. */
    apply_relay("finalize", type,
		([ "group": group, "success": success, "log": log ]));
    check_stack();
}

int query_method(string method) {
    switch (lower_case(method)) {
    case "set_relay":
    case "clear_relay":
    case "query_relay":
    case "query_status":
	return TRUE;
    }
}

mixed call_method(string method, mapping args) {
    switch (lower_case(method)) {
    case "set_relay":
	if (typeof(args["relay"]) != T_OBJECT) {
	    error("set_relay expects $relay");
	}
	relay = args["relay"];
	break;
    case "clear_relay":
	relay = nil;
	break;
    case "query_relay":
	return relay;
    case "query_status":
	break;
    }
}

mixed apply_relay(string func, string mode, mapping args) {
    if (relay) {
	catch {
	    return run_merry(relay, func, mode, args);
	} : {
            switch (func) {
	    case "initialize":
            case "finalize":
            case "create":
                return TRUE;
            case "clear":
                return FALSE;
            case "configure":
                return FALSE;
            }
        }
    }

    /*
     * Backward compatible behaviour in case there is no relay to manage it
     * or if there is some error.
     */
    switch (mode + ":" + func) {
    case "spawn:initialize":
    case "spawn:finalize":
	/* Purely informational */
	return TRUE;
    case "spawn:create":
	/* A.k.a create this new object? */
	return TRUE;
    case "spawn:clear":
	/* A.k.a move aside or not? */
	return TRUE;
    case "spawn:configure": {
	object ob;
	mixed revisions, revision;

	/*
	 * Parameters:
	 * args["object"]
	 * args["objectname"]
	 * args["revision"]
	 */
	revision = 0;

	ob = args["object"];
	revisions = ob->query_property("revisions");
	if (typeof(revisions) == T_ARRAY && sizeof(revisions) % 3 == 0) {
	    revision = revisions[sizeof(revisions) - 3];
	    if (typeof(revision) != T_INT) {
		revision = 0;
	    }
	}
	if (!revision || !args["revision"] || revision != args["revision"]) {
	    return TRUE;
	}
	return FALSE;
    }
    default:
	break;
    }
    /* Fall through */
    return TRUE;
}

mixed shentino()
{
	return states;
}
