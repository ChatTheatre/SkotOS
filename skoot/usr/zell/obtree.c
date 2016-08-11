object *get_obtree(string path) {
   object *out;
   mixed *arr;
   int i;

   out = ({ });

   arr = IDD->query_subdirs(path);
   for (i = 0; i < sizeof(arr); i ++) {
      out += get_obtree(path + ":" + arr[i]);
   }

   arr = IDD->query_objects(path);

   for (i = 0; i < sizeof(arr); i ++) {
      arr[i] = IDD->query_object(path + ":" + arr[i]);
   }
   return out + arr;
}
