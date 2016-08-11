/*
**	/data/urchild_iterator.c
**
**	A concrete subclass of bigmap_iterator, this LWO is used to
**	iterate over all the children of an ur-parent.
**
**	040130	Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999
*/


inherit "/lib/bigmap_iterator";

/* the urchild bigmap's values are indices of the inner mappings */
mixed iterator_value(mixed *ixarr, mixed *valarr, int ix) {
   return ixarr[ix];
}

static atomic
void configure(mapping map) {
   init_iterator(map);
}

static atomic
void create(int clone) {
   ::create();
}
