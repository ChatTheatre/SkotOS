private inherit "/lib/string";

private inherit "./zlib";
private inherit "./util";
private inherit "./crc";

static string make_png_chunk(string type, string data);

# define CHUNK_IHDR	"IHDR"
# define CHUNK_IDAT	"IDAT"
# define CHUNK_IEND	"IEND"

static
string construct_png(int **lines, varargs int colour, int scale) {
   string ihdr, idat, iend;
   string *data;
   int width, height, i, j;

   if (!scale) {
      scale = 1;
   }

   height = scale * sizeof(lines);
   width = scale * sizeof(lines[0]);

   data = allocate(height);
   for (i = 0; i < height; i ++) {
      data[i] = "\000" + spaces((colour ? 3 : 1) * width);
      for (j = 0; j < width; j ++) {
	 if (colour) {
	    data[i][3*j+1] = (lines[i/scale][j/scale] >> 16) & 0xFF;
	    data[i][3*j+2] = (lines[i/scale][j/scale] >>  8) & 0xFF;
	    data[i][3*j+3] = (lines[i/scale][j/scale] >>  0) & 0xFF;
	 } else {
	    data[i][j+1] = lines[i/scale][j/scale] & 0xFF;
	 }
      }
   }
   ihdr = make_png_chunk(CHUNK_IHDR,
			 be_int32(width) +	/* 4 bytes of width */
			 be_int32(height) +	/* 4 bytes of height */
			 byte(8) +		/* bits per sample */
			 byte(colour ? 2 : 0) +	/* no palette, color, alpha */
			 byte(0) +		/* compress method */
			 byte(0) +		/* filter method */
			 byte(0));		/* interlace method */

   idat = make_png_chunk(CHUNK_IDAT, deflate_stream(implode(data, "")));

   iend = make_png_chunk(CHUNK_IEND, "");

   return
      byte(137) + byte(80) + byte(78) + byte(71) +
      byte(13) + byte(10) + byte(26) + byte(10) +
      ihdr + idat + iend;
}

static
string make_png_chunk(string type, string data) {
   return
      be_int32(strlen(data)) +
      type + data +
      be_int32(crc(type + data));
}

