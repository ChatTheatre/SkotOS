/*
**	~UserAPI/lib/ctlapi.c
**
**	Contact a UserDB somewhere for complex interactions.
**
**
**	000202 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

# include <UserAPI.h>

private inherit "/lib/url";

private mapping ix_to_callback;
private mapping ix_to_args;
private mapping ix_to_originator;

static
void create() {
   ix_to_callback = ([ ]);
   ix_to_args = ([ ]);
   ix_to_originator = ([ ]);
}

#if 0
void patch() {
   ix_to_originator = ([ ]);
}
#endif

private atomic string mk_argstr(mixed args...);

static atomic
void start_create(string callback, string user, string password,
		  string email, mixed args...) {
   /* TODO: we must consider encoding */
   call_out("ctl_request", 0, callback, args, user,
	    "create", mk_argstr(user, password, email),
	    60);
}

static atomic
void start_setprop(string callback, string user, string prop, string val,
		   mixed args...) {
   /* TODO: we must consider encoding */
   call_out("ctl_request", 0, callback, args, nil,
	    "setprop", mk_argstr(user, prop, val),
	    60);
}

static atomic
void start_clearprop(string callback, string user, string prop,
		     mixed args...) {
   /* TODO: we must consider encoding */
   call_out("ctl_request", 0, callback, args, nil,
	    "clearprop", mk_argstr(user, prop),
	    60);
}

static atomic
void start_ccreated(string callback, string user, int when, string theatre, mixed args...) {
    call_out("ctl_request", 0, callback, args, nil, "ccreated",
	     mk_argstr(user, when, theatre), 60);
}

static atomic
void start_cplayed(string callback, string user, int when, int duration, string theatre, mixed args...) {
    call_out("ctl_request", 0, callback, args, nil, "cplayed",
	     mk_argstr(user, when, duration, theatre), 60);
}

static atomic
void start_statistics(string callback, string user, string type, mixed args...) {
    call_out("ctl_request", 0, callback, args, nil, "statistics",
	     mk_argstr(user, type), 60);
}

static atomic
void start_broadcast_all(string callback, string channel, string sub,
			 mixed *args, mixed extra...)
{
    BROADCAST->broadcast_all(channel, sub, args);
}

static atomic
void start_broadcast_one(string callback, string to_game, string channel,
			 string sub, mixed *args, mixed extra...)
{
    BROADCAST->broadcast_one(to_game, channel, sub, args);
}

static atomic
void start_staff(string callback, string user, mixed args...) {
   call_out("ctl_request", 0, callback, args, nil, "staff",
	    mk_argstr(user), 60);
}

static atomic
void start_sps_use(string callback, string user, int total, string reason, string theatre, mixed args...)
{
    call_out("ctl_request", 0, callback, args, nil, "sps",
	     mk_argstr(user, "use", total, reason, "", theatre), 60);
}

static atomic
void start_sps_add(string callback, string user, int total, string reason, string theatre, mixed args...)
{
    call_out("ctl_request", 0, callback, args, nil, "sps",
	     mk_argstr(user, "add", total, reason, "", theatre), 60);
}

static atomic
void start_assists_list(string callback, mixed args...)
{
    call_out("ctl_request", 0, callback, args, nil, "assists",
	     mk_argstr("list"), 60);
}

static atomic
void start_assists_query(string callback, string group, mixed args...)
{
    call_out("ctl_request", 0, callback, args, nil, "assists",
	     mk_argstr("query", group), 60);
}

static atomic
void ctl_request(string callback, string *args, string user,
		 string cmd, string argstr, int retries) {
   int ix;

   XDebug("ctl_request()");
   if (ix = CTLD->send_request(this_object(), cmd, argstr)) {
      /* it's in the hands of the CTLD */
      ix_to_callback[ix] = callback;
      ix_to_args[ix] = ({ user }) + args;
      ix_to_originator[ix] = query_originator();
      XDebug("ctl_request returning");
      return;
   }
   if (retries > 0) {
      /* keep trying */
      XDebug("ctl_request retrying");
      call_out("ctl_request", 1, callback, args, user,
	       cmd, argstr, retries - 1);
      return;
   }
   /* else give up */
   XDebug("ctl_request giving up");
   if (callback) {
      call_out(callback, 0, FALSE, "CTL TIMEOUT", args...);
   }
}

atomic
void ctl_reply(int ix, int success, string reply) {
   if (previous_program() == "/usr/UserAPI/sys/ctld") {
      mixed *args;
      string fun;

      if (fun = ix_to_callback[ix]) {
	 set_originator(ix_to_originator[ix]);

	 args = ix_to_args[ix];

	 if (success && args[0]) {
	    UDATD->get_udat(args[0]);
	 }

	 XDebug("CTL: reply [" + ix + ": " + dump_value(ix_to_args[ix])+ "]");
	 args = args[1 ..];
	 call_out(fun, 0, success, reply, args...);
	 ix_to_callback[ix] = nil;
      }
      ix_to_args[ix] = nil;
      ix_to_originator[ix] = nil;
   }
}

private atomic
string mk_argstr(mixed args...) {
   int i;

   for (i = 0; i < sizeof(args); i ++) {
      args[i] = url_encode((string) args[i]);
   }
   return implode(args, " ");
}