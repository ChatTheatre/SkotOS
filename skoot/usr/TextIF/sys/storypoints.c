/*
 * Keep track of +storypoints usage on this server.
 *
 * Copyright Skotos Tech Inc 2003, 2005.
 */

private inherit "/lib/string";

string  *accounts;
mapping per_stamp, per_user;

static void
create()
{
    per_stamp = ([ ]);
    per_user  = ([ ]);
    /*
     * Request to be kept uptodate about objects using SkotOS:API to use
     * storypoints.
     */
    "/usr/SkotOS/sys/api"->register_notifier(this_object(), "notify_api_apply");
}

atomic void
patch()
{
    "/usr/SkotOS/sys/api"->register_notifier(this_object(), "notify_api_apply");
}

void
add_usage(string who, string account, int total, string reason)
{
    int     when, day;
    mapping map;

    when = time();
    day  = when / 86400;

    /*
     * Make sure it's a unique timestamp.
     */
    while (per_stamp[day] && per_stamp[day][when]) {
	when++;
	day = when / 86400;
    }

    /*
     * First add in per_stamp mapping.
     */
    if (!per_stamp[day]) {
        per_stamp[day] = ([ ]);
    }
    per_stamp[day][when] = ({ who, account, total, reason });

    /*
     * Then add in per_user mapping.
     */
    if (!per_user[who]) {
        per_user[who] = ([ ]);
    }
    if (!per_user[who][day]) {
	per_user[who][day] = ([ ]);
    }
    per_user[who][day][when] = ({ account, total, reason });
}

void notify_api_apply(string what, string account, int total, string reason)
{
   catch {
      add_usage(what, account, total, reason);
   }
}

private mixed **
merge_data(mixed **list1, mixed **list2)
{
    int i, i1, i2, sz1, sz2;
    mixed **result;

    sz1 = sizeof(list1);
    sz2 = sizeof(list2);
    result = allocate(sz1 + sz2);
    i = i1 = i2 = 0;

    while (sz1 > 0 && sz2 > 0) {
	if (list1[i1][0] < list2[i2][0]) {
	    result[i] = list1[i1];
	    i1++;
	    sz1--;
	} else {
	    result[i] = list2[i2];
	    i2++;
	    sz2--;
	}
	i++;
    }

    while (sz1 > 0) {
	result[i] = list1[i1];
	i++;
	i1++;
	sz1--;
    }
    while (sz2 > 0) {
	result[i] = list2[i2];
	i++;
	i2++;
	sz2--;
    }
    return result;
}

mixed **
process_data(mapping top, int begin, int end)
{
    int     begin_day, end_day, i, sz_i;
    mapping map;
    mixed   **result;
    mapping *data;

    begin_day = begin / 86400;
    end_day   = end   / 86400;

    map = top[begin_day..end_day];
    sz_i = map_sizeof(map);
    data = map_values(map);

    result = ({ });
    for (i = 0; i < sz_i; i++) {
	int *stamps, j, sz_j;
	mixed **info;

	data[i] = data[i][begin..end];
	sz_j = map_sizeof(data[i]);
	stamps = map_indices(data[i]);
	info = map_values(data[i]);
	for (j = 0; j < sz_j; j++) {
	    result += ({ ({ stamps[j] }) + info[j] });
	}
    }
    return result;
}

mixed **
query_per_stamp(int begin, int end)
{
    return process_data(per_stamp, begin, end);
}

mixed **
query_per_user(string who, int begin, int end)
{
    int     i, sz;
    string  *names;
    mapping map;
    mixed *results;

    if (sscanf(who, "%*s:") > 0) {
	sz = map_sizeof(per_user);
	names = map_indices(per_user);
	for (i = 0; i < sz; i++) {
	    if (lower_case(names[i]) != who && lower_case(names[i]) != "<" + who + ">") {
		names[i] = nil;
	    }
	}
	names -= ({ nil });
    } else {
	names = ({ who });
    }

    sz = sizeof(names);
    results = ({ });
    for (i = 0; i < sz; i++) {
	mapping map;
	map = per_user[names[i]];
	if (map) {
	    results = merge_data(results, process_data(map, begin, end));
	}
    }
    return results;
}

mixed **
query_per_account(string account, int begin, int end)
{
   int i, sz;
   mixed **data;

   data = process_data(per_stamp, begin, end);
   sz = sizeof(data);
   for (i = 0; i < sz; i++) {
      if (data[i][2] != account) {
	 data[i] = nil;
      }
   }
   return data - ({ nil });
}

string *
query_accounts()
{
   return accounts ? accounts[..] : ({ });
}

void
set_accounts(string *str)
{
   accounts = str;
}
