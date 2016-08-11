/*
**	/data/bigmap_iterator.c
**
**	A concrete subclass of bigmap_iterator, this LWO is used to
**	iterate over a generalized bigmap.
**
**	040130	Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/lib/bigmap_iterator";

static atomic
void configure(mapping map) {
   init_iterator(map);
}

static atomic
void create(int clone) {
   ::create();
}
