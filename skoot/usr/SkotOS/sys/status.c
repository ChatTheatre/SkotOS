/*
 * Keep track of the game-status, to see if something is perhaps a little bit
 * overloaded from time to time.
 *
 * Skotos Tech Inc 2002.
 */

# include <status.h>

inherit authapi "~UserAPI/lib/authapi";

/*
 * Bad dependence, not sure how to work around it without violating some
 * rules, and I can't be arsed to put this entire thing in ~TextIF/ now. -EH.
 */

# define INFOD "/usr/TextIF/sys/infod"

# define S_SNAPSHOT  2

# define S_HEARTBEAT 0
# define S_SWAPAVG1  1
# define S_SWAPAVG5  2
# define S_TOTAL     3

# define S_HISTORY   1000

# define U_SNAPSHOT  900
# define U_LOGFILE   "~/data/log/users"

private int   handle;
private int   index;
private int   total;
private mixed *timestamp;
private float **history;

private int   last_users_snapshot;

static void
create()
{
   int i;

   authapi::create();

   history = allocate(S_TOTAL);
   for (i = 0; i < S_TOTAL; i++) {
      history[i] = allocate_float(S_HISTORY);
   }
}

void
patch()
{
   ::create();
}

void
restart_callout()
{
   timestamp = millitime();
   if (handle) {
      remove_call_out(handle);
      handle = 0;
   }
   handle = call_out("make_snapshot", S_SNAPSHOT);
}

void
boot(int block)
{
   if (ur_name(previous_object()) == "/usr/SkotOS/initd") {
      restart_callout();
   }
}

void
reboot(int block)
{
   if (ur_name(previous_object()) == "/usr/SkotOS/initd") {
      restart_callout();
   }
}

void
do_users_snapshot()
{
   int     i, sz;
   object  *users;
   mapping map;

   users = INFOD->query_current_user_objects();
   sz = sizeof(users);
   map = ([
      "_total_": sz,
      "seen":    ([ ]),
      "names":   ([ ]),
      "sums":    ([ "free": 0, "paid": 0, "auth": 0, "trial": 0 ]),
      "chars":   ([ ])
   ]);

   for (i = 0; i < sz; i++) {
      start_getprop("users_snapshot_fetch",
		    users[i]->query_name(),
		    "billing_status",
		    users[i], map);
   }
}

static void
users_snapshot_fetch(int success, string val, object target, mapping map)
{
   /*
    * Check return value.
    */

   if (!success) {
      val = "unknown";
   } else if (val == "") {
      val = "trial";
   }

   /*
    * Categorize result.
    */

   if (target) {
      string name;

      name = target->query_name();
      if (!map["seen"][name]) {
	 map["seen"][name] = TRUE;

	 if (!map["names"][val]) {
	    map["names"][val] = ([ ]);
	 }
	 map["names"][val][name] = 1;

	 if (!map["sums"][val]) {
	    map["sums"][val] = 0;
	 }
	 map["sums"][val]++;
      } else {
	 map["names"][val][name]++;
      }
      if (target->query_body()) {
	 map["chars"][target->query_body()] = TRUE;
      }
   }

   /*
    * Are we done yet?
    */
   map["_total_"]--;
   if (map["_total_"] > 0) {
      return;
   }

   /*
    * All queries have returned some value, be it successful or not.
    */
   write_file(U_LOGFILE,
	      time()                   + "\t" +
	      map_sizeof(map["seen"])  + "\t" +
	      map["sums"]["free"]      + "\t" +
	      map["sums"]["paid"]      + "\t" +
	      map["sums"]["auth"]      + "\t" +
	      map["sums"]["trial"]     + "\t" +
	      map_sizeof(map["chars"]) + "\n");
}

static void
make_snapshot()
{
   mixed *stamp;
   float diff;
   mixed *st;

   call_out("make_snapshot", S_SNAPSHOT);
   stamp = millitime();

   diff = (float)(stamp[0] - timestamp[0]) + (stamp[1] - timestamp[1]);
   st = status();
   history[S_HEARTBEAT][index] = diff;
   history[S_SWAPAVG1][index] = (float)st[ST_SWAPRATE1] /  60.0;
   history[S_SWAPAVG5][index] = (float)st[ST_SWAPRATE5] / 300.0;

   timestamp = stamp;
   total++;
   index  = (index + 1) % S_HISTORY;

   if (time() >= last_users_snapshot + U_SNAPSHOT) {
      last_users_snapshot = time();
      do_users_snapshot();
   }
}

float **
query_history()
{
   return history;
}

mixed *
query_avg(int sub)
{
   int i, avg_index, avg_total;
   float *sums, *mins, *maxs;

   avg_index = (index + S_HISTORY - 1) % S_HISTORY;
   avg_total = (total < S_HISTORY) ? total : S_HISTORY;
   sums = allocate_float(3);
   mins = allocate_float(3);
   maxs = allocate_float(3);
   for (i = 0; i < avg_total;
	i++, avg_index = (avg_index + S_HISTORY - 1) % S_HISTORY) {
      float entry;

      entry = history[sub][avg_index];
      if (i < 10) {
	 sums[0] += entry;
	 mins[0] = (!mins[0] || mins[0] > entry) ? entry : mins[0];
	 maxs[0] = (!maxs[0] || maxs[0] < entry) ? entry : maxs[0];
      }
      if (i < 100) {
	 sums[1] += entry;
	 mins[1] = (!mins[1] || mins[1] > entry) ? entry : mins[1];
	 maxs[1] = (!maxs[1] || maxs[1] < entry) ? entry : maxs[1];
      }
      sums[2] += entry;
      mins[2] = (!mins[2] || mins[2] > entry) ? entry : mins[2];
      maxs[2] = (!maxs[2] || maxs[2] < entry) ? entry : maxs[2];
   }
   sums[0] = sums[0] / (float)(total <   10 ? total :   10);
   sums[1] = sums[1] / (float)(total <  100 ? total :  100);
   sums[2] = sums[2] / (float)(total < 1000 ? total : 1000);
   return ({ sums, mins, maxs, total });
}

string
decimals(float f, int b, int a)
{
   string a_i, b_i;

   if (sscanf((string)f, "%s.%s", b_i, a_i) < 2) {
      b_i = (string)f;
      a_i = "0";
   }
   b_i = "                                        " + b_i;
   b_i = b_i[strlen(b_i) - b..];
   a_i = a_i + "0000000000000000000000000000000000000000";
   a_i = a_i[..a - 1];
   return b_i + "." + a_i;
}

string
dump_heartbeat_avg()
{
   mixed *data;

   data = query_avg(S_HEARTBEAT);
   return
      "Total heartbeats counted (max " + S_HISTORY + " significant): " +
      data[3] + "\n" +
      "\n" +
      "        Avg   Min   Max\n" +
      "  10  " +
      decimals(data[0][0], 2, 2) + " " +
      decimals(data[1][0], 2, 2) + " " +
      decimals(data[2][0], 2, 2) + "\n" +
      " 100  " +
      decimals(data[0][1], 2, 2) + " " +
      decimals(data[1][1], 2, 2) + " " +
      decimals(data[2][1], 2, 2) + "\n" +
      "1000  " +
      decimals(data[0][2], 2, 2) + " " +
      decimals(data[1][2], 2, 2) + " " +
      decimals(data[2][2], 2, 2) + "\n";
}

string
dump_swapavg1()
{
   mixed *data;

   data = query_avg(S_SWAPAVG1);
   return
      "1m:     Avg   Min   Max\n" +
      "  10  " +
      decimals(data[0][0], 2, 2) + " " +
      decimals(data[1][0], 2, 2) + " " +
      decimals(data[2][0], 2, 2) + "\n" +
      " 100  " +
      decimals(data[0][1], 2, 2) + " " +
      decimals(data[1][1], 2, 2) + " " +
      decimals(data[2][1], 2, 2) + "\n" +
      "1000  " +
      decimals(data[0][2], 2, 2) + " " +
      decimals(data[1][2], 2, 2) + " " +
      decimals(data[2][2], 2, 2) + "\n";
}

string
dump_swapavg5()
{
   mixed *data;

   data = query_avg(S_SWAPAVG5);
   return
      "5m:     Avg   Min   Max\n" +
      "  10  " +
      decimals(data[0][0], 2, 2) + " " +
      decimals(data[1][0], 2, 2) + " " +
      decimals(data[2][0], 2, 2) + "\n" +
      " 100  " +
      decimals(data[0][1], 2, 2) + " " +
      decimals(data[1][1], 2, 2) + " " +
      decimals(data[2][1], 2, 2) + "\n" +
      "1000  " +
      decimals(data[0][2], 2, 2) + " " +
      decimals(data[1][2], 2, 2) + " " +
      decimals(data[2][2], 2, 2) + "\n";
}
