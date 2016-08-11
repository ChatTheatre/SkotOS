/*
 * Profiler.  Keeps track of the (Merry) scripts that have run in the past 24
 * hours.
 *
 * Copyright Skotos Tech Inc 2003, 2004
 */

# include <type.h>

# define TRESHOLD 50000
# define HISTORY_LIMIT 50

# define P_COUNT 0
# define P_TICKS 1
# define P_MAX   2
# define P_STAMP 3

private inherit "/lib/asn";
private inherit "/lib/string";

inherit "/usr/SkotOS/lib/merryapi";

mapping profiles;
mapping history;
mapping stamps;

static void create()
{
   int t;

   t = time();

   profiles = ([ "hourly": ([ ]),        "daily": ([ ]) ]);
   history  = ([ "hourly": ({ }),        "daily": ({ }) ]);
   stamps   = ([ "hourly": t - t % 3600, "daily": t - t % 86400 ]);

   set_object_name("SkotOS:Profiler");
}

void patch()
{
   create();
}

void add_to_profile(mapping profile, object ob, string script, string label, int cost)
{
    int stamp;
    string name, mode, signal;
    mixed *data;

    if (cost < TRESHOLD) {
       return;
    }
    if (sscanf(script, "merry:%s:%s", mode, signal) == 2) {
       object code;

       code = find_merry_location(ob, signal, mode);
       if (code) {
	  ob = code;
       }
    }
    name = ob ? name(ob) : "?";
    if (!profile[name]) {
       profile[name] = ([ ]);
    }
    if (!profile[name][script]) {
       profile[name][script] = ([ ]);
    }
    data = profile[name][script][label];
    if (!data) {
       profile[name][script][label] = data = ({ 0, "\0", 0, 0 });
    }
    data[P_COUNT]++;
    data[P_TICKS] = add_int64_int32(data[P_TICKS], cost);
    if (cost > data[P_MAX]) {
       data[P_MAX]   = cost;
       data[P_STAMP] = time();
    }
}

string dump_profile(string type, varargs int cost, string name)
{
   int    i, sz_i;
   string *names, **output;
   mapping *n_data;
   string today, today_date, today_year;
   mapping profile;
   int profile_reset;

   profile = profiles[type];
   profile_reset = stamps[type];

   today = ctime(time());
   today_date = today[.. 9];
   today_year = today[20 ..];
   output = ({
      ({ "Code"   }),
      ({ "Script" }),
      ({ "Label"  }),
      ({ "#"      }),
      ({ "#/min"  }),
      ({ "Avg"    }),
      ({ "Max"    }),
      ({ "When"   }),
   });
   if (name) {
      if (profile[name]) {
	 sz_i = 1;
	 names = ({ name });
	 n_data = ({ profile[name] });
      } else {
	 return nil;
      }
   } else {
      sz_i = map_sizeof(profile);
      names = map_indices(profile);
      n_data = map_values(profile);
   }
   for (i = 0; i < sz_i; i++) {
      int j, sz_j, n_sum;
      string *scripts, n_label;
      mapping *s_data;

      n_sum = 0;
      n_label = names[i];
      sz_j = map_sizeof(n_data[i]);
      scripts = map_indices(n_data[i]);
      s_data = map_values(n_data[i]);
      for (j = 0; j < sz_j; j++) {
	 int k, sz_k, s_sum;
	 string *labels, s_label;
	 mixed **l_data;

	 s_sum = 0;
	 s_label = scripts[j];
	 sz_k = map_sizeof(s_data[j]);
	 labels = map_indices(s_data[j]);
	 l_data = map_values(s_data[j]);
	 for (k = 0; k < sz_k; k++) {
	    int avg;

	    avg = div_int64_int32(l_data[k][P_TICKS], l_data[k][P_COUNT]);
	    if (!cost || avg >= cost) {
	       int l, sz_l, max;
	       string stamp;

	       stamp = ctime(l_data[k][P_STAMP]);
	       if (stamp[20 ..] == today_year) {
		  stamp = stamp[.. 18];
		  if (stamp[.. 9] == today_date) {
		     stamp = stamp[11 ..];
		  }
	       }
	       output[0] += ({ n_label }); n_label = "";
	       output[1] += ({ s_label }); s_label = "";
	       output[2] += ({ labels[k] ? labels[k] : "N/A" });
	       output[3] += ({ pad_left((string)l_data[k][P_COUNT], 4) });
	       output[4] += ({ pad_left(format_float(60.0 * (float)l_data[k][P_COUNT] / (float)(time() - profile_reset), 1), 5) });
	       output[5] += ({ pad_left(comma(avg), 10) });
	       output[6] += ({ pad_left(comma(l_data[k][P_MAX]), 10) });
	       output[7] += ({ stamp });
	    }
	 }
      }
   }
   return
      "Last profile reset at " + ctime(profile_reset) + " (" + (time() - profile_reset) + " seconds ago).\n\n" +
      tabulate(output...);
}

void add_tick_cost(object ob, string script, string label, int cost)
{
  catch {
    int t;

    if (!stamps) {
       patch();
    }
    t = time();
    if (t >= stamps["hourly"]  + 3600) {
       history["hourly"] = ({ time(), dump_profile("hourly") }) + history["hourly"];
       if (sizeof(history["hourly"]) > HISTORY_LIMIT * 2) {
	  history["hourly"] = history["hourly"][.. HISTORY_LIMIT * 2 - 1];
       }
       profiles["hourly"] = ([ ]);
       stamps["hourly"] = t - t % 3600;
    }
    add_to_profile(profiles["hourly"], ob, script, label, cost);

    if (t >= stamps["daily"]  + 86400) {
       history["daily"] = ({ time(), dump_profile("daily") }) + history["daily"];
       if (sizeof(history["daily"]) > HISTORY_LIMIT * 2) {
	  history["daily"] = history["daily"][.. HISTORY_LIMIT * 2 - 1];
       }
       profiles["daily"] = ([ ]);
       stamps["daily"] = t - t % 86400;
    }
    add_to_profile(profiles["daily"],  ob, script, label, cost);
  }
}

/*
 * Merry calling support.
 */

int query_method(string method)
{
   switch (method) {
   case "archives":
   case "archive":
   case "show":
   default:
      return FALSE;
   }
}

mixed call_method(string method, mapping args)
{
   switch (method) {
   case "archives": {
      int i, sz;
      int *stamps;
      string type;

      type = args["type"];
      sz = sizeof(history[type]) / 2;
      if (sz > HISTORY_LIMIT) {
	 sz = HISTORY_LIMIT;
      }
      stamps = allocate_int(sz);
      for (i = 0; i < sz; i++) {
	 stamps[i] = history[type][i * 2];
      }
      return stamps;
   }
   case "archive": {
      mixed index;
      string type;

      index = args["index"];
      type = args["type"];
      if (typeof(index) == T_INT && index >= 0 && index < sizeof(history[type]) / 2) {
	 return
	    "Profiler archive created on " + ctime(history[type][index * 2]) + ":\n\n" +
	    history[type][index * 2 + 1];
      }
      return nil;
   }
   case "show":
      return dump_profile("hourly", args["minimum"] ? args["minimum"] : 1000000);
   default:
      error("Unsupported method");
   }
}
