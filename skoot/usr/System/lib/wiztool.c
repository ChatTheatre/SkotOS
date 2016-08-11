# include <type.h>
# include <status.h>
# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <kernel/user.h>
# include <System.h>
# include <System/log.h>


inherit "/lib/string";
inherit wiztool LIB_WIZTOOL;
inherit auto CLONABLE;

/*
**	SkotOS additions...
*/

private mapping get_leaves(int indices...);
private void upgrade_issues(int indices...);

static string dump_value(mixed value, varargs mapping seen) {
   return auto::dump_value(value);
}

static object new_object(mixed obj, mixed args...) {
   return auto::new_object(obj, args...);
}

static
int destruct_object(mixed obref) {
   object obj;

   if (!this_object()) {
      return FALSE;
   }
   if (typeof(obref) == T_STRING) {
      obj = find_object(obref);
      if (!obj) {
	 /* for whatever reason */
	 return wiztool::destruct_object(obref);
      }
   } else if (typeof(obref) == T_OBJECT) {
      obj = obref;
   } else {
      error("bad argument to destruct_object()");
   }
   /* TODO: this destroys clone chain when wiztool selfdestructs */
   if (obj != this_object() && function_object("destruct", obj) == SYS_AUTO) {
      obj->destruct();
      return TRUE;
   }
   return wiztool::destruct_object(obj);
}

static
void cmd_clones(object user, string verb, string str) {
   mapping obset;
   object master, ob;

   if (str) {
      if (master = find_object(str)) {
	 if (function_object("query_clone_count", master) == CLONABLE) {
	    obset = ([ ]);
	    for (ob = master->query_next_clone();
		 ob != master;
		 ob = ob->query_next_clone()) {
	       obset[ob] = TRUE;
	    }
	    if (map_sizeof(obset) > 0) {
	       store(map_indices(obset));
	       return;
	    }
	    message("No known clones.\n");
	    return;
	 }
	 message("No list of clones maintained for " +
		 ur_name(master) + "\n");
	 return;
      }
      message("No object " + str + " found.\n");
      return;
   }
   message("Usage: clones <master>\n");
}

private string
dump_mapping_rec(mapping map, string prefix)
{
    int     i, sz;
    string *names, result;
    mixed  *values;

    sz     = map_sizeof(map);
    names  = map_indices(map);
    values = map_values(map);

    result = "";

    for (i = 0; i < sz; i++) {
	string str1, str2;

	if (i == sz - 1) {
	    str1 = "`-- ";
	    str2 = "    ";
	} else {
	    str1 = "|-- ";
	    str2 = "|   ";
	}
	result += prefix + str1 + names[i] + "\n";
	if (typeof(values[i]) == T_MAPPING) {
	    result += dump_mapping_rec(values[i], prefix + str2);
	}
    }
    return result;
}

private string
dump_mapping(string title, mapping map)
{
    return title + "\n" + dump_mapping_rec(map, "");
}

private mapping
create_clonetree_map(string path)
{
    int     i, sz;
    string  *dir;
    mapping map;

    map = ([ ]);

    dir = PROGDB->get_directory(path);
    sz = sizeof(dir);
    for (i = 0; i < sz; i++) {
	object  master;
	mapping sub;

	if (master = find_object(path + "/" + dir[i])) {
	    if (function_object("query_clone_count", master) == CLONABLE) {
		map[dir[i] + " [" + master->query_clone_count() + "]"] = 1;
	    }
	}
	sub = create_clonetree_map(path + "/" + dir[i]);
	if (sub && map_sizeof(sub) > 0) {
	    map[dir[i] + "/"] = sub;
	}
    }

    return map;
}

static void
cmd_clonetree(object user, string verb, string str)
{
    mapping map;

    if (str) {
	mixed *info;

	info = expand(str, 0, TRUE);
	if (info[4] != 1) {
	    message("Usage: clonetree [<directory>]\n");
	    return;
	}
	if (sizeof(info[0]) == 1) {
	    str = info[0][0];
	    info = file_info(str);
	    if (!info) {
		message(str + ": No such file or directory.\n");
		return;
	    }
	    if (info[0] >= 0) {
		message(str + ": Not a directory.\n");
	    }
	}
    } else {
	str = query_directory();
    }
    if (str == "/") {
	map = create_clonetree_map("");
    } else {
	map = create_clonetree_map(str);
    }
    message(dump_mapping(str, map));
}

static void
cmd_issues(object user, string verb, string str)
{
    mixed  *files;
    string *names;
    
    if (!str) {
	message("Usage: issues <path> ...\n");
	return;
    }
    files = expand(str, 0, TRUE);
    names = files[0];
    if (sizeof(names) == 1) {
	/* Return a simple array of issues. */
	int     i, sz, *list, len;
	string  file;
	mapping issues;

	issues = ([ ]);
	len = strlen(file = names[0]);
	if (len > 2 && file[len - 2..] == ".c") {
	    file = file[..len - 3];
	}
	list = PROGDB->query_issues(file);
	for (i = 0, sz = sizeof(list); i < sz; i++) {
	    issues[list[i]] = 1;
	}
	store(map_indices(issues));
    } else {
	/* Return a mapping from paths to arrays of issues. */
	int     i, sz;
	mapping map;

	map = ([ ]);
	for (i = 0, sz = sizeof(names); i < sz; i++) {
	    int     j, sz_j, *list, len;
	    string  file;
	    mapping issues;

	    issues = ([ ]);
	    len = strlen(file = names[i]);
	    if (len > 2 && file[len - 2..] == ".c") {
		file = file[..len - 3];
	    }
	    list = PROGDB->query_issues(file);
	    for (j = 0, sz_j = sizeof(list); j < sz_j; j++) {
		issues[list[j]] = 1;
	    }
	    map[names[i]] = map_indices(issues);
	}
	store(map);
    }
}

private mapping
create_inheritance_map(string func, int issue)
{
    int     i, sz, *inherit_list;
    mapping map;

    map = ([ ]);

    inherit_list = call_other(PROGDB, func, issue);
    sz = sizeof(inherit_list);
    for (i = 0; i < sz; i++) {
	string  path;
	mapping sub;

	path = PROGDB->query_path(inherit_list[i]) +
                " (" + inherit_list[i] + ")";
	sub = create_inheritance_map(func, inherit_list[i]);
	if (sub && map_sizeof(sub)) {
	    map[path] = sub;
	} else {
	    map[path] = 1;
	}
    }

    return map;
}

static void
cmd_inheritance(object user, string verb, string str)
{
    int    issue;
    string tmp;

    if (!str) {
	message("Usage: " + verb + " <file> | <issue>\n");
	return;
    }

    if (sscanf(str, "%d%s", issue, tmp) == 2 && !strlen(tmp)) {
	if (!PROGDB->query_path(issue)) {
	    message(issue + ": Unknown program issue.\n");
	    return;
	}
    } else {
	int    *issues, len, i, sz;
	string *names, file, *issues_str;
	mixed **files;

	files = expand(str, 1, TRUE); /* must exist, full filenames */;
	names = files[0];
	if (sizeof(names) != 1) {
	    message("Usage: " + verb +" <file> | <issues>\n");
	    return;
	}
	len = strlen(file = names[0]);
	if (len > 2 && file[len - 2..] == ".c") {
	    file = file[..len - 3];
	}
	issues = PROGDB->query_issues(file);
	switch (sizeof(issues)) {
	case 0:
	    message("There are no (unfreed) issues loaded of that object.\n");
	    return;
	case 1:
	    issue = issues[0];
	    break;
	default:
	    sz = sizeof(issues);
	    issues_str = allocate(sz);
	    for (i = 0; i < sz; i++) {
		issues_str[i] = (string)issues[i];
	    }
	    message("There is more than one (unfreed) issue of that object:\n"+
		    "    " + implode(issues_str, ", ") + ".\n");
	    return;
	}
    }
    message(dump_mapping(PROGDB->query_path(issue) + " (" + issue + ")",
			 create_inheritance_map(verb == "inherits" ?
						"query_inherits" :
						"query_inheritors",
						issue)));
}

static
void cmd_upgrade(object user, string verb, string str) {
   mixed *files;
   string *names, path;
   int i, acted, *indices;

   if (!str) {
      indices = PROGDB->query_unfreed();
      if (!sizeof(indices)) {
	 message("Upgrade: there are no unfreed issues.\n");
	 return;
      }
      message("Upgrade: Working... check results with 'unfreed'.\n");
      upgrade_issues(indices...);
      return;
   }

   files = expand(str, 1, TRUE);	/* must exist, full filenames */
   names = files[0];
   acted = FALSE;
   for (i = 0; i < sizeof(names); i ++) {
      if (!sscanf(names[i], "%s.c", path)) {
	 message("Upgrade: " + names[i] + " is not a LPC file.\n");
	 continue;
      }
      if (!status(path)) {
	 message("Upgrade: " + names[i] + " is not loaded.\n");
	 continue;
      }
      message("Upgrade: Handling " + names[i] + " ...\n");
      upgrade_issues(status(path)[O_INDEX]);
      acted = TRUE;
   }
   if (acted) {
      message("Upgrade: check results with 'unfreed'.\n");
   } else {
      message("Upgrade: nothing done.\n");
   }
}


static
void cmd_unfreed(object user, string verb, string str) {
   mapping out;
   int *ix, i;

   out = ([ ]);
   ix = PROGDB->query_unfreed();
   for (i = 0; i < sizeof(ix); i ++) {
      out[ix[i]] = PROGDB->query_path(ix[i]);
   }
   store(out);
}

static
void cmd_leaves(object user, string verb, string str) {
   int ix;
   string *leaves, path;

   if (str) {
      if (sscanf(str, "%d", ix)) {
	 if (!PROGDB->query_path(ix)) {
	    message(str + ": unknown program index\n");
	    return;
	 }
      } else if (sscanf(str, "%s.c", path)) {
	 if (status(path)) {
	    ix = status(path)[O_INDEX];
	 } else {
	    message(str + ": not loaded.\n");
	    return;
	 }
      } else {
	  message("Usage: leaves <program path>\n" +
		  "   or: leaves <issue index>\n");
	  return;
      }
      store(get_leaves(ix));
      return;
   }
   message("Usage: leaves <program path>\n" +
	   "   or: leaves <issue index>\n");
}

static
void cmd_patch(object user, string verb, string str) {
   object master, clone;
   string *leaves, path, fun;
   int i, ccount, mcount;
   int *indices;

   if (!sscanf(str, "-f %s %s", fun, str)) {
      fun = "patch";
   }

   if (sscanf(str, "%d", i)) {
      if (PROGDB->query_path(i)) {
	 indices = ({ i });
      } else {
	 message(str + ": unknown program index\n");
	 return;
      }
   } else if (str) {
      mixed *files;
      string *names, path;

      files = expand(str, 1, TRUE);	/* must exist, full filenames */
      names = files[0];
      indices = allocate(sizeof(names));

      for (i = 0; i < sizeof(names); i ++) {
	 if (sscanf(names[i], "%s.c", path)) {
	    if (status(path)) {
	       indices[i] = status(path)[O_INDEX];
	    } else {
	       message(names[i] + ": not loaded.\n");
	    }
	 } else {
	    message(names[i] + ": not a LPC file.\n");
	 }
      }
   }
   indices -= ({ nil });
   if (!sizeof(indices)) {
      message("Nothing to patch.\n");
      return;
   }
   leaves = map_indices(get_leaves(indices...));
   for (i = 0; i < sizeof(leaves); i ++) {
      master = find_object(leaves[i]);
      if (master) {
	 if (function_object("query_clone_count", master)) {
	    clone = master->query_next_clone();
	    while (clone != master) {
	       call_other(clone, fun);
	       clone = clone->query_next_clone();
	       ccount ++;
	    }
	 }
	 call_other(master, fun);
	 mcount ++;
      }
   }
   message("Total objects patched: " + mcount + " master objects, " +
	   ccount + " clones.\n");
}

static
void cmd_dpatch(object user, string verb, string str) {
   object master, clone;
   string *leaves, path, fun;
   int i, ccount, mcount;
   int *indices;

   if (!sscanf(str, "-f %s %s", fun, str)) {
      fun = "patch";
   }

   if (sscanf(str, "%d", i)) {
      if (PROGDB->query_path(i)) {
	 indices = ({ i });
      } else {
	 message(str + ": unknown program index\n");
	 return;
      }
   } else if (str) {
      mixed *files;
      string *names, path;

      files = expand(str, 1, TRUE);	/* must exist, full filenames */
      names = files[0];
      indices = allocate(sizeof(names));

      for (i = 0; i < sizeof(names); i ++) {
	 if (sscanf(names[i], "%s.c", path)) {
	    if (status(path)) {
	       indices[i] = status(path)[O_INDEX];
	    } else {
	       message(names[i] + ": not loaded.\n");
	    }
	 } else {
	    message(names[i] + ": not a LPC file.\n");
	 }
      }
   }
   indices -= ({ nil });
   if (!sizeof(indices)) {
      message("Nothing to patch.\n");
      return;
   }
   leaves = map_indices(get_leaves(indices...));
   DEBUG("leaves = " + dump_value(leaves));
   call_out("slow_dpatch", 0, user, fun, leaves, nil, nil, 0, 0);
}

static void
slow_dpatch(object user, string fun, string *leaves, object master,
	    object clone, int ccount, int mcount)
{
   if (master && clone) {
      DEBUG("[slow_dpatch] " + ur_name(clone) + " -> " + fun + "()");
      call_other(clone, fun);
      clone = clone->query_next_clone();
      ccount ++;
      if (clone == master) {
	 clone = master = nil;
      }
   } else {
      master = find_object(leaves[0]);
      leaves = leaves[1 ..];
      if (master) {
	 DEBUG("[slow_dpatch] " + ur_name(master) + " -> " + fun + "()");
	 call_other(master, fun);
	 mcount ++;
	 if (function_object("query_clone_count", master)) {
	    clone = master->query_next_clone();
	    if (clone == master) {
	       clone = master = nil;
	    }
	 } else {
	    master = nil;
	 }
      }
   }
   if (sizeof(leaves) || (master && clone)) {
      call_out("slow_dpatch", 0, user, fun, leaves, master, clone,
	       ccount, mcount);
      return;
   }
   user->message("Total objects patched: " + mcount + " master objects, " +
		 ccount + " clones.\n");
}


static
void cmd_dbstatus(object user, string cmd, string str) {
    if (str && strlen(str)) {
	message("Usage: dbstatus\n");
	return;
    }
    message(PROGDB->db_status());
}

static
void cmd_debug(object user, string cmd, string str) {
   mapping dlev, dcopy;
   mixed *files, *levels;
   string *names, path;
   int lvl, i;

   if (str && sscanf(str, "%s level %d", str, lvl) == 2) {
      files = expand(str, 1, TRUE);	/* must exist, full filenames */
      names = files[0];
      if (sizeof(names) == 0) {
	 message("File not found.\n");
	 return;
      }
      for (i = 0; i < sizeof(names); i ++) {
	 if (sscanf(names[i], "%s.c", path) && status(path)) {
	    SYSLOGD->set_debug_level(path, lvl);
	    message(path + ": debug level set to " + lvl + ".\n");
	 } else {
	    message(path + ": not a LPC file.\n");
	 }
      }
      return;
   }
   dlev = SYSLOGD->query_debug_level();
   if (map_sizeof(dlev) > 0) {
      if (str) {
	 dcopy = ([ ]);
	 names = expand(str, 1, TRUE)[0];
	 for (i = 0; i < sizeof(names); i ++) {
	    if (sscanf(names[i], "%s.c", path)) {
	       dcopy[path] = dlev[path] ? dlev[path] : 0;
	    }
	 }
	 dlev = dcopy;
      }
      files = ({ "Program", "-------" }) + map_indices(dlev);
      levels = ({ "Debug-level", "-----------" }) + map_values(dlev);
      message(tabulate(files, levels));
      return;
   }
   message("No programs are being debugged.\n");
}

private string
wrap_string(string prefix, string text, int width)
{
    int    i, sz, total, current_len;
    string *lines, current, *words, spaces;

    /*
     * Allocate enough lines for a a worst-case scenario, one word per line.
     * Each word would then be at least floor(width / 2) + 1 characters.
     */

    width -= strlen(prefix);
    lines = allocate((strlen(text) + (width / 2)) / (width / 2));

    spaces = "                                        "[0..strlen(prefix) - 1];

    words = explode(text, " ");
    sz = sizeof(words);

    current     = words[0];
    current_len = strlen(current);
    total       = 0;

    for (i = 1; i < sz; i++) {
	if (current_len + 1 + strlen(words[i]) < width) {
	    current += " " + words[i];
	    current_len += 1 + strlen(words[i]);
	} else {
	    lines[total++] = current;
	    current        = words[i];
	    current_len    = strlen(current);
	}
    }
    lines[total++] = current;
    return prefix + implode(lines[..total - 1], "\n" + spaces) + "\n";
}

private void
do_help_command(string c, mixed s, mixed d, varargs string sa...)
{
    if (typeof(s) == T_ARRAY) {
	s = implode(s, "\n             ");
    }
    if (typeof(d) == T_ARRAY) {
	d = implode(d, "\n             ");
    }
    d = implode(explode(d, "\n"), "\n             ");
    message("Command:     " + c + "\n" +
	    "Synopsis:    " + s + "\n" +
	    wrap_string("Description: ", d, 78) +
	    (sizeof(sa) ? "See also:    " + implode(sa, ", ") + "\n" : ""));
}

static void
cmd_help(object user, string cmd, string str) {
    if (!str || !strlen(str)) {
	message(
"Available commands in the wiztool:\n" +
"\n"+
"Coding / compiling / upgrading:\n" +
"  upgrade   issues    unfreed   leaves    patch     possess   depossess\n" +
"  command   dbstatus  debug     clones    code      history   clear\n" +
"  compile   clone     destruct  clonetree inherited inherits\n" +
"\n" +
"Files / directories:\n" +
"  cd        pwd       ls        cp        mv        rm        mkdir\n" +
"  rmdir     ed\n" +
"\n" +
"Access / resources:\n" +
"  access    grant     ungrant   quota     rsrc\n" +
"\n" +
"Miscellaneous:\n" +
"  people    status    swapout   statedump shutdown  reboot\n" +
"\n");
	return;
    }
    switch (lower_case(str)) {
    case "cd":
	do_help_command("cd",
			"cd [<directory>]",
			"Change your current working directory. " +
			"Default is your home-directory.");
	break;
    case "pwd":
	do_help_command("pwd",
			"pwd",
			"Show your current working directory.");
	break;
    case "ls":
	do_help_command("ls",
			"ls [-l] [(<path> | <pattern>) ...]",
			"Show the contents of a directory. " +
			"Default is the current working directory.");
	break;
    case "cp":
	do_help_command("cp",
			"cp <file> ... <target>",
			"Make a copy of one or more files in a different " +
			"location.",
			"mv");
	break;
    case "mv":
	do_help_command("mv",
			({ "mv <orig> <dest>", "mv <file> ... <directory>" }),
			"Move several files to a different directory, or " +
			"move a single file or directory to a different " +
			"location.",
			"cp");
	break;
    case "rm":
	do_help_command("rm",
			"rm <file> ...",
			"Remove one or more files.",
			"rmdir");
	break;
    case "mkdir":
	do_help_command("mkdir",
			"mkdir <path> ...",
			"Create one or more directories.",
			"rmdir");
	break;
    case "rmdir":
	do_help_command("rmdir",
			"rmdir <path> ...",
			"Remove one or more directories.",
			"mkdir", "rm");
	break;
    case "ed":
	do_help_command("ed",
			"ed [<file>]",
			"Lets you use the built-in (DGD) editor.");
	break;
    case "upgrade":
	do_help_command("upgrade",
			({ "upgrade <issue>", "upgrade [<file> ...]" }),
			"Upgrade one or more files, taking care of " +
			"upgrading objects inheriting these files, etc. " +
			"Without arguments another attempt will be made to " +
			"upgrade the objects depending on currently unfreed " +
			"issues.",
			"issues", "unfreed");
	break;
    case "issues":
	do_help_command("issues",
			"issues <file> ...",
			"Returns for each given file an array of the " +
			"existing (if any) issues.  If one file is specified" +
			"it'll return a single array, when more are given " +
			"you'll get a mapping from filenames to arrays.");
	break;
    case "unfreed":
	do_help_command("unfreed",
			"unfreed",
			"Returns the issues that have been destructed but " +
			"not yet released by DGD, because there are other " +
			"objects around still that inherit them.",
			"compile", "destruct", "upgrade");
	break;
    case "leaves":
	do_help_command("leaves",
			({ "leaves <program path>", "leaves <issue index>" }),
			"Given the specific object, will return the 'outer' " +
			"programs that depend upon it.  Usually this will " +
			"result in the clonables and non-lib objects, but " +
			"it may also contain lib objects that have not been " +
			"inherited (yet).",
			"inherited", "inherits");
	break;
    case "patch":
	do_help_command("patch",
			({ "patch <program path>", "patch <program index>" }),
			"<XXX>");
	break;
    case "dbstatus":
	do_help_command("dbstatus",
			"dbstatus",
			"Provides a summary of the program-database status, " +
			"whether there are any inconsistencies, etc.");
	break;
    case "debug":
	do_help_command("debug",
			"debug [<path> level <level>]",
			"Shows or alters the debugging level of an object.");
	break;
    case "clones":
	do_help_command("clones",
			"clones <master>",
			"Lists what clones exist for a given master-object.");
	break;
    case "code":
	do_help_command("code",
			"code <LPC-code>",
			"Allows you to execute LPC code without having to " +
			"specifically create an object, as this has been " +
			"automated for you.  For your convenience the files "+
			"<float.h>, <limits.h>, <status.h>, <trace.h> and " +
			"<type.h> are automatically included and there are " +
			"26 variables (a..z) of type mixed to use.",
			"clear", "clone", "compile", "history");
	break;
    case "history":
	do_help_command("history",
			"history [<num>]",
			"Show command history.",
			"clear", "clone", "code", "compile");
	break;
    case "clear":
	do_help_command("clear",
			"clear",
			"Clear command history.",
			"clone", "code", "compile", "history");
	break;
    case "compile":
	do_help_command("compile",
			"compile <file> ...",
			"Compile one or more objects.",
			"clear", "clone", "code", "destruct", "history");
	break;
    case "clone":
	do_help_command("clone",
			({ "clone <master>", "clone $<ident>" }),
			"Create a clone from an already existing " +
			"master-object.",
			"clear", "code", "destruct", "history");
	break;
    case "destruct":
	do_help_command("destruct",
			"destruct <object>",
			"Destruct an object, which can be a clone, a " +
			"master object, a leaf object, or a (potentially) " +
			"inherited object. " +
			"If you destruct an inherited object, DGD will not " +
			"free up the relevant bit of memory until all the " +
			"inheriting objects have been destructed or for " +
			"instance recompiled with a newer issue.",
			"compile", "unfreed", "upgrade");
	break;
    case "clonetree":
	do_help_command("clonetree",
			"clonetree [<directory>]",
			"Shows where and how many clones there are, given a " +
			"start-directory.  Defaults to the working " +
			"directory.");
	break;
    case "inherits":
	do_help_command("inherits",
			"inherits <file> | <issue>",
			"Shows what programs are inherited, directly or " +
			"indirectly, by this program.\n",
			"inherited", "leaves");
	break;
    case "inherited":
	do_help_command("inherited",
			"inherited <file> | <issue>",
			"Shows what programs inherit, directly or " +
			"indirectly, this program.\n",
			"inherits", "leaves");
	break;
    case "access":
	do_help_command("access",
			({ "access [<user>]", "access <directory>", "access global" }),
			"Shows what access you or someone else has.",
			"grant", "ungrant");
	break;
    case "grant":
	do_help_command("grant",
			({ "grant <user> access",
			   "grant <user> <directory> [read | write | full]",
			   "grant global <directory>" }),
			"Granting access means making someone a developer. " +
			"Granting a user access to a directory gives that " +
			"user the appropriate access to that directory and " +
			"any files and/or subdirectories that it leads to. " +
			"Granting global access will give anyone access to " +
			"a given directory.",
			"access", "ungrant");
	break;
    case "ungrant":
	do_help_command("ungrant",
			({ "ungrant <user> access",
			   "ungrant <user> <directory>",
			   "ungrant global <directory>" }),
			"Revokes the access previously granted in various " +
			"ways.",
			"access", "grant");
	break;
    case "quota":
	do_help_command("quota",
			({ "quota",
			   "quota <user>",
			   "quota <user> <rsrc>",
			   "quota <user> <rsrc> <limit>" }),
			"...",
			"rsrc");
	break;
    case "rsrc":
	do_help_command("rsrc",
			"rsrc [<rsrc>]",
			"Display over-all resource usage, or per resource-type.",
			"quota");
	break;
    case "people":
	do_help_command("people",
			"people",
			"Who is online, and from where.");
	break;
    case "status":
	do_help_command("status",
			"status",
			"Describe the current status in terms of objects, " +
			"callouts, etc.");
	break;
    case "swapout":
	do_help_command("swapout",
			"swapout",
			"Swapout all objects to the swap-device.");
	break;
    case "statedump":
	do_help_command("statedump",
			"statedump",
			"Create a new statedump.");
	break;
    case "shutdown":
	do_help_command("shutdown",
			"shutdown",
			"Shutdown the game, without any further ado.");
	break;
    case "reboot":
	do_help_command("reboot",
			"reboot",
			"Shutdown the game, after first creating a final statedump to restart from.");
	break;
    default:
	message("No help available for this command, unfortunately.\n");
	break;
    }
}

/*
**	Find all non-inheritable programs that rely on the
**	inheritable program (issue) pointed at by prog_ix.
*/

private mapping
get_leaves(int indices...) {
   mapping leaves, seen;
   string path;
   int i, ix, *queue, *inheritors;

   queue = indices[..];
   seen = ([ ]);
   leaves = ([ ]);

   while (sizeof(queue)) {
      ix = queue[0]; queue = queue[1 ..];
      if (!seen[ix]) {
	 seen[ix] = 1;
	 inheritors = PROGDB->query_inheritors(ix);
	 if (inheritors && sizeof(inheritors)) {
	    queue += inheritors;
	 } else {
	    leaves[PROGDB->query_path(ix)] = ix;
	 }
      }
   }
   return leaves;
}


private
void upgrade_issues(int indices...) {
   string *leaves, path;
   int ix, i;

   rlimits(0; -1) {
      for (i = 0; i < sizeof(indices); i ++) {
	 ix = indices[i];

	 if (path = PROGDB->query_path(ix)) {
	    if (sscanf(path, "%*s/lib/") && PROGDB->query_inheritors(ix) &&
		sizeof(PROGDB->query_inheritors(ix))) {
	       if (status(path) && status(path)[O_INDEX] == ix) {
		  /* the requested issue is the loaded one, so nuke it */
		  destruct_object(path);
	       }
	    } else {
	       /* just recompile */
	       Debug("Recompiling " + path + " directly...");
	       wiztool::compile_object(path);
	       indices[i] = -1;
	    }
	 } else {
	    /* Inconsistency! */
	    indices[i] = -1;
	 }
      }
      indices -= ({ -1 });
      leaves = map_indices(get_leaves(indices...));
      for (i = 0; i < sizeof(leaves); i ++) {
	 catch {
	    XDebug("Recompiling " + leaves[i] + "...");
	    wiztool::compile_object(leaves[i]);
	 }
      }
   }
}

object compile_object(string name, string source...) {
   return wiztool::compile_object(name, source...);
}
