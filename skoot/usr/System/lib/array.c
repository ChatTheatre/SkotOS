private
mixed *map(mixed *arr, string fun, object ob, mixed extra...) {
   mixed *out;
   int i;

   if (!ob) {
      ob = this_object();
   }
   out = allocate(sizeof(arr));
   for (i = 0; i < sizeof(arr); i ++) {
      out[i] = call_other(ob, fun, arr[i], extra...);
   }
   return out;
}
