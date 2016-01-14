/*
**	~XML/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

inherit sequencer "/lib/sequencer";
inherit loader "/lib/loader";

static
void create() {
   load_programs("/usr/XML/data");

   sequencer::create("/usr/XML/sys/xml");
}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("boot", FALSE);
   }
}
