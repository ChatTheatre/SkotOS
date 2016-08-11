/*
 * ~SMTP/initd.c
 *
 * Copyright Skotos Tech 2002-2006
 */

# include <SMTP.h>

static void create()
{
    find_or_load(SMTP_SYS);
    find_or_load(SMTP_API);
}
