/*
**	/lib/array.c
**
**	Useful functions for manipulating arrays.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

static
int member(mixed ix, mixed *arr) {
   return sizeof(arr & ({ ix })) > 0;
}

static
int index(mixed ix, mixed *arr) {
   int i;

   for (i = 0; i < sizeof(arr); i ++) {
      if (typeof(ix) == typeof(arr[i]) && ix == arr[i]) {
	 return i;
      }
   }
   return -1;
}

static
mixed *flatten(mixed **arrarr) {
   mixed *arr, *res;
   int i, j, k, len;

   for (i = 0; i < sizeof(arrarr); i ++) {
      if (typeof(arrarr[i]) != T_ARRAY) {
	 error("element is not an array");
      }
      len += sizeof(arrarr[i]);
   }
   catch {
      res = allocate(len);
   } : {
      error("flattened array would be too long");
   }
   k = -1;
   for (i = 0; i < sizeof(arrarr); i ++) {
      arr = arrarr[i];
      for (j = 0; j < sizeof(arr); j ++) {
	 res[++k] = arr[j];
      }
   }
   return res;
}

static
mixed *reverse(mixed *orig) {
   mixed *new_array;
   int i, j, orig_size;

   j = orig_size = sizeof(orig);
   new_array = allocate(orig_size);

   for(i = 0, j --; i < orig_size; i++, j--)
      new_array[j] = orig[i];

   return new_array;
}


static
mixed *filter(mixed *arr, string fun, varargs object ob, mixed extra...) {
   mixed *out;
   int i, j;

   if (!ob) {
      ob = this_object();
   }

   out = allocate(sizeof(arr));
   for (i = 0; i < sizeof(arr); i ++) {
      if (call_other(ob, fun, arr[i], extra...)) {
	 out[j ++] = arr[i];
      }
   }
   return out[.. j-1];
}

static
mixed add(mixed *arr, mixed zero) {
   int i;

   for (i = 0; i < sizeof(arr); i ++) {
      zero += arr[i];
   }
   return zero;
}
   

static
mixed *map(mixed *arr, string fun, varargs object ob, mixed extra...) {
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

static
object *rfilter(object *arr, string fun, mixed extra...) {
   mixed *out;
   int i, j;

   out = allocate(sizeof(arr));
   for (i = 0; i < sizeof(arr); i ++) {
      if (call_other(arr[i], fun, extra...)) {
	 out[j ++] = arr[i];
      }
   }
   return out[.. j-1];
}

static
mixed *rmap(object *arr, string fun, mixed extra...) {
   mixed *out;
   int i;

   out = allocate(sizeof(arr));
   for (i = 0; i < sizeof(arr); i ++) {
      out[i] = call_other(arr[i], fun, extra...);
   }
   return out;
}


static
mixed *last_elements(mixed arr, int num) {
   if (sizeof(arr) <= num) {
      return arr;
   }
   return arr[sizeof(arr) - num ..];
}

static
mixed *first_elements(mixed arr, int num) {
   if (sizeof(arr) <= num) {
      return arr;
   }
   return arr[.. num-1];
}

static
mapping arr_to_set(mixed *arr) {
   mapping tmp;
   int i;

   tmp = ([ ]);
   for (i = 0; i < sizeof(arr); i ++) {
      tmp[arr[i]] = TRUE;
   }
   return tmp;
}

mixed copy(mixed val, varargs mapping seen) {
   mapping map;
   mixed *arr;
   int i;

   if (typeof(val) == T_ARRAY) {
      if (!seen) {
	 seen = ([ ]);
      }
      arr = allocate(sizeof(val));
      seen[val] = arr;
      for (i = 0; i < sizeof(val); i ++) {
	 if (seen[val[i]]) {
	    arr[i] = seen[val[i]];
	    continue;
	 }
	 arr[i] = copy(val[i], seen);
      }
      return arr;
   }
   if (typeof(val) == T_MAPPING) {
      if (!seen) {
	 seen = ([ ]);
      }
      map = ([ ]);
      seen[val] = map;
      arr = map_indices(val);
      for (i = 0; i < sizeof(arr); i ++) {
	 if (seen[val[arr[i]]]) {
	    map[arr[i]] = seen[val[arr[i]]];
	    continue;
	 }
	 map[copy(arr[i])] = copy(val[arr[i]], seen);
      }
      return map;
   }
   return val;
}
