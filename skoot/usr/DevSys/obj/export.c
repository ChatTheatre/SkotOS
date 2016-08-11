/*
**	~DevSys/obj/export.c
**`
**	Hand out XML dumps of select objects states.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/kernel.h>
# include <kernel/user.h>

private inherit "/lib/url";

inherit "/usr/System/lib/output_buffer";

int login(string name) {
   if (previous_program() == LIB_CONN) {
      connection(previous_object());
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
