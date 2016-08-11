/*
 * ~UserDB/sys/promotions.c
 *
 * Copyright Skotos Tech Inc 2002.
 */

/*
  - Generate unique and not too easily guessable codes for a (new) promotion.
  - Know for each promotion what the effects are.  Have ability to set what
    or each promotion are.
  - Know for each code what promotion it is part of, when it has been issued,
    whether it has been used (and when), and for what account.  Codes probably
    should come with some sort of expire date.  Should we keep the codes
    around after they've expired or keep them around for statistical purposes?
  - Make sure that a code can only be used once.
  - Keep a 'history' of when promotional codes are generated, new promotions
    are defined, codes used, etc.
  - When generating codes, use these characters:
    23456789ABCDEFGHJKLMNPQRSTUVWXYZ

  NOTES TO SELF:
  - Put a tripple index in codes, b/c we will likely end up with a shitload of
    them for whatever promotions we plan to participate in.
*/

# define DEBUGGING
# define MD5_CONSTANT "#Skotos Promotions Stuff#"
# define PROMO_DIR    "/usr/UserDB/data/promotions/"

private mapping promos;
private mapping codes;

static void
create()
{
    promos = ([ ]);
    codes  = ([ ]);
}

void
patch()
{
    promos["Verizon 2002 Trial"]["effects"]    = ([
        "free_months":    2,
	"initial_status": "trial"
	]);
    promos["Skotos Player Manuals"]["effects"] = ([
        "free_months":    2,
	"initial_status": "regular"
	]);
}

int
add_promotion(string id, mapping fx)
{
    if (promos[id]) {
	return FALSE;
    }
    promos[id] = ([
		   "effects": fx,
		   "active":  FALSE,
		   "created": time(),
		   "counter": 0,
		   "total":   0,
		   "used":    0,
		   "batches": ([ ]),
		   ]);
    return TRUE;
}

int
change_promotion(string id, mapping fx)
{
    if (!promos[id]) {
	return FALSE;
    }
    promos[id]["effects"] = fx;
    return TRUE;
}

int
disable_promotion(string id)
{
    if (!promos[id]) {
	return FALSE;
    }
    promos[id]["active"] = FALSE;
}

int
enable_promotion(string id)
{
    if (!promos[id]) {
	return FALSE;
    }
    promos[id]["active"] = TRUE;
}

/*
 * Dealing with generating/storing/checking codes.
 */

private mapping
find_code(string code)
{
    string code0, code1, code2;

    if (strlen(code) != 12) {
	return nil;
    }
    code0 = code[0 .. 1];
    code1 = code[2 .. 3];
    code2 = code[4 ..];

    return (codes[code0] && codes[code0][code1]) ? codes[code0][code1][code2] : nil;
}

private void
store_code(string code, mapping data)
{
    string  code0, code1, code2;
    mapping map0, map1;

    if (strlen(code) != 12) {
	return;
    }
    code0 = code[0 .. 1];
    code1 = code[2 .. 3];
    code2 = code[4 ..];

    map0 = codes[code0];
    if (!map0) {
	map0 = codes[code0] = ([ ]);
    }
    map1 = map0[code1];
    if (!map1) {
	map1 = map0[code1] = ([ ]);
    }
    map1[code2] = data;
}

/* private */ string
generate_code(string id, int expires)
{
    int     counter;
    string  code, set;
    mapping data;

    data = promos[id];

    set = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";
    do {
	int i;

	code = hash_md5(id + MD5_CONSTANT + data["counter"]);
	for (i = 0; i < 12; i++) {
	    code[i] = set[code[i] & 0x1F];
	}
	code = code[..11];
	data["counter"]++;
    } while (find_code(code));
    data["total"]++;
    store_code(code, ([ "created": time(), "promo": id,	"expires": expires ]));
    return code;
}

string **
generate_codes(string id, int total, int expires)
{
    int    i, sub, stamp;
    string **list;
    string filename;

    filename = PROMO_DIR + time() + ".batch";
    stamp = time();
    write_file(filename,
	       "Generated at " + ctime(stamp) + " for '" + id + "':\n\n");
    /* XXX: Make sure this filename is unique. */
    if (!promos[id]) {
	return nil;
    }
    sub = (total + 999) / 1000;
    list = allocate(sub);
    for (i = 0; i < sub; i++) {
	list[i] = allocate(1000);
    }
    for (i = 0; i < total; i++) {
	string code;

	list[i / 1000][i % 1000] = code = generate_code(id, expires);
	write_file(filename, code[.. 3] + "-" + code[4 .. 7] + "-" + code[8 ..] + "\n");
    }
    list[sub - 1] -= ({ nil });
    promos[id]["batches"][stamp] = total;
    return list;
}

mapping
query_batches()
{
    int   i, sz;
    mixed **dir;
    mapping result;

    dir = get_dir(PROMO_DIR + "*.batch");
    sz = sizeof(dir[0]);
    result = ([ ]);
    for (i = 0; i < sz; i++) {
	int stamp;

	if (sscanf(dir[0][i], "%d.batch", stamp) && stamp > 0 && dir[1][i] > 0) {
	    string text;

	    text = read_file(PROMO_DIR + dir[0][i]);
	    sscanf(text, "%s\n", text);
	    result[stamp] = text;
	}
    }
    return result;
}

string
query_batch(int stamp)
{
    return read_file(PROMO_DIR + stamp + ".batch");
}

int
valid_code(string code)
{
    mapping code_data, promo_data;

    code = implode(explode(code, " ") - ({ "" }), "");
    code = implode(explode(code, "-") - ({ "" }), "");
    code_data = find_code(code);
    if (!code_data) {
	return FALSE;
    }
    promo_data = promos[code_data["promo"]];
    if (!promo_data) {
	return FALSE;
    }
    if (!promo_data["active"]) {
	/*
	 * The relevant promotion needs to be in effect still.
	 */
	return FALSE;
    }
    if (code_data["used"]) {
	/*
	 * Unless we change the system a code can only be used
	 * once.
	 */
	return FALSE;
    }
    if (code_data["expires"] > 0 && time() >= code_data["expires"]) {
	/*
	 * The code needs to not have expired.
	 */
	return FALSE;
    }
    return TRUE;
}

int
use_code(string code)
{
}

string *
query_promotions()
{
    return map_indices(promos);
}

mapping
query_promotion(string id)
{
    return promos[id] ? promos[id][..] : nil;
}

void
dump_status()
{
    int     i, sz_i,
	    sum_i, sum_j, sum_k,
	    total_j, total_k,
	    used, expired;
    string  *str;
    mapping *data, *data_i, found;

    sz_i = map_sizeof(promos);
    str  = map_indices(promos);
    data = map_values(promos);

    DEBUG("Promotions (" + sz_i + "):");
    for (i = 0; i < sz_i; i++) {
	DEBUG("* " + str[i] + ": " + dump_value(data[i]));
    }

    sz_i   = map_sizeof(codes);
    data_i = map_values(codes);

    sum_i   = 0;
    sum_j   = 0;
    used    = 0;
    expired = 0;
    found   = ([ ]);

    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	mapping *data_j;

	sz_j   = map_sizeof(data_i[i]);
	data_j = map_values(data_i[i]);
	for (j = 0; j < sz_j; j++) {
	    int k, sz_k;
	    mapping *data_k;

	    sz_k   = map_sizeof(data_j[j]);
	    data_k = map_values(data_j[j]);
	    for (k = 0; k < sz_k; k++) {
		string id;
		mapping map;

		map = data_k[k];
		id = map["promo"];
		if (!found[id]) {
		    found[id] = 0;
		}
		found[id]++;
		if (map["expires"] && time() > map["expires"]) {
		    expired++;
		}
		if (map["used"]) {
		    used++;
		}
	    }
	    total_k += sz_k;
	}
	total_j += sz_j;
    }

    DEBUG("Codes:");
    DEBUG("* " + sz_i + " level-0 mappings");
    DEBUG("* " + total_j + " level-1 mappings (avg: " + (float)total_j / (float)sz_i + " per level-0 mapping)");
    DEBUG("* " + total_k + " level-2 mappings (avg: " + (float)total_k / (float)total_j + " per level-1 mapping)");
    DEBUG("* Promos found: " + dump_value(found));
    DEBUG("* Expired: " + expired);
    DEBUG("* Used: " + used);
}
