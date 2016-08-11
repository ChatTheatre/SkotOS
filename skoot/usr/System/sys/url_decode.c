/*
 * Copyright Skotos Tech Inc 2003
 */

# include <status.h>

private string
spaces(int num)
{
    string str;
   
    str = "                    ";
    while (strlen(str) < num) {
	str += str;
    }
    return str[.. num-1];
}

static mixed *
dec_1(mixed *tree)
{
    return ({ "", "" });
}

static mixed *
dec_2(mixed *tree)
{
    return ({ ({ }), tree[0] });
}

static mixed *
dec_3(mixed *tree)
{
    return ({ tree, "" });
}
static mixed *
dec_4(mixed *tree)
{
    return ({ tree[..sizeof(tree) - 2], tree[sizeof(tree) - 1] });
}

static mixed *
dec_5(mixed *tree)
{
    return ({ spaces(strlen(tree[0])) });;
}

string url_decode(string text)
{
    int    i, sz, len, size, offset;
    string *chunks;
    mixed *tree;

    len = strlen(text);
    size = status()[ST_ARRAYSIZE];
    if (size > 2048) {
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
"escape = /(%[0-9a-fA-F][0-9a-fA-F])+/\n" +
"plus   = /\\++/\n" +
"other  = /[^+%]+/\n" +
"left   = /%[0-9a-fA-F]?/\n" +
"\n" +
"opt_text:            ? dec_1\n" +
"opt_text:      left  ? dec_2\n" +
"opt_text: text       ? dec_3\n" +
"opt_text: text left  ? dec_4\n" +
"\n" +
"text:     complete\n" +
"text:     chunks complete\n" +
"\n" +
"chunks:   chunk\n" +
"chunks:   chunks chunk\n" +
"\n" +
"chunk: complete\n" +
"chunk: left\n" +
"\n" +
"complete:    plus    ? dec_5\n" +
"complete:    other\n" +
"complete:    escape  ? dehex_bytes\n", chunks[i]);
	chunks[i] = implode(tree[0], "");
	if (strlen(tree[1]) > 0) {
	    if (i < sz - 1) {
		/* Insert before next chunk, see if that works. */
		chunks[i + 1] = tree[1] + chunks[i + 1];
	    } else {
		/* There's no more chunks left. */
		chunks[i] += tree[1];
	    }
	}
    }
    return implode(chunks, "");
}
mixed *
dehex_bytes(mixed *tree)
{
    int i, sz, offset;
    string str, orig;

    orig = tree[0];
    sz = strlen(orig);
    str = spaces(sz / 3);
    for (offset = 0, i = 0; offset < sz; i++, offset += 3) {
	switch (orig[offset..offset + 2]) {
	case "%00": str[i] = '\x00'; break;
	case "%01": str[i] = '\x01'; break;
	case "%02": str[i] = '\x02'; break;
	case "%03": str[i] = '\x03'; break;
	case "%04": str[i] = '\x04'; break;
	case "%05": str[i] = '\x05'; break;
	case "%06": str[i] = '\x06'; break;
	case "%07": str[i] = '\x07'; break;
	case "%08": str[i] = '\x08'; break;
	case "%09": str[i] = '\x09'; break;
	case "%0A":
	case "%0a": str[i] = '\x0A'; break;
	case "%0B":
	case "%0b": str[i] = '\x0B'; break;
	case "%0C":
	case "%0c": str[i] = '\x0C'; break;
	case "%0D":
	case "%0d": str[i] = '\x0D'; break;
	case "%0E":
	case "%0e": str[i] = '\x0E'; break;
	case "%0F":
	case "%0f": str[i] = '\x0F'; break;
	case "%10": str[i] = '\x10'; break;
	case "%11": str[i] = '\x11'; break;
	case "%12": str[i] = '\x12'; break;
	case "%13": str[i] = '\x13'; break;
	case "%14": str[i] = '\x14'; break;
	case "%15": str[i] = '\x15'; break;
	case "%16": str[i] = '\x16'; break;
	case "%17": str[i] = '\x17'; break;
	case "%18": str[i] = '\x18'; break;
	case "%19": str[i] = '\x19'; break;
	case "%1A":
	case "%1a": str[i] = '\x1A'; break;
	case "%1B":
	case "%1b": str[i] = '\x1B'; break;
	case "%1C":
	case "%1c": str[i] = '\x1C'; break;
	case "%1D":
	case "%1d": str[i] = '\x1D'; break;
	case "%1E":
	case "%1e": str[i] = '\x1E'; break;
	case "%1F":
	case "%1f": str[i] = '\x1F'; break;
	case "%20": str[i] = '\x20'; break;
	case "%21": str[i] = '\x21'; break;
	case "%22": str[i] = '\x22'; break;
	case "%23": str[i] = '\x23'; break;
	case "%24": str[i] = '\x24'; break;
	case "%25": str[i] = '\x25'; break;
	case "%26": str[i] = '\x26'; break;
	case "%27": str[i] = '\x27'; break;
	case "%28": str[i] = '\x28'; break;
	case "%29": str[i] = '\x29'; break;
	case "%2A":
	case "%2a": str[i] = '\x2A'; break;
	case "%2B":
	case "%2b": str[i] = '\x2B'; break;
	case "%2C":
	case "%2c": str[i] = '\x2C'; break;
	case "%2D":
	case "%2d": str[i] = '\x2D'; break;
	case "%2E":
	case "%2e": str[i] = '\x2E'; break;
	case "%2F":
	case "%2f": str[i] = '\x2F'; break;
	case "%30": str[i] = '\x30'; break;
	case "%31": str[i] = '\x31'; break;
	case "%32": str[i] = '\x32'; break;
	case "%33": str[i] = '\x33'; break;
	case "%34": str[i] = '\x34'; break;
	case "%35": str[i] = '\x35'; break;
	case "%36": str[i] = '\x36'; break;
	case "%37": str[i] = '\x37'; break;
	case "%38": str[i] = '\x38'; break;
	case "%39": str[i] = '\x39'; break;
	case "%3A":
	case "%3a": str[i] = '\x3A'; break;
	case "%3B":
	case "%3b": str[i] = '\x3B'; break;
	case "%3C":
	case "%3c": str[i] = '\x3C'; break;
	case "%3D":
	case "%3d": str[i] = '\x3D'; break;
	case "%3E":
	case "%3e": str[i] = '\x3E'; break;
	case "%3F":
	case "%3f": str[i] = '\x3F'; break;
	case "%40": str[i] = '\x40'; break;
	case "%41": str[i] = '\x41'; break;
	case "%42": str[i] = '\x42'; break;
	case "%43": str[i] = '\x43'; break;
	case "%44": str[i] = '\x44'; break;
	case "%45": str[i] = '\x45'; break;
	case "%46": str[i] = '\x46'; break;
	case "%47": str[i] = '\x47'; break;
	case "%48": str[i] = '\x48'; break;
	case "%49": str[i] = '\x49'; break;
	case "%4A":
	case "%4a": str[i] = '\x4A'; break;
	case "%4B":
	case "%4b": str[i] = '\x4B'; break;
	case "%4C":
	case "%4c": str[i] = '\x4C'; break;
	case "%4D":
	case "%4d": str[i] = '\x4D'; break;
	case "%4E":
	case "%4e": str[i] = '\x4E'; break;
	case "%4F":
	case "%4f": str[i] = '\x4F'; break;
	case "%50": str[i] = '\x50'; break;
	case "%51": str[i] = '\x51'; break;
	case "%52": str[i] = '\x52'; break;
	case "%53": str[i] = '\x53'; break;
	case "%54": str[i] = '\x54'; break;
	case "%55": str[i] = '\x55'; break;
	case "%56": str[i] = '\x56'; break;
	case "%57": str[i] = '\x57'; break;
	case "%58": str[i] = '\x58'; break;
	case "%59": str[i] = '\x59'; break;
	case "%5A":
	case "%5a": str[i] = '\x5A'; break;
	case "%5B":
	case "%5b": str[i] = '\x5B'; break;
	case "%5C":
	case "%5c": str[i] = '\x5C'; break;
	case "%5D":
	case "%5d": str[i] = '\x5D'; break;
	case "%5E":
	case "%5e": str[i] = '\x5E'; break;
	case "%5F":
	case "%5f": str[i] = '\x5F'; break;
	case "%60": str[i] = '\x60'; break;
	case "%61": str[i] = '\x61'; break;
	case "%62": str[i] = '\x62'; break;
	case "%63": str[i] = '\x63'; break;
	case "%64": str[i] = '\x64'; break;
	case "%65": str[i] = '\x65'; break;
	case "%66": str[i] = '\x66'; break;
	case "%67": str[i] = '\x67'; break;
	case "%68": str[i] = '\x68'; break;
	case "%69": str[i] = '\x69'; break;
	case "%6A":
	case "%6a": str[i] = '\x6A'; break;
	case "%6B":
	case "%6b": str[i] = '\x6B'; break;
	case "%6C":
	case "%6c": str[i] = '\x6C'; break;
	case "%6D":
	case "%6d": str[i] = '\x6D'; break;
	case "%6E":
	case "%6e": str[i] = '\x6E'; break;
	case "%6F":
	case "%6f": str[i] = '\x6F'; break;
	case "%70": str[i] = '\x70'; break;
	case "%71": str[i] = '\x71'; break;
	case "%72": str[i] = '\x72'; break;
	case "%73": str[i] = '\x73'; break;
	case "%74": str[i] = '\x74'; break;
	case "%75": str[i] = '\x75'; break;
	case "%76": str[i] = '\x76'; break;
	case "%77": str[i] = '\x77'; break;
	case "%78": str[i] = '\x78'; break;
	case "%79": str[i] = '\x79'; break;
	case "%7A":
	case "%7a": str[i] = '\x7A'; break;
	case "%7B":
	case "%7b": str[i] = '\x7B'; break;
	case "%7C":
	case "%7c": str[i] = '\x7C'; break;
	case "%7D":
	case "%7d": str[i] = '\x7D'; break;
	case "%7E":
	case "%7e": str[i] = '\x7E'; break;
	case "%7F":
	case "%7f": str[i] = '\x7F'; break;
	case "%80": str[i] = '\x80'; break;
	case "%81": str[i] = '\x81'; break;
	case "%82": str[i] = '\x82'; break;
	case "%83": str[i] = '\x83'; break;
	case "%84": str[i] = '\x84'; break;
	case "%85": str[i] = '\x85'; break;
	case "%86": str[i] = '\x86'; break;
	case "%87": str[i] = '\x87'; break;
	case "%88": str[i] = '\x88'; break;
	case "%89": str[i] = '\x89'; break;
	case "%8A":
	case "%8a": str[i] = '\x8A'; break;
	case "%8B":
	case "%8b": str[i] = '\x8B'; break;
	case "%8C":
	case "%8c": str[i] = '\x8C'; break;
	case "%8D":
	case "%8d": str[i] = '\x8D'; break;
	case "%8E":
	case "%8e": str[i] = '\x8E'; break;
	case "%8F":
	case "%8f": str[i] = '\x8F'; break;
	case "%90": str[i] = '\x90'; break;
	case "%91": str[i] = '\x91'; break;
	case "%92": str[i] = '\x92'; break;
	case "%93": str[i] = '\x93'; break;
	case "%94": str[i] = '\x94'; break;
	case "%95": str[i] = '\x95'; break;
	case "%96": str[i] = '\x96'; break;
	case "%97": str[i] = '\x97'; break;
	case "%98": str[i] = '\x98'; break;
	case "%99": str[i] = '\x99'; break;
	case "%9A":
	case "%9a": str[i] = '\x9A'; break;
	case "%9B":
	case "%9b": str[i] = '\x9B'; break;
	case "%9C":
	case "%9c": str[i] = '\x9C'; break;
	case "%9D":
	case "%9d": str[i] = '\x9D'; break;
	case "%9E":
	case "%9e": str[i] = '\x9E'; break;
	case "%9F":
	case "%9f": str[i] = '\x9F'; break;
	case "%A0":
	case "%a0": str[i] = '\xA0'; break;
	case "%A1":
	case "%a1": str[i] = '\xA1'; break;
	case "%A2":
	case "%a2": str[i] = '\xA2'; break;
	case "%A3":
	case "%a3": str[i] = '\xA3'; break;
	case "%A4":
	case "%a4": str[i] = '\xA4'; break;
	case "%A5":
	case "%a5": str[i] = '\xA5'; break;
	case "%A6":
	case "%a6": str[i] = '\xA6'; break;
	case "%A7":
	case "%a7": str[i] = '\xA7'; break;
	case "%A8":
	case "%a8": str[i] = '\xA8'; break;
	case "%A9":
	case "%a9": str[i] = '\xA9'; break;
	case "%AA":
	case "%Aa":
	case "%aA":
	case "%aa": str[i] = '\xAA'; break;
	case "%AB":
	case "%Ab":
	case "%aB":
	case "%ab": str[i] = '\xAB'; break;
	case "%AC":
	case "%Ac":
	case "%aC":
	case "%ac": str[i] = '\xAC'; break;
	case "%AD":
	case "%Ad":
	case "%aD":
	case "%ad": str[i] = '\xAD'; break;
	case "%AE":
	case "%Ae":
	case "%aE":
	case "%ae": str[i] = '\xAE'; break;
	case "%AF":
	case "%Af":
	case "%aF":
	case "%af": str[i] = '\xAF'; break;
	case "%B0":
	case "%b0": str[i] = '\xB0'; break;
	case "%B1":
	case "%b1": str[i] = '\xB1'; break;
	case "%B2":
	case "%b2": str[i] = '\xB2'; break;
	case "%B3":
	case "%b3": str[i] = '\xB3'; break;
	case "%B4":
	case "%b4": str[i] = '\xB4'; break;
	case "%B5":
	case "%b5": str[i] = '\xB5'; break;
	case "%B6":
	case "%b6": str[i] = '\xB6'; break;
	case "%B7":
	case "%b7": str[i] = '\xB7'; break;
	case "%B8":
	case "%b8": str[i] = '\xB8'; break;
	case "%B9":
	case "%b9": str[i] = '\xB9'; break;
	case "%BA":
	case "%Ba":
	case "%bA":
	case "%ba": str[i] = '\xBA'; break;
	case "%BB":
	case "%Bb":
	case "%bB":
	case "%bb": str[i] = '\xBB'; break;
	case "%BC":
	case "%Bc":
	case "%bC":
	case "%bc": str[i] = '\xBC'; break;
	case "%BD":
	case "%Bd":
	case "%bD":
	case "%bd": str[i] = '\xBD'; break;
	case "%BE":
	case "%Be":
	case "%bE":
	case "%be": str[i] = '\xBE'; break;
	case "%BF":
	case "%Bf":
	case "%bF":
	case "%bf": str[i] = '\xBF'; break;
	case "%C0":
	case "%c0": str[i] = '\xC0'; break;
	case "%C1":
	case "%c1": str[i] = '\xC1'; break;
	case "%C2":
	case "%c2": str[i] = '\xC2'; break;
	case "%C3":
	case "%c3": str[i] = '\xC3'; break;
	case "%C4":
	case "%c4": str[i] = '\xC4'; break;
	case "%C5":
	case "%c5": str[i] = '\xC5'; break;
	case "%C6":
	case "%c6": str[i] = '\xC6'; break;
	case "%C7":
	case "%c7": str[i] = '\xC7'; break;
	case "%C8":
	case "%c8": str[i] = '\xC8'; break;
	case "%C9":
	case "%c9": str[i] = '\xC9'; break;
	case "%CA":
	case "%Ca":
	case "%cA":
	case "%ca": str[i] = '\xCA'; break;
	case "%CB":
	case "%Cb":
	case "%cB":
	case "%cb": str[i] = '\xCB'; break;
	case "%CC":
	case "%Cc":
	case "%cC":
	case "%cc": str[i] = '\xCC'; break;
	case "%CD":
	case "%Cd":
	case "%cD":
	case "%cd": str[i] = '\xCD'; break;
	case "%CE":
	case "%Ce":
	case "%cE":
	case "%ce": str[i] = '\xCE'; break;
	case "%CF":
	case "%Cf":
	case "%cF":
	case "%cf": str[i] = '\xCF'; break;
	case "%D0":
	case "%d0": str[i] = '\xD0'; break;
	case "%D1":
	case "%d1": str[i] = '\xD1'; break;
	case "%D2":
	case "%d2": str[i] = '\xD2'; break;
	case "%D3":
	case "%d3": str[i] = '\xD3'; break;
	case "%D4":
	case "%d4": str[i] = '\xD4'; break;
	case "%D5":
	case "%d5": str[i] = '\xD5'; break;
	case "%D6":
	case "%d6": str[i] = '\xD6'; break;
	case "%D7":
	case "%d7": str[i] = '\xD7'; break;
	case "%D8":
	case "%d8": str[i] = '\xD8'; break;
	case "%D9":
	case "%d9": str[i] = '\xD9'; break;
	case "%DA":
	case "%Da":
	case "%dA":
	case "%da": str[i] = '\xDA'; break;
	case "%DB":
	case "%Db":
	case "%dB":
	case "%db": str[i] = '\xDB'; break;
	case "%DC":
	case "%Dc":
	case "%dC":
	case "%dc": str[i] = '\xDC'; break;
	case "%DD":
	case "%Dd":
	case "%dD":
	case "%dd": str[i] = '\xDD'; break;
	case "%DE":
	case "%De":
	case "%dE":
	case "%de": str[i] = '\xDE'; break;
	case "%DF":
	case "%Df":
	case "%dF":
	case "%df": str[i] = '\xDF'; break;
	case "%E0":
	case "%e0": str[i] = '\xE0'; break;
	case "%E1":
	case "%e1": str[i] = '\xE1'; break;
	case "%E2":
	case "%e2": str[i] = '\xE2'; break;
	case "%E3":
	case "%e3": str[i] = '\xE3'; break;
	case "%E4":
	case "%e4": str[i] = '\xE4'; break;
	case "%E5":
	case "%e5": str[i] = '\xE5'; break;
	case "%E6":
	case "%e6": str[i] = '\xE6'; break;
	case "%E7":
	case "%e7": str[i] = '\xE7'; break;
	case "%E8":
	case "%e8": str[i] = '\xE8'; break;
	case "%E9":
	case "%e9": str[i] = '\xE9'; break;
	case "%EA":
	case "%Ea":
	case "%eA":
	case "%ea": str[i] = '\xEA'; break;
	case "%EB":
	case "%Eb":
	case "%eB":
	case "%eb": str[i] = '\xEB'; break;
	case "%EC":
	case "%Ec":
	case "%eC":
	case "%ec": str[i] = '\xEC'; break;
	case "%ED":
	case "%Ed":
	case "%eD":
	case "%ed": str[i] = '\xED'; break;
	case "%EE":
	case "%Ee":
	case "%eE":
	case "%ee": str[i] = '\xEE'; break;
	case "%EF":
	case "%Ef":
	case "%eF":
	case "%ef": str[i] = '\xEF'; break;
	case "%F0":
	case "%f0": str[i] = '\xf0'; break;
	case "%F1":
	case "%f1": str[i] = '\xf1'; break;
	case "%F2":
	case "%f2": str[i] = '\xf2'; break;
	case "%F3":
	case "%f3": str[i] = '\xf3'; break;
	case "%F4":
	case "%f4": str[i] = '\xf4'; break;
	case "%F5":
	case "%f5": str[i] = '\xf5'; break;
	case "%F6":
	case "%f6": str[i] = '\xf6'; break;
	case "%F7":
	case "%f7": str[i] = '\xf7'; break;
	case "%F8":
	case "%f8": str[i] = '\xf8'; break;
	case "%F9":
	case "%f9": str[i] = '\xf9'; break;
	case "%FA":
	case "%Fa":
	case "%fA":
	case "%fa": str[i] = '\xFA'; break;
	case "%FB":
	case "%Fb":
	case "%fB":
	case "%fb": str[i] = '\xFB'; break;
	case "%FC":
	case "%Fc":
	case "%fC":
	case "%fc": str[i] = '\xFC'; break;
	case "%FD":
	case "%Fd":
	case "%fD":
	case "%fd": str[i] = '\xFD'; break;
	case "%FE":
	case "%Fe":
	case "%fE":
	case "%fe": str[i] = '\xFE'; break;
	case "%FF":
	case "%Ff":
	case "%fF":
	case "%ff": str[i] = '\xFF'; break;
	}
    }
    return ({ str });
}
