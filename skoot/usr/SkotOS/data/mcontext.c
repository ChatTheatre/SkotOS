/*
**
**	/usr/SkotOS/data/mcontext.c
**
**	We contain the state required for a delayed run blah blah blah.
**
**	Copyright (C) Skotos Tech 2001
*/

# include <status.h>
# include <SkotOS.h>

private inherit "/lib/string";
private inherit "/usr/SkotOS/lib/merryapi";

mapping args;

string mode;
string signal;
string label;

atomic
void configure(string m, string s, string l, mapping a) {
   mode = lower_case(m);
   signal = lower_case(s);
   label = l;
   args = a;
}

void
merry_continuation(object this) {
   int ticks;

   ticks = status()[ST_TICKS];
   run_merries(this, signal, mode, args, label);

   /* TODO: update find_merry_location() to a multi-script version */
   catch {
      object home;
      home = find_merry_location(this, signal, mode);
      if (home) {
	 SYS_MERRY->update_resource(home, signal, mode, label,
				    ticks - status()[ST_TICKS]);
      }
   }
}

string describe_delayed_call(string fun, mixed *args, mixed delay) {
   return "Merry $delay(" + delay + ", " + label + ") on " +
      name(args[0]) + "/" + mode + ":" + signal;
}
