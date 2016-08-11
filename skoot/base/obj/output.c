/*
**      /base/obj/output.c
**
**	Output Object
**
**      Copyright Skotos Tech Inc 1999
*/

# include <base.h>

inherit LIB_OUTPUT;

static
void create(int clone) {
   if (clone) {
      ::create();
      call_out("suicide", 0);
   }
}

void suicide() {
   destruct_object();
}
