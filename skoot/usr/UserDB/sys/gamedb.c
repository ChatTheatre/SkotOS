/*
 * ~UserDB/sys/gamedb.c
 *
 * Keep track of what games are officially open, where they are running
 * and what name we've given them.
 *
 * Copyright 2001 Skotos Tech Inc
 */

# define GBN_HOST    0
# define GBN_BASE    1
# define GBN_THEATRE 2
# define GBN_DESC    3

private inherit "/lib/string";

/*
 * ([ <name>: ({ <host>, <base>, <theatre>, <description> }),
 *    ... ])
 */
private mapping games_by_name;

/*
 * ([ <host>: ([ <base>: ([ <theatre>: <name>, ... ]),
 *               ... ]),
 *    ... ])
 */
private mapping games_by_host;

private mapping distribution;

/*
GAME: The Eternal City
   CATEGORY: Brand Royalties, 10%
     SUBCATEGORY: Retained to repay advance, 1
   CATEGORY: Administration Royalties, 10%
     SUBCATEGORY: Worlds Apart Productions, 1

GAME: Monsters! Monsters!: Grendel's Revenge
   CATEGORY: Brand Royalties, 10%
     SUBCATEGORY: Retained to repay advance, 1
   CATEGORY: Administration Royalties, 10%
     SUBCATEGORY: Worlds Apart Productions, 1

GAME: Castle Marrach
   CATEGORY: Brand Royalties, 10%
     SUBCATEGORY: Retained by Skotos Tech, 1
   CATEGORY: Administration Royalties, 10%
     SUBCATEGORY: Staci Dumoski, 1

GAME: Welcome Room
   CATEGORY: Brand Royalties, 10%
     SUBCATEGORY: Retained by Skotos Tech, 1
   CATEGORY: Administration Royalties, 10%
     SUBCATEGORY: Retained by Skotos Tech, 1

GAME: Galactic Emperor: Hegemony
   CATEGORY: Brand Royalties, 0%
     SUBCATEGORY: Retained by Skotos Tech, 1
   CATEGORY: Administration Royalties, 0%
     SUBCATEGORY: Retained by Skotos Tech, 1

*/

/*
GAME: Galactic Emperor: Succession
   CATEGORY: Brand Royalties, 0%
     SUBCATEGORY: Retained by Skotos Tech, 1
   CATEGORY: Administration Royalties, 0%
     SUBCATEGORY: Retained by Skotos Tech, 1
 */

static void create() {
    games_by_name = ([ ]);
    games_by_host = ([ ]);
    distribution  = ([ ]);
}

/*
 * Prototypes:
 */

atomic void remove_game(string game);

atomic void
add_game(string name, string host, int base, string theatre, string desc)
{
    mapping h_map, b_map;

    if (games_by_name[name]) {
	remove_game(name);
    }
    h_map = games_by_host[host];
    if (!h_map) {
	games_by_host[host] = h_map = ([ ]);
    }
    b_map = h_map[base];
    if (!b_map) {
	h_map[base] = b_map = ([ ]);
    }
    b_map[theatre] = name;
    games_by_name[name] = ({ host, base, theatre, desc });
}

atomic void remove_game(string name) {
    mixed *old;

    if (old = games_by_name[name]) {
	mapping h_map, b_map;

	h_map = games_by_host[old[GBN_HOST]];
	if (h_map) {
	    b_map = h_map[old[GBN_BASE]];
	    if (b_map) {
		b_map[old[GBN_THEATRE]] = nil;
		if (map_sizeof(b_map) == 0) {
		    h_map[old[GBN_BASE]] = nil;
		    if (map_sizeof(h_map) == 0) {
			games_by_host[old[GBN_HOST]] = nil;
		    }
		}
	    }
	}
	games_by_name[name] = nil;
    } else {
	error("No such game: " + name);
    }
}

string
dump_distribution()
{
    int     i, sz;
    string  *games, result;
    mapping *maps;

    result = "Royalties distribution:\n";
    sz     = map_sizeof(distribution);
    games  = map_indices(distribution);
    maps   = map_values(distribution);

    for (i = 0; i < sz; i++) {
	int    t;
	string *types;
        result += "\nGAME: " + games_by_name[games[i]][GBN_DESC] + "\n";

	types = ({ "BASIC", "PREMIUM" });
	for (t = 0; t < sizeof(types); t++) {
	    int     j, sz_j;
	    string  *cats, type;
	    mixed   *cat_data;

	    type = types[t];
	    result += "    " + type + ":\n";
	    type = lower_case(type);
	    sz_j = map_sizeof(maps[i][type]);
	    cats = map_indices(maps[i][type]);
	    cat_data = map_values(maps[i][type]);
	    for (j = 0; j < sz_j; j++) {
		int    k, sz_k;
		string *subcats;
		int  *perc;

		result += "       CATEGORY: " + cats[j] + ", " + cat_data[j][0] + "%\n";
		sz_k    = map_sizeof(cat_data[j][1]);
		subcats = map_indices(cat_data[j][1]);
		perc    = map_values(cat_data[j][1]);
		for (k = 0; k < sz_k; k++) {
		    result += "         SUBCATEGORY: " + subcats[k] + ", " + perc[k] + "\n";
		}
	    }
	}
    }
    return result;
}

atomic void patch() {
    create();
#if 1
    distribution = ([
        "tec": ([
	    "basic": ([
	        "Brand Royalties": ({
		    10, ([ "Retained to repay advance": 100 ])	}),
		"Administration Royalties": ({
		    10, ([ "Worlds Apart Productions": 100 ]) }) ]),
	    "premium": ([
		"Brand Royalties": ({
		    10, ([ "Retained to repay advance": 100 ])	}),
		"Administration Royalties": ({
		    25, ([ "Worlds Apart Productions": 100 ]) }) ]) ]),
	"mm:gr":  ([
	    "basic": ([
		"Brand Royalties": ({
		    10, ([ "Retained to repay advance": 100 ]) }),
		"Administration Royalties": ({
		    10, ([ "Worlds Apart Productions": 100 ]) }) ]),
	    "premium": ([
	        "Brand Royalties": ({
		    10, ([ "Retained to repay advance": 100 ]) }),
		"Administration Royalties": ({
		    25, ([ "Worlds Apart Productions": 100 ]) }) ]) ]),
	"marrach": ([
	    "basic": ([
	        "Brand Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }),
		"Administration Royalties": ({
		  10, ([ "John Zervos": 100 ]) }) ]),
	    "premium": ([
	        "Brand Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }),
		"Administration Royalties": ({
		  10, ([ "John Zervos": 100 ]) }) ]) ]), 
	"welcome": ([
	    "basic": ([
	        "Brand Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }),
		"Administration Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }) ]),
	    "premium": ([
		"Brand Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }),
		"Administration Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }) ]) ]),
	"ge:h": ([
	    "basic": ([
		"Brand Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }),
		"Administration Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }) ]),
	    "premium": ([
		"Brand Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }),
		"Administration Royalties": ({
		  10, ([ "Retained by Skotos Tech":  100 ]) }) ]) ]) ]);
#else
    distribution =
      ([
	"tec":     ([
	    "Brand Royalties": ({
		0.1, ([ "Retained to repay advance": 1.0 ])
	    }),
	    "Administration Royalties": ({
		0.1, ([ "Worlds Apart Productions": 1.0 ])
	    })
	]),
	"mm:gr":     ([
	    "Brand Royalties": ({
		0.1, ([ "Retained to repay advance": 1.0 ])
	    }),
	    "Administration Royalties": ({
		0.1, ([ "Worlds Apart Productions": 1.0 ])
	    })
	]),
	"marrach": ([
	    "Brand Royalties": ({
		0.1, ([ "Retained by Skotos Tech":  1.0 ]) }),
	    "Administration Royalties": ({
		0.1, ([ "John Zervos": 1.0 ])
	     })
	]), 
	"welcome": ([
	    "Brand Royalties": ({
		0.1, ([ "Retained by Skotos Tech":  1.0 ]) }),
	    "Administration Royalties": ({
		0.1, ([ "Retained by Skotos Tech":  1.0 ]) })
	]),
	"ge:h": ([
	    "Brand Royalties": ({
		0.1, ([ "Retained by Skotos Tech":  1.0 ]) }),
	    "Administration Royalties": ({
		0.1, ([ "Retained by Skotos Tech":  1.0 ]) })
	]),
      ]);
#endif
    add_game("marrach",
	     "157.22.237.17", 8000, nil,
	     "Castle Marrach");
    add_game("tec",
	     "131.161.60.7",  6730, nil,
	     "The Eternal City");
    add_game("mm:gr",
	     "131.161.60.7",  2022, nil,
	     "Monsters! Monsters!: Grendel's Revenge");
    add_game("welcome",
	     "157.22.237.19", 7100, "LinkZone",
	     "Welcome Room");
    add_game("ge:h",
	     "157.22.237.19", 7100, "Hegemony",
	     "Galactic Emperor: Hegemony");
    add_game("underlight",
	     "198.31.229.52", 2000, nil,
	     "Underlight");
}

string *query_games(varargs int royalties) {
    if (royalties) {
        return map_indices(games_by_name) & map_indices(distribution);
    } else {
        return map_indices(games_by_name);
    }
}

mixed *
query_game_by_name(string name)
{
    if (games_by_name[name]) {
	return ({ name }) + games_by_name[name];
    }
    return nil;
}

string
query_game_by_host(string host, int base, varargs string theatre)
{
    switch (host) {
    case "198.232.133.4":
    case "198.232.133.145":
	/* tomtefar.skotos.net */
	host = "131.161.60.5";
	break;
    case "198.232.133.5":
	/* korp.skotos.net */
	host = "131.161.60.6";
	break;
    case "198.232.133.6":
	/* tec.skotos.net */
	host = "131.161.60.7";
	break;
    case "198.232.133.7":
	/* heimdall.skotos.net */
	host = "131.161.60.8";
	break;
    case "198.232.133.8":
	/* vildvittra.skotos.net */
	host = "131.161.60.19";
	break;
    case "198.232.133.33":
    case "198.232.133.146":
	/* marrach.skotos.net */
	host = "157.22.237.17";
	break;
    case "198.232.133.34":
    case "198.232.133.147":
	/* skotos-seven.skotos.net */
	host = "157.22.237.20";
	break;
    case "198.232.133.35":
    case "198.232.133.148":
	/* welcome.skotos.net */
	host = "157.22.237.19";
	break;
    case "198.232.133.49":
	/* tec.skotos.net */
	host = "131.161.60.7";
	break;
    case "198.232.133.50":
	/* grendelsrevenge.skotos.net */
	host = "131.161.60.7";
	break;
    case "198.232.133.138":
	/* troll.skotos.net */
	host = "131.161.60.10";
	break;
    }
    if (games_by_host[host] &&
	games_by_host[host][base] &&
	games_by_host[host][base][theatre]) {
	return games_by_host[host][base][theatre];
    }
    return nil;
}

string *
query_distr_cats(string game, string type)
{
    return distribution[game] ? map_indices(distribution[game][type]) : ({ });
}

string *
query_distr_subcats(string game, string type, string cat)
{
    return
	(distribution[game] && distribution[game][type][cat]) ?
	map_indices(distribution[game][type][cat][1]) : ({ });
}

int
query_distr_cat_perc(string game, string type, string cat)
{
    return
	(distribution[game] && distribution[game][type][cat]) ?
	distribution[game][type][cat][0] : 0;
}

int
query_distr_subcat_perc(string game, string type, string cat, string subcat)
{
    return
	(distribution[game] && distribution[game][type][cat] && distribution[game][type][cat][1][subcat]) ?
	(distribution[game][type][cat][0] * distribution[game][type][cat][1][subcat] / 100) : 0;
}
