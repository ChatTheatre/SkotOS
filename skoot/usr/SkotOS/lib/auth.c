
static
int is_root(string name) {
   return !!DEV_USERD->query_wiztool(name);
}

static
int is_builder(string name) {
   return !!DEV_USERD->query_wiztool(name);
}
