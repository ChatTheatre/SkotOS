#include <config.h>
#include <kernel/kernel.h>
#include <kernel/rsrc.h>
#include <System.h>

inherit SYS_AUTO;

inherit rsrc API_RSRC;
inherit "/usr/System/lib/tool/linked_list";

inherit "/lib/url";

private inherit "/lib/array";
private inherit "/lib/file";
private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/XML/lib/xmlgen";
private inherit "/usr/XML/lib/xmlparse";

mixed *dqueue; /* 1: delete/destruct */
mixed *squeue; /* 2: spawn */
mixed *wqueue; /* 3: work */

#define VAULTDIR "/data/vault/"

static void create()
{
	rsrc::create();

	dqueue = ({ nil, nil });
	squeue = ({ nil, nil });
	wqueue = ({ nil, nil });
}

private object query_originator()
{
	object o;

	o = ::query_originator();

	if (o) {
		return o;
	} else {
		return find_object(DRIVER);
	}
}

string name_to_file(string name)
{
	return implode(
		map(explode(name, ":"), "url_encode"), "/"
	);
}

string file_to_name(string file) {
	return implode(
		map(explode(file, "/"), "url_decode"), ":"
	);
}

private void log_vault_error(string err)
{
	write_file("/vault-error.log", err + "\n");
	query_originator()->message(err + "\n");
}

private void write_object(object obj)
{
	int is_clone;
	mixed state;
	object data;
	string oname;
	string owner;
	string path;
	string program;

	oname = obj->query_object_name();

	if (!oname) {
		error("Cannot write a nameless object");
	}

	if (!obj->query_state_root()) {
		log_vault_error(
			"Can't dump " + oname + ", no state root (lpc: " +
			ur_name(obj) + ")");
		return;
	}

	state = export_state(obj, nil, nil, nil, TRUE);
	owner = obj->query_owner();

	if (!owner) {
		owner = "Ecru";
	}

	program = ur_name(obj);

	if (sscanf(program, "%s#", program)) {
		is_clone = 1;
	}

	if (is_clone) {
		state = xmd_elts(
			"object",
			({ "clone", program, "owner", owner }),
			state
		);
	} else {
		state = xmd_elts(
			"object",
			({ "program", program }),
			state
		);
	}

	path = oname;
	path = name_to_file(path);
	path = VAULTDIR + path + ".xml";

	data = new_object("/data/data");
	generate_xml(state, data);

	pave_way(path);

	remove_file(path + ".writing");
	write_data_to_file(path + ".writing", data);
	remove_file(path);
	rename_file(path + ".writing", path);
}

private void read_object(object obj)
{
	mixed data;
	mixed state;
	string *attr;
	string oname;
	string owner;
	string path;
	string source;

	oname = obj->query_object_name();

	if (!oname) {
		error("Cannot read a nameless object");
	}

	path = oname;
	path = name_to_file(path);
	path = VAULTDIR + path + ".xml";

	source = read_file(path);

	if (!source) {
		error("Read error");
	}

	TLSD->set_tls_value(
		"System",
		"vault-context",
		"reading object " + oname
	);

	state = parse_xml(source, path, FALSE, TRUE);
	attr = xmd_attributes(state);

	switch(attr[0]) {
	case "program":
		/* blueprint */
		break;

	case "clone":
		/* clone */
		if (attr[3] != obj->query_owner()) {
			log_vault_error("Warning: reading object " + oname + " that had a different owner.");
		}
	}

	state = query_colour_value(xmd_force_to_data(state))[0];
	state = query_colour_value(xmd_force_to_data(xmd_content(state)));

	catch {
		import_state(obj, state[0]);
	} : {
		log_vault_error("Error importing object " + oname);
	}
}

private void spawn_object_from_state(string oname, varargs string owner)
{
	string path;
	string program;
	string source;
	mixed state;
	string *attr;
	object obj;

	path = oname;
	path = name_to_file(path);
	path = VAULTDIR + path + ".xml";

	source = read_file(path);

	if (!source) {
		error("Read error");
	}

	state = parse_xml(source, path, TRUE, TRUE);
	attr = xmd_attributes(state);

	if (!find_object(attr[1])) {
		error("Program " + attr[1] + " is not compiled");
	}

	switch(attr[0]) {
	case "program":
		/* blueprint */
		obj = find_object(attr[1]);
		obj->set_object_name(oname);
		break;

	case "clone":
		/* clone */
		obj = find_object(oname);

		if (obj) {
			if (attr[3] != obj->query_owner()) {
				query_originator()->message("Warning: reading object " + oname + " that had a different owner.\n");
			}
		} else {
			add_owner(attr[3]);
			obj = clone_object(attr[1], attr[3]);
			obj->set_object_name(oname);
		}
	}
}

void dump_single_object(string oname)
{
	object obj;

	obj = find_object(oname);

	if (!obj) {
		error("Cannot find " + oname);
	}

	catch {
		write_object(obj);
	} : {
		query_originator()->message("Error dumping " + oname);
	}
}

/* work types */

/*
	({ "save-subdir", subdir })

	saves the subdir to disk

	({ "save-object", oname })

	saves the object to disk

	({ "load-subdir", subdir })

	loads the subdir from disk

	({ "load-object", oname })

	loads the object

	({ "delete-subdir", subdir })

	deletes the subdir from disk

	({ "delete-object", oname })

	deletes the object from disk

	({ "destruct-subdir", subdir })

	destructs the subdir

	({ "destruct-object", subdir })

	destructs the object

	({ "sync-save-subdir", subdir })

	Syncs a subdir to disk
	* creates subdirs and files on disk if they don't exist
	* overwrites them if they do exist
	* deletes them from disk if they're not resident

	({ "sync-load-subdir", subdir })

	Syncs a subdir from disk
	* spawns then loads subdirs and files from disk
	* loads subdirs and files from disk
	* destructs objects that don't exist on disk

	({ "signal", object, function, value })

	Calls back to let the caller know something's done
*/

private void do_save_subdir(string subdir)
{
	string *subdirs;
	string *objects;
	int sz;

	subdirs = IDD->query_subdirs(subdir);
	objects = IDD->query_objects(subdir);

	for (sz = sizeof(objects); --sz >= 0; ) {
		list_push_front(wqueue, ({ "save-object",
			subdir ? subdir + ":" + objects[sz] : objects[sz]
		}) );
	}

	for (sz = sizeof(subdirs); --sz >= 0; ) {
		list_push_front(wqueue, ({ "save-subdir",
			subdir ? subdir + ":" + subdirs[sz] : subdirs[sz]
		}) );
	}
}

private void do_spawn_subdir(string subdir)
{
	string file;
	string *subdirs;
	string *objects;

	mixed **dir;
	string *names;
	int *sizes;
	int sz;

	if (subdir) {
		file = "/" + name_to_file(subdir);
	} else {
		file = "";
	}

	if (subdir && sscanf(subdir, "SID:SID:%*s")) {
		query_originator()->message("Skipping " + subdir + "\n");
		return;
	}

	dir = get_dir(VAULTDIR + file + "/*");
	names = dir[0];
	sizes = dir[1];

	subdirs = ({ });
	objects = ({ });

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			subdirs = ({ url_decode(names[sz]) }) + subdirs;
		} else {
			file = names[sz];
			sscanf(file, "%s.xml", file);
			objects = ({ url_decode(file) }) + objects;
		}
	}

	for (sz = sizeof(subdirs); --sz >= 0; ) {
		list_push_front(squeue, ({ "spawn-subdir",
			subdir ? subdir + ":" + subdirs[sz] : subdirs[sz]
		}) );
	}

	for (sz = sizeof(objects); --sz >= 0; ) {
		list_push_front(squeue, ({ "spawn-object",
			subdir ? subdir + ":" + objects[sz] : objects[sz]
		}) );
	}
}

private void do_load_subdir(string subdir)
{
	string file;
	string *subdirs;
	string *objects;

	mixed **dir;
	string *names;
	int *sizes;
	int sz;

	if (subdir) {
		file = "/" + name_to_file(subdir);
	} else {
		file = "";
	}

	if (subdir && subdir == "SID:SID") {
		query_originator()->message("Skipping subdir " + subdir + "\n");
		return;
	}

	dir = get_dir(VAULTDIR + file + "/*");
	names = dir[0];
	sizes = dir[1];

	subdirs = ({ });
	objects = ({ });

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			subdirs = ({ url_decode(names[sz]) }) + subdirs;
		} else {
			file = names[sz];
			sscanf(file, "%s.xml", file);
			objects = ({ url_decode(file) }) + objects;
		}
	}

	for (sz = sizeof(subdirs); --sz >= 0; ) {
		list_push_front(wqueue, ({ "load-subdir",
			subdir ? subdir + ":" + subdirs[sz] : subdirs[sz]
		}) );
	}

	for (sz = sizeof(objects); --sz >= 0; ) {
		list_push_front(wqueue, ({ "load-object",
			subdir ? subdir + ":" + objects[sz] : objects[sz]
		}) );
	}
}

private void do_delete_subdir(string subdir)
{
	string file;
	string *subdirs;
	string *objects;

	mixed **dir;
	string *names;
	int *sizes;
	int sz;

	if (subdir) {
		file = "/" + name_to_file(subdir);
	} else {
		file = "";
	}

	dir = get_dir(VAULTDIR + file + "/*");
	names = dir[0];
	sizes = dir[1];

	subdirs = ({ });
	objects = ({ });

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			subdirs = ({ url_decode(names[sz]) }) + subdirs;
		} else {
			file = names[sz];
			sscanf(file, "%s.xml", file);
			objects = ({ url_decode(file) }) + objects;
		}
	}

	if (subdir) {
		list_push_front(dqueue, ({ "rmdir-subdir", subdir }) );
	}

	for (sz = sizeof(subdirs); --sz >= 0; ) {
		list_push_front(dqueue, ({ "delete-subdir",
			subdir ? subdir + ":" + subdirs[sz] : subdirs[sz]
		}) );
	}

	for (sz = sizeof(objects); --sz >= 0; ) {
		list_push_front(dqueue, ({ "delete-object",
			subdir ? subdir + ":" + objects[sz] : objects[sz]
		}) );
	}
}

private void do_destruct_subdir(string subdir)
{
	string *subdirs;
	string *objects;
	int sz;

	subdirs = IDD->query_subdirs(subdir);
	objects = IDD->query_objects(subdir);

	for (sz = sizeof(subdirs); --sz >= 0; ) {
		list_push_front(dqueue, ({ "destruct-subdir",
			subdir ? subdir + ":" + subdirs[sz] : subdirs[sz]
		}) );
	}

	for (sz = sizeof(objects); --sz >= 0; ) {
		list_push_front(dqueue, ({ "destruct-object",
			subdir ? subdir + ":" + objects[sz] : objects[sz]
		}) );
	}
}

private void do_sync_save_subdir(string subdir)
{
	string *memory_subdirs;
	string *memory_objects;
	int sz;
	string file;

	mixed **dir;
	string *names;
	int *sizes;
	string *file_subdirs;
	string *file_objects;

	string *unique;
	string *common;

	memory_subdirs = IDD->query_subdirs(subdir);
	memory_objects = IDD->query_objects(subdir);

	dir = get_dir(VAULTDIR + subdir + "/*");
	names = dir[0];
	sizes = dir[1];

	file_subdirs = ({ });
	file_objects = ({ });

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			file_subdirs = ({ url_decode(names[sz]) }) + file_subdirs;
		} else {
			file = names[sz];
			sscanf(file, "%s.xml", file);
			file_objects = ({ url_decode(file) }) + file_objects;
		}
	}

	unique = file_subdirs - memory_subdirs;

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(dqueue, ({ "delete-subdir",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	unique = file_objects - memory_objects;

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(dqueue, ({ "delete-object",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	unique = memory_subdirs - file_subdirs;

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(wqueue, ({ "save-subdir",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	common = memory_subdirs & file_subdirs;

	for (sz = sizeof(common); --sz >= 0; ) {
		list_push_front(wqueue, ({ "sync-save-subdir",
			subdir ? subdir + ":" + common[sz] : common[sz]
		}) );
	}

	for (sz = sizeof(memory_objects); --sz >= 0; ) {
		list_push_front(wqueue, ({ "save-object",
			subdir ? subdir + ":" + memory_objects[sz] : memory_objects[sz]
		}) );
	}
}

private void do_sync_load_subdir(string subdir)
{
	string *memory_subdirs;
	string *memory_objects;
	int sz;
	string file;

	mixed **dir;
	string *names;
	int *sizes;
	string *file_subdirs;
	string *file_objects;

	string *unique;
	string *common;

	memory_subdirs = IDD->query_subdirs(subdir);
	memory_objects = IDD->query_objects(subdir);

	dir = get_dir(VAULTDIR + subdir + "/*");
	names = dir[0];
	sizes = dir[1];

	file_subdirs = ({ });
	file_objects = ({ });

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			file_subdirs = ({ url_decode(names[sz]) }) + file_subdirs;
		} else {
			file = names[sz];
			sscanf(file, "%s.xml", file);
			file_objects = ({ url_decode(file) }) + file_objects;
		}
	}

	unique = memory_subdirs - file_subdirs;

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(dqueue, ({ "destruct-subdir",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	unique = memory_objects - file_objects;

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(dqueue, ({ "destruct-object",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	common = memory_subdirs & file_subdirs;

	for (sz = sizeof(common); --sz >= 0; ) {
		list_push_front(wqueue, ({ "sync-load-subdir",
			subdir ? subdir + ":" + common[sz] : common[sz]
		}) );
	}

	unique = file_subdirs - memory_subdirs;

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(wqueue, ({ "load-subdir",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	for (sz = sizeof(unique); --sz >= 0; ) {
		list_push_front(squeue, ({ "spawn-subdir",
			subdir ? subdir + ":" + unique[sz] : unique[sz]
		}) );
	}

	for (sz = sizeof(file_objects); --sz >= 0; ) {
		list_push_front(squeue, ({ "spawn-object",
			subdir ? subdir + ":" + file_objects[sz] : file_objects[sz]
		}) );

		list_push_front(wqueue, ({ "load-object",
			subdir ? subdir + ":" + file_objects[sz] : file_objects[sz]
		}) );
	}
}

private mixed *pop_queue()
{
	mixed *task;

	if (!list_empty(dqueue)) {
		task = list_front(dqueue);
		list_pop_front(dqueue);
		return task;
	}

	if (!list_empty(squeue)) {
		task = list_front(squeue);
		list_pop_front(squeue);
		return task;
	}

	if (!list_empty(wqueue)) {
		task = list_front(wqueue);
		list_pop_front(wqueue);
		return task;
	}
}

private int busy()
{
	if (!list_empty(dqueue)) {
		return 1;
	}

	if (!list_empty(squeue)) {
		return 1;
	}

	if (!list_empty(wqueue)) {
		return 1;
	}
}

static void work()
{
	mixed *task;

	task = pop_queue();

	switch(task[0]) {
	case "save-subdir":
		do_save_subdir(task[1]);
		break;

	case "save-object":
		catch {
			dump_single_object(task[1]);
		} : {
			log_vault_error("Error saving " + task[1]);
		}
		break;

	case "spawn-subdir":
		do_spawn_subdir(task[1]);
		break;

	case "spawn-object":
		if (!find_object(task[1])) {
			catch {
				spawn_object_from_state(task[1]);
			} : {
				log_vault_error("Error spawning " + task[1]);
			}
		}
		break;

	case "load-subdir":
		do_load_subdir(task[1]);
		break;

	case "load-object":
		if (sscanf(task[1], "SID:SID:%*s")) {
			query_originator()->message("Skipping object " + task[1] + "\n");
			break;
		}

		catch {
			object obj;

			obj = find_object(task[1]);

			if (obj) {
				read_object(find_object(task[1]));
			} else {
				query_originator()->message("Could not load " + task[1] + ", not found\n");
			}
		} : {
			log_vault_error("Error loading " + task[1]);
		}
		break;

	case "delete-subdir":
		do_delete_subdir(task[1]);
		break;

	case "rmdir-subdir":
		catch {
			remove_dir(VAULTDIR + name_to_file(task[1]));
		} : {
			log_vault_error("Error deleting " + task[1]);
		}
		break;

	case "delete-object":
		catch {
			remove_file(VAULTDIR + name_to_file(task[1]) + ".xml");
		} : {
			log_vault_error("Error deleting " + task[1]);
		}
		break;

	case "destruct-subdir":
		do_destruct_subdir(task[1]);
		break;

	case "destruct-object":
		if (sscanf(task[1], "SID:SID:%*s")) {
			query_originator()->message("Not destructing " + task[1] + "\n");
		} else {
			object obj;

			obj = find_object(task[1]);

			if (!sscanf(ur_name(obj), "%*s#")) {
				query_originator()->message("Skipping object " + task[1] + ", it's not a clone.\n");
				/* only clones get nuked */
				break;
			}

			catch {
				if (!sscanf(obj->query_object_name(), "SID:%*s")) {
					/* SID nodes cannot be name-wiped */
					obj->set_object_name(nil);
				}
				obj->destruct();
			} : {
				log_vault_error("Error destructing " + task[1]);
			}
		}
		break;

	case "sync-save-subdir":
		do_sync_save_subdir(task[1]);
		break;

	case "sync-load-subdir":
		do_sync_load_subdir(task[1]);
		break;

	case "signal":
		call_other(task[1], task[2]);
		break;

	default:
		error("Vault task " + task[0] + " not yet implemented");
	}

	if (busy()) {
		call_out("work", 0);
	} else {
		query_originator()->message("Finished\n");
	}
}

void save_object(string name)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(wqueue, ({ "save-object", name }) );
}

void save_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(wqueue, ({ "save-subdir", subdir }) );
}

void delete_object(string name)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(dqueue, ({ "delete-object", name }) );
}

void delete_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(dqueue, ({ "delete-subdir", subdir }) );
}

void spawn_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(squeue, ({ "spawn-subdir", subdir }) );
}

void load_object(string name)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(squeue, ({ "spawn-object", name }) );
	list_push_back(wqueue, ({ "load-object", name }) );
}

void load_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(squeue, ({ "spawn-subdir", subdir }) );
	list_push_back(wqueue, ({ "load-subdir", subdir }) );
}

void load_spawned_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(wqueue, ({ "load-subdir", subdir }) );
}

void destruct_object(string name)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(dqueue, ({ "destruct-object", name }) );
}

void destruct_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(dqueue, ({ "destruct-subdir", subdir }) );
}

void sync_save_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(wqueue, ({ "sync-save-subdir", subdir }) );
}

void sync_load_subdir(string subdir)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(wqueue, ({ "sync-load-subdir", subdir }) );
}

void queue_signal(string func)
{
	if (!busy()) {
		call_out("work", 0);
	}

	list_push_back(wqueue, ({ "signal", previous_object(), func }) );
}
