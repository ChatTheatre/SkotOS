/*
 * /usr/XML/data/samref.c
 *
 * LWO for SAM-reference data.
 *
 * Copyright Skotos Tech Inc 2006
 */

mixed data;

int is_samref() { return TRUE; }

static
void configure(mixed d) {
   data = d;
}

mixed query_configuration() {
   return data;
}

void set_data(mixed d) {
   data = d;
}

mixed query_data() {
   return data;
}
