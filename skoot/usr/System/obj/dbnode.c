# define LOGNAME	("DBNODE<" + dir + ">")

# include <System.h>
# include <kernel/kernel.h>
# include <System/log.h>

# define LEAF		0
# define INHERITS	1
# define INHERITORS	2

#define FROM_PROGDB() (previous_program() == PROGDB)

string dir;
mapping ix_to_info;
mapping leaf_to_ix;
mapping leaf_to_node;

static
void create(int clone) {
    if (clone) {
	ix_to_info = ([ ]);
	leaf_to_ix = ([ ]);
	leaf_to_node = ([ ]);
    }
}

void set_directory(string d) {
    if (FROM_PROGDB()) {
	dir = d;
    }
}

void add_sub_directory(string leaf, object node) {
    if (FROM_PROGDB()) {
	if (leaf_to_node[leaf]) {
	    SysLog("Err: sub-directory already registered");
	}
	leaf_to_node[leaf] = node;
    }
}

void add_program(int prog_ix, string leaf, int *inherit_ix) {
    if (FROM_PROGDB()) {
	if (ix_to_info[prog_ix]) {
	  SysLog("Err: program already registered");
	}
	XDebug("adding ix entry for [" + prog_ix + "]");
	ix_to_info[prog_ix] = ({ leaf, inherit_ix, ([ ]) });
	if (leaf_to_ix[leaf]) {
	    leaf_to_ix[leaf] |= ({ prog_ix });
	} else {
	    leaf_to_ix[leaf]  = ({ prog_ix });
	}
    }
}

void del_program(int prog_ix) {
   if (FROM_PROGDB()) {
       mixed *entry;
       string leaf;

      entry = ix_to_info[prog_ix];
      if (!entry) {
	 SysLog("Err: program not registered");
      }
      XDebug("deleting ix entry for [" + prog_ix + "]");
      leaf = entry[LEAF];
      ix_to_info[prog_ix] = nil;
      leaf_to_ix[leaf] -= ({ prog_ix });
      if (!sizeof(leaf_to_ix[leaf])) {
	 leaf_to_ix[leaf] = nil;
      }
   }
}

void add_dependency(int prog_ix, int inheritor_ix) {
    if (FROM_PROGDB()) {
	mixed *entry;

	entry = ix_to_info[prog_ix];
	if (!entry) {
	    SysLog("Err: program not registered");
	}
	if (entry[INHERITORS][inheritor_ix]) {
	    SysLog("Err: dependency of " + inheritor_ix + " on " + prog_ix + " already registered");
	}
	entry[INHERITORS][inheritor_ix] = 1;
	XDebug("Setting dependency of " + inheritor_ix + " on " + prog_ix);
    }
}

void del_dependency(int prog_ix, int inheritor_ix) {
    if (FROM_PROGDB()) {
	mixed *entry;

	entry = ix_to_info[prog_ix];
	if (!entry) {
	    SysLog("Err: program not registered");
	}
	if (!entry[INHERITORS][inheritor_ix]) {
	    SysLog("Err: supposed dependency of " + inheritor_ix + " on " + prog_ix + " unknown");

	}
	entry[INHERITORS][inheritor_ix] = nil;
	XDebug("Clearing dependency of " + inheritor_ix + " on " + prog_ix);
    }
}

string query_directory() {
    if (FROM_PROGDB()) {
	return dir;
    }
}

int *query_ix() {
    if (FROM_PROGDB()) {
	return map_indices(ix_to_info);
    }
}

string query_leaf(int prog_ix) {
    if (FROM_PROGDB() || previous_program() == DBNODE) {
	return ix_to_info[prog_ix][LEAF];
    }
}

string query_path(int prog_ix) {
    if (FROM_PROGDB()) {
	return dir + "/" + query_leaf(prog_ix);
    }
}

int *query_inherits(int prog_ix) {
    if (FROM_PROGDB()) {
	return ix_to_info[prog_ix][INHERITS][..];
    }
}

int *query_inheritors(int prog_ix) {
    if (FROM_PROGDB()) {
	return map_indices(ix_to_info[prog_ix][INHERITORS]);
    }
}

int confirm_dependency(int prog_ix, int inheritor_ix) {
    if (FROM_PROGDB()) {
	return ix_to_info[prog_ix][INHERITORS][inheritor_ix];
    }
}

int *query_leaf_issues(string file) {
    if (FROM_PROGDB()) {
	int *list;

	list = leaf_to_ix[file];
	return list ? list : ({ });
    }
}

string *query_ix_leaves() {
    if (FROM_PROGDB() || previous_program() == DBNODE) {
	return map_indices(leaf_to_ix);
    }
}

string *query_subdir_leaves() {
    if (FROM_PROGDB() || previous_program() == DBNODE) {
	return map_indices(leaf_to_node);
    }
}

string *query_leaves() {
    if (FROM_PROGDB()) {
	return query_ix_leaves() | query_subdir_leaves();
    }
}
