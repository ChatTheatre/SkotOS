/*
**	~Skotos/obj/lognode.c
**
**	Maintain the log information for one assignee/tag combination.
**
**********************************************************************
**	001031 Zell	Initial Revision	(Happy Halloween!)
**
**	Copyright Skotos Tech Inc 1999
*/

# include <limits.h>
# include <mapargs.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>

private inherit "/lib/string";
private inherit "/lib/date";
private inherit "/lib/array";
private inherit "/lib/mapargs";
private inherit "/lib/url";

string tag, assignee;

mapping ix_to_head;
mapping ix_to_desc;
mapping ix_to_stamp;

string query_state_root() { return "Log:LogNode"; }

static
void create(int clone) {
   if (clone) {
      ix_to_head = ([ ]);
      ix_to_desc = ([ ]);
      ix_to_stamp = ([ ]);
   }
}

void clear() {
   ix_to_head = ([ ]);
   ix_to_desc = ([ ]);
   ix_to_stamp = ([ ]);
}

void patch() {
   if (assignee && tag) {
      assignee = implode(explode(assignee, ":"), ".");
      tag = implode(explode(tag, ":"), ".");
      set_object_name("SkotOS:LogNodes:" + assignee + ":" + tag);
      "/usr/SkotOS/sys/logd"->register(this_object());
   }
}


atomic
void initialize(string t, string a) {
   if (assignee || tag) {
      error("object already initialized");
   }
   if (sscanf(a, "%*s:") || sscanf(t, "%*s:")) {
      error("neither assignee nor tag may contain a colon");
   }
   assignee = a;
   tag = t;

   set_object_name("SkotOS:LogNodes:" + a + ":" + t);
}


atomic
void del_entry(int ix) {
   ix_to_stamp[ix] = nil;
   ix_to_head[ix] = nil;
   ix_to_desc[ix] = nil;
   if (!map_sizeof(ix_to_stamp) &&
       !map_sizeof(ix_to_head) &&
       !map_sizeof(ix_to_desc)) {
      call_out("maybe_suicide", 0);
   }
   SysLog(tag + "/" + assignee + ": deleted ix " + ix);
}

static
void maybe_suicide() {
   if (!map_sizeof(ix_to_stamp) &&
       !map_sizeof(ix_to_head) &&
       !map_sizeof(ix_to_desc)) {
      destruct_object();
   }
}

void die() {
   destruct_object();
}


atomic
void set_entry(string t, string a, int ix, int stamp, string head, SAM desc) {
   object node;

  if (t == tag && a == assignee) {
      /* write data */
      ix_to_stamp[ix] = stamp;
      ix_to_head[ix] = head;
      ix_to_desc[ix] = desc;
      return;
   }
   del_entry(ix);

   /* it needs to go elsewhere */
   node = LOGD->get_node(t, a);
   if (node == this_object()) {
      /* can't happen */
      error("found myself unexpected");
   }
   node->set_entry(t, a, ix, stamp, head, desc);
}

string query_assignee() { return assignee; }
string query_tag() { return tag; }

int *get_ixset() {
   return map_indices(ix_to_stamp);
}

SAM query_description(int ix) {
   return ix_to_desc[ix];
}

void set_description(int ix, string desc) {
   /* make sure this is an existing entry */
   if (!ix_to_stamp[ix]) {
      error("unknown entry index " + ix);
   }
   ix_to_desc[ix] = desc;
}

string query_header(int ix) {
   return ix_to_head[ix];
}

void set_header(int ix, string head) {
   /* make sure this is an existing entry */
   if (!ix_to_stamp[ix]) {
      error("unknown entry index " + ix);
   }
   ix_to_head[ix] = head;
}

int query_stamp(int ix) {
   return ix_to_stamp[ix];
}

string query_readable_stamp(int ix) {
   return short_time(ix_to_stamp[ix]);
}


mapping query_ix_to_head() {
   return ix_to_head[..];
}

mapping query_ix_to_desc() {
   return ix_to_desc[..];
}

mapping query_ix_to_stamp() {
   return ix_to_stamp[..];
}
