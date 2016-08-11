/*
 *	Basic iterator interface.
 */


nomask int is_iterator() { return TRUE; }

int has_next() {
   error("implement has_next()");
}

mixed next() {
   error("implement next()");
}
