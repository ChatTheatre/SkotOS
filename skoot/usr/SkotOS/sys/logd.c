/*
**	~Skotos/sys/logd.c
**
**	Maintain a database of e.g bug reports.
**
**********************************************************************
**	991104 Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999
*/

# include <limits.h>
# include <mapargs.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/mapargs";
private inherit "/lib/url";

int counter;

mapping	tag_nodesets;
mapping	assignee_nodesets;

string query_state_root() { return "Log:Log"; }

static
void create() {
   compile_object("/usr/SkotOS/obj/lognode");

   tag_nodesets = ([ ]);
   assignee_nodesets = ([ ]);

   set_object_name("SkotOS:Logs");

   counter = 8000;
}

atomic
object get_node(string tag, string assignee) {
   mapping a_set, t_set;
   object *nodes, node;

   a_set = assignee_nodesets[assignee];
   t_set = tag_nodesets[tag];

   if (!a_set) {
      a_set = ([ ]);
   }
   if (!t_set) {
      t_set = ([ ]);
   }

   nodes = map_indices(a_set) & map_indices(t_set);

   if (!sizeof(nodes)) {
      SysLog("Creating node for " + assignee + "/" + tag);
      node = clone_object(LOG_NODE);
      node->initialize(tag, assignee);

      a_set[node] = node;
      t_set[node] = node;

      assignee_nodesets[assignee] = a_set;

      tag_nodesets[tag] = t_set;

      return node;
   }
   return nodes[0];
}

void register(object ob) {
   string tag, ass;

   tag = ob->query_tag();
   ass = ob->query_assignee();

   if (!tag || !ass) {
      return;
   }

   if (!assignee_nodesets[ass]) {
      assignee_nodesets[ass] = ([ ]);
   }
   assignee_nodesets[ass][ob] = ob;

   if (!tag_nodesets[tag]) {
      tag_nodesets[tag] = ([ ]);
   }
   tag_nodesets[tag][ob] = ob;
}


object query_node(string tag, string assignee) {
   mapping a_set, t_set;
   object *nodes;

   if (a_set = assignee_nodesets[assignee]) {
      if (t_set = tag_nodesets[tag]) {
	 nodes = map_indices(a_set) & map_indices(t_set);
	 if (sizeof(nodes)) {
	    return nodes[0];
	 }
      }
   }
   return nil;
}

atomic
void add_entry(string tag, string head, SAM desc) {
   string assign;

   assign = "unassigned-" + (counter / 32);
   get_node(tag, assign)->set_entry(tag, assign, counter, time(), head, desc);
   counter ++;
}


mapping query_assignee_nodesets() {
   return assignee_nodesets[..];
}

mapping query_tag_nodesets() {
   return tag_nodesets[..];
}

string *query_assignees() {
   return map_indices(assignee_nodesets);
}

string *query_tags() {
   return map_indices(tag_nodesets);
}

string *query_assignee_tags(string assignee) {
   mapping set;
   object *foo;
   string *tags;
   int i;

   if (set = assignee_nodesets[assignee]) {
      foo = map_indices(set);
      tags = allocate(sizeof(foo));
      for (i = 0; i < sizeof(foo); i ++) {
	 tags[i] = foo[i]->query_tag();
      }
      return tags;
   }
   return ({ });
}
