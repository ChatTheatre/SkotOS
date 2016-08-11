/*
**	~System/sys/idd.c
**
**	We keep track of the second-level naming system here.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <System.h>
# include <System/log.h>

mapping tree;		/* directory structure */

static
void create() {
   tree = ([ ]);

   add_event("set");
   add_event("clear");
}

int allow_subscribe(object ob, string event) {
   return TRUE;
}



atomic
void set_name(object ob, string name) {
   if (previous_program() == SYS_AUTO) {
      mapping lvl;
      string *paths;
      int i;

      if (!name || !strlen(name)) {
	 error("bad object name");
      }
      if (name[0] == '/') {
	 error("object name cannot start with '/'");
      }

      lvl = tree;

      paths = explode(name, ":");
      for (i = 0; i < sizeof(paths)-1; i ++) {
	 if (!strlen(paths[i])) {
	    error("names including :: are forbidden");
	 }
	 switch(typeof(lvl[paths[i]])) {
	 case T_NIL:
	    lvl = lvl[paths[i]] = ([ ]);
	    break;
	 case T_MAPPING:
	    lvl = lvl[paths[i]];
	    break;
	 case T_OBJECT:
	    error("IDD: can't create subdir " + implode(paths[..i], ":") + ": taken");
	 }
      }
      switch (typeof(lvl[paths[i]])) {
      case T_NIL:
	 lvl[paths[i]] = ob;
	 event("set", name);
	 break;
      case T_OBJECT:
	 if (lvl[paths[i]] != ob) {
	   error("IDD: object name " + name + " is taken");
	 }
	 break;
      case T_MAPPING:
	error("IDD: object name " + name + " already refers to a folder");
      default:
	error("IDD: internal error for object name " + name);
      }
   }
}

atomic
void clear_name(string name) {
   if (previous_program() == SYS_AUTO) {
      mixed *lvl;
      string *paths;
      int i;

      paths = explode(name, ":");
      lvl = allocate(sizeof(paths)+1);

      lvl[0] = tree;

      for (i = 0; i < sizeof(paths); i ++) {
	 if (typeof(lvl[i]) != T_MAPPING) {
	    return;
	 }
	 lvl[i+1] = lvl[i][paths[i]];
      }

      event("clear", name);

      while (i > 0) {
	 if (typeof(lvl[i]) == T_MAPPING && map_sizeof(lvl[i]) > 0) {
	    return;
	 }
	 i --;
	 lvl[i][paths[i]] = nil;
      }
   }
}


object query_object(string name) {
   mixed lvl;
   string *paths;
   int i;

/*   XDebug("Looking up object: " + name); */

   if (!name || !strlen(name)) {
      return nil;
   }
   lvl = tree;
   paths = explode(name, ":");

   for (i = 0; i < sizeof(paths)-1; i ++) {
      lvl = lvl[paths[i]];
      if (typeof(lvl) != T_MAPPING) {
	 return nil;
      }
   }
   if (typeof(lvl = lvl[paths[i]]) == T_OBJECT) {
      return lvl;
   }
}


/* TODO debug function */

mapping query_tree() { return tree; }

string *query_subdirs(varargs string path) {
   mixed lvl;
   string *paths;
   mixed *entries;
   int i;

   lvl = tree;

   paths = path ? explode(path, ":") : ({ });

   for (i = 0; i < sizeof(paths); i ++) {
      lvl = lvl[paths[i]];
      if (typeof(lvl) != T_MAPPING) {
	 return ({ });
      }
   }
   entries = map_indices(lvl);
   for (i = 0; i < sizeof(entries); i ++) {
      if (typeof(lvl[entries[i]]) != T_MAPPING) {
	 entries[i] = nil;
      }
   }
   return entries - ({ nil });
}

string *query_objects(varargs string path) {
   mixed lvl;
   string *paths;
   mixed *entries;
   int i;

   lvl = tree;

   paths = path ? explode(path, ":") : ({ });

   for (i = 0; i < sizeof(paths); i ++) {
      lvl = lvl[paths[i]];
      if (typeof(lvl) != T_MAPPING) {
	 return ({ });
      }
   }
   entries = map_indices(lvl);
   for (i = 0; i < sizeof(entries); i ++) {
      if (typeof(lvl[entries[i]]) != T_OBJECT) {
	 entries[i] = nil;
      }
   }
   return entries - ({ nil });
}

/*
**	export our functionality as properties
*/

int query_method(string method) {
   if (sscanf(method, "idd:%s", method)) {
      return method == "get-objects" || method == "get-folders";
   }
   return FALSE;
}

mixed
call_method(string method, mapping args) {
   if (!sscanf(method, "idd:%s", method)) {
      error("only idd: methods served here");
   }
   if (method == "get-objects") {
      return query_objects(args["folder"]);
   }
   if (method == "get-folders") {
      return query_subdirs(args["folder"]);
   }
   error("only idd:get-objects and idd:get-folders served here");
}
