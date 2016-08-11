/*
**	/usr/SkotOS/data/ssnugget.c
**
**	A little LWO that keeps one nugget of information regarding
**	modifiable attributes for /usr/SkotOS/obj/startstory.c
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";

string property;
string description;
mapping choices;

static
void create(int clone) {
   if (clone) {
      choices = ([ ]);
   }
}

static
void configure(string str) {
   if (previous_program() == SYS_AUTO) {
      property = str;
   }
}

void set_property(string str) {
   property = lower_case(str);
}

string query_property() {
   return property;
}


void set_description(string str) {
   description = str;
}

string query_description() {
   return description;
}


void set_choice(string id, string name) {
   choices[lower_case(id)] = name;
}

string query_choice_name(string id) {
   return choices[lower_case(id)];
}

string *query_choices() {
   return map_indices(choices);
}

void clear_choices() {
   choices = ([ ]);
}

mapping query_nugget_mapping() {
   mapping *choice_chunks;
   string *arr;
   int i;

   arr = map_indices(choices);
   choice_chunks = allocate(sizeof(arr));

   for (i = 0; i < sizeof(arr); i ++) {
      choice_chunks[i]= ([
	 "id": arr[i],
	 "name": choices[arr[i]],
	 ]);
   }

   return ([
      "description": description,
      "property": property,
      "choices": choice_chunks,
      ]);
}
