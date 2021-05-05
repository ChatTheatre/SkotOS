/*
**	~HTTP/obj/httpconn.c
**
**	Each time ~/sys/httpd.c receives a HTTP request, it clones this
**	object to handle the request. In the terminology of the kernel
**	library, this is a 'user' object. It is the SkotOS side of a
**	network connection.
**
**	The duty of this object is to receive and parse input and to
**	help external objects respond to the request properly.
**
**	TODO: idle timeout (but not non-idle timeout)
**
**	Copyright Skotos Tech Inc 1999 - 2005
*/


# include <type.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <HTTP.h>
# include <UserAPI.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/date";
private inherit "/lib/url";

inherit "/usr/System/lib/output_buffer";
inherit "/usr/UserAPI/lib/authapi";

object udat;
string user, request;
mapping args, headers;
string header, method, url, body;
int http_state;
int major, minor;
int conlen;

int arguments_secure;	/* an arghash was supplied & is validated */

object node;
string origin;

int java, activex;
int win, mac;
int ns, ie, moz, webkit;

string auth_reply;

mixed *stamp;
int   idle_callout;

static mapping decode_args(string argstr);
static void respond();
static void respond_to_request(string method, string url, mapping args);
static void check_authorization();
static void check_useragent();



# define HTTP_VIRGIN		1
# define HTTP_HEADERS		2
# define HTTP_BODY		3
# define HTTP_RESPONDING	4

/* public API */

static
void create(int clone) {
   ::create();
}

int query_http_state() { return http_state; }
int query_stamp() { return stamp ? stamp[0] : 0; }

int html_connection() { return TRUE; }

string query_name() { return user; }
object query_udat() { return udat; }

int query_java() { return java; }
int query_activex() { return activex; }

int query_windows() { return win; }
int query_macintosh() { return mac; }

int query_netscape() { return ns; }
int query_mozilla() { return moz; }
int query_explorer() { return ie; }

void set_origin(string str) { origin = str; }
string query_origin() { return origin; }

void set_node(object n) { node = n; }
object query_node() { return node; }

int query_arguments_secure() { return arguments_secure; }

private
string url_absolute(string url) {
   string url_hostname, url_protocol;
   int url_port;

   url_protocol = SYS_INITD->query_url_protocol();

   url_hostname = headers["host"];
   if (!url_hostname || !HTTPD->query_hostname(url_hostname)) {
        url_hostname = SYS_INITD->query_hostname();
   }
   sscanf(lower_case(url_hostname), "%s:", url_hostname);
   url_port = node->query_port() == SYS_INITD->query_real_webport() ? SYS_INITD->query_webport() : node->query_port();
   return url_protocol + "://" + url_hostname + ":" + url_port + url;
}

private
string pad_zeroes(int i, int width) {
    string str;

    str = (string)i;
    if (strlen(str) >= width) {
	return str;
    }
    return "0000000000"[..width - strlen(str) - 1] + str;
}

private
void log_http(int code, int size) {
   string logfile, header_host;
   mixed *now;

   if (!stamp) {
     DEBUG("stamp was nil!");
     stamp = millitime();
   }
   catch {
       now = millitime();
   logfile = "/usr/HTTP/log/access.log";
   header_host = headers["host"];
   if (header_host) {
       sscanf(lower_case(header_host), "%s:", header_host);
   }
   write_file(logfile,
	      (query_conn() ? query_ip_number(query_conn()) : "?.?.?.?") + " " + /* remote ipnumber */
	      "- " +                                                             /* ? */
	      (user ? user : "-") +                                        " " + /* user ? */
#if 1
	      "[" + stamp[0] + "] " +
#else
	      "[" + pad_zeroes(get_day(stamp[0]),    2) + "/" +
	            ctime(stamp[0])[4..6]               + "/" +
	            pad_zeroes(get_year(stamp[0]),   4) + ":" +
	            pad_zeroes(get_hour(stamp[0]),   2) + ":" +
	            pad_zeroes(get_minute(stamp[0]), 2) + ":" +
	            pad_zeroes(get_second(stamp[0]), 2) + " -0600] " +
#endif
	      "\"" + (method ? method : "NOMETHOD") + " " + (request ? request : "NOREQUEST") + "\" " +
	      code + " " +
	      size + " " +
	      "\"" + (headers["referer"] ? headers["referer"] : "-") + "\" " +
	      "\"" + (headers["user-agent"] ? headers["user-agent"] : "-") + "\"" + /* user agent */
#if 1
	      " " + (int)(1000.0 * ((float)(now[0] - stamp[0]) + now[1] - stamp[1])) + " " +
	      (header_host ? header_host : "-") +                                    /* virtual host */
#endif
	      "\n");
   }
}

private
void send_headers(int code, string *headers) {
   string res;
   int i;

   headers = ({
      "Server: " + HTTP_SERVER_NAME,
      "Date: " + rfc_1123_date(time()),
      "Allow: GET, POST" }) + headers;
   if (url && strlen(url) > 4 && url[strlen(url) - 4..] == ".sam") {
      headers += ({
	 "Cache-Control: no-cache",
	 "Pragma: no-cache",
	 "Expires: " + rfc_1123_date(time() - 1)
      });
   }
   res = "HTTP/1.0 " + code + " " + code + "\n";
   for (i = 0; i < sizeof(headers); i ++) {
      XDebug("sending header:: " + headers[i]);
      res += headers[i] + "\n";
   }
   ::send_message(res + "\n");
}

private
void send_entity_headers(int code, string type, int size, string * headers) {
   headers += ({
      "Content-Type: " + type,
      "Content-Length: " + size,
   });
   send_headers(code, headers);
   log_http(code, size);
}

void send_html(int code, mixed html, varargs string *headers) {
   if (!headers) {
      headers = ({ });
   }
   if (typeof(html) == T_STRING) {
      send_entity_headers(code, "text/html", strlen(html), headers);
      ::send_message(html);
   } else if (typeof(html) == T_OBJECT) {
      /* TODO make this neater */
      send_entity_headers(code, "text/html", html->query_length(), headers);
      ::send_data(html);
   }      
}

void send_file(int code, string file, string type, varargs string *headers) {
   int size;

   size = file_size(file);
   if (size < 0) {
      clear_output();
      send_html(404, "<body>Page not found.</body>\n");
      return;
   }
   send_entity_headers(code, type, size, headers ? headers : ({ }));
   /* this call goes to the output buffer code */
   ::send_file(file, size);
}

void handle_error(string error, mapping *wtrace, mapping *cline) {
   if (!query_committed_output()) {
      if (query_clearable_output()) {
	 clear_output();
      }
      call_out("respond_to_request", 0,
	       "GET", HTTP_ERROR_DOCUMENT, ([
		  "trace": wtrace, "error": error, "compilation": cline,
		  ]));
   }
}

void redirect_to(string url, varargs mapping args, string *headers) {
   if (args) {
      url = make_url(url, args);
   }
   if (!headers) {
      headers = ({ });
   }
   url = url_absolute(url);
   XDebug("redirecting to " + url);
   send_html(302, "<a href=\"" + url + "\">Here</a>\n",
	     headers + ({ "Location: " + url, "Connection: close" }));
}

/* system API */

int login(string name) {
   if (previous_program() == LIB_CONN) {
      connection(previous_object());
      Debug("incoming request");
      http_state = HTTP_VIRGIN;
      arguments_secure = FALSE;
      idle_callout = call_out("idle_disconnect", RESPONSE_TIMEOUT);
      return MODE_LINE;
   }
}

static void idle_disconnect()
{
    DEBUG("idle_disconnect:" + name(this_object()) + " [" +
	  ((query_conn() && query_ip_number(query_conn())) ?
	   query_ip_number(query_conn()) : "?") + "]; state = " + http_state +
	  (request ? "; request = " + dump_value(request) : ""));
    ::disconnect();
}

void logout(int dest) {
    if (previous_program() == LIB_CONN) {
	Debug("logged out: destructing");
	destruct_object();
    }
}

static int state(varargs string data);
static int state_virgin(string data);
static int state_headers(string data);
static int state_body(string data);
static int state_responding(string data);

/*
**	This is the main input function, which is called from
**	the kernel library when there is new input, in somewhat
**	different ways depending on whether the connection is
**	currently operating in line mode or raw mode.
**
**	For HTTP, we start off in line mode, which means we are
**	guaranteed to be called with one full line of input at
**	a time.
**
*/

int receive_message(string data) {
   if (previous_program() == LIB_CONN) {
      remove_call_out(idle_callout);
      idle_callout = call_out("idle_disconnect", RESPONSE_TIMEOUT);
      return state(data);
   }
}

static
int state(varargs string data) {
   switch(http_state) {
   case HTTP_VIRGIN:
      return state_virgin(data);
   case HTTP_HEADERS:
      return state_headers(data);
   case HTTP_BODY:
      return state_body(data);
   case HTTP_RESPONDING:
      return state_responding(data);
   default:
    error("bad internal http state");
   }
}

static
int state_virgin(string data) {
   /* parse the first line */
   string argstr;

   /* Make sure these two variables have sane values before we get started */
   args = ([ ]);
   headers = ([ ]);

   if (!sscanf(data, "%s %s HTTP/%d.%d", method, request, major, minor)) {
      send_html(404, "nonsensical request line");
      http_state = HTTP_RESPONDING;
      return MODE_ECHO;
   }
   Debug("URL " + request + " requested");
   if (major == 0) {
      http_state = HTTP_RESPONDING;
      send_message("<blink>HTTP versions 0.x not supported</blink>\n");
      return MODE_ECHO;
   }
   /* chew URL */
   if (method != "GET" && method != "POST") {
      send_html(501, "can't handle method " +  method);
      http_state = HTTP_RESPONDING;
      return MODE_ECHO;
   }
   if (sscanf(request, "%s?%s", url, argstr) == 2) {
      args = decode_args(argstr);
   } else {
      args = ([ ]);
      url = request;
   }
   args["request"] = request;

   /* start collecting headers now */
   headers = ([ ]);
   http_state = HTTP_HEADERS;
   return MODE_ECHO;
}


static
int state_headers(string data) {
   if (strlen(data) > 0) {
      if (data[0] == ' ' || data[0] == '\t') {
	 /* this is a continued header */
	 headers[header] += data;
      } else if (sscanf(data, "%s: %s", header, data) == 2) {
	 /* this is a new header */
	 header = lower_case(header);
	 headers[header] = data;
	 XDebug("Header: " + header + " set to " + data);
      }
      return MODE_ECHO;
   }
   /* an empty line, ie a double CRLF, marks the end of the headers */
   /* and the start of a possible body */

   previous_object()->set_mode(MODE_RAW);

   body = "";
   if (headers["content-length"]) {
      sscanf(headers["content-length"], "%d", conlen);
   }
   if (conlen == 0) {
      http_state = HTTP_RESPONDING;
      call_out("state", 0);
      return MODE_BLOCK;	/* no more input, please */
   }
   /* with a valid content length, we wait for a body */
   if (headers["content-type"]) {
     /*
      * Anticipate this:
      * "content-type":"application/x-www-form-urlencoded; charset=UTF-8"
      */
     string content_type;

     content_type = headers["content-type"];
     sscanf(content_type, "%s;", content_type);
     if (content_type == "application/x-www-form-urlencoded") {
       /* at least if it's of a type we like */
       http_state = HTTP_BODY;
       return MODE_RAW;
     }
   }
   send_html(500, "unknown content type of request body");
   return MODE_BLOCK;
}

static
int state_body(string data) {
   XDebug("adding to body: " + strlen(data));
   body += data;
   if (strlen(body) >= conlen) {
      Debug("responding to body " + method);
      args += decode_args(body);
      http_state = HTTP_RESPONDING;
      call_out("state", 0);
      return MODE_BLOCK;
   }
   return MODE_RAW;
}

static
int state_responding(string data) {
   string header_host, instance_host;

   stamp = millitime();
   if (method == "GET" && headers["host"] && request[0] == '/') {
      instance_host = SYS_INITD->query_hostname();

      header_host = lower_case(headers["host"]);
      sscanf(header_host, "%s:", header_host);

      if (header_host != lower_case(instance_host) &&
	  !HTTPD->query_hostname(header_host)) {
	 SysLog("Warning: HOST header is " + header_host +
		" but instance host is " + instance_host);
      }
   }
   check_useragent();
   check_authorization();
   return MODE_BLOCK;
}

static
void check_useragent() {
   string agent;

   /*
    * Recognized browsers + platforms:
    *
    * Netscape on W98:
    *   Mozilla/4.61 [en]C-CCK-MCD   (Win98; U)
    *
    * MSIE x.y on W98:
    *   Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)
    *   Mozilla/4.0 (compatible; MSIE 5.01; Windows 98; AT&T WNS5.0)
    *   Mozilla/4.0 (compatible; MSIE 5.5; Windows 98; Win 9x 4.90)
    *
    * MSIE x.y on NT:
    *   Mozilla/4.0 (compatible; MSIE 4.01; Windows NT)
    *
    * MSIE x.y on Windows 2000:
    *   Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)
    *   Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)
    *
    * Opera as Mozilla/5.0 on Linux:
    *   Mozilla/5.0 (Linux 2.4.10 i686; U) Opera 5.0  [en]
    * Opera as MSIE 5.0 on Linux:
    *   Mozilla/4.0 (compatible; MSIE 5.0; Linux 2.4.10 i686) Opera 5.0  [en]
    *
    * Netscape 4.77 on Linux:
    *   Mozilla/4.77 [en] (X11; U; Linux 2.4.10 i686; Nav)
    *
    * MSIE x.y on Macintosh:
    *   Mozilla/4.0 (compatible; MSIE 4.5; Mac_PowerPC)
    * Odd ones out:
    *   Java1.3.0
    */


   /* why is this necessary? */
   if (!headers) {
      headers = ([ ]);
   }

#if 0
   DEBUG("headers = " + dump_value(headers));
#endif

   if (agent = headers["user-agent"]) {
#if 0
      int    i, sz;
      string pre, data, post, *bits, browser, os, platform;

      if (sscanf(agent, "%s(%s)%s", pre, data, post) < 3) {
	  return;
      }
      pre  = strip(pre);
      post = strip(post);
      bits = explode(data, ";");
      sz = sizeof(bits);
      for (i = 0; i < sz; i++) {
	  bits[i] = strip(bits[i]);
	  if (sscanf(bits[i], "MSIE %*d.%*d") == 2) {
	      ie = TRUE;
	      browser = " IE";
	  } else if (sscanf(bits[i], "Windows %*s")) {
	      win = TRUE;
	      os = "Win32";
	      platform = "Mac";
	  } else if (sscanf(bits[i], "Win%*s")) {
	      /* Probably netscape? */
	      win = TRUE;
	      os = "Win32";
	      platform = "PC";
	  } else if (sscanf(bits[i], "Mac%*s")) {
	      os = "Macintosh";
	  } else if (bits[i] == "PPC") {
	      platform = "PPC";
	  }
      }
#endif
      win = sscanf(agent, "%*sWin");
      mac = sscanf(agent, "%*sMac");
      ie = sscanf(agent, "%*sMSIE");
      webkit = sscanf(agent, "%*sKHTML");
      if (!ie && !webkit) {
	 ns = sscanf(agent, "%*sMozilla");
	 moz =
	    sscanf(agent, "%*sMozilla/5.%*s") ||
	    sscanf(agent, "%*sMozilla/6.%*s");
	    
      }
      XDebug(dump_value(win));
      XDebug(dump_value(mac));
      XDebug(dump_value(ie));
      XDebug(dump_value(ns));

      if (sscanf(agent, "WebhitRelay %*d.%*d")) {
	 /* Known agent, be quiet about it. */
      } else if (!win && !mac) {
	 Debug("Unknown OS Agent: " + agent);
      } else if (win && mac) {
	 Debug("Overly known OS Agent: " + agent);
      } else if (!ie && !ns) {
	 Debug("Unknown browser Agent: " + agent);
      } else if (ie && ns) {
	 Debug("Overly known browser Agent: " + agent);	 
      }
   }
   activex = win && ie;
   java = ns || (win && ie);
}

static
void check_authorization() {
   string cookie;
   string u, p, s;
   object o;

   if (cookie = headers["cookie"]) {
      mapping jar;
      string *pairs, var, val;
      int i;

      jar = ([ ]);
      pairs = explode(cookie, ";");
      for (i = 0; i < sizeof(pairs); i ++) {
	 if (sscanf(strip(pairs[i]), "%s=%s", var, val) == 2) {
	    jar[lower_case(var)] = val;
	 }
      }
      u = jar["user"];
      p = jar["pass"];
      s = jar["secret"];
      if (u && p) {
	 call_out("start_auth", 0,
		  lower_case(url_decode(u)),
		  url_decode(p));
	 return;
      }
   }
   call_out("start_auth", 0);
}

static
void start_auth(varargs string u, string p) {
   if (u) {
      ::start_keycodeauth("finish_auth", u, p, u);
      return;
   }
   /* else let main code redirect to login page */
   respond_to_request(method, url, args);
}

private void
restore_zsession_data(mapping args)
{
   mapping data;

   if (args["zid"] && (data = udat->query_data((int)args["zid"]))) {
      int    i, sz;
      string *names;
      mixed  *values;

      /*
       * Restore session data, but only where it doesn't override
       * parameters that were provided in the GET/POST form, just in case
       * that might cause confusing backward incompatible situations.
       */
      sz     = map_sizeof(data);
      names  = map_indices(data);
      values = map_values(data);
      for (i = 0; i < sz; i++) {
	 if (!args[names[i]]) {
	    args[names[i]] = values[i];
	 }
      }
   }
}

static
void finish_auth(int success, string reply, string u) {
   user = u;

   if (success) {
      string account_type, account_flags;

      udat = UDATD->get_udat(user);
      if (!udat) {
	 error("yach! no udat!");
      }
      catch {
	  udat->add_last_ipnumber("http",
				  query_ip_number(query_conn()),
				  query_ip_name(query_conn()));
      }
      if (sscanf(reply, "%*s(%s;%s)", account_type, account_flags) == 3) {
	 udat->update_account(account_type, explode(account_flags, " "));
      }
      restore_zsession_data(args);
   }
   auth_reply = reply;

   respond_to_request(method, url, args);
}


static
mapping decode_args(string argstr) {
    mapping args;
    string *argarr, *hasharr;
    int i;

    while (sscanf(argstr, "%s\r\n", argstr));

    args = ([ ]);
    argarr = explode(argstr, "&");
    hasharr = allocate(sizeof(argarr));
    for (i = 0; i < sizeof(argarr); i ++) {
	string ix;
	mixed val;

	if (!sscanf(argarr[i], "%s=%s", ix, val)) {
	   ix = argarr[i];
	   val = 1;
	} else {
	   val = url_decode(val);
	   if (ix != "arghash") {
	      hasharr[i] = argarr[i];
	   }
	}
	ix = lower_case(url_decode(ix));
	if (strlen(ix) > 2 && ix[strlen(ix) - 2..] == "[]") {
	   ix = ix[..strlen(ix) - 3];
	   if (!args[ix]) {
	      args[ix] = ({ val });
	   } else if (typeof(args[ix]) != T_ARRAY) {
	      args[ix] = ({ args[ix], val });
	   } else {
	      args[ix] += ({ val });
	   }
	} else {
	   args[lower_case(url_decode(ix))] = val;
	}
    }
    hasharr = ({ "SKO" }) + (hasharr - ({ nil })) + ({ "TOS" });
    arguments_secure = (args["arghash"] ==
			to_hex(hash_md5(implode(hasharr, "&"))));
    return args;
}


static void respond_to_request(string method, string url, mapping args) {
   string root, orig, type;
   object handler;

   if (url == "/Request") {
      mixed dat;
      int id;

      if (!args["__id"]) {
	 error("/Request without id");
      }
      if (sscanf(args["__id"], "%d", id)) {
	 if (!udat) {
	    error("request without valid id");
	 }
	 if (dat = udat->query_data(id)) {
	    url = dat[0];
	    args += dat[1]; 	/* TODO */
	    args["__id"] = nil;
	    restore_zsession_data(args);
	    arguments_secure = TRUE;
	 } else {
	    SysLog("Argh, no data known for id=" + id);
	 }
      }
   } else if (url == "/Binary") {
      mixed dat;
      int id;

      if (!args["__id"]) {
	 error("/Binary without id");
      }
      if (sscanf(args["__id"], "%d", id)) {
	 udat = UDATD->query_udat(args["name"]);
	 if (dat = udat->query_data(id)) {
	    send_entity_headers(200, dat[1], strlen(dat[0]), ({ }));
	    send_message(dat[0]);
	    return;
	 }
	 error("argh, no data known for id = " + id);
      }
   } else if (url == "/FullyBooted") {
      if (SYS_BOOT->is_fully_booted()) {
        send_html(200, "OK");
        return;
      } else {
        send_html(404, "Not fully booted!");
        return;
      }
   }
   if (!url || !strlen(url) || url == "/") {
      /* generalize later ... */
      url = node->query_document("root_url");
   } else if (url) {
      url = DRIVER->normalize_path(url, "/");
   }
   orig = url;
   if (sscanf(url, "/%s/%s", root, url) == 2) {
      string val, suffix;
      int trial;

      if (!member("open", explode(lower_case(orig), "/"))) {
	 if (!udat) {
	    switch(auth_reply) {
	    case "TOS":
	       redirect_to(node->query_document("tos"), ([
		  "redirect": url_absolute(make_url(orig, args))
		  ]));
	       return;
	    case "UNPAID":
	       redirect_to(node->query_document("cc"), ([
		  "redirect": url_absolute(make_url(orig, args))
		  ]));
	       return;
	    case "LOCAL BAD KEYCODE":
	      redirect_to(node->query_document("login"), ([
	       "redirect": url_absolute(make_url(orig, args)),
	       ]));
	      return;
	    }
	    redirect_to(node->query_document("login"), ([
	       "redirect": url_absolute(make_url(orig, args)),
	       "complaint": auth_reply ?
	       "USERDB ERROR: " + auth_reply :
	       nil
	       ]));
	    return;
	 }
      }
      root = lower_case(root);
      url = "/" + url;

      if (handler = HTTPD->query_root_handler(root)) {
	 /* handler might to want to deal on its own */
	 if (handler->handle_http_request(method, this_object(),
					  root, url, args)) {
	    /* if it did, we're done */
	    return;
	 }
	 /* else it might want to remap the URL and leave rest to us */
	 val = handler->remap_http_request(root, url, args);

	 if (val && file_size(val) >= 0) {
	    if (suffix = path_suffix(val)) {
	       suffix = lower_case(suffix);
	       if (handler = HTTPD->query_suffix_handler(suffix)) {
		  Debug("File " + val + " suffix handled by " +
			ur_name(handler));
		  handler->handle_http_file(method, this_object(),
					    val, suffix, args);
		  return;
	       }
	    }
	 }
	 switch(suffix) {
	 case "html": case "htm":
	    type = "text/html"; break;
	 case "jpg":
	    type = "image/jpeg"; break;
         case "css":
            type = "text/css"; break;
	 case "gif":
	    type = "image/gif"; break;
	 case "png":
	    type = "image/png"; break;
	 case "text": case "txt":
	    type = "text/plain"; break;
	 case "class":
	    type = "application/x-java"; break;
	 case "xpi":
	    type = "application/x-xpinstall"; break;
	 case "xul":
	    type = "application/vnd.mozilla.xul+xml"; break;
	 default:
	    type = "application/octet-stream"; break;
	 }
	 send_file(200, val, type);
	 return;
      }
   } else {
      /* A few special cases: */
      switch (url) {
      case "/robots.txt":
	 send_file(200, "/usr/HTTP/data/www/robots.txt", "text/plain");
	 return;
      case "/favicon.ico":
	 send_file(200, "/usr/HTTP/data/www/favicon.ico", "text/plain");
	 return;
      case "/crossdomain.xml":
	 send_file(200, "/usr/HTTP/data/www/crossdomain.xml", "text/xml");
	 return;
      default:
	 break;
      }
   }
   clear_output();
   send_html(404, "<body>I cannot find the file [" + orig + "]</body>\n");
}


/*
**	This function is called up from ~/lib/output_buffer when DGD
**	has finished sending data in non-blocking mode. Because we
**	currently demand that all data is sent to this user object in
**	a single thread, we know it's all been sent when we get this
**	call, and can thus simply disconenct with an easy conscience.
*/

static
void message_sent() {
   disconnect();
}

string query_header(string field) {
   return headers ? headers[field] : nil;
}

void disconnect() {
   ::disconnect();
}
