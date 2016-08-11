/*
**	/data/n2n.c
**
**	Maintain an N-N bigmap relationship, indexed both ways.
**
**	Copyright Skotos Tech Inc 2003
*/

private inherit "/lib/type";

object a2b, b2a;

static
void configure(object map1, object map2) {
   if (!map1 || !map2) {
      error("bad (null) argument");
   }
   a2b = map1;
   b2a = map2;
}

atomic
void add_association(mixed a, mixed b, varargs int head) {
   mixed *arr, *brr;

   brr = Arr(a2b->get(a));
   arr = Arr(b2a->get(b));

   if (head) {
      arr = ({ a }) | arr;
      brr = ({ b }) | brr;
   } else {
      arr = arr | ({ a });
      brr = brr | ({ b });
   }
   a2b->set(a, brr);
   b2a->set(b, arr);
}

atomic
void remove_association(mixed a, mixed b) {
   mixed *arr, *brr;

   brr = Arr(a2b->get(a));
   arr = Arr(b2a->get(b));

   brr -= ({ b });
   arr -= ({ a });

   a2b->set(a, sizeof(brr) ? brr : nil);
   b2a->set(b, sizeof(arr) ? arr : nil);
}

mixed *get_associates(mixed a) {
   return a2b->get(a);
}

mixed *get_associators(mixed b) {
   return b2a->get(b);
}

object query_a2b() { return a2b; }
object query_b2a() { return b2a; }
