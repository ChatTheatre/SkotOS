#include <config.h>
#include <kernel/kernel.h>
#include <System.h>
#include <status.h>

inherit SYS_AUTO;
inherit USR_DIR + "/System/lib/tool/linked_list";

static void create()
{
	mixed *paths;
	mixed *objs;
	/* list: ({ first, last }) */
	/* node: ({ prev, path, next }) */

	paths = ({ nil, nil });
	objs = ({ nil, nil });

	/* step 1, scan the filesystem */
	/* step 2, destruct all the libs (done immediately on discovery) */
	/* step 3, compile everything else */

	list_push_front(paths, "/");

	remove_file("/dormant.log");
	remove_file("/destruct.log");
	remove_file("/compile.log");

	call_out("gather_directory", 0, paths, objs);
}

static void gather_directory(mixed *paths, mixed *olist)
{
	catch {
		string path;
		mixed **dir;
		string *entries;
		mixed *objs;
		int *sizes;
		int sz;

		path = list_front(paths);
		list_pop_front(paths);

		if (path[strlen(path) - 1] != '/') {
			path += "/";
		}

		dir = get_dir(path + "*");
		entries = dir[0];
		sizes = dir[1];
		objs = dir[3];

		for (sz = sizeof(entries); --sz >= 0; ) {
			string subpath;

			subpath = path + entries[sz];

			if (sizes[sz] == -2) {
				list_push_front(paths, subpath);
			} else {
				string program;

				program = subpath[.. strlen(subpath) - 3];

				if (objs[sz]) {
					if (sscanf(program, "%*s" + INHERITABLE_SUBDIR)) {
						write_file("/destruct.log", program + "\n");
						destruct_program(program);
						if (status(program)) {
							error("Didn't destruct!");
						}
					} else {
						list_push_back(olist, program);
					}
				} else if (!sscanf(program, "%*s" + INHERITABLE_SUBDIR)) {
					if (subpath[strlen(subpath) - 2 ..] == ".c") {
						write_file("/dormant.log", subpath + "\n");
					}
				}
			}
		}

		if (list_empty(paths)) {
			call_out("compile_list", 0, olist);
		} else {
			call_out("gather_directory", 0, paths, olist);
		}
	} : {
		query_originator()->message("Error, self destructing.\n");
		destruct();
	}
}

static void nuke_orphans()
{
	if (file_info("/namedump")) {
		string *lines;
		int sz;

		query_originator()->message("Found namedump, checking for orphans...\n");

		lines = explode(read_file("/namedump"), "\n");

		for (sz = sizeof(lines); --sz >= 0; ) {
			string path;

			path = "/" + lines[sz];

			if (status(path) && !file_info(path + ".c")) {
				query_originator()->message("Destructing orphan " + path + "\n");
				destruct_program(path);
			}
		}
	}

	query_originator()->message("Finished, self destructing\n");

	destruct();
}

static void compile_list(mixed *olist)
{
	catch {
		string path;

		path = list_front(olist);
		list_pop_front(olist);

		if (status(path)) {
			write_file("/compile.log", path + "\n");
			compile_object(path);
		} else {
			query_originator()->message("Skipping " + path + " (not compiled)\n");
		}

		if (list_empty(olist)) {
			query_originator()->message("Done rebuilding\n");
			call_out("nuke_orphans", 0);
		} else {
			call_out("compile_list", 0, olist);
		}
	} : {
		query_originator()->message("Error, self destructing.\n");
		destruct();
	}
}
