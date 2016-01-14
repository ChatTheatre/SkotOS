/*
**	~SkotOS/lib/canode.c
**
**	A cellular automata node. Performs a CA simulation. Whee.
**
**	(C) Copyright Skotos Tech Inc 2002
*/

# include <type.h>

private inherit "/lib/array";
private inherit "/lib/string";

inherit "/lib/notes";

# define COND_WRAP	0
# define COND_VALUE	1
# define COND_DIFF	2

void suicide() { call_out("destruct_object", 0); }

string query_state_root() { return "SkotOS:CANode"; }

void initialize();
void stop();
void start();

int **lattice;
int **query_lattice() { return lattice; }
void set_lattice(int ** x) { lattice = x; }

int width;
int query_width() { return width; }
void set_width(int x) {
   if (x < 2) {
      x = 2;
   }
   if (width != x) {
      width = x;
      initialize();
   }
}

int height;
int query_height() { return height; }
void set_height(int y) {
   if (y < 2) {
      y = 2;
   }
   if (height != y) {
      height = y;
      initialize();
   }
}

object rule;
object query_rule() { return rule; }
void set_rule(object x) { rule = x; }

int delay;
int query_delay() { return delay; }
void set_delay(int x) { delay = x ? x : 1; start(); }

private int left_condition;
private int left_value;

int query_left_condition() { return left_condition; }
void set_left_condition(int n) {left_condition = n; }

int query_left_value() { return left_value; }
void set_left_value(int n) { left_value = n; }

private int right_condition;
private int right_value;

int query_right_condition() { return right_condition; }
void set_right_condition(int n) { right_condition = n; }

int query_right_value() { return right_value; }
void set_right_value(int n) { right_value = n; }

private int top_condition;
private int top_value;

int query_top_condition() { return top_condition; }
void set_top_condition(int n) { top_condition = n; }

int query_top_value() { return top_value; }
void set_top_value(int n) { top_value = n; }

private int bottom_condition;
private int bottom_value;

int query_bottom_condition() { return bottom_condition; }
void set_bottom_condition(int n) { bottom_condition = n; }

int query_bottom_value() { return bottom_value; }
void set_bottom_value(int n) { bottom_value = n; }


private int handle;


static
void create(int clone) {
   ::create();

   height = 2;
   width = 2;
   delay = 60;	/* default is -very- slow */
   initialize();
   if (clone) {
      start();
   }
}

void stop() {
   if (handle) {
      remove_call_out(handle);
      handle = 0;
   }
}

void start() {
   stop();
   handle = call_out("iteration_beat", 0);
}

void initialize() {
   int y;

   lattice = allocate(height);
   for (y = 0; y < height; y ++) {
      lattice[y] = allocate_int(width);
   }
}


void randomize(varargs mixed val) {
   int x, y;

   initialize();
   for (y = 0; y < height; y ++) {
      for (x = 0; x < width; x ++) {
	 lattice[y][x] = random(typeof(val) == T_INT ? val : 256);
      }
   }
}


void set_point(int x, int y, int value) {
   if (x >= 0 && x < width && y >= 0 && y < height) {
      lattice[y][x] = value;
   }
}

int get_point(int x, int y) {
   if (x < 0) {
      if (left_condition == COND_WRAP) {
	 return get_point(width+x, y);
      }
      if (left_condition == COND_VALUE) {
	 return left_value;
      }
      if (left_condition == COND_DIFF) {
	 return get_point(0, y) + left_value;
      }
      error("unknown left condition: " + left_condition);
   }
   if (x >= width) {
      if (right_condition == COND_WRAP) {
	 return get_point(x-width, y);
      }
      if (right_condition == COND_VALUE) {
	 return right_value;
      }
      if (right_condition == COND_DIFF) {
	 return get_point(width-1, y) + right_value;
      }
      error("unknown right condition: " + right_condition);
   }
   if (y < 0) {
      if (top_condition == COND_WRAP) {
	 return lattice[height+y][x];
      }
      if (top_condition == COND_VALUE) {
	 return top_value;
      }
      if (top_condition == COND_DIFF) {
	 return lattice[0][x] + top_value;
      }
      error("unknown top condition: " + top_condition);
   }
   if (y >= height) {
      if (bottom_condition == COND_WRAP) {
	 return lattice[y-height][x];
      }
      if (bottom_condition == COND_VALUE) {
	 return bottom_value;
      }
      if (bottom_condition == COND_DIFF) {
	 return lattice[height-1][x] + bottom_value;
      }
      error("unknown bottom condition: " + bottom_condition);
   }
   return lattice[y][x];
}

static
void iterate() {
   mapping args;
   mixed val;
   int **new_val;
   int x, y;

   if (rule) {
      new_val = copy(lattice);
      for (y = 0; y < height; y ++) {
	 for (x = 0; x < width; x ++) {
	    args = ([ "cell": lattice[y][x],
		      "lattice": lattice,
		      "x": x,
		      "y": y,
		      "nn": get_point(x  , y-1),
		      "ne": get_point(x+1, y-1),
		      "ee": get_point(x+1, y  ),
		      "se": get_point(x+1, y+1),
		      "ss": get_point(x  , y+1),
		      "sw": get_point(x-1, y+1),
		      "ww": get_point(x-1, y  ),
		      "nw": get_point(x-1, y-1),
		      ]);
	    val = rule->evaluate(this_object(), nil, nil, args, nil);
	    if (typeof(val) == T_INT) {
	       if (val < 0) {
		  val = 0;
	       } else if (val > 0xFF) {
		  val = 0xFF;
	       }
	       new_val[y][x] = val;
	    }
	 }
      }
      lattice = new_val;
   }
}

static
void iteration_beat() {
   iterate();
   handle = call_out("iteration_beat", delay);
}


mixed query_property(string str) {
   int x, y;

   if (!str || !strlen(str)) {
      return nil;
   }
   str = lower_case(str);
   switch(str) {
   case "running":
      return !!handle;
   case "stopped":
      return !handle;
   case "width":
      return width;
   case "height":
      return height;
   case "delay":
      return delay;
   case "iteration-rule":
      return rule;
   case "left-condition":
      return left_condition;
   case "left-value":
      return left_value;
   case "right-condition":
      return right_condition;
   case "right-value":
      return right_value;
   case "top-condition":
      return top_condition;
   case "top-value":
      return top_value;
   case "bottom-condition":
      return bottom_condition;
   case "bottom-value":
      return bottom_value;
   }
   if (sscanf(str, "point(%d,%d)", x, y)) {
      return get_point(x, y);
   }
}

mapping query_properties(varargs int derived) {
   mapping out;
   string *props;
   int i;

   if (!derived) {
      return ([ ]);
   }

   props = ({
      "running",
	 "stopped",
	 "width",
	 "height",
	 "delay",
	 "iteration-rule",
	 "left-condition",
	 "left-value",
	 "right-condition",
	 "right-value",
	 "top-condition",
	 "top-value",
	 "bottom-condition",
	 "bottom-value",
	 });

   out = ([ ]);
   for (i = 0; i < sizeof(props); i ++) {
      out[props[i]] = query_property(props[i]);
   }
   return out;
}

string *query_property_indices(varargs int derived) {
   return map_indices(query_properties(derived));
}

void set_property(string str, mixed val) {
   int x, y;

   if (!str || !strlen(str)) {
      return;
   }
   str = lower_case(str);
   switch(str) {
   case "start":
      start();
      return;
   case "iterate":
      iterate();
      return;
   case "stop":
      stop();
      return;
   case "clear":
      initialize();
      return;
   case "randomize":
      randomize(val);
      return;
   case "width":
      set_width(val);
      return;
   case "height":
      set_height(val);
      return;
   case "delay":
      set_delay(val);
      return;
   case "iteration-rule":
      set_rule(rule);
      return;
   case "left-condition":
      set_left_condition(val);
      return;
   case "left-value":
      set_left_value(val);
      return;
   case "right-condition":
      set_right_condition(val);
      return;
   case "right-value":
      set_right_value(val);
      return;
   case "top-condition":
      set_top_condition(val);
      return;
   case "top-value":
      set_top_value(val);
      return;
   case "bottom-condition":
      set_bottom_condition(val);
      return;
   case "bottom-value":
      set_bottom_value(val);
      return;
   }
   if (sscanf(str, "point(%d,%d)", x, y)) {
      if (typeof(val) != T_INT) {
	 error("can only set lattice cells to integer values");
      }
      set_point(x, y, val);
   }
}
