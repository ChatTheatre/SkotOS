/*
**	/base/lib/scheduler.c
**
**	Provide timed event callbacks.
**
**	Copyright Skotos Tech Inc 1999
**
*/


private inherit "/lib/string";

/* entry: ({ event, recur, next_stamp }) */

# define ENTRY_EVENT	0
# define ENTRY_RECUR	1
# define ENTRY_STAMP	2
# define ENTRY_ARGS	3

# include "/base/include/events.h"

private mapping	id_to_entry;
private mapping	stamp_to_ids;

private int	current_stamp;
private int	callout_handle;

private int	rescheduling;

private void insert(string id);
private void delete(string id);

mapping query_id_to_entry() { return id_to_entry; }
mapping query_stamp_to_ids() { return stamp_to_ids; }

int query_current_stamp() { return current_stamp; }
int query_callout_handle() { return callout_handle; }

int query_rescheduling() { return rescheduling; }

# define Patch() { if (!id_to_entry || !stamp_to_ids) { id_to_entry = ([ ]); stamp_to_ids = ([ ]); } }

static
void create() {
   id_to_entry = ([ ]);
   stamp_to_ids = ([ ]);
}

void clear_scheduler() {
   id_to_entry = ([ ]);
   stamp_to_ids = ([ ]);
}

atomic
string schedule_entry(string event, int first, varargs int recur,
		      mapping args) {
   string id;

   Patch();

   id = to_hex(hash_md5(event + "#" + recur + "#" + first));

   if (id_to_entry[id]) {
      /* identical entry already exists */
      DEBUG("id_to_entry[" + id + "] already exists");
   } else {
      id_to_entry[id] = ({ event, recur, first, args });
      insert(id);
   }
   return id;
}

atomic
void unschedule_entry(string id) {
   Patch();

   if (id_to_entry[id]) {
      delete(id);
   }
}

private atomic
void delete(string id) {
   mixed *data;
   int stamp;

   data = id_to_entry[id];

   if (!data) {
      error("attempt to schedule nonexistent entry");
   }
   stamp = data[ENTRY_STAMP];
   if (stamp_to_ids[stamp]) {
      stamp_to_ids[stamp][id] = nil;
      if (!map_sizeof(stamp_to_ids[stamp])) {
	 stamp_to_ids[stamp] = nil;
      }
   } else {
      WARNING("deleted id " + id + " not in stamp_to_ids[" + stamp + "]");
   }
   id_to_entry[id] = nil;
   if (!rescheduling && (!current_stamp ||
			 stamp <= current_stamp ||
			 current_stamp < time())) {
      rescheduling = call_out("reschedule", 0);
   }
}

private atomic
void insert(string id) {
   mixed *data;
   int delta, stamp, recur;

   data = id_to_entry[id];

   if (!data) {
      error("attempt to schedule nonexistent entry");
   }

   recur = data[ENTRY_RECUR];
   stamp = data[ENTRY_STAMP];

   delta = stamp - time();
   if (delta < 0) {
      if (recur) {
	 /* fast-forward N recurrences where N is ceil(lag/recur) */
	 WARNING("Fast-forwarding " + (1 - delta/recur) + " recurrences, from " + ctime(stamp) + " to " + ctime(stamp + delta));
	 stamp += recur * (1 - delta/recur);
      } else {
	 stamp = time();
      }
   }
   data[ENTRY_STAMP] = stamp;

   if (!stamp_to_ids[stamp]) {
      stamp_to_ids[stamp] = ([ ]);
   }
   stamp_to_ids[stamp][id] = TRUE;
   if (!rescheduling && (!current_stamp ||
			 stamp != current_stamp ||
			 current_stamp < time())) {
      rescheduling = call_out("reschedule", 0);
   }
}

static atomic
void reschedule() {
   int first, delta;

   rescheduling = 0;

   if (!map_sizeof(stamp_to_ids)) {
      /* nothing left! */
      if (map_sizeof(id_to_entry)) {
	 WARNING("stamp_to_ids empty but id_to_entry = " + dump_value(id_to_entry));
      }
      current_stamp = 0;
      return;
   }
   first = map_indices(stamp_to_ids)[0];
   if (first != current_stamp || current_stamp < time()) {
      if (callout_handle) {
	 remove_call_out(callout_handle);
	 callout_handle = 0;
      }
      delta = first - time();
      if (delta < 0) {
	 delta = 0;
      }
      current_stamp = first;
      callout_handle = call_out("tick", delta);
   }
}

static
void tick() {
   string *ids;
   mixed *data;
   int i;

   catch {
      callout_handle = 0;
      if (current_stamp && stamp_to_ids[current_stamp]) {
	 ids = map_indices(stamp_to_ids[current_stamp]);
	 for (i = 0; i < sizeof(ids) ;i ++) {
	    if (data = id_to_entry[ids[i]]) {
	       call_out("handle_event", 0, data[0], ids[i],
			(sizeof(data) > 3 ? data[3] : ([ ])));
	       if (data[ENTRY_RECUR]) {
		  /* do this again */
		  data[ENTRY_STAMP] += data[ENTRY_RECUR];
		  insert(ids[i]);
	       } else {
		  /* or don't */
		  id_to_entry[ids[i]] = nil;
	       }
	    }
	 }
	 stamp_to_ids[current_stamp] = nil;
      }
   }
   if (!rescheduling) {
      rescheduling = call_out("reschedule", 0);
   }
}

static
void handle_event(string event, string id, mapping args) {
   special_signal("timer", event, args ? args : ([ ]));
}
