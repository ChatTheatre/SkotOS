/*
**	~System/initd.c
**
**	This object is loaded by the kernel library after a cold boot,
**	which means the rest of the system must be loaded from here.
**
**	Before the driver goes down, prepare_reboot() is called here
**	and after a warm boot, reboot() is called. This is where we
**	e.g. re-open network ports.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <status.h>

# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <kernel/tls.h>

# include <System.h>
# include <System/log.h>

inherit access API_ACCESS;
inherit rsrc API_RSRC;
inherit tls API_TLS;

# include "/usr/System/lib/find_or_load.c"
# include "/lib/loader.c"

# define DUMP_INTERVAL	200	/* very frequently, for now */

string hostname;
int portbase, textport;
int dump;
string access_code;
int memory_high, memory_max;
int statedump_offset;
int real_textport, webport, real_webport;
string url_protocol;
string freemote;
string helpverb;
string login_hostname;

void get_instance();

static
void create() {
   access::create();
   rsrc::create();
   tls::create();

   /* give System full access */
   add_user("System");
   set_access("System", "/", FULL_ACCESS);
   /* allow everyone to read us (especially ~/include/std.h) */
   set_global_access("System", TRUE);

   add_user("SkotOS");
   add_owner("SkotOS");
   set_global_access("SkotOS", TRUE);

   add_user("DTD");
   add_owner("DTD");
   set_global_access("DTD", TRUE);

   add_user("XML");
   add_owner("XML");
   set_global_access("XML", TRUE);

   add_user("SAM");
   add_owner("SAM");
   set_global_access("SAM", TRUE);

   add_user("SID");
   add_owner("SID");
   set_global_access("SID", TRUE);

   /* Added by shentino during cold boot work */
   add_user("SMTP");
   add_owner("SMTP");
   set_global_access("SMTP", TRUE);

   add_user("Tool");
   add_owner("Tool");
   set_global_access("Tool", TRUE);

   set_rsrc("ticks",	   900000000, 0, 0);
   set_rsrc("stack",	   250, 0, 0);

   set_tls_size(4);
   call_out("continue_create", 0);
}

static
void continue_create() {
   /* start fundamental services */

   find_or_load(TLSD);
   find_or_load(METRICSD);
   find_or_load(SYSLOGD);
   find_or_load(PROGDB);
   find_or_load(SYS_TEXTDATA);
   find_or_load(SYS_TD_ESCAPE);
   find_or_load(SYS_TD_UNESCAPE);
   find_or_load(SYS_URL_ENCODE);
   find_or_load(SYS_URL_DECODE);
   find_or_load(IDD);

   find_or_load(MODULED);
   find_or_load(OUTBOUND);
   find_or_load(TOUCHD);

   get_instance();

   find_or_load(CONFIGD);

   find_or_load("/usr/SAM/sys/sam");
   find_or_load("/usr/SID/sys/vault");
   find_or_load("/base/initd");

   find_or_load("/base/sys/actions");
   find_or_load("/base/sys/properties");
   find_or_load("/base/sys/base");
   find_or_load("/base/sys/details");

   find_or_load("/base/obj/error");
   find_or_load("/base/obj/output");
   find_or_load("/base/obj/thing");

   find_or_load("/core/sys/registry");
   find_or_load("/core/sys/handler");
   find_or_load("/core/sys/notes");

   find_or_load("/usr/DTD/sys/dtd");
   find_or_load("/usr/XML/sys/xml");

   find_or_load("/usr/SID/sys/rescue");
   find_or_load("/usr/SID/sys/sid");

   find_or_load("/usr/SkotOS/obj/meracha");
   find_or_load("/usr/SkotOS/obj/mersamtag");
   find_or_load("/usr/SkotOS/obj/verb");

   find_or_load("/usr/SkotOS/sys/bilbo");
   find_or_load("/usr/SkotOS/sys/merry");

   /* Discovered by shentino during cold boot work */
   load_programs("/data");
   load_programs("/obj");

   find_or_load(SYS_BOOT);

   find_or_load("/usr/System/sys/tool/vault");

   call_out("boot", 0);

   call_out("memory_watcher", 1, FALSE);

   dump = call_out("perform_statedump",
		   DUMP_INTERVAL - (time() % DUMP_INTERVAL) +
		   statedump_offset);
}

void patch() {
   tls::create();
   set_tls_size(3);
}

static
void boot() {
   SYS_BOOT->boot();
}

void prepare_reboot() {
   if (previous_program() == DRIVER) {
      SYS_BOOT->prepare_reboot();
   }
}

void reboot() {
   if (previous_program() == DRIVER) {
      if (dump) {
	 remove_call_out(dump);
      }
      dump = call_out("perform_statedump",
		      DUMP_INTERVAL - (time() % DUMP_INTERVAL) +
		      statedump_offset);

      get_instance();
      PROGDB->reboot();
      SYS_BOOT->reboot();
   }
}

static
void perform_statedump() {
# if 0
   INFO("Performing full swapout...");
   swapout();
# endif
   INFO("Saving global state...");
   dump_state();
   dump = call_out("perform_statedump",
		   DUMP_INTERVAL - (time() % DUMP_INTERVAL) +
		   statedump_offset);
   call_out("signal_completion", 0);
}

void
reschedule_statedump()
{
   if (dump) {
      remove_call_out(dump);
   }
   dump = call_out("perform_statedump",
		   DUMP_INTERVAL - (time() % DUMP_INTERVAL) +
		   statedump_offset);
}

void foo() {
   perform_statedump();
}

static
void signal_completion() {
   INFO("Global state save complete.");
}

private
string lower_case(string str) {
    int i;

    for (i = 0; i < strlen(str); i ++) {
       if (str[i] >= 'A' && str[i] <= 'Z') {
	  str[i] = (str[i] - 'A') + 'a';
       }
    }
    return str;
}

int query_portbase() {
   return portbase;
}

int query_textport() {
   return textport ? textport : portbase;
}

int query_real_textport() {
   return real_textport ? real_textport : query_textport();
}

int query_webport() {
   return webport ? webport : portbase + 80;
}

int query_real_webport() {
   return real_webport ? real_webport : query_webport();
}

string query_url_protocol() {
   return url_protocol;
}

string query_freemote() {
   return freemote ? freemote : "act";
}

string query_helpverb() {
   return helpverb ? helpverb : "@help";
}

string query_instance() {
   return "local";
}

string query_hostname() {
   return hostname;
}

string query_login_hostname() {
    return login_hostname;
}

string query_userdb_hostname(varargs int force) {
   string text;

   if (text = read_file(USERDB_FILE)) {
      if (sscanf(lower_case(text), "%*suserdb-hostname %s\n", text)) {
	 return text;
      }
   }
   if (force) {
      error("userdb-hostname does not exist in ~System/data/userdb");
   }
}

int query_userdb_portbase(varargs int force) {
   string text;
   int val;

   if (text = read_file(USERDB_FILE)) {
      if (sscanf(lower_case(text), "%*suserdb-portbase %d\n", val)) {
	 return val;
      }
   }
   if (force) {
      error("userdb-portbase does not exist in ~System/data/userdb");
   }
   return 0;
}

string *query_boot_modules() {
   string text;

   if (text = read_file(INSTANCE_FILE)) {
      if (!sscanf(text, "%*sbootmods %s\n", text)) {
	 return ({ });
      }
      return explode(text, " ");
   }
}

string query_access_code() {
   return access_code;
}

int query_memory_high() {
   return memory_high ? memory_high : 128;
}

int query_memory_max() {
   return memory_max ? memory_max : 256;
}

void get_instance() {
   string before, after, text;

   textport = 0;
   real_textport = 0;
   webport = 0;
   real_webport = 0;
   url_protocol = "http";
   freemote = nil;
   helpverb = nil;
   access_code = nil;
   memory_high = 0;
   memory_max = 0;
   statedump_offset = 0;
   if (text = read_file(INSTANCE_FILE)) {
      /* remove all # lines */
      while (sscanf(text, "%s#%*s\n%s", before, after)) {
	 text = before + after;
      }
      /* find data */
      if (!sscanf(text, "%*sportbase %d\n", portbase)) {
	 error(INSTANCE_FILE + " lacks a 'portbase 8000' style line");
      }
      DEBUG("Instance file: portbase is " + portbase);
      if (!sscanf(text, "%*shostname %s\n", hostname)) {
	 error(INSTANCE_FILE + " lacks a 'hostname localhost' style format");
      }
      DEBUG("Instance file: hostname is " + hostname);
      sscanf(text, "%*stextport %d\n", textport);
      sscanf(text, "%*sreal_textport %d\n", real_textport);
      sscanf(text, "%*swebport %d\n", webport);
      sscanf(text, "%*sreal_webport %d\n", real_webport);
      sscanf(text, "%*surl_protocol %s\n", url_protocol);
      sscanf(text, "%*saccess %s\n", access_code);
      sscanf(text, "%*smemory_high %d\n", memory_high);
      sscanf(text, "%*smemory_max %d\n",  memory_max);
      sscanf(text, "%*sstatedump_offset %d\n", statedump_offset);
      sscanf(text, "%*sfreemote %s\n", freemote);
      sscanf(text, "%*shelpverb %s\n", helpverb);
      sscanf(text, "%*slogin_hostname %s\n", login_hostname);
      return;
   }
   error("missing file " + INSTANCE_FILE);
}   


void shutdown() {
   ::shutdown();
}

# define MEGA (1024 * 1024)

void
memory_watcher(int high, varargs int last_used, int last_allocated)
{
   int used, allocated;

   used      = status()[ST_SMEMUSED] + status()[ST_DMEMUSED];
   allocated = status()[ST_SMEMSIZE] + status()[ST_DMEMSIZE];

   if (allocated > query_memory_max() * MEGA) {
       DEBUG("memory_watcher: Max allocation triggered swapout: " +
	     ((used + MEGA - 1) / MEGA) + " MB in use; " + ((allocated + MEGA - 1) / MEGA) + " MB allocated");
       high = FALSE;
       swapout();
   } else {
       if (!high && allocated > query_memory_high() * MEGA) {
	   high = TRUE;
       }
       if (high && used / 2 < allocated / 3) {
	   DEBUG("memory_watcher: Triggered swapout: " +
		 ((used + MEGA - 1) / MEGA) + " MB in use; " + ((allocated + MEGA - 1) / MEGA) + " MB allocated");
	   high = FALSE;
	   swapout();
       }
   }
   call_out("memory_watcher", 1, high, used, allocated);
}
