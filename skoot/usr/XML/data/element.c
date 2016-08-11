/*
 * /usr/XML/data/element.c
 *
 * LWO for XML elements.
 *
 * Copyright Skotos Tech Inc 2006
 */

mixed data;

int is_element() { return TRUE; }

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
