/*
**	/lib/ur.c
**
**	Inherit this program to ur-enable yourself. This makes you
**	both an ur-parent and an ur-child -- which is why the data
**	structure handling in this program is a little complex.
**
**	Rewritten by Zell 030612. Summary of interim changes:
**	 - two new variables; see below
**	 - test childcount in destructive_desire()
**	 - remove_child() and add_child() in release/adopt_ur_child()
**	 - query_child_count() and query_child_mapping() functions
**
**	Copyright Skotos Tech Inc 1999
*/

# include <adt.h>
# include <status.h>


private object ur;		/* our ur-parent */

/* the new data structure */

private mapping childmap;	/* the two-level mapping of children */
private int childcount;		/* how many children we have */

private void unlink();
private void link();

atomic
void patch_child_child() {
   if (ur && this_object()->query_first_child()) {
      ur->update_child(this_object());
   }
}

atomic
void patch_map() {
   if (!childmap) {
      childmap = ([ ]);
      childcount = 0;
   }
}

atomic
void patch_urchildren() {
   patch_map();
   if (ur && !ur->is_immediate_child(this_object())) {
      ur->add_child(this_object());
   }
}

static
void create() {
   childmap = ([ ]);
}

static void
destructive_desire() {
   if (childcount > 0) {
      error("cannot destruct ur-object with ur-children");
   }
}

static atomic
void destructing() {
   if (ur) {
      unlink();
   }
}

/* TODO: set up the linked list thing */
atomic nomask
void set_ur_object(object ob) {
   object tmp;

   for (tmp = ob; tmp; tmp = tmp->query_ur_object()) {
      if (tmp == this_object()) {
	 error("would cause ur-object cycle");
      }
   }
   if (ur) {
      unlink();
   }
   tmp = ur;

   ur = ob;

   if (ur) {
      link();
      catch {
	 this_object()->ur_object_set(ur, tmp);
      }
   }
}

nomask
object query_ur_object() {
   return ur;
}

private void unlink() {
   ur->release_ur_child(this_object());
}

private void link() {
   ur->adopt_ur_child(this_object());
}

private
void update_child(object child) {
   mapping row;
   int ix;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();
   row = childmap[ix / 1024];
   if (!row) {
      error("internal error");
   }
   if (child->query_first_child()) {
      XDebug("update: child <" + name(child) + "> marked as having children");
      row[child] = -1;
   } else {
      XDebug("update: child <" + name(child) + "> marked as childless");
      row[child] = time();
   }
}


/* three functions to be called in an ur object */
int is_immediate_child(object child) {
   mapping row;
   int ix;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();
   row = childmap[ix / 1024];
   return row && row[child];
}

nomask atomic
void delete_child(object child) {
   mapping row;
   int ix;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();
   row = childmap[ix / 1024];
   if (row && row[child]) {
      childcount --;
      if (ur && childcount == 0) {
	 ur->child_loses_children();
      }
   } else {
      SysLog("NewUR: releasing unknown child: " + dump_value(child));
   }
   if (row) {
      row[child] = nil;
      if (!map_sizeof(row)) {
	 childmap[ix / 1024] = nil;
      }
   }
}

nomask atomic
void add_child(object child) {
   mapping row;
   int ix;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();
   row = childmap[ix / 1024];
   if (!row) {
      row = childmap[ix / 1024] = ([ ]);
   }

   if (!row[child]) {
      if (ur && childcount == 0) {
	 ur->child_acquires_children();
      }
      childcount ++;
   } else {
      SysLog("NewUR: adopting existing child: " + dump_value(child));
   }

   update_child(child);
}

nomask
void release_ur_child(object child) {
   if (previous_program() == "/lib/ur") {
      delete_child(child);
   }
}

nomask
void adopt_ur_child(object child) {
   if (previous_program() == "/lib/ur") {
      add_child(child);
   }
}

/* called in A's ur-parent when A loses its last ur-child */
nomask
void child_loses_children(object child) {
   if (previous_program() == "/lib/ur") {
      update_child(child);
   }
}

/* called in A's ur-parent when A acquires its first ur-child */
nomask
void child_acquires_children(object child) {
   if (previous_program() == "/lib/ur") {
      update_child(child);
   }
}


nomask
int query_child_count() {
   return childcount;
}

nomask
int is_child_with_children(object child) {
   mapping row;
   int ix;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();
   row = childmap[ix / 1024];
   if (!row) {
      error("not a child");
   }
   return row[child] == -1;
}


/* debug */
nomask
mapping query_child_mapping() {
   return childmap;
}

object
query_ur_child_iterator() {
   return new_object(URCHILD_ITERATOR, childmap);
}

object *
query_ur_children()
{
   int     i, sz;
   object  *list;
   mapping *maps;

   if (childcount > status()[ST_ARRAYSIZE]) {
      error("More ur-children than can fit in an array");
   }
   list = ({ });
   sz = map_sizeof(childmap);
   maps = map_values(childmap);
   for (i = 0; i < sz; i++) {
      list += map_indices(maps[i]);
   }
   return list;
}

int is_child_of(object ancestor) {
   if (!ur) {
      return FALSE;
   }
   if (ur == ancestor) {
      return TRUE;
   }
   return ur->is_child_of(ancestor);
}

nomask
object query_next_ur_sibling() {
   if (ur) {
      return ur->query_next_ur_child(this_object());
   }
}

nomask
object query_previous_ur_sibling() {
   if (ur) {
      return ur->query_previous_ur_child(this_object());
   }
}

nomask
object query_first_child()
{
   if (childcount > 0) {
      int i, sz;
      mapping *maps;

      sz   = map_sizeof(childmap);
      maps = map_values(childmap);
      for (i = 0; i < sz; i++) {
	 if (map_sizeof(maps[i]) > 0) {
	    return map_indices(maps[i])[0];
	 }
      }
   }
   return nil;
}

nomask
object query_next_ur_child(object child) {
   mapping row;
   object *list;
   int ix, sz;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();

   if (childcount == 1) {
      return child;
   }
   ix = ix / 1024;
   sz = 1 + map_indices(childmap)[map_sizeof(childmap) - 1];
   list = map_indices(childmap[ix][child ..]);
   while (sizeof(list) < 2) {
      ix = (ix + 1) % sz;
      if (childmap[ix]) {
	 list += map_indices(childmap[ix]);
      }
   }
   return list[1];
}

nomask
object query_previous_ur_child(object child) {
   mapping row;
   object *list;
   int ix, sz;

   if (!sscanf(ur_name(child), "%*s#%d", ix)) {
      error("not a clone");
   }
   patch_map();

   if (childcount == 1) {
      return child;
   }
   ix = ix / 1024;
   sz = 1 + map_indices(childmap)[map_sizeof(childmap) - 1];
   list = map_indices(childmap[ix][.. child]);
   while (sizeof(list) < 2) {
      ix = (ix + sz - 1) % sz;
      if (childmap[ix]) {
	 list = map_indices(childmap[ix]) + list;
      }
   }
   return list[sizeof(list) - 2];
}
