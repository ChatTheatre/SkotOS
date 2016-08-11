/*
 * Generic profiler.
 *
 * Copyright Skotos Tech Inc 2004
 */

private inherit "/lib/asn";
private inherit "/lib/string";

mapping data;
mapping stamps;
mapping archives;

static void create()
{
   int t;

   data = ([
       "running": ([ ]),
       "hourly":  ([ ]),
       "daily":   ([ ]),
   ]);
   t = time();
   stamps = ([
       "hourly": t - t % 3600,
       "daily":  t - t % 86400
   ]);
   archives = ([
       "hourly": ({ }),
       "daily":  ({ }),
   ]);
}

void patch()
{
   create();
}

private void
add_to_map(mapping map, int t, string verb, int cost, varargs float diff)
{
   if (!map[verb]) {
      map[verb] = ({ 0, "\0", 0, 0, 0.0, 0.0, 0 });
   }
   map[verb][0]++;
   map[verb][1] = add_int64_int32(map[verb][1], cost);
   if (cost > map[verb][2]) {
      map[verb][2] = cost;
      map[verb][3] = t;
   }
   map[verb][4] += diff;
   if (diff > map[verb][5]) {
      map[verb][5] = diff;
      map[verb][6] = t;
   }
}

mapping query_data(string type)
{
   return data[type];
}

mixed dump_profile(string type, int threshold)
{
   int i, sz, reset;
   string *verbs;
   mapping map;
   mixed *values;
   string **output;
   string today, today_date, today_year;

   today = ctime(time());
   today_date = today[.. 9];
   today_year = today[20 ..];

   if (type == "running") {
      int i, sz_i;
      mapping *maps;

      map = ([ ]);
      sz_i = map_sizeof(data["running"]);
      maps = map_values(data["running"]);
      for (i = 0; i < sz_i; i++) {
	 int j, sz_j;
	 string *verbs;
	 mixed **totals;

	 sz_j   = map_sizeof(maps[i]);
	 verbs  = map_indices(maps[i]);
	 totals = map_values(maps[i]);
	 for (j = 0; j < sz_j; j++) {
	    if (!map[verbs[j]]) {
	       map[verbs[j]] = ({ 0, "\0", 0, 0, 0.0, 0.0, 0 });
	    }
	    map[verbs[j]][0] += totals[j][0];
	    map[verbs[j]][1] = add_int64_int64(map[verbs[j]][1], totals[j][1]);
	    if (totals[j][2] > map[verbs[j]][2]) {
	       map[verbs[j]][2] = totals[j][2];
	       map[verbs[j]][3] = totals[j][3];
	    }
	    map[verbs[j]][4] += totals[j][4];
	    if (totals[j][5] > map[verbs[j]][5]) {
	       map[verbs[j]][5] = totals[j][5];
	       map[verbs[j]][6] = totals[j][6];
	    }
	 }
      }
      reset = time() - 300;
   } else {
      map = data[type];
      reset = stamps[type];
   }
   output = ({
      ({ "Verb"   }),
      ({ "#"      }),
      ({ "#/min"  }),
      ({ "T. Avg" }),
      ({ "Max"    }),
      ({ "When"   }),
      ({ "D. Avg" }),
      ({ "D. Max" }),
      ({ "When"   }),
   });
   sz     = map_sizeof(map);
   verbs  = map_indices(map);
   values = map_values(map);
   for (i = 0; i < sz; i++) {
      int avg;

      avg = div_int64_int32(values[i][1], values[i][0]);
      if (avg >= threshold) {
	 string stamp;

	 stamp = ctime(values[i][3]);
	 if (stamp[20 ..] == today_year) {
	    stamp = stamp[.. 18];
	    if (stamp[.. 9] == today_date) {
	       stamp = stamp[11 ..];
	    }
	 }
	 output[0] += ({ verbs[i] });
	 output[1] += ({ pad_left((string)values[i][0], 4) });
	 output[2] += ({ pad_left(format_float(60.0 * (float)values[i][0] / (float)(time() - reset), 1), 5) });
	 output[3] += ({ pad_left(comma(avg), 10) });
	 output[4] += ({ pad_left(comma(values[i][2]), 10) });
	 output[5] += ({ stamp });
	 output[6] += ({ pad_left(format_float(values[i][4] / (float)values[i][0], 3), 6) });
	 output[7] += ({ pad_left(format_float(values[i][5], 3), 6) });

	 stamp = ctime(values[i][6]);
	 if (stamp[20 ..] == today_year) {
	    stamp = stamp[.. 18];
	    if (stamp[.. 9] == today_date) {
	       stamp = stamp[11 ..];
	    }
	 }
	 output[8] += ({ stamp });
      }
   }
   return tabulate(output...);
}

void add_entry(string verb, int cost, varargs mixed *mbefore, mixed *mafter)
{
   int t;
   mapping map;
   float diff;

   diff = (float)(mafter[0] - mbefore[0]) + (mafter[1] - mbefore[1]);

   /*
    * Update 'running' average data.
    */
   t = time();
   data["running"] = data["running"][t - 300 ..];
   map = data["running"][t];
   if (!map) {
      data["running"][t] = map = ([ ]);
   }
   add_to_map(map, t, verb, cost, diff);

   if (t >= stamps["hourly"] + 3600) {
      /* Time to flush the hourly stats. */
      data["hourly"] = ([ ]);
      stamps["hourly"] = t - t % 3600;
   }
   add_to_map(data["hourly"], t, verb, cost, diff);

   if (t >= stamps["daily"] + 86400) {
      /* Time to flush the daily stats. */
      data["daily"] = ([ ]);
      stamps["daily"] = t - t % 86400;
   }
   add_to_map(data["daily"], t, verb, cost, diff);
}
