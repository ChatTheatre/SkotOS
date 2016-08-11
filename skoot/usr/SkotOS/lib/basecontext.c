/*
**      /usr/SkotOS/lib/basecontext.c
**
**	API to perform various checks on the /base layer.
**
**      Copyright Skotos Tech Inc 2000
*/

# include <type.h>
# include <SkotOS.h>

private inherit "/lib/string";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";

# include "/usr/SAM/include/sam.h"

/* checks an object to see if it is safe to slay */
int safe_to_slay(object what) {
   if (what->query_property("virtualhome:home")) {
      return FALSE;
   }

   /* check for an Ur parent */
   if (!what->query_ur_object()) {
      return FALSE;
   }

   /* check for Ur children */
   if (what->query_first_child()) {
      return FALSE;
   }

   return TRUE;
}

