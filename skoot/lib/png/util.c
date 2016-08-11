static
string be_int32(int i) {
   string str;

   str = "    ";
   str[0] = (i >> 24) & 0xFF;
   str[1] = (i >> 16) & 0xFF;
   str[2] = (i >>  8) & 0xFF;
   str[3] = (i >>  0) & 0xFF;

   return str;
}

static
string be_int16(int i) {
   string str;

   str = "  ";
   str[0] = (i >> 8) & 0xFF;
   str[1] = (i >> 0) & 0xFF;

   return str;
}

static
string le_int32(int i) {
   string str;

   str = "    ";
   str[3] = (i >> 24) & 0xFF;
   str[2] = (i >> 16) & 0xFF;
   str[1] = (i >>  8) & 0xFF;
   str[0] = (i >>  0) & 0xFF;

   return str;
}

static
string le_int16(int i) {
   string str;

   str = "  ";
   str[1] = (i >> 8) & 0xFF;
   str[0] = (i >> 0) & 0xFF;

   return str;
}

static
string byte(int i) {
   string str;

   str = " ";
   str[0] = i & 0xFF;

   return str;
}
