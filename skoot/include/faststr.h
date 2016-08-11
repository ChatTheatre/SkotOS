/*
**	/include/faststr.h
**
**	Provides an LPC solution to the O(N^2) problem with building
**	strings incrementally, by extending the string in large chunks,
**	reducing the reallocation severely.
**
**	Copyright Skotos Tech Inc 1999
*/

private string fstr;
private int strix;

# define FStrNew() {				\
   fstr = "        ";				\
   strix = 0;					\
}

# define FStrApp(instr) {						\
   string _s;								\
									\
   _s = instr;								\
   if (8 * strlen(_s) > strlen(fstr)) {					\
      /* appended bit is significant portion of string, just add */	\
      fstr = fstr[.. strix-1] + _s;					\
      strix = strlen(fstr);						\
   } else {								\
      int i;								\
									\
      while (strix + strlen(_s) >= strlen(fstr)) {			\
	 fstr += "                                        ";		\
      }									\
      for (i = 0; i < strlen(_s); i ++) {				\
	 fstr[strix ++] = _s[i];					\
      }									\
   }									\
}

# define FStrLen()	(strix)
# define FStrRes()	(fstr[.. strix-1])
