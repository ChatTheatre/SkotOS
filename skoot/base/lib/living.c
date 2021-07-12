/*
**	/base/lib/living.c
**
**	Contain the code relevant for living objects.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

inherit gender		"./living/gender";
inherit properties	"/lib/properties";

int query_volition() {
   mixed tmp;
   tmp = query_property("volition");

   return typeof(tmp) == T_INT ? tmp : (!!tmp);
}

void set_volition(int i) {
   /* volition = i; */
   set_property( "volition", i );
}

