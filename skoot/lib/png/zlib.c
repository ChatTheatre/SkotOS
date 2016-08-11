private inherit "./util";

# define MAX_BLOCK 16384

static
string deflated_blocks(string data) {
   string *blocks;
   int i;

   blocks = allocate(1+(strlen(data)-1)/MAX_BLOCK);
   for (i = 0; i < sizeof(blocks)-1; i ++) {
      blocks[i] = 
	  "\000" +
	 le_int16(MAX_BLOCK) + le_int16(~MAX_BLOCK) +
	 data[.. MAX_BLOCK-1];
      data = data[MAX_BLOCK ..];
   }
   blocks[i] = "\001" +
      le_int16(strlen(data)) + le_int16(~strlen(data)) +
      data;

   return implode(blocks, "");
}

static
string deflate_stream(string data) {
   string blocks;
   int s1, s2, i;

   blocks = deflated_blocks(data);

   s1 = 1; s2 = 0;
   for (i = 0; i < strlen(data); i ++) {
      s1 = (s1 + data[i]) % 65521;
      s2 = (s1 + s2) % 65521;
   }
   return
      byte(120) +
      byte(31 - (120*256 % 31)) +
      blocks +
      be_int32(s2 << 16 | s1);
}
