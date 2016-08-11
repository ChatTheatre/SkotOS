int *crc_table;

static
void build_table() {
   int c, i, k;

   crc_table = allocate(256);
   for (i = 0; i < 256; i ++) {
      c = i;
      for (k = 0; k < 8; k ++) {
	 if (c & 1) {
	    c = 0xedb88320 ^ (c >> 1);
	 } else {
	    c = c >> 1;
	 }
	 crc_table[i] = c;
      }
   }
}

static
int crc(string data) {
   int c, i;

   if (!crc_table) {
      build_table();
   }

   c = 0xffffffff;

   for (i = 0; i < strlen(data); i ++) {
      c = crc_table[(c ^ data[i]) & 0xff] ^ (c >> 8);
   }
   return ~c;
}
