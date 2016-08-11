/*
**	Included by ~XML/lib/xmlparse.c
**
**	Copyright Skotos Tech Inc 1999
*/


# include <status.h>

/* output state */
private mixed *res, *attr;
private string nugget, tag, type;
private int out_ix, out_rix;

# define NewOut() {				\
   res = allocate(8);				\
   out_rix = 0;					\
   nugget = "    ";				\
   out_ix = 0;					\
   tag = nil;					\
   type = nil;					\
   attr = nil;					\
}

# define CharOut(c) {				\
   if (out_ix >= strlen(nugget)) {		\
      nugget += "                ";		\
   }						\
   nugget[out_ix ++] = c;			\
}

# define TxtOut(s) {					\
   int i;						\
							\
   while (out_ix + strlen(s) > strlen(nugget)) {	\
      nugget += "                ";			\
   }							\
   for (i = 0; i < strlen(s); i ++) {			\
      nugget[out_ix ++] = (s)[i];			\
   }							\
}

# define _Nug(n) {				\
   if (out_rix >= sizeof(res)) {		\
      res += allocate(1+sizeof(res)/2);		\
   }						\
   res[out_rix ++] = n;				\
}

# define Flush() {				\
   if (out_ix > 0) {				\
      _Nug(nugget[.. out_ix-1]);		\
      nugget = "    ";				\
      out_ix = 0;				\
   }						\
}

# define Nugget(n)  { Flush(); _Nug(n) }

# define Result() (res[.. out_rix-1])

/* output stack */
private mixed *stack;

# define Empty() (!stack)

# define NewStack() { stack = nil; }

# define Push() {							\
   stack = ({ stack, res, out_rix, nugget, out_ix, tag, type, attr });	\
}

# define TopTag() (stack[5])

# define Pop() {				\
   if (!stack) {				\
      LexErr("popping empty stack");		\
   }						\
   res = stack[1];				\
   out_rix = stack[2];				\
   nugget = stack[3];				\
   out_ix = stack[4];				\
   tag = stack[5];				\
   type = stack[6];				\
   attr = stack[7];				\
						\
   stack = stack[0];				\
}

