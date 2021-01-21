# include <type.h>

private inherit "/lib/string";


/* the base64 encoding is defined in RFC 1521 */

static
int base64_char(int c) {
    switch(c) {
    case 'A' .. 'Z': return  0 + c - 'A';
    case 'a' .. 'z': return 26 + c - 'a';
    case '0' .. '9': return 52 + c - '0';
    case '+': return 62;
    case '/': return 63;
    }
    return 0;
}

/*
**			AAAAAABBBBBBCCCCCCDDDDDD
**	Input:		654321654321654321654321
**
**	Output:         876543218765432187654321
**			AAAAAAAABBBBBBBBCCCCCCCC
*/

static
string base64_decode(string str) {
    int i, j, n;

    while (i+1 < strlen(str)) {
	str[j] = base64_char(str[i]) << 2;
	n = base64_char(str[i+1]);
	str[j] |= n >> 4;
	j ++; i += 2;
	/* are we done? */
	if (i >= strlen(str) || str[i] == '=') {
	    return str[.. j-1];
	}
	str[j] = (n & 0x0F) << 4;
	n = base64_char(str[i]);
	str[j] |= n >> 2;
	j ++; i ++;
	if (i >= strlen(str) || str[i] == '=') {
	    return str[.. j-1];
	}
	str[j] = (n & 0x03) << 6;
	str[j] |= base64_char(str[i]);
	j ++; i ++;
    }
    return str[.. j-1];
}

private
int dehex(int c) {
    switch(c) {
    case '0' .. '9': return c - '0';
    case 'a' .. 'h': return c + 0x0A - 'a';
    case 'A' .. 'H': return c + 0x0A - 'A';
    }
    return 0;
}

private
int hex(int c) {
   if (c < 10) {
      return '0' + c;
   }
   if (c < 16) {
      return 'A' + (c - 10);
   }
   return 'X';
}

static
string url_decode(string str) {
   return SYS_URL_DECODE->url_decode(str);
}

static
string url_encode(string str) {
   return SYS_URL_ENCODE->url_encode(str);
}

static
string make_url(string base, mapping args, varargs int secure) {
   string *argix;
   mixed  *argval;
   int i;

   if (!base) {
      base = "/";
   }
   if (map_sizeof(args) == 0) {
      return base;
   }
   args -= ({ "args", "request" });
   argix  = map_indices(args);
   argval = map_values(args);
   for (i = 0; i < sizeof(argix); i ++) {
      if (typeof(argval[i]) == T_OBJECT) {
	 argval[i] = name(argval[i]);
      }
      argix[i] += "=" + url_encode((string) argval[i]);
   }
   if (secure) {
      argix += ({ "arghash=" + to_hex(hash_md5("SKO&" +
					       implode(argix, "&") +
					       "&TOS")) });
   }

   if (sizeof(argix)) {
      if (sscanf(base, "%*s?")) {
	 /* append to existin arguments */
	 return base + "&" + implode(argix, "&");
      }
      /* new argument section of URL */
      return base + "?" + implode(argix, "&");
   }
   /* or no arguments */
   return base;
}


string url_absolute(string url) {
   if (!sscanf(url, "%*s://")) {
      /* rewrite the URL */
      url = "http://" +	 SYS_INITD->query_hostname() + ":" + SYS_INITD->query_webport() + url;
   }
   return url;
}
