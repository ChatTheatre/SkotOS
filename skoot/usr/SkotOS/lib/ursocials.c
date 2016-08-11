/*
**	/usr/SkotOS/lib/ursocials.c
**
**	Define the ur-calls for social verbs and advcats.
**
**	Copyright Skotos Tech Inc 1999 - 2003
*/

# include <SAM.h>
# include <urcalls.h>

R0(int, ur_evoke, query_evoke)
R0(int, ur_secrecy, query_secrecy)
R0(int, ur_privacy, query_privacy)
R0(int, ur_obscurity, query_obscurity)
R0(int, ur_target_abstracts, query_target_abstracts)
R0(string, ur_log_evokes, query_log_evokes)
R0(string, ur_action, query_action)
O0(string *, ur_actions, query_actions)

R0(int, ur_audible, query_audible)
O0(string *, ur_roles, query_roles)

R0(int, ur_disabled, query_disabled)
R0(int, ur_ooc, query_ooc)
R0(int, ur_raw_verb, query_raw_verb)

/* per-role stuff */
R1(int, ur_requirement, query_requirement, string)
R1(int, ur_distance, query_distance, string)
O1(string *, ur_prepositions, query_prepositions, string)
R1(int, ur_direct, query_direct, string)
R1(int, ur_raw, query_raw, string)
R1(int, ur_single, query_single, string)
R1(object, ur_filter_object, query_filter_object, string)
