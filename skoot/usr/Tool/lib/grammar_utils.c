/*
 * ~Tool/lib/grammar_utils.c
 *
 * Parser token recognition.
 *
 * Copyright Skotos Tech Inc 2002.
 */

private inherit "/lib/string";

private mapping tokens;

static void
initialize_tokens(mapping map)
{
   tokens = map;
}

static void
initialize()
{
}

static int
prefix_match(string prefix, string full)
{
    int len;
    string prefix1;

    if (!tokens) {
       initialize();
    }
    prefix = lower_case(prefix);
    if (prefix == full) {
       return TRUE;
    }
    len = strlen(prefix);
    if (len > strlen(full) ||  full[..len - 1] != prefix) {
        return FALSE;
    }
    prefix1 = prefix;
    prefix1[len - 1]++;
    return map_sizeof(tokens[prefix..prefix1] - ({ prefix1 })) == 1;
}
