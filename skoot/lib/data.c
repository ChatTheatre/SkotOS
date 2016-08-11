/*
**	/lib/data.c
**
**	This is the basic class for representing very large binary
**	strings, and might later become the basis for a more general
**	data representation class.
**
**	Copyright Skotos Tech Inc 1999
*/

# define STRMAX		16384		/* fairly arbitrary, < 64K */

# include <fastarr.h>
# include <faststr.h>

private int	length;
private int     write_ix;
private int	frozen;

# define Flush() if (FStrLen() > 0) { FArrApp(FStrRes()); FStrNew(); }

atomic
void clear() {
   if (frozen) {
      error("data object is frozen");
   }
   FArrNew();
   FStrNew();
}

/* atomic */
int append_string(string str) {
   if (frozen) {
      error("data object is frozen");
   }
   length += strlen(str);
   if (FStrLen() > 0 && FStrLen() + strlen(str) > STRMAX) {
      FArrApp(FStrRes());
      FStrNew();
   }
   FStrApp(str);
   return length;
}

string *query_chunks() {
   Flush();
   return FArrRes();
}

string query_chunk(int i) {
   Flush();
   return FArrRes()[i];
}

int query_length() {
   return length;
}

int query_frozen() {
   return frozen;
}

void freeze() {
   frozen = TRUE;
}

void unfreeze() {
   frozen = FALSE;
}
