/*
**	/lib/string.c
**
**	Useful functions for manipulating strings.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

static
string strip_left(string str, varargs int leave_newlines) {
   int i;

   for (i = 0; i < strlen(str); i ++) {
      switch(str[i]) {
      case ' ': case '\t':
	 break;
      case '\n': case '\r':
	 if (!leave_newlines) {
	    break;
	 }
      default:
	 if (i == 0) {
	    return str;
	 }
	 return str[i ..];
      }
   }
   return "";
}

static
string strip_right(string str, varargs int leave_newlines) {
  int i;

  for (i = strlen(str)-1; i >= 0; i --) {
      switch(str[i]) {
      case ' ': case '\t':
	 break;
      case '\n': case '\r':
	 if (!leave_newlines) {
	    break;
	 }
      default:
	 if (i == strlen(str)-1) {
	    return str;
	 }
	 return str[.. i];
      }
  }
  return "";
}

static
string strip(string str, varargs int leave_newlines) {
   return strip_right(strip_left(str, leave_newlines), leave_newlines);
}

# define DOWN(c)	((c) |= 0x20)
# define UP(c)		((c) &= ~0x20)

static
string lower_case(string str) {
    int i;

    for (i = 0; i < strlen(str); i ++) {
       if (str[i] >= 'A' && str[i] <= 'Z') {
	  DOWN(str[i]);
       }
    }
    return str;
}

static
string upper_case(string str) {
    int i;

    for (i = 0; i < strlen(str); i ++) {
       if (str[i] >= 'a' && str[i] <= 'z') {
	  UP(str[i]);
       }
    }
    return str;
}

static
int contains(string str, string sub) {
  return !!sscanf(str, "%*s" + sub);
}

static
string capitalize(string str) {
    int i;
    if (strlen(str) > 0 && str[0] >= 'a' && str[0] <= 'z') {
       UP(str[0]);
    }
    return str;
}

static
string decapitalize(string str) {
    int i;
    if (strlen(str) > 0 && str[0] >= 'A' && str[0] <= 'Z') {
       DOWN(str[0]);
    }
    return str;
}


static
string proper ( string str ) {
   int i;
   for(i = 0; i < strlen(str); i ++) {
      if (i == 0 || (i > 0 && str[i-1] == ' ')) {
         if (str[i] >= 'a' && str[i] <= 'z') {
	    UP(str[i]);
	 }
      }
   }
   return str;
}

static
string spaces(int num) {
   string str;

   str = "                    ";
   while (strlen(str) < num) {
      str += str;
   }
   return str[.. num-1];
}

static
string pad_left(string var, int size) {
   if (strlen(var) >= size) {
      return var;
   }
   return spaces(size-strlen(var)) + var;
}

static
string pad_right(string var, int size) {
   if (strlen(var) >= size) {
      return var;
   }
   return var + spaces(size-strlen(var));
}


static
string format_float(float f, int dec) {
    string res, frac, pre;

    if (f > -1. && f < 1.) {
	int i;
	float p;

	p = pow(10., (float)(dec + 1));
	i = (int)(f * p);
	f = (float)i / p;
    }
    res = (string) f;
    if (sscanf(res, "%s.%s", pre, frac) == 2) {
	return pre + ("." + frac + "00000000")[.. dec];
    }
    return res + ".000000000"[.. dec];
}

static
string comma(int num) {
    string res;

    res = (string) (num % 1000);
    while (num / 1000) {
       /* zero-pad 'res' to the left */
       res = ("00" + res)[(strlen(res) % 4)-1 ..];
       num /= 1000;
       res = (num % 1000) + "," + res;
    }
    return res;
}

static
string assemble_nuggets(string *nuggets, int len, int indent, string delim) {
   string *out, *words, spaces, line;
   int i;

   out = ({ });

   if (sizeof(nuggets)) {
      line = nuggets[0];
      for (i = 1; i < sizeof(nuggets); i ++) {
	 if (strlen(nuggets[i])) {
	    if (strlen(line) + strlen(nuggets[i] + 1) < len) {
	       switch(nuggets[i][0]) {
	       case '.': case ',': case '!':
		  line += nuggets[i];
		  break;
	       default:
		  line += delim + nuggets[i];
	       }
	    } else {
	       out += ({ line });
	       line = spaces(indent) + nuggets[i];
	    }
	 }
      }
      out += ({ line });
   }
   return implode(out, "\n");
}

static
string break_paragraph(string head, string str, int len) {
   if (!head) {
      head = "";
   }
   return implode(explode(head + str, "\n"), "\n") + "\n";
}

static
string brick_paragraph(string str, int indent, int len) {
   return implode(explode(str, "\n"), "\n") + "\n";
}

static
string break_string(string str, int len) {
   string *lines;
   int i;

    if (!len) {
	len = 76;
    }
    lines = explode(str, "\n");
    for (i = 0; i < sizeof(lines); i ++) {
	lines[i] = break_paragraph(nil, lines[i], len);
    }
    return implode(lines, "");
}

static
string tabulate(mixed *arrlist...) {
   mixed *arr;
   string *lines;
   int *colwidth;
   int i, j, rows;

   colwidth = allocate_int(sizeof(arrlist));

   XDebug(dump_value(arrlist));
   for (i = 0; i < sizeof(arrlist); i ++) {
      arr = arrlist[i];
      if (sizeof(arr) > rows) {
	 rows = sizeof(arr);
      }
      for (j = 0; j < sizeof(arr); j ++) {
	 if (typeof(arr[j]) != T_STRING) {
	    catch {
	       arr[j] = (string) arr[j];
	    } : {
	       arr[j] = "ERR";
	    }
	 }
	 if (strlen(arr[j]) > colwidth[i]) {
	    colwidth[i] = strlen(arr[j]);
	 }
      }
   }
   lines = allocate(rows);
   for (j = 0; j < rows; j ++) {
      int edge, cursor;

      edge = 0; cursor = 0;
      lines[j] = "";
      for (i = 0; i < sizeof(arrlist); i ++) {
	 if (j < sizeof(arrlist[i])) {
	    if (edge < cursor) {
	       lines[j] += spaces(cursor-edge);
	       edge = cursor;
	    }
	    lines[j] += arrlist[i][j];
	    edge += strlen(arrlist[i][j]);
	    cursor += colwidth[i] + 2;
	 }
      }
   }
   return implode(lines, "\n") + "\n";
}

static
int hex_digit(int nibble) {
   switch(nibble) {
   case 0 .. 9:
      return '0' + nibble;
   case 10 .. 15:
      return 'a' + (nibble - 10);
   }
}

static
string to_hex(string bytes) {
   string hex;
   int i, j;

   hex = bytes + bytes;	/* just to allocate a double-length string */

   for (i = 0; i < strlen(bytes); i ++) {
      hex[j ++] = hex_digit(bytes[i] >> 4);
      hex[j ++] = hex_digit(bytes[i] & 0x0F);
   }
   return hex;
}

static
int number_digit(int hexvalue) {
   int number;

   switch(hexvalue) {
   case 48 .. 57:
      number = hexvalue - 48; break;
   default:
      number = 0; break;
   }

   return number;
}

static
string desc_cardinal(int n) {
   string str;

   /* don't set str = "" here, its nil-ness is vital below */

   if (n <= 0) 
      return "zero";

   if (n >= 1000000000) {
      str = desc_cardinal(n / 1000000000) + " billion";
      n %= 1000000000;
   }
   if (n >= 1000000) {
      if (str) str += ", "; else str = "";
      str += desc_cardinal(n / 1000000) + " million";
      n %= 1000000;
   }
   if (n >= 1000) {
      if (str) str += ", "; else str = "";
      str += desc_cardinal(n / 1000) + " thousand";
      n %= 1000;
   }
   if (n >= 100) {
      if (str) str += ", "; else str = "";
      str += desc_cardinal(n / 100) + " hundred";
      n %= 100;
   }
   if (n == 0) {
      return str;
   }
   if (n >= 20) {
      if (!str) str = ""; else str += " ";
      switch(n / 10) {
      case 2:	str += "twenty";	break;
      case 3:	str += "thirty";	break;
      case 4:	str += "forty";		break;
      case 5:	str += "fifty";		break;
      case 6:	str += "sixty";		break;
      case 7:	str += "seventy";	break;
      case 8:	str += "eighty";	break;
      case 9:	str += "ninety";	break;
      }
      n %= 10;
      if (n > 0) {
	 str += "-";
      }
   } else if (!str) {
      str = "";
   } else {
      str += " ";
   }
   switch(n) {
   case 1:	str += "one";		break;
   case 2:	str += "two";		break;
   case 3:	str += "three";		break;
   case 4:	str += "four";		break;
   case 5:	str += "five";		break;
   case 6:	str += "six";		break;
   case 7:	str += "seven";		break;
   case 8:	str += "eight";		break;
   case 9:	str += "nine";		break;
   case 10:	str += "ten";		break;
   case 11:	str += "eleven";	break;
   case 12:	str += "twelve";	break;
   case 13:	str += "thirteen";	break;
   case 14:	str += "fourteen";	break;
   case 15:	str += "fifteen";	break;
   case 16:	str += "sixteen";	break;
   case 17:	str += "seventeen";	break;
   case 18:	str += "eighteen";	break;
   case 19:	str += "nineteen";	break;
   }
   return str;
}

static
string desc_ordinal(int n) {
   string str;

   if (n == 0)
      return "zero";

   if (n >= 1000000000) {
      str = desc_cardinal(n / 1000000000) + " billion";
      n %= 1000000000;
   }
   if (n >= 1000000) {
      if (str) str += ", "; else str = "";
      str += desc_cardinal(n / 1000000) + " million";
      n %= 1000000;
   }
   if (n >= 1000) {
      if (str) str += ", "; else str = "";
      str += desc_cardinal(n / 1000) + " thousand";
      n %= 1000;
   }
   if (n >= 100) {
      if (str) str += ", "; else str = "";
      str += desc_cardinal(n / 100) + " hundred";
      n %= 100;
   }
   if (n == 0) {
      return str + "th";
   }
   if (n >= 20) {
      if (!str) str = ""; else str += " ";
      switch(n) {
      case 20:	return str + "twentieth";	break;
      case 30:	return str + "thirtieth";	break;
      case 40:	return str + "fortieth";	break;
      case 50:	return str + "fiftieth";	break;
      case 60:	return str + "sixtieth";	break;
      case 70:	return str + "seventieth";	break;
      case 80:	return str + "eightieth";	break;
      case 90:	return str + "ninetieth";	break;
      }

      switch(n / 10) {
      case 2:	str += "twenty";	break;
      case 3:	str += "thirty";	break;
      case 4:	str += "forty";		break;
      case 5:	str += "fifty";		break;
      case 6:	str += "sixty";		break;
      case 7:	str += "seventy";	break;
      case 8:	str += "eighty";	break;
      case 9:	str += "ninety";	break;
      }
      n %= 10;
      if (n > 0) {
	 str += "-";
      }
   } else if (!str) {
      str = "";
   } else {
      str += " ";
   }
   switch(n) {
   case 1:	str += "first";		break;
   case 2:	str += "second";	break;
   case 3:	str += "third";		break;
   case 4:	str += "fourth";	break;
   case 5:	str += "fifth";		break;
   case 6:	str += "sixth";		break;
   case 7:	str += "seventh";	break;
   case 8:	str += "eighth";	break;
   case 9:	str += "ninth";		break;
   case 10:	str += "tenth";		break;
   case 11:	str += "eleventh";	break;
   case 12:	str += "twelfth";	break;
   case 13:	str += "thirteenth";	break;
   case 14:	str += "fourteenth";	break;
   case 15:	str += "fifteenth";	break;
   case 16:	str += "sixteenth";	break;
   case 17:	str += "seventeenth";	break;
   case 18:	str += "eighteenth";	break;
   case 19:	str += "nineteenth";	break;
   }
   return str;
}

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

string replace_html(string str) {
    return replace_strings(str,
			   "&",  "&amp;",
			   "\"", "&quot;",
			   "<",  "&lt;",
			   ">",  "&gt;");
}

string replace_xml(string str) {
    str = replace_html(str);
    str = replace_strings(str,
			  "\\", "\\\\",
			  "&",  "\\&",
			  "$",  "\\$",
			  "<",  "\\<",
			  ">",  "\\>",
			  "{",  "\\{",
			  "}",  "\\}",
			  "|",  "\\|");
    return str;
}

string *full_explode(string str, string del) {
   string *args;
   int i;

   args = explode("\0" + str + "\0", del);
   i = sizeof(args)-1;
   args[i] = args[i][.. strlen(args[i])-2];
   args[0] = args[0][1 ..];

   return args;
}

static
int cardinal(string str) {
   int num;
   string str1, str2;

   if (sscanf(str, "%d", num)) {
       return num;
   }
   if (sscanf(str, "%s-%s", str1, str2) == 2) {
       str = str1 + str2;
   }
   switch(str) {
   case "one":          return  1;
   case "two":          return  2;
   case "three":        return  3;
   case "four":         return  4;
   case "five":         return  5;
   case "six":          return  6;
   case "seven":        return  7;
   case "eight":        return  8;
   case "nine":         return  9;
   case "ten":          return 10;
   case "eleven":       return 11;
   case "twelve":       return 12;
   case "thirteen":     return 13;
   case "fourteen":     return 14;
   case "fifteen":      return 15;
   case "sixteen":      return 16;
   case "seventeen":    return 17;
   case "eighteen":     return 18;
   case "nineteen":     return 19;
   case "twenty":       return 20;
   case "twentyone":    return 21;
   case "twentytwo":    return 22;
   case "twentythree":  return 23;
   case "twentyfour":   return 24;
   case "twentyfive":   return 25;
   case "twentysix":    return 26;
   case "twentyseven":  return 27;
   case "twentyeight":  return 28;
   case "twentynine":   return 29;
   case "thirty":       return 30;
   case "thirtyone":    return 31;
   case "thirtytwo":    return 32;
   case "thirtythree":  return 33;
   case "thirtyfour":   return 34;
   case "thirtyfive":   return 35;
   case "thirtysix":    return 36;
   case "thirtyseven":  return 37;
   case "thirtyeight":  return 38;
   case "thirtynine":   return 39;
   case "forty":        return 40;
   case "fortyone":     return 41;
   case "fortytwo":     return 42;
   case "fortythree":   return 43;
   case "fortyfour":    return 44;
   case "fortyfive":    return 45;
   case "fortysix":     return 46;
   case "fortyseven":   return 47;
   case "fortyeight":   return 48;
   case "fortynine":    return 49;
   case "fifty":        return 50;
   case "fiftyone":     return 51;
   case "fiftytwo":     return 52;
   case "fiftythree":   return 53;
   case "fiftyfour":    return 54;
   case "fiftyfive":    return 55;
   case "fiftysix":     return 56;
   case "fiftyseven":   return 57;
   case "fiftyeight":   return 58;
   case "fiftynine":    return 59;
   case "sixty":        return 60;
   case "sixtyone":     return 61;
   case "sixtytwo":     return 62;
   case "sixtythree":   return 63;
   case "sixtyfour":    return 64;
   case "sixtyfive":    return 65;
   case "sixtysix":     return 66;
   case "sixtyseven":   return 67;
   case "sixtyeight":   return 68;
   case "sixtynine":    return 69;
   case "seventy":      return 70;
   case "seventyone":   return 71;
   case "seventytwo":   return 72;
   case "seventythree": return 73;
   case "seventyfour":  return 74;
   case "seventyfive":  return 75;
   case "seventysix":   return 76;
   case "seventyseven": return 77;
   case "seventyeight": return 78;
   case "seventynine":  return 79;
   case "eighty":       return 80;
   case "eightyone":    return 81;
   case "eightytwo":    return 82;
   case "eightythree":  return 83;
   case "eightyfour":   return 84;
   case "eightyfive":   return 85;
   case "eightysix":    return 86;
   case "eightyseven":  return 87;
   case "eightyeight":  return 88;
   case "eightynine":   return 89;
   case "ninety":       return 90;
   case "ninetyone":    return 91;
   case "ninetytwo":    return 92;
   case "ninetythree":  return 93;
   case "ninetyfour":   return 94;
   case "ninetyfive":   return 95;
   case "ninetysix":    return 96;
   case "ninetyseven":  return 97;
   case "ninetyeight":  return 98;
   case "ninetynine":   return 99;
   default:             return -1;
   }
   return num;
}

static
int ordinal(string str) {
   int num;
   string str1, str2;

   if (sscanf(str, "%dst", num) ||
       sscanf(str, "%dnd", num) ||
       sscanf(str, "%drd", num) ||
       sscanf(str, "%dth", num)) {
       return num;
   }
   if (sscanf(str, "%s-%s", str1, str2) == 2) {
       str = str1 + str2;
   }

   switch(str) {
   case "first":          return  1;
   case "second":         return  2;
   case "third":          return  3;
   case "fourth":         return  4;
   case "fifth":          return  5;
   case "sixth":          return  6;
   case "seventh":        return  7;
   case "eighth":         return  8;
   case "ninth":          return  9;
   case "tenth":          return 10;
   case "eleventh":       return 11;
   case "twelfth":        return 12;
   case "thirteenth":     return 13;
   case "fourteenth":     return 14;
   case "fifteenth":      return 15;
   case "sixteenth":      return 16;
   case "seventeenth":    return 17;
   case "eighteenth":     return 18;
   case "nineteenth":     return 19;
   case "twentieth":      return 20;
   case "twentyfirst":    return 21;
   case "twentysecond":   return 22;
   case "twentythird":    return 23;
   case "twentyfourth":   return 24;
   case "twentyfifth":    return 25;
   case "twentysixth":    return 26;
   case "twentyseventh":  return 27;
   case "twentyeighth":   return 28;
   case "twentyninth":    return 29;
   case "thirtieth":      return 30;
   case "thirtyfirst":    return 31;
   case "thirtysecond":   return 32; 
   case "thirtythird":    return 33;
   case "thirtyfourth":   return 34;
   case "thirtyfifth":    return 35;
   case "thirtysixth":    return 36;
   case "thirtyseventh":  return 37;
   case "thirtyeighth":   return 38;
   case "thirtyninth":    return 39;
   case "fortieth":       return 40;
   case "fortyfirst":     return 41;
   case "fortysecond":    return 42;
   case "fortythird":     return 43;
   case "fortyfourth":    return 44;
   case "fortyfifth":     return 45;
   case "fortysixth":     return 46;
   case "fortyseventh":   return 47;
   case "fortyeighth":    return 48;
   case "fortyninth":     return 49;
   case "fiftieth":       return 50;
   case "fiftyfirst":     return 51;
   case "fiftysecond":    return 52;
   case "fiftythird":     return 53;
   case "fiftyfourth":    return 54;
   case "fiftyfifth":     return 55;
   case "fiftysixth":     return 56;
   case "fiftyseventh":   return 57;
   case "fiftyeighth":    return 58;
   case "fiftyninth":     return 59;
   case "sixtieth":       return 60;
   case "sixtyfirst":     return 61;
   case "sixtysecond":    return 62;
   case "sixtythird":     return 63;
   case "sixtyfourth":    return 64;
   case "sixtyfifth":     return 65;
   case "sixtysixth":     return 66;
   case "sixtyseventh":   return 67;
   case "sixtyeighth":    return 68;
   case "sixtyninth":     return 69;
   case "seventieth":     return 70;
   case "seventyfirst":   return 71;
   case "seventysecond":  return 72;
   case "seventythird":   return 73;
   case "seventyfourth":  return 74;
   case "seventyfifth":   return 75;
   case "seventysixth":   return 76;
   case "seventyseventh": return 77;
   case "seventyeighth":  return 78;
   case "seventyninth":   return 79;
   case "eightieth":      return 80;
   case "eightyfirst":    return 81;
   case "eightysecond":   return 82;
   case "eightythird":    return 83;
   case "eightyfourth":   return 84;
   case "eightyfifth":    return 85;
   case "eightysixth":    return 86;
   case "eightyseventh":  return 87;
   case "eightyeighth":   return 88;
   case "eightyninth":    return 89;
   case "ninetieth":      return 90;
   case "ninetyfirst":    return 91;
   case "ninetysecond":   return 92;
   case "ninetythird":    return 93;
   case "ninetyfourth":   return 94;
   case "ninetyfifth":    return 95;
   case "ninetysixth":    return 96;
   case "ninetyseventh":  return 97;
   case "ninetyeighth":   return 98;
   case "ninetyninth":    return 99;
   default:               return -1;
   }
   return num;
}

string oneof(string arr...) {
   if (sizeof(arr)) {
      return arr[random(sizeof(arr))];
   }
   return "";
}

string char_to_string(int char) {
   string res;

   res = " ";
   res[0] = char;
   return res;
}
