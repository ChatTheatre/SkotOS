private
string strip_left(string str) {
   int i;

   for (i = 0; i < strlen(str); i ++) {
      switch(str[i]) {
      case ' ': case '\t': case '\n': case '\r':
	 break;
      default:
	 if (i == 0) {
	    return str;
	 }
	 return str[i ..];
      }
   }
   return "";
}

private
string strip_right(string str) {
  int i;

  for (i = strlen(str)-1; i >= 0; i --) {
      switch(str[i]) {
      case ' ': case '\t': case '\n': case '\r':
	 break;
      default:
	 if (i == strlen(str)-1) {
	    return str;
	 }
	 return str[.. i];
      }
  }
  return "";
}

private
string strip(string str) {
  return strip_right(strip_left(str));
}

private
string replace_strings(string str, string swaps...) {
   int i;

   if (sizeof(swaps) % 2) {
      error("uneven swap arguments");
   }
   for (i = 0; i < sizeof(swaps); i += 2) {
      str = implode(explode(swaps[i] + str + swaps[i], swaps[i]), swaps[i+1]);
   }
   return str;
}

