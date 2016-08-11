/*
**	Included by ~XML/lib/xmlparse.c
**
**	Copyright Skotos Tech Inc 1999
*/

/* input state */

private string *_streams;
private string _stream, _file;
private int    _in_ix, _line;
private int    char;

# define LexErr(s) error("XML: " + (_file ? "file " + _file : "") + " line " + _line + ": " + s);


# define CheckStreams() \
   if (_in_ix >= strlen(_stream) && sizeof(_streams) > 0) {	\
      _stream = _streams[0];					\
      _streams = _streams[1 ..];				\
      _in_ix = 0;						\
   }

# define Scan() {					\
   CheckStreams();					\
   if (_in_ix >= strlen(_stream)) {			\
      char = EOF;					\
   } else {						\
      char = _stream[_in_ix];				\
      if (char == '\n') {				\
	 _line ++;					\
      } else if (char == '\t' || char == '\r') {	\
	 char =  ' ';					\
      }							\
      _in_ix ++;					\
   }							\
}

# define ScanPastBlanks() {			\
   while (char == ' ' || char == '\n') {	\
      Scan();					\
   }						\
}

# define NewStream(s, f) {			\
   _streams = (s);				\
   _file = (f);					\
   _stream = "";				\
   _in_ix = 0;					\
   _line = 1;					\
   Scan();					\
}

# define Expect(t) {				\
   if (char != (t)) {				\
      LexErr("expecting " + translate[t] +	\
	     "; got " + (char == EOF ? "EOF" : translate[char]));	\
   }						\
}

# if 1
private
string ScanEntity() {
   string res;

   res = nil;
   for (;;) {
      switch(char) {
      case EOF:
	 LexErr("unexpected EOF parsing entity");
      case 'a' .. 'z':
      case 'A' .. 'Z':
	 if (!res) {
	    res = "";
	 }
	 res += " ";
	 res[strlen(res)-1] = char;
	 break;
      case ';':
      default:
	 return res;
      }
      Scan();
   }
}

private
string ScanName() {
   string res;
   int stop;

   res = "";

   /* allow a name to contain weird characters if inside quotes */
   if (char == '\"' || char == '\'') {
      stop = char;
      Scan();
      while (char != stop) {
	 if (char == EOF) {
	    LexErr("unexpected EOF in reference");
	 }
	 res += " ";
	 res[strlen(res)-1] = char;
	 Scan();
      }
      Scan();
      return res;
   }

   for (;;) {
      switch(char) {
      case 'a'..'z': case 'A'..'Z': case '0'..'9':
      case '.': case ':': case '_': case '-':
	 break;
      default:
	 return res;
      }
      res += " ";
      res[strlen(res)-1] = char;
      Scan();
   }
}

private
string ScanValue() {
   string res;

   res = "";

   for (;;) {
      switch(char) {
      case 'a'..'z': case 'A'..'Z': case '0'..'9':
      case '.': case ':': case '_': case '%': case '#':
	 break;
      default:
	 return res;
      }
      res += " ";
      res[strlen(res)-1] = char;
      Scan();
   }
}

private
string ScanMerry() {
   string str;
   int level, stop;

   str = "";
   /* believe it or not, there's a tiny Merry parser here */
   for (;;) {
      switch(char) {
      case EOF:
	 DEBUG("ScanMerry: str = " + dump_value(str));
	 LexErr("unexpected EOF in Merry");
	 break;
      case '\\':
	 str += char_to_string(char);
	 Scan();
	 break;
      case '\'': case '\"':
	 stop = char;
	 str += char_to_string(char);
	 Scan();

	 while (char != stop) {
	    switch(char) {
	    case EOF:
	       DEBUG("ScanMerry: str = " + dump_value(str));
	       LexErr("unexpected EOF in Merry string");
	       break;
	    case '\n':
	       DEBUG("ScanMerry: str = " + dump_value(str));
	       LexErr("unexpected newline in Merry string");
	       break;
	    case '\\':
	       str += char_to_string(char);
	       Scan();
	       break;
	    }
	    str += char_to_string(char);
	    Scan();
	 }
	 break;
      case '/':
	 /* Possible comment start */
	 str += "/";
	 Scan();
	 if (char == '*') {
	    int found;

	    /* Indeed it is. */
	    str += "*";
	    Scan();
	    found = FALSE;
	    while (!found) {
	       switch (char) {
	       case EOF:
		  DEBUG("ScanMerry: str = " + dump_value(str));
		  LexErr("unexpected EOF in Merry comment");
		  break;
	       case '*':
		  str += "*";
		  Scan();
		  /* Is the next one a '/'? */
		  if (char == '/') {
		     /* That's the end-comment then. */
		     found = TRUE;
		  }
		  break;
	       default:
		  str += char_to_string(char);
		  Scan();
		  break;
	       }
	    }
	    break;
	 } else {
	    /* Oh, it isn't.  As you were... */
	    break;
	 }
      case '[':
	 level ++;
	 break;
      case ']':
	 if (level == 0) {
	    return str;
	 }
	 level --;
	 break;
      }

      str += char_to_string(char);
      Scan();
   }
}

# else
private
string ScanEntity() {
   string res;
   int left, ix;

   left = _in_ix-1;
   while (_in_ix < strlen(_stream)) {
      if (_stream[_in_ix] == ';') {
	 res = _stream[left .. _in_ix-1];
	 Scan();
	 return res;
      }
      _in_ix ++;
   }
   LexErr("unexpected EOF parsing entity");
}

private
string ScanName() {
   string res;
   int left, ix;

   left = _in_ix-1;
   while (_in_ix < strlen(_stream)) {
      switch(_stream[_in_ix]) {
      case 'a'..'z': case 'A'..'Z': case '0'..'9':
      case '.': case ':': case '_': case '-':
	 break;
      default:
	 res = _stream[left .. _in_ix-1];
	 Scan();
	 return res;
      }
      _in_ix ++;
   }
   char = EOF;
   return _stream[left ..];
}

private
string ScanValue() {
   string res;
   int left, ix;

   left = _in_ix-1;
   while (_in_ix < strlen(_stream)) {
      switch(_stream[_in_ix]) {
      case 'a'..'z': case 'A'..'Z': case '0'..'9':
      case '.': case ':': case '_': case '%': case '#':
	 break;
      default:
	 res = _stream[left .. _in_ix-1];
	 Scan();
	 return res;
      }
      _in_ix ++;
   }
   return _stream[left ..];
}
# endif
