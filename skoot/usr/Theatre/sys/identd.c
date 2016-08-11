/*
 * ~Theatre/sys/identd.c
 *
 * This object's sole purpose is to give objects a meaningful name in the
 * call-traces that ~System/sys/syslogd.c creates from atomic_error() and
 * runtime_error().
 *
 * This is an exact copy of ~Marrach/sys/identd.c
 */

# include <System.h>
# include <SkotOS.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

/*
 * Bit much just for describing one object, but 
 */

inherit "/usr/SkotOS/lib/auth";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SAM/lib/sam";

private int locked;

static void
create()
{
    SYSLOGD->set_identify_object(this_object());
}

string
syslogd_identify(string objname)
{
    if (previous_program() == SYSLOGD) {
	if (objname) {
	    string username, charname, name;
	    object ob;

	    ob = find_object(objname);
	    if (ob) {
	       if (sscanf(objname, "/usr/SkotOS/merry/%*s")) {
		  /* merry nodes play different roles at different times */
		  if (name = ob->query_description()) {
		     return "M<" + name + ">";
		  }
		  return nil;
	       }
		if (sscanf(objname, "%s#%*d", name)) {
		    object user;

		    switch (name) {
		    case "/kernel/obj/binary":
		    case "/kernel/obj/telnet":
		        user = ob->query_user();
			if (user) {
			    username = user->query_name();
			    if (username) {
				return "'" + username + "'";
			    }
			}
			break;
		    case "/usr/System/obj/devuser":
		    case "/usr/HTTP/obj/http":
			username = ob->query_name();
			if (username) {
			    return "'" + username + "'";
			}
			break;
		    case "/usr/TextIF/obj/user":
			username = ob->query_name();
			if (username) {
#if 0
			    object body;

			    body = ob->query_body();
			    if (body) {
				charname = describe_one(body, nil, nil, STYLE_DEFINITE);
				if (charname) {
				    return "'" + username + " / " + charname + "'";
				}
			    }
#endif
			    return "'" + username + "'";
			}
			break;
		    default:
			break;
		    }
		}
		name = name(ob);
		if (name != objname) {
		    return name;
		}
	    }
	}
    }
    return nil;
}

/*
**	This function spends too much time deconstructing strings
**	put together partially in syslogd.c and partially in the
**	merryapi.c itself (see the sscanf below). It would take a
**	fair bit of work to get those parameters to this function
**	in a primal form, and I think we have more important stuff
**	to do first...
**
**	Zell
*/




atomic
void log_merry_error(string error, mapping *wtrace) {
   mapping culprits;
   string obname, signame, *bits, mhash, hashstr, desc;
   object obj;
   mixed *out;
   int i, j, offset;

   if (locked) {
       /* Don't go into endless loops, we have enough problems already. */
       return;
   }
   /*
    * Make sure that any statement that exits out of here prematurely sets the locked back to FALSE!
    */
   locked = TRUE;

   culprits = ([ ]);
   hashstr = error;

   offset = SYS_MERRY->query_line_offset();
   out = allocate(sizeof(wtrace) * 3);
   for (i = 0; i < sizeof(wtrace); i ++) {
      if (sscanf(wtrace[i]["rawprog"], "/usr/SkotOS/merry/%s", mhash)) {
	 desc = wtrace[i]["rawprog"]->query_description();
	 if (!desc) {
	    continue;
	 }
	 if (!sscanf(desc, "%s/%s", obname, signame)) {
	    obname = desc;
	    signame = "";
	 }

	 /* we have to explicitly look for embedded SAM */
	 if (strlen(obname) > 5 && obname[.. 3] == "SAM[" && obname[strlen(obname) - 1] == ']') {
	    /* Can't use sscanf() b/c it would potentially find the ']' of the a spawn. */
	    obname = obname[4 .. strlen(obname) - 2];
	 }

	 /* very likely a spawn: use name of parent */
	 sscanf(obname, "[%s]#%*d", obname);

	 out[j ++] = wtrace[i]["line"] - offset;
	 out[j ++] = obname;
	 out[j ++] = signame;
	 hashstr += ":" + out[j-3] + ":" + out[j-2] + "/" + out[j-1];
	 bits = explode(obname, ":");
	 culprits[bits[0]] = TRUE;
	 if (obj = find_object(obname)) {
	    catch {
	       string creator;

	       creator = obj->query_property("log:creator");
	       if (creator && strlen(creator)) {
		  culprits["Creator " + creator] = TRUE;
	       }
	    }
	 }
      }
   }
   if (map_sizeof(culprits) && j > 0) {
      hashstr = to_hex(hash_md5(hashstr));
      out = out[.. j-1];
      bits = map_indices(culprits);

      for (i = 0; i < sizeof(bits); i ++) {
	 int k, len;
	 string name;

	 name = bits[i];
	 len = strlen(name);
	 for (k = 0; k < len; k++) {
	    /* Flatten the offending characters to '_' */
	    switch(name[k]) {
	    case 0..31:
	    case 127:	
	       /* unsafe */
	    case '<': case '>': case '\"' : case '#': case '%':
	    case '{': case '}': case '|': case '\\': case '^':
	    case '~': case '[': case ']': case '`': case '\'':
	       name[k] = '_';
	       break;
	    }
	 }
	 obj = find_object("Log:" + name);
	 if (!obj) {
	    obj = clone_object("/obj/properties");
	    obj->set_object_name("Log:" + name);
	 }
	 obj->set_property("err:" + hashstr + ":error", error);
	 obj->set_property("err:" + hashstr + ":call-trace", out);
	 obj->set_property("err:" + hashstr + ":most-recently-at", time());
	 obj->set_property(
	    "err:" + hashstr + ":total-times",
	    Int(obj->query_property("err:" + hashstr + ":total-times")) + 1);
      }
   }

   /*
    * Remove block, so the next call will proceed again.
    */
   locked = FALSE;
}

void
syslogd_handle_anonymous_error(string error, mapping *wtrace,
			       mapping *cline, string str, varargs int caught) {
   /* this is a 'background' error without originator */
   log_merry_error(error + (caught ? " [caught]" : ""), wtrace);
}

void
syslogd_handle_error(object obj, string error, mapping *wtrace,
		     mapping *cline, string str, varargs int caught) {
    log_merry_error(error + (caught ? " [caught]" : ""), wtrace);
    if (!caught && obj) {
	if (obj->html_connection()) {
#if 1
	    obj->handle_error(error, wtrace, cline);
#endif
	} else {
	    object body;

	    if (is_root(obj->query_name()) ||
		is_builder(obj->query_name())) {
	       /* A storyhosts or other staff-member. */
		obj->message("\n" + str);
	    } else if (obj->query_udat() &&
		       sizeof(rfilter(obj->query_udat()->query_bodies(),
			       "query_property", "GUIDE"))) {
		/* A storyguide user. */
		obj->message("\n" + str);
	    } else {
		obj->message("\n[" + ctime(time())[4..18] + "] " +
			     "Oops!  Your action could not be completed " +
			     "correctly, something went awry!  Please report "+
			     "this to CE including a description of what you " +
			     "were doing (exactly what you typed) and what " +
			     "lead up to that.\n");
	    }
	}
    }
}
