/*
**	/include/urcalls.h
**
**	The first attempt at an ur-object implementation for Skoot.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <nref.h>

# define Eval(n)	(n)


/* this is not used */
private mixed eval(mixed val) {
   if (typeof(val) == T_OBJECT && val->query_merry()) {
      return val->evaluate( ([ ]) );
   }
   return val;
}


/* addition behaviour: query() + ur->query() */

# define A0(t, u, q) \
/* static */ t u (object ob) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q() + u(ob->query_ur_object())); \
   }\
   return Eval(ob->q()); \
}

# define A1(t, u, q, t1) \
/* static */ t u (object ob, t1 a1) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1) + u(ob->query_ur_object(), a1)); \
   }\
   return Eval(ob->q(a1)); \
}

# define A2(t, u, q, t1, t2) \
/* static */ t u (object ob, t1 a1, t2 a2) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1, a2) + u(ob->query_ur_object(), a1, a2)); \
   }\
   return Eval(ob->q(a1, a2)); \
}

# define A3(t, u, q, t1, t2, t3) \
/* static */ t u (object ob, t1 a1, t2 a2, t3 a3) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1, a2, a3) + u(ob->query_ur_object(), a1, a2, a3)); \
   }\
   return Eval(ob->q(a1, a2, a3)); \
}

/* or behaviour: query() | ur->query() */

# define O0(t, u, q) \
/* static */ t u (object ob) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q() | u(ob->query_ur_object())); \
   }\
   return Eval(ob->q()); \
}

# define O1(t, u, q, t1) \
/* static */ t u (object ob, t1 a1) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1) | u(ob->query_ur_object(), a1)); \
   }\
   return Eval(ob->q(a1)); \
}

# define O2(t, u, q, t1, t2) \
/* static */ t u (object ob, t1 a1, t2 a2) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1, a2) | u(ob->query_ur_object(), a1, a2)); \
   }\
   return Eval(ob->q(a1, a2)); \
}

# define O3(t, u, q, t1, t2, t3) \
/* static */ t u (object ob, t1 a1, t2 a2, t3 a3) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1, a2, a3) | u(ob->query_ur_object(), a1, a2, a3)); \
   }\
   return Eval(ob->q(a1, a2, a3)); \
}


/* multiplication behaviour: query() * ur->query() */

# define M0(t, u, q) \
/* static */ t u (object ob) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q() * u(ob->query_ur_object())); \
   }\
   return Eval(ob->q()); \
}

# define M1(t, u, q, t1) \
/* static */ t u (object ob, t1 a1) { \
   if (ob->query_ur_object()) { \
      return Eval(ob->q(a1) * u(ob->query_ur_object(), a1)); \
   }\
   return Eval(ob->q(a1)); \
}


/* replacement behaviour: query() ? query() : ur->query() */
/* this almost certainly must be replaced with a colour check */

# define R0(t, u, q) \
/* static */ t u (object ob) { \
   if (ob) { \
      mixed val; \
      if ((val = ob->q())) {\
         return Eval(val); \
      } \
      return Eval(u(ob->query_ur_object())); \
   }\
}

# define R1(t, u, q, t1) \
/* static */ t u (object ob, t1 a1) { \
   if (ob) { \
      mixed val; \
      if ((val = ob->q(a1))) {\
         return Eval(val); \
      } \
      return Eval(u(ob->query_ur_object(), a1)); \
   }\
}

# define R2(t, u, q, t1, t2) \
/* static */ t u (object ob, t1 a1, t2 a2) { \
   if (ob) { \
      mixed val; \
      if ((val = ob->q(a1, a2))) {\
         return Eval(val); \
      } \
      return Eval(u(ob->query_ur_object(), a1, a2)); \
   }\
}

/* these macros will define prototypes for urcalls */
# define P0(t, u, q) t u (object ob);
# define P1(t, u, q, t1)  t u (object ob, t1 a1);
# define P2(t, u, q, t1, t2) t u (object ob, t1 a1, t2 a2);
