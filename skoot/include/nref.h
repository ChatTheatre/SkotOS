# include <type.h>
# include <DTD.h>

# define NRef			object
# define NRefOb(n)		(n)->query_object()
# define NRefDetail(n)		(n)->query_detail()
# define NewNRef(o, d)		new_object("/base/data/nref", o, d)
# define CopyNRef(r)		NewNRef(NRefOb(r), NRefDetail(r))

# define IsNRef(n)		(typeof(n) == T_OBJECT && (n)->is_nref())

# define NRefAmount(n)		(n)->query_amount()

# define NewMNRef(o, d, a)	new_object("/base/data/nref", o, d, a)
# define MutatedNRef(n, a)	new_object("/base/data/nref", (n)->query_object(), (n)->query_detail(), a)

# define ChipMNRef(n)		((n)->chip())

# define NRefsEqual(a, b)	((IsNRef(a) && IsNRef(b)) ? (a)->equals(b) : FALSE)

# define IsPrime(ref)		(ur_prime(NRefOb(ref), NRefDetail(ref)))

/* DTD definitions */

# define THING_NREF		"thing_nref"
# define COL_NREF		43
