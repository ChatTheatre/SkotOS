/*
**	~UserAPI/lib/authapi.c
**
**	Contact a UserDB somewhere for authentication. This class
**	shares a lot of code with ctlapi.c but the differences are
**	subtle enough that abstracting the generic functionality
**	would be cumbersome.
**
**	000202 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

# include <UserAPI.h>

private inherit "/lib/string";
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

private atomic string mk_argstr(mixed args...);

static atomic
void start_md5login(string callback, string user, string hash, mixed args...) {
   call_out("auth_request", 0, callback, args, user,
           "md5login", mk_argstr(user, hash),
           60);
   return;
}

static atomic
void start_passlogin(string callback, string user, string pass,
		     mixed args...) {
   call_out("auth_request", 0, callback, args, user,
            "passlogin", mk_argstr(user, pass),
            60);
   return;
}

static atomic
void start_keycodeauth(string callback, string user, string code,
		       mixed args...) {
   call_out("auth_request", 0, callback, args, user,
	    "keycodeauth", mk_argstr(user, code),
	    60);
   return;
}

static atomic
void start_challenge(string callback, string user, mixed args...) {
   call_out("auth_request", 0, callback, args, nil,
	    "challenge", mk_argstr(user),
	    60);
}

static atomic
void start_getprop(string callback, string user, string prop,
		  mixed args...) {
   /* TODO: we must consider encoding */
   call_out("auth_request", 0, callback, args, nil,
	    "getprop", mk_argstr(user, prop), 60);
}

static atomic
void start_accountping(string callback, string user, mixed args...)
{
   call_out("auth_request", 0, callback, args, nil,
	    "accountping", mk_argstr(user),
	    60);
}

static atomic
void auth_request(string callback, string *args, string user,
		  string cmd, string argstr, int retries) {
   int ix;

   if (ix = AUTHD->send_request(this_object(), cmd, argstr)) {
      /* it's in the hands of the AUTHD */
      ix_to_callback[ix] = callback;
      ix_to_originator[ix] = query_originator();
      ix_to_args[ix] = ({ user }) + args;
      return;
   }
   if (retries > 0) {
      /* keep trying */
      call_out("auth_request", 1, callback, args, user,
	       cmd, argstr, retries - 1);
      return;
   }
   /* else give up */
   if (callback) {
      call_out(callback, 0, FALSE, "TIMEOUT", args...);
   }
}

atomic
void auth_reply(int ix, int success, string reply) {
   if (previous_program() == "/usr/UserAPI/sys/authd") {
      mixed *args;
      string fun;

      if (fun = ix_to_callback[ix]) {
	 set_originator(ix_to_originator[ix]);

	 args = ix_to_args[ix];

	 if (success && args[0]) {
	    UDATD->get_udat(args[0]);
	 }
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
