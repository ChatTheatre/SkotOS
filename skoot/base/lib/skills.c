/*
 * /base/lib/skills.c
 *
 * An API for some of the <skill_handler>->skill_* functions.
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <System.h>

# define SKILL_HANDLER "Base:Skill:Handler"

/* Returns TRUE if a skill by that name exists at all, FALSE otherwise. */
int
skill_exists(string name)
{
    string handler;

    handler = CONFIGD->query(SKILL_HANDLER);
    return handler ? handler->skill_exists(name) : FALSE;
}

/* Returns the value of a skill, or 0 if no handler exists */
int
skill_value(string name)
{
    string handler;

    handler = CONFIGD->query(SKILL_HANDLER);
    return handler ? handler->skill_value(this_object(), name) : 0;
}

/* Describe the value of the current skill-value */
string
skill_description(string name)
{
    int    value;
    string handler;

    handler = CONFIGD->query(SKILL_HANDLER);
    value = handler ? handler->skill_value(this_object(), name) : 0;
    return handler ? handler->skill_description(value) : nil;
}
