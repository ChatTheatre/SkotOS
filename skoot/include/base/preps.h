/*
**	/include/base/preps.h
**
**	Archetypal preposition categories: bitmask values used
**	in /base/lib/detail.c
**
**	Copyright Skotos Tech Inc 1999
*/


/* intimate proxes (deniable consent) */

# define PREP_CLOSE_TO	0x01	/* close to, close by, close beside */
# define PREP_AGAINST	0x02	/* against */
# define PREP_UNDER	0x04	/* under, beneath, below */
# define PREP_ON	0x08	/* on, upon, atop, on top of, across (?) */
# define PREP_INSIDE	0x10	/* within, in, inside */

/* near proxes (delayable consent) */

# define PREP_NEAR	0x0100	/* near, by, nearby, at */
# define PREP_OVER	0x0200	/* over, above */
# define PREP_BEHIND	0x0400	/* behind */
# define PREP_BEFORE	0x0800	/* in front of, before */
# define PREP_BESIDE	0x1000	/* beside, next to */
