/*
**	/lib/file.c
**
**	Useful functions for manipulating files and path names.
**
**	Copyright Skotos Tech Inc 1999
*/

static
string path_suffix(string path) {
   int i;

   for (i = strlen(path)-1; i >= 0; i --) {
      if (path[i] == '.') {
	 return path[i+1 ..];
      }
   }
   return nil;
}


string path_base(string path) {
   int i;

   for (i = strlen(path)-1; i >= 0; i --) {
      if (path[i] == '.') {
	 return path[.. i-1];
      }
   }
   return nil;
}

static
int file_size(string str) {
    mixed **info;
  
    info = get_dir(str);
    if (sizeof(info[0]) == 1) {
	return info[1][0];
    }
    return -1;
}

static
int file_stamp(string str) {
    mixed **info;
  
    info = get_dir(str);
    if (sizeof(info[0]) == 1) {
	return info[2][0];
    }
    return -1;
}


void write_data_to_file(string file, object data) {
   string *chunks;
   int i;

   chunks = data->query_chunks();
   for (i = 0; i < sizeof(chunks); i ++) {
      write_file(file, chunks[i]);
   }
}

/* create all directories leading up to a given full path */
static
void pave_way(string path) {
   string *bits;
   int i, *sizes;

   bits = explode(path, "/");
   for (i = 0; i < sizeof(bits); i ++) {
      path = "/" + implode(bits[.. i-1], "/");

      sizes = get_dir(path)[1];
      if (sizeof(sizes) == 0) {
	 make_dir(path);
      }
   }
}

void copy_file(string f1, string f2) {
   string buf;
   int sz, ix;

   sz = file_size(f1);
   remove_file(f2 + ".copying");
   catch {
      while (ix + 0x8000 < sz) {
	 write_file(f2 + ".copying", read_file(f1, ix, 0x8000), ix);
	 ix += 0x8000;
      }
      write_file(f2 + ".copying", read_file(f1, ix), ix);
   } : {
      remove_file(f2 + ".copying");
      error("copy failed");
   }
   remove_file(f2);
   rename_file(f2 + ".copying", f2);
   return;
}
