/*
**	/include/fastarr.h
**
**	Provides an LPC solution to the O(N^2) problem with building
**	arrays incrementally, by extending the array in large chunks,
**	reducing the reallocation severely.
**
**	Copyright Skotos Tech Inc 1999
*/

private mixed *farr;
private int arrix;

# define FArrNew() {				\
   farr = allocate(8);				\
   arrix = 0;					\
}

# define FArrLen()	(arrix)

# define FArrApp(e) {				\
   if (arrix >= sizeof(farr)) {			\
      farr += allocate(1+sizeof(farr)/3);	\
   }						\
   farr[arrix ++] = e;				\
}

# define FArrRes() (farr[.. arrix-1])
