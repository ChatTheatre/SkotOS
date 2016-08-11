/*
**	~DevSys/obj/filewoe.c
**
**	Handle one specific Tree of Woe
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/access.h>
# include <kernel/kernel.h>
# include <kernel/user.h>

private inherit "/lib/url";
private inherit "/lib/file";

inherit "/usr/System/lib/toolkit";
inherit "/usr/System/lib/output_buffer";

static
void create(int clone) {
  ::create();
}

int login(string name) {
   if (previous_program() == LIB_CONN) {
      connection(previous_object());

      call_out("refresh_folder", 0, ({ "" }));
      return MODE_LINE;
   }
}

void logout(int dest) {
    if (previous_program() == LIB_CONN) {
	destruct_object();
    }
}

int receive_message(string str) {
   return MODE_NOCHANGE;
}

void send_set(string name) {
   send_message("SET " + make_url("/Dev/File.sam",
				  ([ "file": name ])) +
		" " + name + "\n");
}

static
void refresh_folder(string *queue) {
   if (sizeof(queue) > 0) {
      mixed **dir;
      int i;

      if (access(ur_name(this_object()), queue[0] + "/*", READ_ACCESS)) {
	 dir = get_dir(queue[0] + "/*");

	 for (i = 0; i < sizeof(dir[0]); i ++) {
	    if (dir[1][i] == -2) {
	       if (dir[0][i] != "CVS") {
		  queue += ({ queue[0] + "/" + dir[0][i] });
	       }
	    } else if (dir[1][i] >= 0) {
	       switch (path_suffix(dir[0][i])) {
	       case "html":
	       case "sam":
	       case "txt":
		  send_set(queue[0] + "/" + dir[0][i]);
		  break;
	       default:
		  break;
	       }
	    }
	 }
      }
      call_out("refresh_folder", 0, queue[1 ..]);
   } else {
      send_message("SYNC\n");
   }
}
