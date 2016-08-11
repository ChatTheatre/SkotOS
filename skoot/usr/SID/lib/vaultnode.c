/*
**	/usr/SID/lib/vaultnode.c
**
**	Inherited by each directory that wants to be part of the
**	distributed state storage system.
**
**	Copyright Skotos Tech Inc 1999-2005
*/

# include <type.h>
# include <System.h>
# include <SID.h>

private inherit "/lib/mapping";
private inherit "/lib/array";
private inherit "/lib/string";
private inherit "/lib/file";
private inherit "/lib/url";
private inherit "/lib/date";

private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/XML/lib/xmd";

inherit "/usr/XML/lib/xmlgen";
inherit "/usr/XML/lib/xmlparse";

mapping initializers;

private string root;
private int halt;
private int nice;	/* how long to wait between each callout */
private int total_errors; /* how many create/configure errors? */
private int test_only;	/* don't actually create/configure */

private mapping blocking;	/* objects blocking a distribution */
private string *include;	/* from vault.lst: folders to include */
private string *exclude;	/* from vault.lst: folders to exclude */

private string	master;		/* from vault.lst: location of master copy */

/* private */ void refresh_list_file(varargs string prefix);
static  void find_named_objects_recursively(mapping o2n, mapping n2o,
					    string paths...);


static
void create(string str) {
   ::create();

   if (!str || !strlen(str)) {
      error("bad root");
   }
   if (str[strlen(str)-1] == '/') {
      str = str[.. strlen(str)-2];
   }
   root = str;

   VAULT->register_node();
}

void patch_register() {
   VAULT->register_node();
}

static
void claim_node(string node) {
   /* not used */
}

object *query_local_blocking_objects() {
   if (blocking) {
      return map_indices(blocking);
   }
   return ({ });
}

void kill_local_blocking_objects() {
   object *ob;
   int i;

   ob = query_local_blocking_objects();
   for (i = 0; i < sizeof(ob); i ++) {
      ob[i]->suicide();
   }
}

string rename_local_blocking_objects(object relay) {
   int    i, sz, year, month, day, hour, minute, second, stamp;
   string prefix, log;
   object *obs;

   stamp  = time();
   year   = get_year(stamp);
   month  = get_month(stamp);
   day    = get_day(stamp);
   hour   = get_hour(stamp);
   minute = get_minute(stamp);
   second = get_second(stamp);

   log = "";
   prefix =
       "MOVED:" + year +
       (month  > 9 ? month   : "0" + month ) +
       (day    > 9 ? day     : "0" + day   ) + "-" +
       (hour   > 9 ? hour    : "0" + hour  ) +
       (minute > 9 ? minute  : "0" + minute) +
       (second > 9 ? second  : "0" + second) + ":";
   obs = query_local_blocking_objects();
   for (i = 0, sz = sizeof(obs); i < sz; i++) {
       if (obs[i]->query_state_root()) {
	   int do_clear;

	   do_clear = TRUE;
	   if (relay) {
	       catch {
		   mixed ret;

		   ret = relay->apply_relay(
		       "clear", "spawn",
		       ([ "object":     obs[i],
			  "objectname": name(obs[i]),
			  ]));
		   do_clear = !!ret;
	       }
	   }
	   if (do_clear) {
	       if (sscanf(ur_name(obs[i]), "/usr/SkotOS/obj/verb#%*d") == 1 && !obs[i]->query_disabled()) {
		   call_out("do_disable", 1, obs[i], prefix);
		   log += ctime(time()) + ": Moving and disabling " + name(obs[i]) + ".\n";
	       } else {
		   call_out("do_rename", 1, obs[i], prefix);
		   log += ctime(time()) + ": Moving " + name(obs[i]) + ".\n";
	       }
	   } else {
	       log += ctime(time()) + ": Skipping " + name(obs[i]) + " (relay request)\n";
	   }
       } else {
	   log += ctime(time()) + ": Skipping " + name(obs[i]) + " (no WOE state)\n";
       }
   }
   return log;
}

static void
do_rename(object ob, string prefix)
{
    DEBUG("Renaming " + name(ob) + "...");
    ob->set_object_name(prefix + name(ob));
}

static void
do_disable(object ob, string prefix)
{
    DEBUG("Renaming " + name(ob) + " and marking 'disabled'...");
    ob->set_object_name(prefix + name(ob));
    ob->set_disabled(TRUE);
}

string query_master() {
   return master;
}

string check_mastery(string name) {
   int i;

   refresh_list_file();

   for (i = 0; i < sizeof(exclude); i ++) {
      if (sscanf(name, exclude[i] + "%*s")) {
	 /* it's forbidden by an exclude rule */
	 return nil;
      }
   }
   /* passed all the exclusions -- let's see if it's included at all */
   for (i = 0; i < sizeof(include); i ++) {
      if (sscanf(name, include[i] + "%*s")) {
	 /* it's allowed by an include rule */
	 return master;
      }
   }
}


/* static */
void spawn_now(int n, varargs int test, object cb_obj, string cb_func, mixed cb_args...) {
   mixed *tmp;
   string *arr, log;
   int i;

   arr = get_dir(root + "-*.xma")[0];
   for (i = 0; i < sizeof(arr); i ++) {
      arr[i] = root + "/../" + arr[i];
   }
   nice = n;
   total_errors = 0;
   test_only = test;
   if (test_only) {
      SysLog(name(this_object()) + ":: testing blockage against " + sizeof(arr) + " archive files.");
      log = ctime(time()) + ": Testing blockage against " + sizeof(arr) + " archive files.\n";
   } else {
      SysLog(name(this_object()) + ":: spawning from " + sizeof(arr) + " archive files.");
      log = ctime(time()) + ": Spawning from " + sizeof(arr) + " archive files.\n";
   }
   initializers = ([ ]);

   refresh_list_file();

   blocking = ([ ]);

   /* fill 'blocking' with all the existing objects */
   find_named_objects_recursively(blocking, nil, include...);

   call_out("parse_archives", 0, ({ cb_obj, cb_func, cb_args }), log, arr);
}

/*
 * NAME:        spawn_create_one()
 * DESCRIPTION: Create an object if it needs to be.
 */
int spawn_create_one(string name, string path, string source, varargs object relay) {
   string *text;
   object ob;
   mixed state;
   int ix, n;

   state = parse_xml(source, path, TRUE);	/* PEEK flag set */
   state = query_colour_value(xmd_force_to_data(state));
   if (sizeof(state) == 0 || xmd_element(state[0]) != "object") {
      error("root element is not 'object'");
   }
   state = state[0];
   if (xmd_attributes(state)[0] != "program") {
      error("bad or missing 'program' attribute to 'object' element");
   }
   if (!find_object(name)) {
      if (sscanf(xmd_attributes(state)[1], "%*s/obj/")) {
	 string before, after;

	 if (relay) {
	     mixed ret;

	     ret = relay->apply_relay(
		 "create", "spawn",
		 ([ "objectname": name,
		    "objecttype": xmd_attributes(state)[1],
		    "initial":    FALSE ]));
	 }
	 ob = clone_object(xmd_attributes(state)[1]);
	 ob->set_object_name(name);

	 SysLog("Created object: " + name(ob));

	 /* for clones only, see if this is an initializer object */
	 if (sscanf(name, "%s:Initial:%s", before, after)) {
	    name = before + ":" + after;
	    if (!find_object(name)) {
	       if (relay) {
		   mixed ret;

		   ret = relay->apply_relay(
		       "create", "spawn",
		       ([ "objectname": name,
			  "objecttype": xmd_attributes(state)[1],
			  "initial":    TRUE ]));
	       }
	       /* only create if does not already exist */
	       ob = clone_object(xmd_attributes(state)[1]);
	       ob->set_object_name(name);
	       initializers[name] = TRUE;
	       SysLog("Adding initializer: " + dump_value(initializers));
	       return 2;
	    }
	 }
	 return 1;
      }
      ob = find_object(xmd_attributes(state)[1]);
      if (!ob) {
         if (relay) {
	    mixed ret;

	    ret = relay->apply_relay(
		"create", "spawn",
		([ "objectname": name,
		   "objecttype": xmd_attributes(state)[1],
		   "initial":    FALSE ]));
         }
	 ob = find_or_load(xmd_attributes(state)[1]);
	 ob->set_object_name(name);
	 return 1;
      }
      ob->set_object_name(name);
      return 0;
   } else {
      /* Check if the object is an initializer and its target object has been destroyed. */
      if (sscanf(xmd_attributes(state)[1], "%*s/obj/")) {
	 string before, after;

	 /* for clones only, see if this is an initializer object */
	 if (sscanf(name, "%s:Initial:%s", before, after)) {
	    name = before + ":" + after;
	    if (!find_object(name)) {
	       if (relay) {
		  mixed ret;

		  ret = relay->apply_relay(
		      "create", "spawn",
		      ([ "objectname": name,
			 "objecttype": xmd_attributes(state)[1],
			 "initial":    TRUE ]));
	       }
	       /* only create if does not already exist */
	       ob = clone_object(xmd_attributes(state)[1]);
	       ob->set_object_name(name);
	       initializers[name] = TRUE;
	       SysLog("Adding lost initializer: " + dump_value(initializers));
	       return 3;
	    }
	 }
	 return 0;
      }
   }
}

int spawn_configure_one(string name, string path, string source) {
   object ob;
   mixed state;

   if (ob = find_object(name)) {
      string before, after;

      state = parse_xml(source, path, FALSE, TRUE);
      /*
       * Assumption here is that the content of the source hasn't changed 
       * since spawn_create_one() looked at it.
       */
      state = query_colour_value(xmd_force_to_data(state))[0];
      state = query_colour_value(xmd_force_to_data(xmd_content(state)));
      if (sizeof(state) > 1) {
	 error("'object' element has more than one child");
      }
      import_state(ob, state[0]);
      Debug("Configured " + name);

      if (sscanf(name, "%s:Initial:%s", before, after)) {
	 name = before + ":" + after;
	 if (initializers[name]) {
	    if (ob = find_object(name)) {
	       import_state(ob, state[0]);
	       return 2;
	    }
	 }
      }
      return 1;
   }
   return 0;
}



/* write stuff */

static
void find_named_objects_recursively(mapping o2n, mapping n2o,
				    string paths...) {
   string *arr, path;
   object ob;
   int i, j;

   for (j = 0; j < sizeof(paths); j ++) {
      path = paths[j];

      if (member(path, exclude)) {
	 continue;
      }
      arr = IDD->query_objects(path);
      for (i = 0; i < sizeof(arr); i ++) {
	 if (ob = IDD->query_object(path + ":" + arr[i])) {
	     string file;

	     file = path + ":" + arr[i];
	     if (member(file, exclude)) {
		 continue;
	     }
	     if (o2n) {
		 o2n[ob] = file;
	     }
	     if (n2o) {
		 n2o[file] = ob;
	     }
	 }
      }
      arr = IDD->query_subdirs(path);
      for (i = 0; i < sizeof(arr); i ++) {
	 arr[i] = path + ":" + arr[i];
      }
      find_named_objects_recursively(o2n, n2o, arr...);
   }
}

/* callback for subclasses to mask */
void archives_parsed() {}

# define ST_CLEAR	1
# define ST_CREATE	2
# define ST_CONFIGURE	3

void parse_archives(mixed extra, string log, string *archives, varargs int aix, int fix, int state) {
   string str, name, uname, trash, body, mod_pre, mod_post;
   int done, mod_int, skipped;
   float mod_flt;

   if (halt) {
      SysLog(name(this_object()) + ": HALTING.");
      halt = 0;
      if (extra && extra[0] && extra[1]) {
	  call_other(extra[0], extra[1], FALSE, log + ctime(time()) + ": HALTING.\n", extra[2]...);
      }
      return;
   }
   if (!blocking) {
      blocking = ([ ]);
   }
   if (!state) {
      state = ST_CLEAR;
   }

   skipped = FALSE;

   if (sizeof(archives) > 0) {
      int delay;

      str = read_file(archives[aix], fix, 0xc0000); 

      if (!str) {
	  if (extra && extra[0] && extra[1]) {
	      call_other(extra[0], extra[1], FALSE,
			 log + ctime(time()) + ": Cannot find " + archives[aix] + "\n",
			 extra[2]...);
	      return;
	  } else {
	      error("cannot find " + archives[aix]);
	  }
      }
      if (!sscanf(str, "%s>>>%s %s.%s\n%s", trash, uname, mod_pre, mod_post, str)) {
	  if (extra && extra[0] && extra[1]) {
	      call_other(extra[0], extra[1], FALSE,
			 log + ctime(time()) + ": Bad text in " + archives[aix] + ", fix=" + fix + "\n",
			 extra[2]...);
	      return;
	  } else {
	      error("bad text in " + archives[aix] + ", fix=" + fix);
	  }
      }
      name    = url_decode(uname);
      mod_int = (int)mod_pre;
      mod_flt = (float)("0." + mod_post);

      if (strlen(strip(trash))) {
	  if (extra && extra[0] && extra[1]) {
	      call_other(extra[0], extra[1], FALSE,
			 log + ctime(time()) + ": Trash in " + archives[aix] + "[" + fix + "]: " + dump_value(trash) + "\n",
			 extra[2]...);
	      return;
	  } else {
	      error("trash in " + archives[aix] + "[" + fix + "]: " +
		    dump_value(strlen(trash)));
	  }
      }
      if (!sscanf(str, "%s>>>", body)) {
	 body = str;
	 done = TRUE;
      }
      switch(state) {
      case ST_CLEAR:
	 blocking[find_object(name)] = nil;
	 break;
      case ST_CREATE:
	 catch {
	    switch (spawn_create_one(name, archives[aix], body, extra[0])) {
	    default:
	       /* Object was already there. */
	       break;
	    case 1:
	       log += ctime(time()) + ": Created " + name + ".\n";
	       break;
	    case 2:
	       log += ctime(time()) + ": Created " + name + " (initializer).\n";
	       break;
	    case 3:
	       log += ctime(time()) + ": Created " + name + " (lost initializer).\n";
	       break;
	    }
	 } : {
	    string err;

	    err = SYSLOGD->query_last_error();
	    err = err ? " [" + err + "]" : "";
	    SysLog(ur_name(this_object()) + ": Creation of " + name + " failed." + err);
	    log += ctime(time()) + ": Creation of " + name + " failed." + err + "\n";
	    total_errors++;
         }
	 break;
      case ST_CONFIGURE:
	 catch {
	    int do_configure;
	    object ob;
	    mixed revisions, revision;

	    do_configure = FALSE;
	    ob = find_object(name);
	    catch {
	       if (extra && extra[0]) {
		  mixed ret;

		  ret = extra[0]->apply_relay(
		      "configure", "spawn",
		      ([ "object":     ob,
			 "objectname": name,
			 "revision":   mod_int ]));
		  do_configure = !!ret;
	       } else {
		  revision = 0;
		  revisions = ob->query_property("revisions");
		  if (typeof(revisions) == T_ARRAY && sizeof(revisions) % 3 == 0) {
		      revision = revisions[sizeof(revisions) - 3];

		      if (typeof(revision) != T_INT) {
			  revision = 0;
		      }
		  }
		  do_configure = !revision || !mod_int || mod_int != revision;
	       }
	    }
	    if (do_configure) {
	        spawn_configure_one(name, archives[aix], body);
	        log += ctime(time()) + ": (Re)configured " + name + ".\n";
                catch {
                    if (extra && extra[0]) {
                        mixed ret;

                        ret = extra[0]->apply_relay(
                            "import", "spawn",
                            ([ "object": ob,
                               "objectname": name,
                               ]));
                    }
                }
	    } else {
		skipped = TRUE;
	    }
	 } : {
	    string err;

	    err = SYSLOGD->query_last_error();
	    err = err ? " [" + err + "]" : "";
	    SysLog(ur_name(this_object()) + ": Configuration of " + name + " failed." + err);
	    log += ctime(time()) + ": Configuration of " + name + " failed." + err + "\n";
	    total_errors++;
         }
	 break;
      }
      switch (state) {
      case ST_CLEAR:
      case ST_CREATE:
	 delay = 0;
	 break;
      default:
	 delay = skipped ? 0 : nice;
	 break;
      }

      if (!done) {
	 call_out("parse_archives", delay, extra, log, archives, aix,
		  fix + strlen(trash) + 3 + strlen(uname) + 1 + strlen(mod_pre) + 1 + strlen(mod_post) + 1 + strlen(body),
		  state);
	 return;
      }
      aix ++;
      if (aix < sizeof(archives)) {
	 call_out("parse_archives", delay, extra, log, archives, aix, 0, state);
	 return;
      }
      switch(state) {
      case ST_CLEAR:
	 if (map_sizeof(blocking)) {
	     if (extra && extra[0] && extra[1]) {
		 if (test_only) {
		     call_other(extra[0], extra[1], FALSE,
				log + ctime(time()) + ": Blocking objects must be cleared.\n",
				extra[2]...);
		     return;
		 } else {
		     log += ctime(time()) + ": Moving blocking objects out of the way.\n";
		     log += rename_local_blocking_objects(extra[0]);
		 }
	     } else {
		 error("old objects must be cleared");
	     }
	 }
	 if (test_only) {
	    SysLog("Area is clear.");
	    if (extra && extra[0] && extra[1]) {
	       call_other(extra[0], extra[1], TRUE,
			  log + ctime(time()) + ": Area is clear.\n",
			  extra[2]...);
	    }
	    return;
	 }
	 SysLog("Area clear... going for creation...");
	 log += ctime(time()) + ": Area clear... going for creation...\n";
	 state = ST_CREATE;
	 break;
      case ST_CREATE:
	 SysLog("Creation done... going for configuration...");
	 log += ctime(time()) + ": Creation done... going for configuration...\n";
	 state = ST_CONFIGURE;
	 break;
      case ST_CONFIGURE:
	 SysLog("Configuration done!");
	 log += ctime(time()) + ": Configuration done!\n";
	 archives_parsed();
	 if (extra && extra[0] && extra[1]) {
	    call_other(extra[0], extra[1], !total_errors, log, extra[2]...);
	 }
	 return;
      }
      call_out("parse_archives", nice, extra, log, archives, 0, 0, state);
      return;
   } else {
      SysLog("Configuration done!");
      log += ctime(time()) + ": Configuration done!\n";
      archives_parsed();
      if (extra && extra[0] && extra[1]) {
	 call_other(extra[0], extra[1], !total_errors, log, extra[2]...);
      }
   }
}


/* private */
void refresh_list_file(varargs string prefix) {
   mapping imap, emap;
   string text, *lines;
   int i;

   imap = ([ ]);
   emap = ([ ]);

   include = ({ });
   exclude = ({ });
   master = nil;

   if (root) {
      text = read_file(root + ".lst");
   }
   if (!text) {
      return;
   }

   lines = explode(text, "\n");
   for (i = 0; i < sizeof(lines); i ++) {
      if (sscanf(lower_case(lines[i]), "master:%s", master)) {
	 master = strip(master);
	 continue;
      }
      if (lines[i][0] == '-') {
	 emap[strip(lines[i][1 ..])] = TRUE;
	 continue;
      }
      if (lines[i][0] == '+') {
	 lines[i] = lines[i][1 ..];
      }
      lines[i] = strip(lines[i]);
      if (prefix) {
	 if (strlen(lines[i]) < strlen(prefix)) {
	    if (prefix[.. strlen(lines[i])-1] == lines[i]) {
	       Debug("Prefix='" + prefix + "'; line='" + lines[i] + "'; using prefix!");
	       lines[i] = prefix;
	    } else {
	       Debug("Prefix='" + prefix + "'; line='" + lines[i] + "'; skipping!");
	       lines[i] = nil;
	    }
	 } else if (lines[i][.. strlen(prefix)-1] != prefix) {
	    Debug("Prefix='" + prefix + "'; line='" + lines[i] + "'; skipping!");
	    lines[i] = nil;
	 } else {
	    Debug("Prefix='" + prefix + "'; line='" + lines[i] + "'; using line!");
	 }
      }
      if (lines[i]) {
	 imap[lines[i]] = TRUE;
      }
   }
   include = map_indices(imap);
   exclude = map_indices(emap);
}

void compile_archives(string prefix, int n, varargs object cb_obj, string cb_func, mixed cb_args...) {
   string **tmp;
   string log;
   nice = n;

   log = ctime(time()) + ": Refreshing vault.lst data.\n";
   refresh_list_file(prefix);

   if (!sizeof(include)) {
      SysLog(name(this_object()) + ": Nothing to archive.");
      if (cb_obj && cb_func) {
	  call_other(cb_obj, cb_func, FALSE, log + ctime(time()) + ": Nothing to archive.\n", cb_args...);
      }
      return;
   }
   call_out("collect", 0, nil, "compile", ({ cb_obj, cb_func, cb_args }), log, ([ ]));
}

static
string append(object ob, string file) {
   string program, name;
   object result;
   mixed state;
   int i;
   mixed revisions, revision;

   name = ob->query_object_name();
   if (!name) {
      error("object is not named");
   }

   catch {
       revision = 0;
       revisions = ob->query_property("revisions");

       if (!revisions) {
 	   /* Try to kick-start the property, assuming that the object -has- properties. */
	   ob->set_property("revisions", ({ time(), "-", "SYNC" }));
	   revisions = ob->query_property("revisions");
       }

       /* Check if this is an array with a multiple of 3 in size: */
       if (typeof(revisions) == T_ARRAY && sizeof(revisions) % 3 == 0) {
	   revision = revisions[sizeof(revisions) - 3];

	   /* Check if the first of the last 3 entries is an integer: */
	   if (typeof(revision) != T_INT) {
	       revision = 0;
	   }
       }
   }

   state = export_state(ob, nil, nil, nil, TRUE); /* vaultflag TRUE */

   program = ur_name(ob);
   sscanf(program, "%s#", program);

   state = xmd_elts("object",
		    ({ "program", program }),
		    state);

   result = new_object("/data/data");
   generate_xml(state, result);

   write_file(file, ">>>" + url_encode(name) + " " + (revision ? revision + ".0" : "0.0") + "\n");
   write_data_to_file(file, result);
   SysLog(name(this_object()) + ": Wrote " + name(ob) +
	  " to archive " + file + " successfully.");
}

int query_halt() { return halt; }

void halt() {
   halt = TRUE;
}

static
void collect(string *paths, string callback, mixed extra, string log, mapping obset) {
   string *arr;
   object ob;
   int i;

   if (halt) {
      SysLog(name(this_object()) + ": HALTING.");
      halt = 0;
      if (extra && extra[0] && extra[1]) {
	  call_other(extra[0], extra[1], FALSE, log + ctime(time()) + ": HALTING.\n", extra[2]...);
      }
      return;
   }

   if (!obset) {
      obset = ([ ]);
   }

   if (paths == nil) {
      refresh_list_file();
      paths = include;
   }

   /* find more objects */
   if (!sizeof(paths)) {
       /* we're done! */
       call_other(this_object(), callback, extra, log,
		 map_indices(obset), map_values(obset));
       return;
   }
   if (!member(paths[0], exclude)) {
      /* see if there are any subdirs to add to the queue */

      arr = IDD->query_subdirs(paths[0]);
      for (i = 0; i < sizeof(arr); i ++) {
	 arr[i] = paths[0] + ":" + arr[i];
      }
      paths += arr;
      arr = IDD->query_objects(paths[0]);

      for (i = 0; i < sizeof(arr); i ++) {
	 ob = IDD->query_object(paths[0] + ":" + arr[i]);
	 if (ob->query_state_root() &&
	     !member(paths[0] + ":" + arr[i], exclude)) {
	    obset[name(ob)] = paths[0];
	 }
      }
   }
   call_out("collect", nice, paths[1 ..], callback, extra, log, obset);
}

static
void compile(mixed extra, string log, string *store, string *paths, varargs mapping seen) {
   string file;

   if (halt) {
      SysLog(name(this_object()) + ": HALTING.");
      halt = 0;
      if (extra && extra[0] && extra[1]) {
	  call_other(extra[0], extra[1], FALSE, log + ctime(time()) + ": HALTING.\n", extra[2]...);
      }
      return;
   }

   if (!seen) {
      seen = ([ ]);
   }

   if (sizeof(store)) {
      file = root + "-" + implode(explode("/" + paths[0] + "/", "/"), "_") + ".xma";

      if (!seen[file]) {
	 remove_file(file);
	 seen[file] = TRUE;
      }
      catch {
	 append(find_object(store[0]), file);
	 /* log += ctime(time()) + ": Added " + store[0] + ".\n"; */
      } : {
	 log += ctime(time()) + ": Failed to add " + store[0] + "!\n";
      }
      call_out("compile", nice, extra, log, store[1 ..], paths[1 ..], seen);
      return;
   }
   SysLog(name(this_object()) + ": Archives compiled.");
   if (extra && extra[0] && extra[1]) {
       call_other(extra[0], extra[1], TRUE, log + ctime(time()) + ": Archives compiled.\n", extra[2]...);
   }
}
