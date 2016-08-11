/*
**	~DevSys/obj/woe.c
**
**	Handle one specific Tree of Woe
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/kernel.h>
# include <kernel/user.h>

private inherit "/lib/url";

inherit "/usr/System/lib/output_buffer";

private string	ipnr, ipname;
private int	handle;

int login(string name) {
   if (previous_program() == LIB_CONN) {
      connection(previous_object());

      subscribe_event(find_object("/usr/System/sys/idd"), "set");
      subscribe_event(find_object("/usr/System/sys/idd"), "clear");

      handle = call_out("refresh_folder", 3, ({ "" }));

      return MODE_LINE;
   }
}

void logout(int dest) {
    if (previous_program() == LIB_CONN) {
	destruct_object();
    }
}

void suicide() {
   if (sscanf(ur_name(this_object()), "%*s#")) {
      call_out("disconnect", 0);
   }
}

void send_set(string name);
void send_dir(string name);
string find_first_object(string path);

int receive_message(string str) {
   string path;
   int major, minor;

   if (sscanf(str, "TreeOfWoe %d.%d", major, minor) == 2) {
      if (major == 0 && minor == 1) {
	 SysLog("WOE:: Version TreeOfWoe 0.1 -- returning...");
	 return MODE_NOCHANGE;
      }
      SysLog("WOE:: Version TreeOfWoe " + major + "." + minor);
      remove_call_out(handle);
      handle = 0;
      return MODE_NOCHANGE;
   }
   if (sscanf(str, "SEND %s", path)) {
      string *arr, prepath;
      int i;

      Debug("WOE:: Received SEND demand for: " + path);

      prepath = path;
      if (strlen(prepath) > 0) {
	 prepath += ":";
      }
      arr = IDD->query_objects(path);
      for (i = 0; i < sizeof(arr); i ++) {
	 Debug("WOE:: Sending OB " + prepath + arr[i]);
	 send_set(prepath + arr[i]);
      }
      arr = IDD->query_subdirs(path);
      for (i = 0; i < sizeof(arr); i ++) {
	 str = find_first_object(prepath + arr[i]);
	 if (str) {
	    Debug("WOE:: For " + prepath + arr[i] + " --" + find_first_object(prepath + arr[i]));
	    send_set(str);
	 } else {
	    Debug("WOE:: For " + prepath + arr[i] + ": IDD ERROR");
	 }
      }
   }
   return MODE_NOCHANGE;
}


string find_first_object(string path) {
   string *arr;
   int i;

   arr = IDD->query_objects(path);
   if (sizeof(arr) > 0) {
      return path + ":" + arr[0];
   }
   Debug("Looking for subdirs of " + path);
   arr = IDD->query_subdirs(path);
   for (i = 0; i < sizeof(arr); i ++) {
      string str;

      if (str = find_first_object(path + ":" + arr[i])) {
	 return str;
      }
   }
   return nil;
}

void send_set(string name) {
   send_message("SET " + make_url("/Dev/View.sam",
				  ([ "obj": name ])) +
		" " + name + "\n");
}

/* atomic */static
void refresh_folder(string *queue) {
   if (!handle) {
      /* just in case */
      return;
   }
   if (sizeof(queue) > 0) {
      string *arr, prepath;
      int i;

      prepath = queue[0];

      if (prepath != "Duplicate") {
	 if (strlen(prepath) > 0) {
	    prepath += ":";
	 }
	 arr = IDD->query_objects(queue[0]);
	 for (i = 0; i < sizeof(arr); i ++) {
	    send_set(prepath + arr[i]);
	 }
	 arr = IDD->query_subdirs(queue[0]);
	 for (i = 0; i < sizeof(arr); i ++) {
	    queue += ({ prepath + arr[i] });
	 }
      }
      call_out("refresh_folder", 0, queue[1 ..]);
   } else {
      SysLog("WOE:: STILL USING THE OLD APPLET...");
      send_message("SYNC\n");
   }
}

void evt_set(object foo, string name) {
   if (sscanf(name, "Duplicate:%*s") < 1) {
       send_set(name);
   }
}

void evt_clear(object foo, string name) {
   if (sscanf(name, "Duplicate:%*s") < 1) {
       send_message("CLEAR " + name + "\n");
   }
}
