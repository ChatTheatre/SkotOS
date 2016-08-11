/*
 * ~UserDB/sys/scratch.c
 *
 * This object is intended to stores some data associated with a key and
 * marked by a timestamp each time it's updated.  The timestamp will be used
 * by an hourly cleanup call_out to kick out any values that can be considered
 * obsolete.
 *
 * Initially used to store data during the account-creation process but can
 * be used for other purposes as well of course.
 *
 * Datastructure layout:
 *
 * ([ <st> : ([
 *     <ring>: ([
 *         0: <timestamp>,
 *         1: <delay>,
 *         ...other data... ])
 *     ])
 * ])
 *
 * Copyright Skotos Tech Inc 2002
 */

# define PURGE_DELAY 3600

# define S_STAMP 0
# define S_DELAY 1

# define SSS     "!hiccup!"

inherit "/lib/string";

private mapping scratch;

static void
create()
{
    scratch = ([ ]);
    call_out("do_purge", PURGE_DELAY);
}


static void
do_purge()
{
    int     i, sz_i;
    string  *str1;
    mapping *map1;

    call_out("do_purge", PURGE_DELAY);

    sz_i = map_sizeof(scratch);
    str1 = map_indices(scratch);
    map1 = map_values(scratch);

    for (i = 0; i < sz_i; i++) {
	int     j, sz_j;
	string  *str2;
	mapping *map2;

	sz_j = map_sizeof(map1[i]);
	str2 = map_indices(map1[i]);
	map2 = map_values(map1[i]);

	for (j = 0; j < sz_j; j++) {
	    if (map2[j][S_STAMP] + map2[j][S_DELAY] < time()) {
		map1[i][str2[j]] = nil;
	    }
	}
	if (map_sizeof(map1[i]) == 0) {
	    scratch[str1[i]] = nil;
	}
    }
}

/*
 * Function:    create_entry()
 * Description: Find a unique id to be used for storing/fetching values, to
 *              expire after the entry hasn't been 'touched' (by querying or
 *              storing data) for 'delay' seconds.
 */

atomic string
create_entry(int delay)
{
    mixed   *data;
    string  id, str1, str2;
    mapping map, nap;

    data = millitime();
    id = to_hex(hash_md5(data[0] + SSS + data[1]));
    str1 = id[..1];
    str2 = id[2..];
    while (scratch[str1] && scratch[str1][str2]) {
	data[0]++;
	id = to_hex(hash_md5(data[0] + SSS + data[1]));
	str1 = id[..1];
	str2 = id[2..];
    }
    map = scratch[str1];
    if (!map) {
	scratch[str1] = map = ([ ]);
    }
    nap = map[str2];
    if (!nap) {
	map[str2] = nap = ([ ]);
    }
    nap[S_DELAY] = delay;
    nap[S_STAMP] = time();

    return id;
}

/*
 * Function:    purge_entry()
 * Description: Remove the entry, if it hasn't already expired all by itself
 *              anyway.  Returns TRUE if an entry was removed, FALSE otherwise.
 */

atomic int
purge_entry(string id)
{
    string str1, str2;
    mapping map;

    str1 = id[..1];
    str2 = id[2..];

    map = scratch[str1];
    if (map) {
	if (map[str2]) {
	    map[str2] = nil;
	    if (map_sizeof(map) == 0) {
		scratch[str1] = nil;
	    }
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * Function:    store_data()
 * Description: Returns TRUE if the value for the key in the scratch-entry
 *              could be updated, and FALSE if there was no such entry.
 */

atomic int
store_data(string id, string key, string value)
{
    string str1, str2;
    mapping map;

    str1 = id[..1];
    str2 = id[2..];
    map = scratch[str1];
    if (map) {
	mapping nap;

	nap = map[str2];
	if (nap) {
	    nap[key] = value;
	    nap[S_STAMP] = time();
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * Function:    fetch_data()
 * Description: Returns 0 if the scratch-entry doesn't exist (anymore) or
 *              the relevant value for the key (which can be nil) otherwise.
 */

atomic mixed
fetch_data(string id, string key)
{
    string  str1, str2;
    mapping map;

    str1 = id[..1];
    str2 = id[2..];
    map = scratch[str1];
    if (map) {
	mapping nap;

	nap = map[str2];
	if (nap) {
	    nap[S_STAMP] = time();
	    return nap[key];
	}
    }
    return FALSE;
}

mapping
dump_scratch_mapping()
{
    return scratch;
}
