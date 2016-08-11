/*
**	~System/lib/toolkit.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <config.h>
# include <kernel/rsrc.h>
# include <kernel/objreg.h>
# include <kernel/access.h>

inherit rsrc   API_RSRC;
inherit objreg API_OBJREG;
inherit access API_ACCESS;

static
void create() {
   rsrc::create();
   objreg::create();
   access::create();
}

static
object *query_objects(string user) {
   mapping obs;
   object ob, head;

   obs = ([ ]);
   if (ob = head = first_link(user)) {
      do {
	 obs[ob] = TRUE;
	 ob = next_link(ob);
      } while (ob != head);
   }
   return map_indices(obs);
}
