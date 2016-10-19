/*
**	/usr/SID/sys/vault.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <SAM.h>
# include <mapargs.h>

inherit "/lib/url";

private inherit "/lib/file";
private inherit "/lib/mapping";
private inherit "/lib/array";
private inherit "/lib/string";
private inherit "/lib/mapargs";

private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/XML/lib/xmd";

# define VAULT "/usr/SID/data/vault"

inherit branch "/usr/SID/lib/vaultnode";

object *nodes;

private void do_spawn(string name, string path);
private string name_to_path(string name);
private string path_to_name(string path);

string query_state_root() { return "Vault:TheVault"; }


/*
**	Note that we take care of DTD's SID needs here, because
**	DTD is lower in the dependency hierarchy than we are and
**	they cannot inherit vaultnode.
*/

static
void create() {
   INFO("Initializing State Vault...");

   ::create("/usr/SID/data/vault");

   SAMD->register_root("Vault");
   set_object_name("SID:TheVault");
}

void patch() {
   set_object_name("SID:TheVault");
}

void boot(int boot) {
   if (ur_name(previous_object()) == "/usr/SID/initd") {
      /* do nothing */
   }
}

void register_node() {
   if (previous_program() == "/usr/SID/lib/vaultnode") {
      if (!nodes) {
	 nodes = ({ });
      }
      nodes |= ({ previous_object() });
   }
}

object *query_nodes() {
   return nodes[..];
}

object *query_blocking_objects() {
   object *out;
   int i;

   out = ({ });
   for (i = 0; i < sizeof(nodes); i ++) {
      out |= nodes[i]->query_local_blocking_objects();
   }
   return out;
}

/* call this function to load a single object from the vault */
void spawn_one_by_name(string name) {
   do_spawn(name, name_to_path(name));
}

/* call this function to load a single object from the vault */
void spawn_one_by_path(string path) {
   do_spawn(path_to_name(path), path);
}

void save_hierarchy(string claim) {
   mapping m;

   m = ([ ]);

   find_named_objects_recursively(m, nil, claim);

   call_out("store_more", 1, map_indices(m));
}

string store(object ob) {
   string program, name, file, *bits;
   object result;
   mixed state;
   int i;

   name = ob->query_object_name();
   if (!name) {
      error("object is not named");
   }

   state = export_state(ob, nil, nil, nil, TRUE); /* vaultflag TRUE */

   program = ur_name(ob);
   sscanf(program, "%s#", program);

   state = xmd_elts("object",
		    ({ "program", program }),
		    state);

   result = new_object("/data/data");
   generate_xml(state, result);

   /* create a safe (for unix paths) representation of the name */
   bits = explode(name, ":");
   for (i = 0; i < sizeof(bits); i ++) {
      bits[i] = url_encode(bits[i]);
   }
   name = implode(bits, "/");

   file = "/usr/SID/data/vault/" + name + ".xml";

   pave_way(file);
   catch {
      write_data_to_file(file + ".writing", result);
   } : {
      remove_file(file + ".writing");
      error("failed to write file");
   }
   remove_file(file);
   rename_file(file + ".writing", file);
   SysLog(ur_name(this_object()) + ": Wrote " + file + " successfully.");
   return file;
}

static
void store_more(object *obs) {
   if (sizeof(obs) == 0) {
      return;
   }
   if (obs[0]->query_state_root()) {
      catch {
	 store(obs[0]);
      }
   }
   call_out("store_more", 0.1, obs[1 ..]);
}

/*
 * Store in the vault and also create a snapshot, so that others can use this
 * functionality as well.
 */
void
store_snapshot(object ob)
{
   string file;

   if (file = store(ob)) {
      copy_file(file, file + "." + time());
   }
}


/* internals */

private
string name_to_path(string name) {
   return VAULT + "/" +
      implode(map(explode(name, ":"), "url_encode"), "/") +
      ".xml";
}

private
string path_to_name(string path) {
   if (!path || !sscanf(path, VAULT + "/%s.xml", path)) {
      error("path not in VAULT: " + dump_value(path));
   }
   return implode(map(explode(path, "/"), "url_decode"), ":");
}

private
void do_spawn(string name, string path) {
   string text;

   text = read_file(path);

   if (!text) {
      error("spawn_by_name: no such file: " + dump_value(path));
   }

   SysLog("do_spawn(" + dump_value(name) + ", " + dump_value(path) + ")");

   catch {
      int n;

      if (spawn_create_one(name, path, text)) {
	 SysLog("VAULT: Cloned: " + dump_value(name));
      } else {
	 SysLog("VAULT: Loaded: " + dump_value(name));
      }
   } : {
      SysLog("VAULT: Failed to create: " + dump_value(name));
   }

   catch {
      if (spawn_configure_one(name, path, text)) {
	 SysLog("VAULT: Configured: " + dump_value(name));
      }
   } : {
      SysLog("VAULT: Configuration failed: " + dump_value(name));
   }
}

mixed eval_sam_ref(string service, string ref, object context, mapping args) {
   string master, name;
   int i;

   if (service == "vault") {
      if (lower_case(ref) == "mastery" ||
	  lower_case(ref) == "externalmastery") {
	 name = ObArg("obj", "Vault.Mastery")->query_object_name();
	 if (!name) {
	    return nil;
	 }
	 for (i = 0; i < sizeof(nodes); i ++) {
	    if (!nodes[i]) {
		continue;
	    }
	    master = nodes[i]->check_mastery(name);
	    if (master) {
	       if (lower_case(ref) != "externalmastery" ||
		   master != SYS_INITD->query_instance()) {
		  return master;
	       }
	    }
	 }
	 return nil;
      }
   }
   error("only $(Vault.Mastery) served here");
}

/*
 * Merry calls:
 */

int
query_method(string method)
{
   switch (lower_case(method)) {
   case "xmlsnapshot":
      return TRUE;
   default:
      return FALSE;
   }
}

mixed
call_method(string method, mapping args)
{
   switch (lower_case(method)) {
   case "xmlsnapshot":
      if (typeof(args["obj"]) != T_OBJECT) {
	 error("xmlsnapshot expects an object $obj");
      }
      store_snapshot(args["obj"]);
      break;
   default:
      break;
   }
}
