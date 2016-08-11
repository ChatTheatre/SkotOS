/*
**	/lib/bigmap_iterator.c
**
**	A utility class to help concrete subclasses iterate over
**	bigmaps of various types. A default implementation of the
**	maskable function iterator_value() is supplied.
**
**	040130	Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999
*/


inherit "/lib/iterator";

private mapping bigmap;

private mixed *outer_ixarr;
private int outer_ix;

private mixed *inner_ixarr;
private mixed *inner_valarr;
private int inner_ix;

private void forward();

static atomic
void init_iterator(mapping map) {
   bigmap = map;

   outer_ixarr = map_indices(map);
   outer_ix = -1;

   inner_ixarr = inner_valarr = nil;

   forward();
}

static atomic
void create() {
   outer_ixarr = nil;
}

int has_next() {
   return !!outer_ixarr;
}

/* this is meant to be masked for slightly different bigmaps */
mixed iterator_value(mixed *ixarr, mixed *valarr, int ix) {
   return valarr[ix];
}

mixed next() {
   mixed res;

   if (!outer_ixarr) {
      error("read past end");
   }
   res = iterator_value(inner_ixarr, inner_valarr, inner_ix);
   forward();

   return res;
}

private
void forward() {
   while (TRUE) {
      if (inner_ixarr) {
	 inner_ix ++;
	 if (inner_ix < sizeof(inner_ixarr)) {
	    /* character available */
	    return;
	 }
      }
      outer_ix ++;
      if (outer_ix >= sizeof(outer_ixarr)) {
	 /* no more characters; we're done */
	 outer_ixarr = inner_ixarr = nil;
	 outer_ix = inner_ix = -1;
	 return;
      }
      inner_ixarr  = map_indices(bigmap[outer_ixarr[outer_ix]]);
      inner_valarr = map_values(bigmap[outer_ixarr[outer_ix]]);
      inner_ix = -1;
   }
}
