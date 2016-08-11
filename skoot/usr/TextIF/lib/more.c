/*
**	/usr/TextIF/lib/more.c
**
**	Copyright Skotos Tech Inc 2000
*/

#include <SkotOS.h>

inherit "/lib/string";

mapping buffer;
int linecnt, cursor;

static
void create() {
   buffer = ([ ]);
   linecnt = cursor = 0;
}

void clear_more ( void ) {
   buffer = ([ ]);
   linecnt = cursor = 0;
}

void add_more ( string text ) {
   string *lines;
   int a;

   lines = explode( break_paragraph( "", text, 61 ), "\n" );
   for ( a=0; a<sizeof(lines); a++ ) {
      buffer[linecnt++] = lines[a];
   }
}

void show_more ( void ) {
   int a, showlines;
   object user, udat;

   user = this_object();
   udat = user->query_udat();

   if ( !linecnt ) {
      user->message("There is nothing more to read.\n");
	  return;
   }
  
   if ( udat->query_property("more")==nil ) {
      udat->set_property( "more", 18 );
   }
  
   showlines = cursor + udat->query_property("more");
   if ( showlines >= linecnt )
      showlines = linecnt;
   
   for ( a=cursor; a<showlines; a++ ) {
      user->message( buffer[a]+"\n" );
   }
   if ( showlines < linecnt-1 )
      user->message("[Type MORE for more]\n");
   cursor = cursor + udat->query_property("more");
   if ( cursor >= linecnt )
      clear_more();
}

