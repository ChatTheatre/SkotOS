/*
 * ~SID/sys/rescue.c
 *
 * Dial 911, create a final XML version before the object disappears.
 *
 * Copyright Skotos Tech Inc 2002
 */

# include <System.h>

private inherit "/lib/file";
private inherit "/lib/url";
private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/XML/lib/xmd";

inherit "/usr/SID/lib/vaultnode";

private string root;

static void
create()
{
   INFO("Initializing SID Cemetery...");

   ::create(root = "/usr/SID/data/dead");
   PROGDB->register_destructing_hook(this_object());
}

void
patch()
{
   root = "/usr/SID/data/dead";
}

private void
store_delayed(object ob)
{
   string program, name;
   mixed state;

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

   call_out("store_delayed_0", 0, name, state);
}

static
void store_delayed_0(string name, mixed state) {
   string file, *bits;
   object result;
   int i;

   result = new_object("/data/data");
   generate_xml(state, result);

   /* create a safe (for unix paths) representation of the name */
   bits = explode(name, ":");
   for (i = 0; i < sizeof(bits); i ++) {
      bits[i] = url_encode(bits[i]);
   }
   name = implode(bits, "/");

   file = root + "/" + name + ".xml";

   call_out("store_delayed_1", 0, file, result->query_chunks());
}

static void
store_delayed_1(string file, string *chunks)
{
   pave_way(file);
   remove_file(file + ".writing");
   catch {
      /*
       * Reimplementing write_data_to_file() alas.
       */
      int i, sz;

      sz = sizeof(chunks);
      for (i = 0; i < sz; i++) {
	 write_file(file + ".writing", chunks[i]);
      }
   } : {
      error("failed to write file");
   }
   remove_file(file);
   rename_file(file + ".writing", file);
   SysLog(ur_name(this_object()) + ": Wrote " + file + " successfully.");
}

void
hook_destructing(object ob)
{
   string name;

   name = name(ob);
   if (sscanf(name, "Duplicate:%*s")) {
      return;
   }
   if (sscanf(name, "[%*s]#%*d") == 2) {
      return;
   }
   if (ob->query_object_name() && ob->query_state_root()) {
      DEBUG("hook_destructing: Trying to rescue " + name + "!");
      catch {
	 store_delayed(ob);
      }
   }
}
