/*
 * Copyright Skotos Tech Inc 2003
 */

# include <status.h>

private string
plusses(int num)
{
    string str;
   
    str = "++++++++++++++++++++";
    while (strlen(str) < num) {
	str += str;
    }
    return str[.. num-1];
}

static mixed *
enc_0(mixed *tree)
{
    return ({ plusses(strlen(tree[0])) });
}

string
url_encode(string text)
{
    int    i, sz, len, size, offset;
    string *chunks;
    mixed *tree;

    len = strlen(text);
    size = status()[ST_ARRAYSIZE];
    if (size > 2048) {
	/*
	 * Large arrays cause much bigger chunks, this is a trade-off which
	 * will still allow strings of at least 4MB to be processed fast.
	 */
	size = 2048;
    }
    sz = (len + size - 1) / size;
    chunks = allocate(sz);
    for (i = offset = 0; i < sz; i++, offset += size) {
	if (offset + size > len) {
	    /* At the end of the file. */
	    chunks[i] = text[offset ..];
	} else {
	    chunks[i] = text[offset .. offset + size - 1];
	}
    }
    for (i = 0; i < sz; i++) {
	mixed *tree;

	tree = parse_string(
"spaces = / +/\n" +
"encode = /[\x00-\x1f\x7f<>\"#%{}|\\^~\\[\\]`;\\/?:+@=&]+/\n" +
"other  = /[^\x00-\x1f\x7f<>\"#%{}|\\^~\\[\\]`;\\/?:+@=& ]+/\n" +
"\n" +
"opt_text:\n" +
"opt_text: text\n" +
"\n" +
"text:     chunk\n" +
"text:     text chunk\n" +
"\n" +
"chunk:    spaces     ? enc_0\n" +
"chunk:    other\n" +
"chunk:    encode     ? hex_bytes\n", chunks[i]);
	chunks[i] = implode(tree, "");
    }
    return implode(chunks, "");
}

static mixed *
hex_bytes(mixed *tree)
{
    int i, sz;
    string *str, orig;

    orig = tree[0];
    sz = strlen(orig);
    str = allocate(sz);
    for (i = 0; i < sz; i++) {
	switch (orig[i]) {
	case 0x00: str[i] = "%00"; break;
	case 0x01: str[i] = "%01"; break;
	case 0x02: str[i] = "%02"; break;
	case 0x03: str[i] = "%03"; break;
	case 0x04: str[i] = "%04"; break;
	case 0x05: str[i] = "%05"; break;
	case 0x06: str[i] = "%06"; break;
	case 0x07: str[i] = "%07"; break;
	case 0x08: str[i] = "%08"; break;
	case 0x09: str[i] = "%09"; break;
	case 0x0a: str[i] = "%0A"; break;
	case 0x0b: str[i] = "%0B"; break;
	case 0x0c: str[i] = "%0C"; break;
	case 0x0d: str[i] = "%0D"; break;
	case 0x0e: str[i] = "%0E"; break;
	case 0x0f: str[i] = "%0F"; break;
	case 0x10: str[i] = "%10"; break;
	case 0x11: str[i] = "%11"; break;
	case 0x12: str[i] = "%12"; break;
	case 0x13: str[i] = "%13"; break;
	case 0x14: str[i] = "%14"; break;
	case 0x15: str[i] = "%15"; break;
	case 0x16: str[i] = "%16"; break;
	case 0x17: str[i] = "%17"; break;
	case 0x18: str[i] = "%18"; break;
	case 0x19: str[i] = "%19"; break;
	case 0x1a: str[i] = "%1A"; break;
	case 0x1b: str[i] = "%1B"; break;
	case 0x1c: str[i] = "%1C"; break;
	case 0x1d: str[i] = "%1D"; break;
	case 0x1e: str[i] = "%1E"; break;
	case 0x1f: str[i] = "%1F"; break;
	case 0x20: str[i] = "%20"; break;
	case 0x21: str[i] = "%21"; break;
	case 0x22: str[i] = "%22"; break;
	case 0x23: str[i] = "%23"; break;
	case 0x24: str[i] = "%24"; break;
	case 0x25: str[i] = "%25"; break;
	case 0x26: str[i] = "%26"; break;
	case 0x27: str[i] = "%27"; break;
	case 0x28: str[i] = "%28"; break;
	case 0x29: str[i] = "%29"; break;
	case 0x2a: str[i] = "%2A"; break;
	case 0x2b: str[i] = "%2B"; break;
	case 0x2c: str[i] = "%2C"; break;
	case 0x2d: str[i] = "%2D"; break;
	case 0x2e: str[i] = "%2E"; break;
	case 0x2f: str[i] = "%2F"; break;
	case 0x30: str[i] = "%30"; break;
	case 0x31: str[i] = "%31"; break;
	case 0x32: str[i] = "%32"; break;
	case 0x33: str[i] = "%33"; break;
	case 0x34: str[i] = "%34"; break;
	case 0x35: str[i] = "%35"; break;
	case 0x36: str[i] = "%36"; break;
	case 0x37: str[i] = "%37"; break;
	case 0x38: str[i] = "%38"; break;
	case 0x39: str[i] = "%39"; break;
	case 0x3a: str[i] = "%3A"; break;
	case 0x3b: str[i] = "%3B"; break;
	case 0x3c: str[i] = "%3C"; break;
	case 0x3d: str[i] = "%3D"; break;
	case 0x3e: str[i] = "%3E"; break;
	case 0x3f: str[i] = "%3F"; break;
	case 0x40: str[i] = "%40"; break;
	case 0x41: str[i] = "%41"; break;
	case 0x42: str[i] = "%42"; break;
	case 0x43: str[i] = "%43"; break;
	case 0x44: str[i] = "%44"; break;
	case 0x45: str[i] = "%45"; break;
	case 0x46: str[i] = "%46"; break;
	case 0x47: str[i] = "%47"; break;
	case 0x48: str[i] = "%48"; break;
	case 0x49: str[i] = "%49"; break;
	case 0x4a: str[i] = "%4A"; break;
	case 0x4b: str[i] = "%4B"; break;
	case 0x4c: str[i] = "%4C"; break;
	case 0x4d: str[i] = "%4D"; break;
	case 0x4e: str[i] = "%4E"; break;
	case 0x4f: str[i] = "%4F"; break;
	case 0x50: str[i] = "%50"; break;
	case 0x51: str[i] = "%51"; break;
	case 0x52: str[i] = "%52"; break;
	case 0x53: str[i] = "%53"; break;
	case 0x54: str[i] = "%54"; break;
	case 0x55: str[i] = "%55"; break;
	case 0x56: str[i] = "%56"; break;
	case 0x57: str[i] = "%57"; break;
	case 0x58: str[i] = "%58"; break;
	case 0x59: str[i] = "%59"; break;
	case 0x5a: str[i] = "%5A"; break;
	case 0x5b: str[i] = "%5B"; break;
	case 0x5c: str[i] = "%5C"; break;
	case 0x5d: str[i] = "%5D"; break;
	case 0x5e: str[i] = "%5E"; break;
	case 0x5f: str[i] = "%5F"; break;
	case 0x60: str[i] = "%60"; break;
	case 0x61: str[i] = "%61"; break;
	case 0x62: str[i] = "%62"; break;
	case 0x63: str[i] = "%63"; break;
	case 0x64: str[i] = "%64"; break;
	case 0x65: str[i] = "%65"; break;
	case 0x66: str[i] = "%66"; break;
	case 0x67: str[i] = "%67"; break;
	case 0x68: str[i] = "%68"; break;
	case 0x69: str[i] = "%69"; break;
	case 0x6a: str[i] = "%6A"; break;
	case 0x6b: str[i] = "%6B"; break;
	case 0x6c: str[i] = "%6C"; break;
	case 0x6d: str[i] = "%6D"; break;
	case 0x6e: str[i] = "%6E"; break;
	case 0x6f: str[i] = "%6F"; break;
	case 0x70: str[i] = "%70"; break;
	case 0x71: str[i] = "%71"; break;
	case 0x72: str[i] = "%72"; break;
	case 0x73: str[i] = "%73"; break;
	case 0x74: str[i] = "%74"; break;
	case 0x75: str[i] = "%75"; break;
	case 0x76: str[i] = "%76"; break;
	case 0x77: str[i] = "%77"; break;
	case 0x78: str[i] = "%78"; break;
	case 0x79: str[i] = "%79"; break;
	case 0x7a: str[i] = "%7A"; break;
	case 0x7b: str[i] = "%7B"; break;
	case 0x7c: str[i] = "%7C"; break;
	case 0x7d: str[i] = "%7D"; break;
	case 0x7e: str[i] = "%7E"; break;
	case 0x7f: str[i] = "%7F"; break;
	case 0x80: str[i] = "%80"; break;
	case 0x81: str[i] = "%81"; break;
	case 0x82: str[i] = "%82"; break;
	case 0x83: str[i] = "%83"; break;
	case 0x84: str[i] = "%84"; break;
	case 0x85: str[i] = "%85"; break;
	case 0x86: str[i] = "%86"; break;
	case 0x87: str[i] = "%87"; break;
	case 0x88: str[i] = "%88"; break;
	case 0x89: str[i] = "%89"; break;
	case 0x8a: str[i] = "%8A"; break;
	case 0x8b: str[i] = "%8B"; break;
	case 0x8c: str[i] = "%8C"; break;
	case 0x8d: str[i] = "%8D"; break;
	case 0x8e: str[i] = "%8E"; break;
	case 0x8f: str[i] = "%8F"; break;
	case 0x90: str[i] = "%90"; break;
	case 0x91: str[i] = "%91"; break;
	case 0x92: str[i] = "%92"; break;
	case 0x93: str[i] = "%93"; break;
	case 0x94: str[i] = "%94"; break;
	case 0x95: str[i] = "%95"; break;
	case 0x96: str[i] = "%96"; break;
	case 0x97: str[i] = "%97"; break;
	case 0x98: str[i] = "%98"; break;
	case 0x99: str[i] = "%99"; break;
	case 0x9a: str[i] = "%9A"; break;
	case 0x9b: str[i] = "%9B"; break;
	case 0x9c: str[i] = "%9C"; break;
	case 0x9d: str[i] = "%9D"; break;
	case 0x9e: str[i] = "%9E"; break;
	case 0x9f: str[i] = "%9F"; break;
	case 0xa0: str[i] = "%A0"; break;
	case 0xa1: str[i] = "%A1"; break;
	case 0xa2: str[i] = "%A2"; break;
	case 0xa3: str[i] = "%A3"; break;
	case 0xa4: str[i] = "%A4"; break;
	case 0xa5: str[i] = "%A5"; break;
	case 0xa6: str[i] = "%A6"; break;
	case 0xa7: str[i] = "%A7"; break;
	case 0xa8: str[i] = "%A8"; break;
	case 0xa9: str[i] = "%A9"; break;
	case 0xaa: str[i] = "%AA"; break;
	case 0xab: str[i] = "%AB"; break;
	case 0xac: str[i] = "%AC"; break;
	case 0xad: str[i] = "%AD"; break;
	case 0xae: str[i] = "%AE"; break;
	case 0xaf: str[i] = "%AF"; break;
	case 0xb0: str[i] = "%B0"; break;
	case 0xb1: str[i] = "%B1"; break;
	case 0xb2: str[i] = "%B2"; break;
	case 0xb3: str[i] = "%B3"; break;
	case 0xb4: str[i] = "%B4"; break;
	case 0xb5: str[i] = "%B5"; break;
	case 0xb6: str[i] = "%B6"; break;
	case 0xb7: str[i] = "%B7"; break;
	case 0xb8: str[i] = "%B8"; break;
	case 0xb9: str[i] = "%B9"; break;
	case 0xba: str[i] = "%BA"; break;
	case 0xbb: str[i] = "%BB"; break;
	case 0xbc: str[i] = "%BC"; break;
	case 0xbd: str[i] = "%BD"; break;
	case 0xbe: str[i] = "%BE"; break;
	case 0xbf: str[i] = "%BF"; break;
	case 0xc0: str[i] = "%C0"; break;
	case 0xc1: str[i] = "%C1"; break;
	case 0xc2: str[i] = "%C2"; break;
	case 0xc3: str[i] = "%C3"; break;
	case 0xc4: str[i] = "%C4"; break;
	case 0xc5: str[i] = "%C5"; break;
	case 0xc6: str[i] = "%C6"; break;
	case 0xc7: str[i] = "%C7"; break;
	case 0xc8: str[i] = "%C8"; break;
	case 0xc9: str[i] = "%C9"; break;
	case 0xca: str[i] = "%CA"; break;
	case 0xcb: str[i] = "%CB"; break;
	case 0xcc: str[i] = "%CC"; break;
	case 0xcd: str[i] = "%CD"; break;
	case 0xce: str[i] = "%CE"; break;
	case 0xcf: str[i] = "%CF"; break;
	case 0xd0: str[i] = "%D0"; break;
	case 0xd1: str[i] = "%D1"; break;
	case 0xd2: str[i] = "%D2"; break;
	case 0xd3: str[i] = "%D3"; break;
	case 0xd4: str[i] = "%D4"; break;
	case 0xd5: str[i] = "%D5"; break;
	case 0xd6: str[i] = "%D6"; break;
	case 0xd7: str[i] = "%D7"; break;
	case 0xd8: str[i] = "%D8"; break;
	case 0xd9: str[i] = "%D9"; break;
	case 0xda: str[i] = "%DA"; break;
	case 0xdb: str[i] = "%DB"; break;
	case 0xdc: str[i] = "%DC"; break;
	case 0xdd: str[i] = "%DD"; break;
	case 0xde: str[i] = "%DE"; break;
	case 0xdf: str[i] = "%DF"; break;
	case 0xe0: str[i] = "%E0"; break;
	case 0xe1: str[i] = "%E1"; break;
	case 0xe2: str[i] = "%E2"; break;
	case 0xe3: str[i] = "%E3"; break;
	case 0xe4: str[i] = "%E4"; break;
	case 0xe5: str[i] = "%E5"; break;
	case 0xe6: str[i] = "%E6"; break;
	case 0xe7: str[i] = "%E7"; break;
	case 0xe8: str[i] = "%E8"; break;
	case 0xe9: str[i] = "%E9"; break;
	case 0xea: str[i] = "%EA"; break;
	case 0xeb: str[i] = "%EB"; break;
	case 0xec: str[i] = "%EC"; break;
	case 0xed: str[i] = "%ED"; break;
	case 0xee: str[i] = "%EE"; break;
	case 0xef: str[i] = "%EF"; break;
	case 0xf0: str[i] = "%F0"; break;
	case 0xf1: str[i] = "%F1"; break;
	case 0xf2: str[i] = "%F2"; break;
	case 0xf3: str[i] = "%F3"; break;
	case 0xf4: str[i] = "%F4"; break;
	case 0xf5: str[i] = "%F5"; break;
	case 0xf6: str[i] = "%F6"; break;
	case 0xf7: str[i] = "%F7"; break;
	case 0xf8: str[i] = "%F8"; break;
	case 0xf9: str[i] = "%F9"; break;
	case 0xfa: str[i] = "%FA"; break;
	case 0xfb: str[i] = "%FB"; break;
	case 0xfc: str[i] = "%FC"; break;
	case 0xfd: str[i] = "%FD"; break;
	case 0xfe: str[i] = "%FE"; break;
	case 0xff: str[i] = "%FF"; break;
	}
    }
    return ({ implode(str, "") });
}
