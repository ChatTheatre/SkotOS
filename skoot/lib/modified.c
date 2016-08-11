/*
 * /lib/modified.c
 *
 * Keep track of when an object was most recently modified.  The one catch is if
 * a query returns an array, mapping or LWO which can then indirectly be
 * modified without the object realizing it.
 *
 * Copyright Skotos Tech Inc 2004.
 */

private mixed *stamp;
private mixed *sync;

void 
