/*
**	~System/sys/progdb.c
**
**	This object maintains a database of all currently loaded program
**	issues, their exact dependencies on other program issues (through
**	inheritance), and also the dependency of other issues on them.
**
**
**	Motivation:
**
**	When a program is destructed but still has dependents in the form
**	of inheritors or clones, it is not freed but remains in memory.
**	Only when the last reference is removed is the issue freed.
**
**	Because the theoretical uptime of a persistent Mud is infinite,
**	and DGD does not (by design) allow access to such destructed but
**	not-yet-freed issues, it is essential that we keep track of the
**	dependencies ourselves.
**
**	Apart from the necessity of doing detailed maintenance-style
**	cleanups, the great practical application of this database is
**	through program upgrading. Consider a file like /lib/string,
**	which is inherited by practically every program in the lib, and
**	imagine that a bad bug has been found in it, and is now fixed.
**
**	The problem then becomes to recompile every program that depends 
**	on /lib/string, so as to start using the new version. Without a
**	database such as this, this cannot be done in a persistent LPMud.
**
**	------------------------------------------------------------------
**	Program issues are identified by their unique integer number, or
**	index. We need to store information associated with each issue,
**	and we'd also like to be able to do lookups purely by path name.
**
**	Because this database will grow very large, we do not store all
**	the actual data in this object. Instead, we associate with each
**	directory in the filesystem hierarchy a clone of ~/obj/dbnode.c
**	and let that clone keep track of all the issues corresponding to
**	files in that directory. For example, there is a clone associated
**	with /lib which keeps track of all issues of e.g. /lib/string.c
**	------------------------------------------------------------------
**
**	TODO: All the upgrading stuff is limited to ARRAYSIZE objects.
**
**	Copyright Skotos Tech Inc 1999
*/

# define LOGNAME	"PROGDB"

# include <status.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <kernel/access.h>
# if 0
# include <kernel/net.h>
# endif
# include <kernel/rsrc.h>
# include <kernel/objreg.h>
# include <kernel/tls.h>
# include <System/log.h>
# include <System.h>

# define FROM_WIZTOOL() (previous_program() == SYS_LIB_WIZTOOL)

# define BITS		10
# define CHUNK		(1 << BITS)
# define Hi(ix)		((ix) >> BITS)
# define Lo(ix)		((ix) & (CHUNK-1))
# define Node(ix)	(ix_to_node[Hi(ix)][Lo(ix)])

int wild_clones;	/* count of all clones not under ~System control */
mapping *ix_to_node;	/* maps unique object index to directory node */
mapping dir_to_node;	/* maps directory name to directory node */
mapping	unfreed;	/* keeps track of destructed but unfreed issues */
mapping destructing_hooks;

mapping handles;

static void grow_ix_to_node();
static void add_program(string path, int prog_ix, string *inherits);
static void del_program(string path, int prog_ix);
static void add_preloaded_objects();

static
void create() {
    object root_node;
    int i;

    wild_clones = 3;	/* /kernel/obj/rsrc for Ecru, System and admin */

    ix_to_node = ({ });
    grow_ix_to_node();

    ::compile_object(DBNODE);

    /* create the root of the program filesystem hierarchy */
    root_node = clone_object(DBNODE);
    wild_clones ++;

    root_node->set_directory("/");
    dir_to_node = ([ "/" : root_node ]);

    unfreed = ([ ]);

    /* register ourselves as the object manager of the system */
    DRIVER->set_object_manager(this_object());

    /* finally enter /kernel programs explicitly into the database */
    add_preloaded_objects();

    handles = ([ ]);
}

/* after a driver reboot, status()[ST_OTABSIZE] may have changed */
void reboot() {
   if (object_name(previous_object()) == SYS_INITD) {
      grow_ix_to_node();
   }
}

/* queries */

int *
query_unfreed()
{
    if (FROM_WIZTOOL()) {
	return map_indices(unfreed);
    }
}

mapping query_dir_to_node() {
    return ([ ]) + dir_to_node;
}

/*
 * Translate an 'issue' back to a fully expanded pathname.
 */
string
query_path(int ix)
{
    if (FROM_WIZTOOL()) {
	if (Node(ix)) {
	    return Node(ix)->query_path(ix);
	}
    }
}

/*
 * Return the directly inherited programs.
 */
int *
query_inherits(int ix)
{
    if (FROM_WIZTOOL()) {
	if (Node(ix)) {
	    return Node(ix)->query_inherits(ix);
	}
    }
}

/*
 * Return the objects directly inheriting this program.
 */
int *
query_inheritors(int ix)
{
    if (FROM_WIZTOOL()) {
	if (Node(ix)) {
	    return Node(ix)->query_inheritors(ix);
	}
    }
}

string *
get_directory(varargs string root)
{
    object node;

    if (!root || !strlen(root) || root == "/") {
	node = dir_to_node["/"];
    } else {
	node = dir_to_node[root];
    }
    if (node) {
	return node->query_leaves();
    }
    return ({ });
}

/*
 * Find the / separating the directory from the filename.
 */
private int
split_filename(string file)
{
    /* Assume file is an absolute filename. */
    int i;

    for (i = strlen(file) - 1; i >= 0; i--) {
	if (file[i] == '/') {
	    return i;
	}
    }
    /* Shouldn't get here. */
    return -1;
}

/*
 * Return an array with the issues currently registered for an object.
 */
int *
query_issues(string file)
{
    if (FROM_WIZTOOL()) {
	int    i;
	string path;
	object node;

	i = split_filename(file);
	path = file[..i - 1];
	if (!strlen(path)) {
	    node = dir_to_node["/"];
	} else {
	    node = dir_to_node[path];
	}
	if (!node) {
	    return ({ });
	}
	return node->query_leaf_issues(file[i + 1..]);
    }
}

/*
**	Next follows a number of hooks called from /kernel/sys/driver,
**	to inform us whenever important program/object-related events
**	take place. We use this information to build the database.
**
**	Note that there are two different kinds of programs here, ones
**	which the kernel library allows object-pointer access to, and
**	ones that are pure inheritables, which can only be referenced
**	by name. See /doc/kernel/overview for details.
*/


/* a normal program has been compiled: add it */
void compile(string owner, object obj, mixed source,
	     string inherits...) {
    if (previous_program() == DRIVER) {
        if (sizeof(source) != 0) {
	    source = source[0];
	} else {
	    source = nil;
	}
	if (!sizeof(inherits) && object_name(obj) != DRIVER) {
	    inherits = ({ AUTO });
	}
	add_program(object_name(obj), status(obj)[O_INDEX], inherits);
    }
}

/* a lib program has been compiled (or recompiled) */
void compile_lib(string owner, string path, mixed source,
		 string inherits...) {
    if (previous_program() == DRIVER) {
	if (sizeof(source) != 0) {
	    source = source[0];
	} else {
	    source = nil;
	}
	if (!sizeof(inherits) && path != AUTO) {
	    inherits = ({ AUTO });
	}
	add_program(path, status(path)[O_INDEX], inherits);
    }
}


/* all references to a program issue are gone, and it's been freed */
void remove_program(string owner, string path, int stamp, int index) {
    if (previous_program() == DRIVER) {
	if (sscanf(path, "%*s/lib/")) {
	    del_program(path, index);
	}
    }
}

/* informative for now: non-lib object about to be destructed */
void destruct(string owner, object obj) {
    if (previous_program() == DRIVER) {
	if (sscanf(object_name(obj), "%*s#")) {
	    if (function_object("query_clone_count", obj) != CLONABLE) {
		wild_clones --;
	    }
	} else {
	    del_program(object_name(obj), status(obj)[O_INDEX]);
	}
    }
}

/* informative for now: lib program about to be destructed */
void destruct_lib(string owner, string path) {
    if (previous_program() == DRIVER) {
	unfreed[status(path)[O_INDEX]] = TRUE;
    }
}

/* informative for now: clone created */
void clone(string owner, object obj) {
    if (previous_program() == DRIVER) {
	if (function_object("query_clone_count", obj) != CLONABLE) {
	    wild_clones ++;
	}
    }
}

string path_special(string compiled) {
    if (sscanf(compiled, "%*s/obj/") == 1 &&
	sscanf(compiled, "%*s/lib/") == 0) {
        return "/usr/System/include/clonable.h";
    }
    return "/usr/System/include/std.h";
}

void include(string a, string b) {
    /* Not using this feature. */
}

mixed include_file(string compiled, string from, string path)
{
    if (path == "/include/AUTO" && from == "/include/std.h" &&
	DRIVER->creator(compiled) != "System") {
	/*
	 * special object-dependent include file
	 */
	return path_special(compiled);
    }

    include(from, path);
    return path;
}

int touch(object obj, string function) {
   rlimits (0; -1) {
      rlimits (0; 400000000) {
	 obj->touch();
      }
   }
   return FALSE;
}


/* TODO protect */
string db_status() {
   object node, newnode;
   string *dirs, res;
   int tot_issues, tot_clones, dir_clones, clone_count, i, j, k;
   int *ix, *inherits;

   res = "ProgDB status:\n";

   dirs = map_indices(dir_to_node);

   for (i = 0; i < sizeof(dirs); i ++) {
      node = dir_to_node[dirs[i]];
      ix = node->query_ix();
      if (!sizeof(ix)) {
	 continue;
      }
      if (sscanf(dirs[i] + "/", "%*s/lib/")) {
	 res += " " + dirs[i] + ": " + sizeof(ix) + " pure issues.\n";
      } else if (sscanf(dirs[i] + "/", "%*s/obj/")) {
	 dir_clones = 0;
	 for (j = 0; j < sizeof(ix); j ++) {
	    object master, head;
	    master = find_object(node->query_path(ix[j]));
	    clone_count = 0;
	    if (master) {
	       if (function_object("query_clone_count", master) == CLONABLE) {
		  head = master->query_next_clone();
		  while (head != master) {
		     clone_count ++;
		     head = head->query_next_clone();
		  }
		  if (master->query_clone_count() != clone_count) {
		     res += "ERROR: " + object_name(master) + " clonecount " + master->query_clone_count() + " but " + clone_count + " clones.\n";
		  }
	       }
	    } else {
	       res += "ALERT: " + node->query_path(ix[j]) + " destructed but ProgDB.\n";
	    }
	    tot_clones += clone_count;
	    dir_clones += clone_count;
	 }
	 res += " " + dirs[i] + ": " + sizeof(ix) + " clonables with " + dir_clones + " known clones.\n";
      } else {
	 res += " " + dirs[i] + ": " + sizeof(ix) + " program issues.\n";
      }
      for (j = 0; j < sizeof(ix); j ++) {
	 inherits = node->query_inherits(ix[j]);
	 for (k = 0; k < sizeof(inherits); k ++) {
	    newnode = Node(inherits[k]);
	    if (!newnode->confirm_dependency(inherits[k], ix[j])) {
	       res += "ALERT: " + ix[j] + " inherits " + inherits[k] + ": backlink fails!\n";
	    }
	 }
      }
      tot_issues += sizeof(ix);
   }
   res +=
      "Issue count: " + tot_issues + "\n" +
      "Clone count: " + tot_clones + "\n" +
      "Wild clones: " + wild_clones + "\n" +
      "Total object count: " + (tot_clones+tot_issues+wild_clones) + "\n" +
      "Driver object count: " + status()[ST_NOBJECTS] + "\n";
   return res;
}


/* internals */

static
void add_program(string path, int prog_ix, string *inherits) {
   mapping iset;
   object node;
   string leaf, sub_path, *path_parts;
   int i, *inherit_ix;

   if (Node(prog_ix)) {
      /* this is a recompile, so delete first */
      del_program(path, prog_ix);
   }
   /* make sure there is a node for every sub-path leading up to path */
   path_parts = explode(path, "/");
   sub_path = "";
   node = dir_to_node["/"];
   for (i = 0; i < sizeof(path_parts)-1; i ++) {
      object new_node;
      sub_path += "/" + path_parts[i];
      new_node = dir_to_node[sub_path];
      if (!new_node) {
	 new_node = clone_object(DBNODE);
	 new_node->set_directory(sub_path);
	 node->add_sub_directory(path_parts[i], new_node);
	 dir_to_node[sub_path] = new_node;
      }
      node = new_node;
   }
   Node(prog_ix) = node;
   leaf = path_parts[i];

   /* use a mapping to get rid of redundant inheritance dependencies */
   iset = ([ ]);
   for (i = 0; i < sizeof(inherits); i ++) {
      iset[status(inherits[i])[O_INDEX]] = TRUE;
   }
   inherit_ix = map_indices(iset);

   node->add_program(prog_ix, leaf, inherit_ix);

   for (i = 0; i < sizeof(inherit_ix); i ++) {
      object new_node;
      new_node = Node(inherit_ix[i]);
      if (!new_node) {
	 error("add_program: inherited program (ix=" + inherit_ix[i] + ") not registered");
      }
      new_node->add_dependency(inherit_ix[i], prog_ix);
   }
}

static
void del_program(string path, int prog_ix) {
    object node, inherit_node;
    int i, *inherit_ix;

    node = Node(prog_ix);
    if (!node) {
	error("program not registered");
    }
    unfreed[prog_ix] = nil;
    inherit_ix = node->query_inherits(prog_ix);
    for (i = 0; i < sizeof(inherit_ix); i ++) {
	inherit_node = Node(inherit_ix[i]);
	if (inherit_node) {
	    inherit_node->del_dependency(inherit_ix[i], prog_ix);
	} /* else fine; we were their last dependency */
    }
    node->del_program(prog_ix);

    Node(prog_ix) = nil;
}

static
void grow_ix_to_node() {
   int i, j;

    /*
    **	ix_to_node is an array of mappings, each mapping having to deal
    **	with at most CHUNK indices -- i.e. this number must be less than
    **	the maximum array size. Thus, if the system can hold N objects,
    **	we need approximately N/CHUNK such mappings:
    */

   i = sizeof(ix_to_node);
   j = 1 + ((status()[ST_OTABSIZE]-1)/CHUNK);
   if (j > i) {
      ix_to_node += allocate(j-i);
      while (i < j) {
	 ix_to_node[i++] = ([ ]);
      }
   }
}

private mixed *
get_initial_kernel_programs() {
    return ({
	DRIVER,		({ }),
	AUTO,		({ }),

	OBJREGD,	({ AUTO }),
	RSRCD,		({ AUTO }),
	ACCESSD,	({ AUTO }),
	USERD,		({ AUTO }),

	API_ACCESS,	({ AUTO }),
	API_RSRC,	({ AUTO }),
	API_USER,	({ AUTO }),
	API_TLS,	({ AUTO }),

	LIB_CONN,	({ AUTO }),
	LIB_USER,	({ AUTO }),
	LIB_WIZTOOL,	({ API_ACCESS, API_RSRC, API_USER }),

# if 0
# ifdef SYS_NETWORKING
	LIB_PORT,	({ AUTO }),
	PORT_OBJECT,	({ LIB_PORT }),
# endif
#endif
	RSRCOBJ,	({ AUTO }),
	BINARY_CONN,	({ LIB_CONN }),
	TELNET_CONN,	({ LIB_CONN }),
	DEFAULT_USER,	({ LIB_USER, API_USER, API_RSRC }),
	DEFAULT_WIZTOOL,	({ LIB_WIZTOOL }),
	    });
}

static
void add_preloaded_objects() {
   mapping premap;
   mixed *preloads;
   int i;

   preloads = get_initial_kernel_programs();

   preloads += ({
      SYS_INITD, ({ AUTO }),
      DBNODE, ({ AUTO }),
      PROGDB, ({ AUTO }),
      METRICSD, ({ AUTO }),
      SYSLOGD, ({ AUTO }),
   });

   premap = ([ ]);

   for (i = 0; i < sizeof(preloads); i += 2) {
      if (status(preloads[i]) == nil) {
	 compile_object(preloads[i]);
      }
      add_program(preloads[i], status(preloads[i])[O_INDEX], preloads[i+1]);
      premap[preloads[i]] = nil;
   }

   preloads = map_indices(premap);

   for (i = 0; i < sizeof(preloads); i ++) {
      add_program(preloads[i], status(preloads[i])[O_INDEX], ({ }));
   }
}


/* New September 2001; compile_object() support */

atomic
void write_source(string name, string *source) {
   if (previous_program() == SYS_AUTO) {
      if (handles[name]) {
	 remove_call_out(handles[name]);
      }
      handles[name] = call_out("do_write_source", 0, name, source);
   }
}

static
void do_write_source(string name, string *source) {
   rlimits(0; -1) {
      int i, sz;

      handles[name] = nil;
      for (i = 0, sz = sizeof(source); i < sz; i++) {
	 write_file(name + ".tmp", source[i]);
      }
      remove_file(name + ".c");
      rename_file(name + ".tmp", name + ".c");
   }
}

void
register_destructing_hook(object ob)
{
   if (!destructing_hooks) {
      destructing_hooks = ([ ]);
   }
   destructing_hooks[ob] = TRUE;
}

void
unregister_destructing_hook(object ob)
{
   if (destructing_hooks) {
      destructing_hooks[ob] = nil;
   }
}

static void
relay_destructing_hook(object hooked, object ob)
{
   hooked->hook_destructing(ob);
}

void
destructing()
{
   catch {
      int i, sz;
      object *list, ob;

      if (destructing_hooks && (sz = map_sizeof(destructing_hooks))) {
	 int i;
	 object ob, *list;

	 ob = previous_object();
	 list = map_indices(destructing_hooks);
	 for (i = 0; i < sz; i++) {
	    call_limited("relay_destructing_hook", list[i], ob);
	 }
      }
   }
}
