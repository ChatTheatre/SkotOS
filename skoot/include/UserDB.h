# define USERDB		"/usr/UserDB/sys/userd"
# define GAMEDB		"/usr/UserDB/sys/gamedb"
# define STATSDB        "/usr/UserDB/sys/stats"
# define BILLINGD       "/usr/UserDB/sys/billing"
# define POINTSDB       "/usr/UserDB/sys/points"
# define PROMOTIONS     "/usr/UserDB/sys/promotions"

# define SEARCHDB       "/usr/UserDB/sys/searchdb"
# define SEARCHNODE     "/usr/UserDB/sys/searchnode"

# define BROADCAST      "/usr/UserDB/sys/broadcast"
# define EMAILD         "/usr/UserDB/sys/email"

/*
 * The possible account status values:
 */

# define AS_REGULAR        0
# define AS_TRIAL          1
# define AS_FREE           2
# define AS_DEVELOPER      3
# define AS_STAFF          4
# define AS_TYPE_MASK      7

/*
 * These can be combined (bitwise or'd) with:
 */

# define AS_BANNED   (1 << 16)
# define AS_DELETED  (1 << 17)
# define AS_NO_EMAIL (1 << 18)
# define AS_PREMIUM  (1 << 19)
# define AS_GRAND    (1 << 20)

/*
 * Convenience:
 */

# define IS_REGULAR(_as)   ((_as & AS_TYPE_MASK) == AS_REGULAR  )
# define IS_TRIAL(_as)     ((_as & AS_TYPE_MASK) == AS_TRIAL    )
# define IS_FREE(_as)      ((_as & AS_TYPE_MASK) == AS_FREE     )
# define IS_DEVELOPER(_as) ((_as & AS_TYPE_MASK) == AS_DEVELOPER)
# define IS_STAFF(_as)     ((_as & AS_TYPE_MASK) == AS_STAFF    )

# define IS_BANNED(_as)    ((_as & AS_BANNED)    == AS_BANNED  )
# define IS_DELETED(_as)   ((_as & AS_DELETED)   == AS_DELETED )
# define IS_NO_EMAIL(_as)  ((_as & AS_NO_EMAIL)  == AS_NO_EMAIL)
# define IS_PREMIUM(_as)   ((_as & AS_PREMIUM)   == AS_PREMIUM )
# define IS_GRAND(_as)     ((_as & AS_GRAND)     == AS_GRAND   )

/*
 * History definitions:
 */
# define BH_AUTH      0 /* Card (pre)authenticated                         */
# define BH_CANCEL    1 /* Card canceled/cleared                           */
# define BH_CHARGE    2 /* Card charged                                    */
# define BH_EXTERNAL  3 /* External payment made                           */
# define BH_EXPIRE    4 /* Account expired (4th sale failure or otherwise) */
# define BH_CREATED   5 /* Account created                                 */
# define BH_BUMPED    6 /* Dueday bumped                                   */
# define BH_AFAILURE  7 /* Authentication failure                          */
# define BH_SFAILURE  8 /* Sale failure                                    */
# define BH_CAUTH     9 /* Clearing AUTH OID                               */
# define BH_TEXT     10
# define BH_CREDIT   11
# define BH_FEE      12
# define BH_UPGRADE  13
# define BH_UFAILURE 14
# define BH_DEBIT    15