/*
**	/base/data/nref.c
**
**	This is the LWO version of NRefs. Hurray!
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <nref.h>

private inherit "/lib/mapping";
private inherit "/base/lib/urcalls";

string	det;
object	obj;
mixed	amt;

int is_nref() { return TRUE; }

static
void configure(object o, string d, varargs mixed a) {
   obj = o;
   det = d;
   if (a != nil) {
      amt = a;
   }
}

mixed *query_configuration() {
   if (amt == nil) {
      return ({ obj, det });
   }
   return ({ obj, det, amt });
}
      

object query_object() {
   return obj;
}

string query_detail(varargs object looker) {
   return det;
}

int is_prime() {
   return ur_prime(obj, det);
}

int is_mutated() {
   return typeof(amt) == T_FLOAT;
}

float query_amount() {
   if (typeof(amt) == T_FLOAT) {
      return amt;
   }
   return obj->query_amount();
}

object chip() {
   if (!is_prime()) {
      error("nref is not prime");
   }
   if (!is_mutated()) {
      return query_object();
   }
   return query_object()->chip_combinable(query_amount());
}


int equals(NRef b) {
   return query_object() == NRefOb(b) &&
      query_detail() == NRefDetail(b) &&
      query_amount() == NRefAmount(b);
}

object query_environment() {
   if (is_prime()) {
      return obj->query_environment();
   }
   return obj;
}

mixed query_property(string prop) {
   if (sscanf(prop, "detail:%s", prop)) {
      return query_object()->query_property("details:" + query_detail() +
					    ":" + prop);
   }
   return nil;
}

mixed set_property(string prop, mixed val) {
   if (sscanf(prop, "detail:%s", prop)) {
      return query_object()->set_property("details:" + query_detail() +
					  ":" + prop,
					  val);
   }
}
