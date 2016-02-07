/*
**	~DTD/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/
#include <SAM.h>

inherit sequencer "/lib/sequencer";

static
void create() {
   sequencer::create("/usr/DTD/sys/dtd");
}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("boot", FALSE);
   }
}
