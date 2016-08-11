/*
**	/usr/SkotOS/lib/bilbo.c
**
**	Copyright Skotos Tech Inc 2000
*/

# include <type.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <base/living.h>
# include <base/preps.h>
# include <base/act.h>
# include <base.h>

# include <SAM.h>
# include <XML.h>
# include <HTTP.h>

private inherit "/lib/string";
private inherit "/lib/array";

inherit "/usr/XML/lib/xmlparse";
inherit "/usr/SAM/lib/sam";

private inherit "/base/lib/toolbox";
private inherit "/base/lib/urcalls";
private inherit "/usr/SkotOS/lib/ursocials";

private inherit "/usr/SkotOS/lib/describe";
private inherit "/usr/SkotOS/lib/noun";

# include "/usr/SAM/include/sam.h"

mixed resolve_global( string addr );
static mixed resolve_variables ( mixed code, mixed text, string detail, string mode, object obj, mixed other, string verb, mapping params );
void run_emit1( mixed code, object o1, mixed m1, string text, string mode, string detail, string verb, mapping params );
string run_sam( string text, object obj, object actor, string mode );
mixed resolve_ref ( object obj, mixed other, mixed varname, string mode, mixed detail, mapping params );
static mixed resolve_property( mixed code, mixed v1, object obj, mixed other, string mode, string detail, mapping params, string verb );
static mixed resolve_obj ( mixed code, mixed text, object obj, mixed other, string mode, string detail, mapping params, string verb );


static 
void err ( string msg ) {
  SysLog( msg );
}

#if 0
static 
void dbg ( string msg ) {
   SysLog( msg );
}
#else
#define dbg(str)
#endif

# if 0
private void
debug_property_update(string where, object code, object obj, object target, string prop)
{
    catch {
	if (sscanf(prop, "bilbo:%*s")) {
	    /*
	     * DEBUG code, track when bilbo:inherit:* properties are being
	     * overwritten.
	     */
	    if (target->query_property(prop) != nil) {
		DEBUG("BILBO:"+ where + ":Overwriting \"" + prop + "\" in " +
		      (target ? name(target) : "<nil>") + " by " +
		      (obj    ? name(obj)    : "<nil>") + " (" +
		      (code   ? name(code)   : "<nil>") + ")");
	    } else {
		DEBUG("BILBO:"+ where + ":Writing \"" + prop + "\" in " +
		      (target ? name(target) : "<nil>") + " by " +
		      (obj    ? name(obj)    : "<nil>") + " (" +
		      (code   ? name(code)   : "<nil>") + ")");
	    }
	}
    }
}
# else
#   define debug_property_update(w,c,o,t,p)
# endif

static
void do_fail ( mixed code, string id, object obj, object other, string text, string mode, string detail, string verb ) {
  run_emit1( code, obj, other, "["+id+"] " + text, mode, detail, verb, nil );
}

/* nomask */ void message(string str) {
   /* we could add debug information here */
}

/* nomask */ void paragraph(string str) {
   /* we could add debug information here */
}

string fourdigits ( int line ) {
  string sline;
   
  sline = "0" + line;
  if ( line < 999 ) {
    while( strlen(sline) < 4 )
      sline = "0"+sline;
  } else {
    sline=sline[1..];
  }
			
  return sline;
}

static
int bilbo_scripts ( object user, object obj ) {
  mapping script, remap, newscript;
  string *keys, *remapkeys, s1, s2, test, newline, oldline, buf;
  int i, line, j, change, thischange;

  remap = ([ ]);
  newscript = ([ ]);

  script = obj->query_properties();
  keys = map_indices(script);
  keys -= ({ nil });
  for( i=0; i<sizeof(keys); i++ ) {
    if ( obj->query_property(keys[i]) == "run" )
      user->message( keys[i] + "\n" );
  }
  return TRUE;
}

static
int bilbo_info ( object user, object obj, string sname, string infomode ) {
  mapping script, remap, newscript;
  string *keys, *remapkeys, s1, s2, test, newline, oldline, buf;
  int i, line, j, change, thischange;

  remap = ([ ]);
  newscript = ([ ]);

  if ( sscanf( sname, "%*s:%*s:%*s" ) != 3 ) {
    return FALSE;
  }

  script = obj->query_properties();
  keys = map_indices(script);
  keys -= ({ nil });
  for( i=0; i<sizeof(keys); i++ ) {
    test = " "+keys[i]+" ";
    if ( sscanf( keys[i], sname+":%*s" ) < 1 )
      script[ keys[i] ] = nil;
  }
  keys = map_indices(script);

  if ( infomode=="list" ) {
    user->clear_more();
    for( i=0; i<sizeof(keys); i++ ) {
      user->add_more( keys[i] + " " + script[keys[i]] );
    }
    user->show_more();
    return TRUE;
  }

  if ( infomode=="erasescript" ) {
    for( i=0; i<sizeof(keys); i++ ) {
      obj->set_property( keys[i], nil );
    }
    user->message("Script erased.\n");
    return TRUE;
  }

  return TRUE;
}
											

static 
int bilbo_renumber ( string rmode, object obj, string sname ) {
  mapping script, remap, newscript;
  string *keys, *remapkeys, s1, s2, test, newline, oldline, buf;
  int i, line, j, change, thischange;

  remap = ([ ]);
  newscript = ([ ]);
   
  if ( sscanf( sname, "%*s:%*s:%*s" ) != 3 ) {
    return FALSE;
  }
   
  dbg( "rmode = " + rmode );
  if ( rmode=="normal" )
    script = obj->query_properties();
  else
    script = obj->query_all_initial_properties();

  dbg( rmode + " script = " + dump_value(script) );

  keys = map_indices(script);
  keys -= ({ nil });
  for( i=0; i<sizeof(keys); i++ ) {
    test = " "+keys[i]+" ";
    if ( sscanf( keys[i], sname+":%*s" ) < 1 )
      script[ keys[i] ] = nil;
    if ( sscanf( keys[i], "%*s#%*s" )==2 )
      script[ keys[i] ] = nil;
  }
   
  keys = map_indices(script);
  for( i=0; i<sizeof(keys); i++ ) {
    dbg( "LINE: " + keys[i] );
  }
  if ( sizeof(keys) < 1 )
    return FALSE;

  for( i=0, line=10; i<sizeof(keys); i++, line+=10 ) {
    newline = fourdigits(line);
    sscanf( keys[i], sname+":%s", oldline );
    remap[oldline] = newline;
    newscript[ sname+":"+newline ] = script[ keys[i] ];
    if ( rmode=="normal" )
      obj->set_property( keys[i], nil );
    else
      obj->set_initial_property( keys[i], nil );
  }

  dbg( "remap: " + dump_value(remap) );
   
  for( i=0; i<sizeof(keys); i++ ) {
    sscanf( keys[i], sname+":%s", oldline );
    test = "  " + script[ keys[i] ] + "  ";
    remapkeys = map_indices( remap );
    change = FALSE;

    buf = "";
    for( j=0; j<sizeof(remapkeys); j++ ) {
      thischange = FALSE;
      while( sscanf( test, "%s "+remapkeys[j]+" %s", s1, s2 )==2 ) {
	dbg( "s1="+s1+" s2="+s2 );
	buf += s1 + " " + remap[ remapkeys[j] ] + " ";
	dbg( "buf is now " + buf );
	test = s2;
	dbg( "test becomes " + test );
	change = thischange = TRUE;
	j = 0;
      }
      dbg( "new line " + oldline + ": " + test );
    }
    if ( change )
      test = buf + s2;
	  
    test = test[2..strlen(test)-3];
    if ( rmode=="normal" )
      obj->set_property( sname+":"+remap[oldline], test );
    else
      obj->set_initial_property( sname+":"+remap[oldline], test );
  }
   
  return TRUE;
}


static
mixed resolve_property( mixed code, mixed v1, object obj, mixed other, string mode, string detail, mapping params, string verb ) {
  mixed m1;
  string p1, p2;
  object env;

  if ( typeof(v1)==T_OBJECT || v1==nil )
    return v1;

  dbg( "resprop: v1="+dump_value(v1) );
   
  if ( sscanf( v1, "this.%s", p1 )==1 ) {
    m1 = obj->query_property(p1);
    dbg( "in resprop, m1 for "+p1+" = " + dump_value(m1) );
    if ( m1==nil )
      m1 = code->query_property(p1);
  } else if ( sscanf( v1, "other.%s", p1 )==1 ) {
    m1 = other->query_property(p1);
  } else {
    if ( sscanf( v1, "global.%s", p1 )==1 ) {
      m1 = resolve_global( p1 );
      if ( m1=="error-in-global-resolve" ) {
	err( "Fail at X1" );
	return m1;
      }
    } else if ( sscanf( v1, "$obj(%s).%s", p1, p2 )==2 ) {
      dbg("obj res: p1 = "+dump_value(p1)+", p2 = " + dump_value(p2) );
      m1 = resolve_ref( obj, other, p1, mode, detail, params );
      m1 = resolve_property( code, m1, obj, other, mode, detail, 
        params, verb );
      dbg("res m1 = " + dump_value(m1) );
      if ( m1 )
	m1 = m1->query_property( p2 );	  
    } else if ( sscanf( v1, "$obj(%s)", p1) ==1 ) {

      m1 = resolve_obj( code, p1, obj, other, mode,detail,params, verb );
      if ( m1==nil ) {
        m1 = resolve_ref( obj, other, p1, mode, detail, params );
        m1 = resolve_variables( code, m1, detail, mode, obj, other,
          verb, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail,
          params, verb );
      }

    } else if ( sscanf( v1, "env.%s.%s", p2, p1 )==2 ) {
      if ( p2=="this" )
	env = obj->query_environment();
      else
	env = other->query_environment();
      m1 = env->query_property(p1);
    } else {
      m1 = v1;
    }
  }
  
  dbg( "resolved " + v1 + " to " + dump_value(m1) );
   
  return m1;		
}

static
mixed resolve_property_update ( mixed code, string val, object obj, mixed other, string mode, string detail, mapping params, string verb ) {
  mixed m1;
  object global_obj, env;
  string p1, p2;
   
  if ( sscanf( val, "this.%s", p1 )==1 ) {
    m1 = obj->query_property(p1);
    if ( m1==nil )
      m1 = code->query_property(p1);
    if ( typeof(m1)!=T_STRING ) {
      m1 = ">" + m1;
      m1 = m1[1..];
      debug_property_update("resolve_property_update[1]", code, obj, obj, p1);
      obj->set_property( p1, m1 );
    }
  } else if ( sscanf( val, "other.%s", p1 )==1 ) {
    m1 = other->query_property(p1);
    if ( typeof(m1)!=T_STRING ) {
      m1 = ">" + m1;
      m1 = m1[1..];
      debug_property_update("resolve_property_update[2]", code, obj, other, p1);
      other->set_property( p1, m1 );
    }
  } else {
    if ( sscanf( val, "global.%s.%s", p1, p2 )==2 ) {
      global_obj = find_object( p1 );
      if ( global_obj==nil )
	return nil;
      m1 = global_obj->query_property(p2);
    } else if ( sscanf( val, "$obj(%s).%s", p1, p2 )==2 ) {
      m1 = resolve_ref( obj, other, p1, mode, detail, params );
      global_obj = resolve_property( code, m1, obj, other, mode, detail, params, verb );
      if ( global_obj==nil )
	return nil;
      m1 = global_obj->query_property(p2);			
    } else if ( sscanf( val, "env.%s.%s", p2, p1 )==2 ) {
      if ( p2=="this" )
	env = obj->query_environment();
      else
	env = other->query_environment();
      m1 = env->query_property( p1 );
      if ( typeof(m1)!=T_STRING ) {
	m1 = ">" + m1;
	m1 = m1[1..];
      }
    } else {
      m1 = val;
    }
  }
  if ( typeof(m1)==T_STRING )
    sscanf( m1, "%f", m1 );
		
  return m1;		
}

static
mixed resolve_obj ( mixed code, mixed text, object obj, mixed other, string mode, string detail, mapping params, string verb ) {
  mixed o;
  int do_env;
  string s1;
  
  if ( typeof(text)==T_OBJECT || text==nil )
    return text;
 
  dbg( "------- resolve " + text );
   
  do_env = FALSE;
  if ( sscanf( text, "env.%s", s1 )==1 ) {
    text = s1;		   
    do_env = TRUE;
  }
  if ( text=="actor" ) {
    if ( mode=="react" || mode=="auto" )
      text = "other";
    else
      text = "this";
  }
  if ( text=="target" ) {
    if ( mode=="react" || mode=="auto"  )
      text = "this";
    else
      text = "other";
  }

  dbg( "resolve obj: text=" + text + ", obj="+describe_one(obj) + ", other=" + describe_one(other) );
   
  if ( text=="this" ) {
    if ( do_env )
      return obj->query_environment();
    else	  
      return obj;
  }
  if ( text=="other" ) {
    if ( do_env )
      return other->query_environment();
    else
      return other;
  }
  if ( sscanf( text, "global.%s", s1 )==1 ) {
    return find_object( s1 );
  }
  if ( sscanf( text, "obj.%s", s1 )==1 ) {
    return resolve_property( code, s1, obj, other, mode, detail, params, verb );
  }

  return nil;
}

static
mixed resolve_ref ( object obj, mixed other, mixed varname, string mode, mixed detail, mapping params ) {
  string propname;

  if ( typeof(varname)==T_OBJECT || varname==nil )
    return varname;
  
  dbg( "resolve_ref resolving " + dump_value(varname) ); 
   
  if ( sscanf( varname, "actor.%s",propname )==1 ) {
    if ( mode=="react" || mode=="auto" )
      varname = "other."+propname;
    else
      varname = "this."+propname;
  }

  if ( sscanf( varname, "target.%s",propname )==1 ) {
    if ( mode=="react" || mode=="auto" )
      varname = "this."+propname;
    else
      varname = "other."+propname;
  }

  dbg( "to " + varname ); 
   
  return varname;
}

static
mixed resolve_arrays( mixed code, string text, object obj, object other, string mode, string verb, mapping params, string detail ) {
  string s1, arname, index, next, s, a1, a2, prefix;
  mixed m1, m2;
   
  text = " " + text + " ";
  while( sscanf( text, "%s#%s[%s]%s", s1, arname, index, next )==4 ) {
    index = resolve_ref( obj, other, index, mode, detail, params );
    index = resolve_property( code, index, obj, other, mode, detail, params, verb );
    dbg( "arname: " + dump_value(arname) );

    m2 = nil;
    prefix = "bilbo:"+mode+":"+verb+":#";
	  
    if ( sscanf( s1, "  %s.", a1 )==1 ) {
      m2 = resolve_obj( code, a1, obj, other, mode, detail, params, verb );
      if ( m2 )
	s1 = "  ";
      prefix = "#";
    }
	  
    dbg("res_array s1 = "+s1+" m2 = " + dump_value(m2) );
    if ( !m2 )
      m2 = obj;
	 
    m1 = m2->query_property( prefix+arname+"["+index+"]"  );

    dbg( "Z. prefix = " + prefix + " arname = " + arname + " m2 = " + dump_value(m2) );

    dbg( "A. m1 = " + dump_value(m1) );
	  
    if ( m1==nil && m2==obj )
      m1 = code->query_property( prefix+arname+"["+index+"]" );

    dbg( "B. m1 = " + dump_value(m1) );

    if ( typeof(m1)==T_OBJECT )
      return m1;
	  
    if ( m1 ) {
      text = s1 + m1 + next;
    } else {
      m1 = obj->query_property( "bilbo:"+mode+":"+verb+":#"+arname+"[default]" );

      if ( typeof(m1)==T_OBJECT )
	return m1;
		 
      if ( m1 )
	text = s1 + m1 + next;
      else
	text = s1 + "  empty  " + next;
    }
  }
  return text[1..strlen(text)-2];
}


/*
 * Utility functions that fetch/store lists from/in properties in objects.
 */

mixed pull_array ( mixed ob, mixed list ) {
    mixed str;
  
    if (!ob) {
	return nil;
    }
    str = ob->query_property( list );
    if ( str==nil )
	return ({ });

    str = strip(str);

    if (str[0] != '{' || str[strlen(str) - 1] != '}') {
	return ({ });
    }

    return explode("|" + str[1 .. strlen(str) - 2] + "|", "|");
}

mixed push_array( mixed ob, mixed listname, mixed array ) {
    if ( ob==nil ) 
	return nil;

    ob->set_property( listname, "{" + implode(array, "|") + "}");
  
    return TRUE;
}

/*
 * The implementation of the $list and $list_* functions:
 */

mixed list_add( mixed objname, string listname, string elval ) {
    mixed array;
  
    if ( !objname )
	return nil;

    array = pull_array( objname, listname );

    array += ({ elval });

    return push_array( objname, listname, array );
}

mixed list_remove( mixed objname, string listname, mixed num ) {
    mixed array, newarray;
    int i;
  
    if ( !objname )
	return nil;

    if ( typeof(num)==T_STRING ) {
	sscanf( num, "%d", num );
    }
  
    array = pull_array( objname, listname );

    if ( num < sizeof(array) ) {
	for( i=0, newarray=({ }); i<sizeof(array); i++ ) {
	    if ( i != (num-1) ) {
		newarray += ({ array[i] });
	    }
	}
    }
  
    return push_array( objname, listname, newarray );
}

mixed list( mixed objname, string listname, mixed num ) {
    mixed array, newarray;
    int i;
  
    if ( !objname )
	return nil;

    if ( typeof(num)==T_STRING ) {
	sscanf( num, "%d", num );
    }
  
    array = pull_array( objname, listname );
    if ( num > sizeof(array) ) 
	return nil;

    return array[ num-1 ];
}

mixed list_count( mixed objname, string listname ) {
    mixed array;

    if ( !objname )
	return nil;
  
    array = pull_array( objname, listname );
    if ( !array )
	return nil;
    return( sizeof(array) );
}

mixed list_find( mixed objname, string listname, string elname ) {
    mixed array;
    int i;

    if ( !objname )
	return nil;
  
    array = pull_array( objname, listname );
    if ( !array )
	return nil;

    for( i=0; i<sizeof(array); i++ ) {
	if ( elname==array[i] )
	    return i+1;
    }
  
    return nil;
}

mixed list_replace( mixed objname, string listname, mixed idx, string elname ) {
    mixed array;
    int i;

    if ( !objname )
	return nil;
  
    array = pull_array( objname, listname );
    if ( !array )
	return nil;

    if ( typeof(idx)==T_STRING ) {
	sscanf( idx, "%d", idx );
    }
    if ( idx > sizeof(array) ) 
	return nil;

    array[ idx-1 ] = elname;
    return push_array( objname, listname, array );
}

mixed list_range( mixed objname, string listname, mixed idx1, mixed idx2 ) {
    mixed array, ret;
    int i;

    if ( !objname )
	return nil;

    array = pull_array( objname, listname );

    if ( !array )
	return nil;

    if ( typeof(idx1)==T_STRING ) 
	sscanf( idx1, "%d", idx1 );

    if ( typeof(idx2)==T_STRING ) 
	sscanf( idx2, "%d", idx2 );

    if ( idx1 < idx2 ) { 
	if ( idx1 < 1 )
	    idx1 = 1;
	if ( idx2 > sizeof(array) )
	    idx2 = sizeof(array);
	
	for( ret="", i=idx1; i<=idx2; i++ ) {
	    ret += array[i-1];
	    if ( i<idx2 )
		ret += " ";
	}
    } else {
	if ( idx2 < 1 )
	    idx2 = 1;
	if ( idx1 > sizeof(array) )
	    idx1 = sizeof(array);
	
	for( ret="", i=idx1; i>=idx2; i-- ) {
	    ret += array[i-1];
	    if ( i>idx2 )
		ret += " ";
	}
    }

    return ret;
}

mixed html_filter ( mixed line ) {
  mixed m1, m2, m3;
  
  line = " " + line + " ";
  dbg( "filter 1 line " + dump_value(line) );
  while( sscanf( line, "%s<%s>%s", m1, m2, m3 )==3 ) {
    line = m1 + m3;
  }
  while( sscanf( line, "%s&lt;%s>%s", m1, m2, m3 )==3 ) {
    line = m1 + m3;
  }
  dbg( "filter 2 line " + dump_value(line) );
  return line[1..strlen(line)-2];
}

string *
split_matching_parenthesis(string str)
{
    int i, sz, start, nesting;

    sz = strlen(str);
    start = -1;
    for (i = 0; i < sz; i++) {
	if (str[i] == '(') {
	    start = i;
	    break;
	}
    }
    if (start == -1) {
	/* No starting parenthesis? */
	return nil;
    }
    nesting = 1;
    for (i = start + 1; i < sz; i++) {
	switch (str[i]) {
	case '(':
	    nesting++;
	    break;
	case ')':
	    nesting--;
	    if (nesting == 0) {
	      return ({
		str[..start - 1],
		  str[start + 1..i - 1],
		  str[i + 1..] });
	    }
	    break;
	default:
	    break;
	}
    }
    /* Missing the closing parenthesis. */
    return nil;
}

string *
scan_for_function_call(string function, string text)
{
    string pre, post, *bits;

    if (sscanf(text, "%s$" + function + "(%s", pre, post) < 2) {
        return nil;
    }
    bits = split_matching_parenthesis("(" + post);
    if (!bits) {
        return nil;
    }
    return ({ pre, bits[1], bits[2] });
}

static
mixed resolve_variables ( mixed code, mixed text, string detail, string mode, object obj, mixed other, string verb, mapping params ) {
  string s1, s2, s3, s4, s5, s6, var, a1, a2, a3, ref, temps, *keys, s, *bits;
  int i, do_env, count;
  object *obs, tempo;
  mixed m1, m2, m3, m4, m5, o, listob;
  mixed *parkeys;

  if ( typeof(text)==T_OBJECT || text==nil )
     return text;

  if ( params )   
    parkeys = map_indices( params );
  else
    parkeys = ({ });

  text = " " + text + " ";
  dbg( "res_variables Resolving " + text );
  
  if ( params )
    dbg( "Params: " + dump_value(params) );
   
  while ( sscanf( text, "%s$date%s", s1, s2 )==2 ) {		   
    var = ctime( time() );
    text = s1 + var + s2;
  }

  while ( sscanf( text, "%s$timeofday%s", s1, s2 )==2 ) {
    var = ctime( time() );
    var = var[11..12] + var[14..15];
    text = s1 + var + s2;
  }
		 
  while ( sscanf( text, "%s$detail%s", s1, s2 )==2 ) {		 
    text = s1 + detail + s2;
  }
  
  while ( sscanf( text, "%s$sam(%s)%s", s1, s2, s3 )==3 ) {
    text = s1 + run_sam( s2, obj, other, mode ) + s3;
  }
   
  while ( sscanf( text, "%s$systime%s", s1, s2 )==2 ) {
    text = s1 + time() + s2;
  }
   
  while ( sscanf( text, "%s$dayofweek%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[0..2] + s2;
  }

  while ( sscanf( text, "%s$month%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[4..6] + s2;
  }

  while ( sscanf( text, "%s$dayofmonth%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[8..9] + s2;
  }

  while ( sscanf( text, "%s$year%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[20..23] + s2;
  }

  while ( sscanf( text, "%s$seconds%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[17..18] + s2;
  }

  while ( sscanf( text, "%s$hour%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[11..12] + s2;
  }

  while( sscanf( text, "%s$minutes%s", s1, s2 )==2 ) {
    var = ctime( time() );
    text = s1 + var[14..15] + s2;
  }

  while ( sscanf( text, "%s$random(%s)%s", s1, s2, s3 )==3 ) {
    
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_variables( code, m1, detail, mode, obj, other,
      verb, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail,
      params, verb );
    dbg( "random " + s2 + " into " + dump_value(m1) );
    sscanf( m1, "%d", m2 );

    text = s1 + random(m2) + s3;
  }

  for( i=0; i<sizeof(parkeys); i++ ) {
     if (typeof(parkeys[i]) != T_STRING) {
	continue;
     }
    while ( sscanf( text, "%s$"+parkeys[i]+"%s", s1, s2 )==2 ) {
       if ( typeof(params[parkeys[i]])==T_OBJECT ) {
	  DEBUG("Bilbo shortcuts " + dump_value(text) + " to " +
		dump_value(parkeys[i]) + "[" + name(params[parkeys[i]]) + "]");
	  return params[parkeys[i]];
       }
       text = s1 + params[parkeys[i]] + s2;
    }
  }
  
  while ( sscanf( text, "%s$name(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_obj( code, s2, obj, other, mode, detail, params, verb );
    if ( m1==nil ) {
      m1 = resolve_ref( obj, other, s2, mode, detail, params );
      m1 = resolve_variables( code, m1, detail, mode, obj, other,
        verb, params );
      m1 = resolve_property( code, m1, obj, other, mode, detail,
        params, verb );
    }
    if ( typeof(m1)!=T_OBJECT )
       return m1;

    text = s1 + name(m1) + s3;
  }


 
  while ( sscanf( text, "%s$count(%s)%s", s1, s2, s3 )==3 ) {
    do_env = FALSE;
    dbg( "count " + s1 + " " + s2 + " " + s3 );
    if ( sscanf( s2, "%s.%s.%s", a1, a2, a3 )==3 ) {
      if ( a1=="env" ) {
	do_env = TRUE;
	ref = a2;
	var = a3;
      }
      if ( a1=="global" ) {
	ref = "skip";
	o = find_object( a2 );
	dbg( "Finding object " + a2 + ": " + dump_value(o) );
	var = a3;
      }
      if ( a1=="obj" ) {
	ref = "skip";
	o = resolve_property( code, a2, obj, other, mode, detail, params, verb );
	var = a3;
      }
    } else if ( sscanf( s2, "%s.%s", a1, a2 )==2 ) {
      if ( a1=="global" ) {
	ref = "skip";
	o = find_object(a2);
	var = nil;
      } else if ( a1=="obj") {
	ref = "skip";
	o = resolve_property( code, a2, obj, other, mode, detail, params, verb );
	var = nil;
      } else if ( a1=="env") {
	do_env = TRUE;
	ref = a2;
	var = nil;
      } else  {
	ref = a1;
	var = a2;
      }
    } else {
      sscanf( s2, "%s", ref );
      var = nil;
    }
    if ( mode=="react" || mode=="auto" ) {
      if ( ref=="target" )
	ref = "this";
      if ( ref=="actor" )
	ref = "other";
    } else {
      if ( ref=="target" )
	ref = "other";
      if ( ref=="actor" )
	ref = "this";		 
    }			
    switch( ref ) {
    case "this":
      o = obj;
      break;
    case "other":
      o = other;
      break;
    case "global":
    case "obj":				 
      break;
    }
    if ( do_env ) {
      o = o->query_environment();
      dbg( "environment: " + dump_value(o) );
    }
    dbg( ref + ": " +  dump_value(o) );

    obs = o->query_inventory();

    count = sizeof(obs);
    if ( var ) {
      dbg( "looking for " + var );
      for( count=i=0; i<sizeof(obs); i++ ) {
	if ( obs[i] ) {
	  tempo = obs[i]->query_ur_object();
	  if ( tempo ) {
	    dbg( "looking for " + var + " in ur: " + dump_value( name(tempo) ) ); 
	    temps = " " + name(tempo) + " ";
	    if ( sscanf( temps, "%*s"+var+"%*s") )
	      count++;
	  }
	}
      }
    }
	  
    text = s1 + count + s3;
  }


  while (sscanf(text, "%s$list%s(%s,%s", s1, s2, s3, s4) == 4) {
      int   expect;
      mixed *param_data, result;

      expect = -1;
      switch (s2) {
      /* 2 parameter $list functions: */
      case "_count":
	  expect = 2;
	  break;
      /* 3 parameter $list functions: */
      case "_add":
      case "_remove":
      case "_find":
      case "":
	  expect = 3;
	  break;
      /* 4 parameter $list functions: */
      case "_replace":
      case "_range":
	  expect = 4;
	  break;
      default:
	  break;
      }
      if (expect == -1) {
	  break;
      }
      bits = split_matching_parenthesis("(" + s3 + "," + s4);
      if (!bits) {
	  break;
      }
      param_data = explode("," + bits[1] + ",", ",");
      if (sizeof(param_data) != expect) {
	  break;
      }

      /*
       * Eric did the first (the 'list' parameter) differently, so I'll go
       * along with that until I understand more of the resolve_* stuff.
       * 2001/Aug/29 - EH.
       */
      param_data[0] = resolve_ref(obj, other, param_data[0], mode, detail,
				  params);
      param_data[0] = resolve_property(code, param_data[0], obj, other, mode,
				       detail, params, verb);
      if (typeof(param_data[0]) != T_OBJECT) {
	  param_data[0] = resolve_obj(code, param_data[0], obj, other, mode,
				      detail, params, verb);
      }
      for (i = 1; i < expect; i++) {
	  param_data[i] = resolve_ref(obj, other, param_data[i], mode, detail,
				      params);
	  param_data[i] = resolve_variables(code, param_data[i], detail, mode,
					    obj, other, verb, params);
	  param_data[i] = resolve_property(code, param_data[i], obj, other,
					   mode, detail, params, verb);
      }
      result = call_other(this_object(), "list" + s2, param_data...);
      text = s1 + (result == nil ? "[NIL]" : (string)result)  + bits[2];
  }

  /* The end of the $variables, everything else is properties or errors */

  while ( sscanf( text, "%s$prop(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
    if ( typeof(m1)==T_OBJECT || m1==nil )
       return m1;

#if 0
    /* So what's wrong with return s1 + "0" + s3 here? -EH */
    if ( !m1 )
      text = s1 + "[ERROR: "+s2+"]" + s3;
    else
#endif
       if (s2 == "trait:color") {
	  SysLog("BILBO:: result is " + dump_value(s1 + m1 + s3));
       }
      text = s1 + m1 + s3;
  }
  
  while( sscanf( text, "%s$toascii(%s)%s", s1, s2, s3 )==3 ) {
    m2 = resolve_ref( obj, other, s2, mode, detail, params );
    m2 = resolve_property( code, m2, obj, other, mode, detail, params, verb );
    if ( typeof(m2)==T_STRING )
      sscanf( m2, "%d", m2 );
    
    m1 = " ";
    m1[0] = m2;
    text = s1 + m1 + s3;
  }
  
  while( sscanf( text, "%s$tonumber(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

    if ( typeof(m1)==T_STRING ) {
      i = m1[0];
      text = s1 + i + s3;
    } else {
      return nil;
    }
  }

  while( sscanf( text, "%s$htmltest(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
    obj->set_property( "zzzz", "<b>bold</b><br/><i>italic</i><font color=red>red</font><u>underline</u><a href=\"www.yahoo.net\">yahoos</a>" );
    text = s1 + m1 + s3;
  }

  while( sscanf( text, "%s$strstr(%s,%s)%s", s1, s2, s3, s4 )==4 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

    m2 = resolve_ref( obj, other, s3, mode, detail, params );
    m2 = resolve_property( code, m2, obj, other, mode, detail, params, verb );

    m3 = "%s"+m2+"%s";
    if ( sscanf( m1, m3, m4, m5 )==2 ) {
      text = s1 + strlen(m4) + s4; 
    } else {
      return nil;
    }
  }

  while( sscanf( text, "%s$strlen(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
    if ( typeof(m1) != T_STRING )
      m1 = "";
    m1 = strlen(m1);
    text = s1 + m1 + s3;
  }

  while( sscanf( text, "%s$substr(%s,%s,%s)%s", s1, s2, s3, s4, s5 )==5 ) {
    sscanf( s3, "%d", m3 );
    sscanf( s4, "%d", m4 );
    if (m3 < 0) m3 = 0;
    if (m4 > strlen(s2)-1) m4 = strlen(s2)-1;
    m1 = s2[ m3..m4 ];
    text = s1 + m1 + s5;
  }

  if ( sscanf( text, "%s$player(%s)%s", s1, s2, s3 )==3 ) {
    m1 = UDATD->query_body( s2 );
    dbg( "$player "+s2+" = " + dump_value(m1) );
    return m1;  
  }  

  if ( sscanf( text, "%s$lookup(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    dbg( "** lookup m1 = " + dump_value(m1) );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
    dbg( "$lookup " + dump_value(m1) + " = " + dump_value(find_object(m1)) );
    return m1 ? find_object(m1) : nil;
  }

  if ( sscanf( text, "%s$env(%s)%s", s1, s2, s3 )==3 ) {
    m1 = resolve_ref( obj, other, s2, mode, detail, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
    m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
    dbg( "env m1 = " + dump_value(m1) );
    m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

    return m1 ? m1->query_environment() : nil;
  }

  while ( sscanf( text, "%s$find(%s,%s,%s)%s", s1, s2, s3, s4, s5 )==5 ) {
    count = 0;
    i = 1;
    a1 = s3; 
    dbg( "a1 = " + a1 );
    o = resolve_obj( code, s2, obj, other, mode, detail, params, verb );
    m4 = resolve_property( code, s4, obj, other, mode, detail, 
      params, verb );
    if ( m4==nil )
      m4 = s4;
    dbg( "m4 = " + dump_value(m4) );
    dbg( "a1 = " + dump_value(a1) );
    dbg( "o = " + dump_value(o) );

    if ( o==nil ) {
      o = resolve_ref( obj, other, s2, mode, detail, params );
      dbg( "o " + dump_value(o) );
      o = resolve_variables( code, o, detail, mode, obj, other,
        verb, params );
      dbg( "o " + dump_value(o) );
      o = resolve_property( code, o, obj, other, mode, detail,
        params, verb );
    }
    if ( o==nil )
      return nil;
 
    for( i=1; m1 = o->query_property(a1+"["+i+"]"); i++ ) {
      dbg( "$find m1 = " + dump_value(m1) );
      dbg( "comparing " + dump_value(m1) + " and " + dump_value(m4) );
      if ( m1 == m4 )
        return ""+i;
    }
    return nil;
  }

  while ( sscanf( text, "%s$element(%s,%s,%s)%s", s1, s2, s3, s4, s5 )==5 ) {
    count = 0;
    i = 1;
    a1 = s3;
    dbg( "a1 = " + a1 );
    m1 = resolve_obj( code, s2, obj, other, mode, detail, params, verb );
    if ( m1==nil ) {
      m1 = resolve_ref( obj, other, s2, mode, detail, params );
      dbg( "m1 " + dump_value(m1) );
      m1 = resolve_variables( code, m1, detail, mode, obj, other,
        verb, params );
      dbg( "m1 " + dump_value(m1) );
      m1 = resolve_property( code, m1, obj, other, mode, detail,
        params, verb );
    }
    if ( m1==nil )
      return nil;

    m4 = resolve_ref( obj, other, s4, mode, detail, params );
    dbg( "m4 " + dump_value(m4) );
    m4 = resolve_variables( code, m4, detail, mode, obj, other,
      verb, params );
    dbg( "m4 " + dump_value(m4) );
    m4 = resolve_property( code, m4, obj, other, mode, detail,
      params, verb );
    dbg( "m4 " + dump_value(m4) );

    if ( m4==nil )
      m4 = s4;
    dbg( "m4 = " + dump_value(m4) );
    dbg( "a1 = " + dump_value(a1) );
    dbg( "m1 = " + dump_value(m1) );
    
    s = a1 + "[" + m4 + "]";
    dbg( "s="+dump_value(s) );
    m1 = m1->query_property( s );
    return m1;
  }


  while ( sscanf( text, "%s$brief(%s)%s", s1, s2, s3 )==3 ) {
    dbg( "$brief " + text + " s1 " + s1 + " s2 " + s2 + " s3 " + s3 );
    a1 = resolve_ref( obj, other, s2, mode, detail, params );
    dbg( "a1 = " + a1 ) ;
    m1 = resolve_property( code, a1, obj, other, mode, detail, params, verb );
    /* m1 = resolve_arrays( code, text, obj, other, mode, verb, params, detail ); */
    if ( typeof(m1)!=T_OBJECT ) {
      m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb ); 
      dbg( "turned " + s2 + " into " + dump_value(m1) );
    }
    if ( typeof(m1)==T_OBJECT )
      text = s1 + describe_one(m1) + s3;
    else
      text = " " + s2 + " is not a valid object. ";
  }
   
  dbg( "resolved to " + text );
  m1 = resolve_arrays( code, text, obj, other, mode, verb, params, detail );
  if ( typeof(m1)==T_OBJECT )
    return m1;
  
  while ( sscanf( text, "%s$capitalize(%s)%s", s1, s2, s3 )==3 ) {
    text = s1 + capitalize(s2) + s3;
    dbg( "new text: " + text );
    m1 = text;
  }
  while ( sscanf( text, "%s$lowercase(%s)%s", s1, s2, s3 )==3 ) {
    text = s1 + lower_case(s2) + s3;
    dbg( "new text: " + text );
    m1 = text;
  }
  while ( sscanf( text, "%s$filter(%s)%s", s1, s2, s3 )==3 ) {
    text = s1 + html_filter(s2) + s3;
    dbg( "FILTER text: " + text );
    m1 = text;
  }

  dbg( "returned m1 = " + dump_value(m1) );
   
  return m1[1..strlen(m1)-2];
}

static
string run_sam ( string text, object obj, object actor, string mode ) {
  SAM samdata;

  if (text) {
     text = implode(explode(text, "\n"), "<BR/>");
  }
  samdata = XML->parse( text );

  dbg( "samdata: " + dump_value(samdata) );
  dbg( "actor: " + dump_value(actor) );
  dbg( "obj: " + dump_value(obj) );
   
  if ( mode=="react" || mode=="auto" )
    return unsam( samdata, ([ "actor":actor, "object":obj, "other":actor, "this":obj,
			    "target":obj ]) );
  else
    return unsam( samdata, ([ "actor":obj, "object":obj, "other":actor, "this":obj,
			    "target":actor ]) );
   
}


static
void run_emit ( mixed dest, mixed code, object obj, mixed other, string text, string mode, string detail, string verb, mapping params ) {
  object user;
  mixed *souls, m1;
  int i;

  dbg( "emit1 obj: " + dump_value(obj) );
  dbg( "emit1 other: " + dump_value(other) );
  dbg( "text : " + text + " mode: " + mode  );
 
  souls = dest->query_souls();
  for( i=0; i<sizeof(souls); i++ ) {
    user = souls[i];
    /* dbg( "emitting1 to " + dump_value(user) ); */
    user->html_message( text + "\n" );
  }
}


static
void run_emit1 ( mixed code, object obj, mixed other, string text, string mode, string detail, string verb, mapping params ) {
  object user;
  mixed *souls, m1;
  int i;

  dbg( "emit1 obj: " + dump_value(obj) );
  dbg( "emit1 other: " + dump_value(other) );
  dbg( "text : " + text + " mode: " + mode  );
  
  m1 = resolve_variables( code, text, detail, mode, obj, other, verb, params );
  if ( m1==nil )
    return;

  text = m1;

  run_emit( other, code, obj, other,
    run_sam( text, obj, other, mode ),
    mode, detail, verb, params );

}

static
void run_emit2 ( mixed code, object obj, mixed other, string text, string mode, string detail, string verb, mapping params ) {
  object user;
  mixed *souls, m1;
  int i;

  dbg( "emit2 obj: " + dump_value(obj) );
  dbg( "emit2 other: " + dump_value(other) );
  
  m1 = resolve_variables( code, text, detail, mode, obj, other, verb, params );
  text = typeof(m1)==T_STRING ? m1 : "Error resolving: " + text;
  
  run_emit( obj, code, obj, other,
    run_sam( text, obj, other, mode ),
    mode, detail, verb, params );

}

static
void run_emit3 ( mixed code, object obj, mixed other, string text, string mode, string detail, string verb, mapping params ) {
  object *ppl, user;
  mixed *souls, m1;
  int i, j;

  dbg( "emit3 obj: " + dump_value(obj) );
  dbg( "emit3 other: " + dump_value(other) );
  
  if ( mode=="react" || mode=="auto" ) {
    ppl = other->query_environment()->query_inventory();
    ppl -= ({ obj });
    ppl -= ({ other });
  } else {
    ppl = obj->query_environment()->query_inventory();
    ppl -= ({ obj });
    ppl -= ({ other });
  }

  m1 = resolve_variables( code, text, detail, mode, obj, other, verb, params );
  text = typeof(m1)==T_STRING ? m1 : "Error resolving: " + text;
   
  for( i=0; i<sizeof(ppl); i++ ) {
    souls = ppl[i]->query_souls();
    for( j=0; j<sizeof(souls); j++ ) {
      user = souls[j];
      user->message( run_sam( text, obj, other, mode ) + "\n" );
    }
  }
}

static
int run_modval( mixed code, object obj, mixed other, mixed val, string dest, string op, string mode, string detail, mapping params, string verb ) {
  string p1, p2, s;
  mixed m1, m2, m, dest_obj, global_obj, env;
  float old, new;

  m1 = resolve_property_update( code, val, obj, other, mode, detail, params, verb );
  dbg( "run_modval on " + val );

  if ( sscanf( dest, "this.%s", p1 )==1 ) {
    dest_obj = obj;
  } else if ( sscanf( dest, "other.%s", p1 )==1 ) {
    dest_obj = other;
  } else if ( sscanf( dest, "global.%s.%s", p2, p1 )==2 ) {
    dest_obj = find_object(p2);
    if ( dest_obj==nil )
      return FALSE;
  } else if ( sscanf( dest, "$obj(%s).%s", p2, p1 )==2 ) {
    m1 = resolve_ref( obj, other, p2, mode, detail, params );
    dest_obj = resolve_property( code, m1, obj, other, mode, detail, params, verb );
    if ( dest_obj==nil )
      return FALSE;
  } else if ( sscanf( dest, "env.%s.%s", p2, p1 )==2 ) {
    if ( p2=="this" )
      env = obj->query_environment();
    else
      env = other->query_environment();
    dest_obj = env;
  } else {
    return FALSE;
  }
		   
  if ( dest_obj ) {
    if ( dest_obj->query_property(p1)==nil )
      old = 0.0;
    else {
      m = dest_obj->query_property(p1);
      if ( typeof(m)==T_INT ) {
	s = ">"+m;
	s = s[1..];			
	debug_property_update("run_modval[1]", code, obj, dest_obj, p1);
	dest_obj->set_property( p1, s );
      }
      sscanf( dest_obj->query_property( p1 ), "%f", old );
    }
    switch( op ) {
    case "add":
       /* DEBUG("run_modval(...) add: " + dump_value(({ code, obj, other, val, dest, op, mode, detail, params, verb }))); */
      new = old + m1;
      break;
    case "sub":
      new = old - m1;
      break;
    case "mul":
      new = old * m1;
      break;
    case "div":
      new = old / m1;
      break;
    }
    s = ">" + new;
    debug_property_update("run_modval[2]", code, obj, dest_obj, p1);
    dest_obj->set_property( p1, s[1..] );
  } else {
    return FALSE;
  }

  return TRUE;
}

static
int is_alpha ( mixed v1 ) {
  if ( typeof(v1) != T_STRING )
    return FALSE;

  if ( strlen(v1) == 0 ) {
     return FALSE;
  }

  if ( v1[0] >= '0' && v1[0] <= '9' )
    return FALSE;

  return TRUE;
}

static
mixed resolve_global ( string addr ) {
  string objname, propname;
  mixed obj;

  sscanf( addr, "%s.%s", objname, propname );
  obj = find_object( objname );
  if ( obj ) {
    return obj->query_property( propname );
  } else {
    dbg( "Error, didn't find global object: " + dump_value(objname) );
    return "error-in-global-resolve";
  }
}

float no_nils ( mixed v ) {
    switch (typeof(v)) {
    case T_INT:
        return (float)v;
    case T_FLOAT:
        return v;
    case T_NIL:
    default:
        return 0.0;
    }
}


static
int resolve_test( mixed code, object obj, mixed other, mixed v1, mixed v2, string op, string mode, string detail, mapping params, string verb, mixed show_debug ) {
  string p1, p2;
  mixed m1, m2, env, msav;
   
  dbg( "resolve_test v1=" + dump_value(v1) + " v2=" + dump_value(v2) + " op="+op );

  m1 = resolve_ref( obj, other, v1, mode, detail, params );
  m1 = resolve_obj( code, m1, obj, other, mode,detail,params, verb );
  if ( m1==nil ) {
    m1 = resolve_ref( obj, other, v1, mode, detail, params );
    m1 = resolve_variables( code, m1, detail, mode, obj, other,
      verb, params );
    m1 = resolve_property( code, m1, obj, other, mode, detail,
      params, verb );
  }

  m2 = resolve_ref( obj, other, v2, mode, detail, params );
  m2 = resolve_obj( code, m2, obj, other, mode,detail,params, verb );
  if ( m2==nil ) {
    m2 = resolve_ref( obj, other, v2, mode, detail, params );
    m2 = resolve_variables( code, m2, detail, mode, obj, other,
      verb, params );
    m2 = resolve_property( code, m2, obj, other, mode, detail,
      params, verb );
  }
   
  if ( m1=="nil" )
    m1 = nil;

  if ( m2=="nil" )
    m2 = nil;

  dbg( "is_alpha("+dump_value(m1)+") = " + is_alpha(m1) );
  dbg( "is_alpha("+dump_value(m2)+") = " + is_alpha(m2) );
  
 
  if ( is_alpha(m1) && is_alpha(m2) ) {
    dbg( "both are alpha" );
    m1 = strip(m1);
    m2 = strip(m2);
  } else {
    if ( typeof(m1)==T_INT ) {
      m1 = ">"+m1;
      m1 = m1[1..];
    }
    if ( typeof(m1)==T_STRING && !is_alpha(m1) ) {
      msav = m1;
      sscanf( m1, "%f", m1 );
      dbg( "turned " + msav + " into " + dump_value(m1) );
      if ( m1==nil )
        m1 = msav;
    }
   
    if ( typeof(m2)==T_INT ) {
      m2 = ">"+m2; 
      m2 = m2[1..];
    }
    if ( typeof(m2)==T_STRING && !is_alpha(m2) )
      sscanf( m2, "%f", m2 );
  }

  dbg( " OP resolving " + dump_value(m1) + " and " + dump_value(m2) ); 

  if ( show_debug ) {
    run_emit( show_debug, code, obj, other, 
      "** -- (COMPARE) " + dump_value(m1) + " and " + dump_value(m2), 
      mode, detail, verb, params );
  }

  switch( op ) {
  case "=":
    dbg( "testing for equality between " + dump_value(m1) + " and " +dump_value(m2) );
    return ( m1 == m2 );
  case ">":
    m1 = no_nils(m1);
    m2 = no_nils(m2);
    dbg( "m1 = " + dump_value(m1) + "  m2 = " + dump_value(m2) );
    return ( m1 > m2 );
  case "<":
    m1 = no_nils(m1);
    m2 = no_nils(m2);
    return ( m1 < m2 );
  case ">=":
    m1 = no_nils(m1);
    m2 = no_nils(m2);
    return ( m1 >= m2 );
  case "<=":
    m1 = no_nils(m1);
    m2 = no_nils(m2);
    return ( m1 <= m2 );
  case "<>":
    dbg( "testing for not equal: " + dump_value(m1) + " " + dump_value(m2) );
    return ( m1 != m2 );	  
  }
  err( "Fail at X3" );
  return -2;
}

static
int run_if ( mixed code, object obj, mixed other, string test, mapping params, string detail, string mode, string verb, mixed show_debug ) {
  string s1, s2, s3, s4, dest, newval, setval, op1, op2;
  mixed m1, m2;
  int r1, r2;
  
  dbg( "test: " + test );
   
  if ( sscanf( test, "%s %s %s and %s %s %s", s1, op1, s2, s3, op2, s4 )==6 ) {
    s1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
    s2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
    s3 = resolve_variables( code, s3, detail, mode, obj, other, verb, params );
    s4 = resolve_variables( code, s4, detail, mode, obj, other, verb, params );
	  
    r1 = resolve_test( code, obj, other, s1, s2, op1, mode, detail, params, verb, show_debug );
    r2 = resolve_test( code, obj, other, s3, s4, op2, mode, detail, params, verb, show_debug );
    if ( r1==-2 || r2==-2 ) {
      err( "Fail at X4" );
      return -2;
    }
    return ( r1 && r2 );
  }

  if ( sscanf( test, "%s %s %s or %s %s %s", s1, op1, s2, s3, op2, s4 )==6 ) {
    s1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
    s2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
    s3 = resolve_variables( code, s3, detail, mode, obj, other, verb, params );
    s4 = resolve_variables( code, s4, detail, mode, obj, other, verb, params );
	  
    r1 = resolve_test( code, obj, other, s1, s2, op1, mode, detail, params, verb, show_debug );
    r2 = resolve_test( code, obj, other, s3, s4, op2, mode, detail, params, verb, show_debug );
    if ( r1==-2 || r2==-2 ) {
      err( "Fail at X5" );
      return -2;
    }
    return ( r1 || r2 );
  }

  if ( sscanf( test, "%s %s %s", s1, op1, s2 ) < 2 ) {
    err( "Fail at X6" );
    return -2;
  }

  m1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
  m2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
  r1 = resolve_test( code, obj, other, m1, m2, op1, mode, detail, params, verb, show_debug );
  return r1;
}

static
int run_set ( mixed code, object obj, mixed other, mixed dest, mixed newval, string mode, string detail, mapping params, string verb, mixed show_debug ) {
  string s1, s2, propname;
  mixed setval, global_dest, global_from, env, m1, final;
  float f;

  if (!dest) {
     return FALSE;
  }

  if ( typeof(newval)==T_STRING ) {
   
    if ( sscanf( newval, "this.%s", s1 )==1 ) {
      /* if ( obj->query_property(s1)==nil )
         return FALSE; */
      setval = obj->query_property(s1);
    } else if ( sscanf( newval, "other.%s", s1 )==1 ) {
      /* if ( other->query_property(s1)==nil )
         return FALSE; */
      setval = other->query_property(s1);
    } else {
      if ( sscanf( newval, "global.%s.%s", s1, s2 )==2 ) {
	global_from = find_object(s1);
	if ( global_from==nil ) {
	  err( "Fail at X7" );
	  return -2;
	}
	setval = global_from->query_property( s2 );
      } else if ( sscanf( newval, "$obj(%s).%s", s1, s2 )==2 ) {
	m1 = resolve_ref( obj, other, s1, mode, detail, params );
	dbg( "$obj pre, m1 = " + dump_value(m1) );
	global_from = resolve_property( code, m1, obj, other, mode, detail, params, verb );
	dbg( "global_from = " + dump_value(global_from) );
	if ( global_from==nil ) {
	  err( "Fail at X7B" );
	  return -2;
	}
	setval = global_from->query_property( s2 );
      } else if ( sscanf( newval, "$obj(%s)", s1 )==1 ) {

      m1 = resolve_ref( obj, other, s1, mode, detail, params );
      m1 = resolve_obj( code, m1, obj, other, mode,detail,params, verb );
      if ( m1==nil ) {
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        m1 = resolve_variables( code, m1, detail, mode, obj, other,
          verb, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail,
          params, verb );
      }
      setval = m1;

      } else if ( sscanf( newval, "env.%s.%s", s2, s1 )==2 ) {
	if ( s2=="this" )
	  env = obj->query_environment();
	else
	  env = other->query_environment();
	dbg( "Finding "+s1+" in environment, " + dump_value(env) );
	setval = env->query_property(s1);
      } else {
	setval = newval;
      }
    }

    if ( setval=="nil" )
      setval = nil;
 
  } else {
    setval = newval; 
  }
   
  dbg( "Set value for "+dest+": " + dump_value( setval ) );
   
  if ( sscanf( dest, "this.%s", propname )==1 ) {
    /* obj->set_property( s1, setval ); */
    final = obj;
  } else if ( sscanf( dest, "other.%s", propname )==1 ) {
    /* other->set_property( s1, setval ); */
    final = other;
  } else {
    if ( sscanf( dest, "global.%s.%s", s1, propname )==2 ) {
      global_dest = find_object(s1);
      if ( global_dest==nil ) {
	err( "Fail at X8" );				 
	return -2;
      }
      /* global_dest->set_property( s2, setval ); */
      final = global_dest;
    } else if ( sscanf( dest, "$obj(%s).%s", s1, propname )==2 ) {
      m1 = resolve_ref( obj, other, s1, mode, detail, params );
      dbg( "m1 in $obj = " + dump_value(m1) );
      global_dest = resolve_property( code, m1, obj, other, mode, detail, params, verb );
      if ( global_dest==nil ) {
	err( "Fail at X8B" );
	return -2;
      }
      /* global_dest->set_property( s2, setval ); */
      final = global_dest;
    } else if ( sscanf( dest, "$obj(%s)", s1 )==1 ) {
      m1 = resolve_ref( obj, other, s1, mode, detail, params );
      m1 = resolve_obj( code, m1, obj, other, mode,detail,params, verb );
      if ( m1==nil ) {
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        m1 = resolve_variables( code, m1, detail, mode, obj, other,
          verb, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail,
          params, verb );
      }
      final = m1;

    } else if ( sscanf( dest, "env.%s.%s", s1, propname )==2 ) {
      if ( s1=="this" )
	env = obj->query_environment(); 
      else
	env = other->query_environment();
      final = env;
      /* env->set_property( s1, setval ); */
    } else {
      return FALSE;
    }
  }
  debug_property_update("run_set", code, obj, final, propname);
  final->set_property( propname, setval );
  if ( show_debug ) {
    if ( typeof(setval)==T_OBJECT )
      s1 = dump_value(setval) + " ("+describe_one(setval)+")";
    else
      s1 = dump_value(setval);

    run_emit( show_debug, code, obj, other, 
      "** -- (SET) " + dest + " to " + s1, 
      mode, detail, verb, params );
  }

  return TRUE;
}

mixed find_bilbo(object obj, string verb, string mode) {
  mapping sanity;
  mixed code, new;

  if (!obj) {
     return nil;
  }

  if (obj->query_property("bilbo:" + mode + ":" + verb) == "run") {
     return obj;
  }

  sanity = ([ ]);

  new = obj->query_property("bilbo:inherit:" + mode + ":" + verb);
  while (typeof(new) == T_OBJECT) {
     code = new;
     if (sanity[code]) {
	error("bilbo inheritance recursion");
     }
     sanity[code] = TRUE;
     new = code->query_property("bilbo:inherit:" + mode + ":" + verb);
  }
  if (code) {
     return code;
  }
  /* if there is neither a top-level script nor an inherit, do ur-search */
  while (new = obj->query_ur_object()) {
     obj = new;
     if (obj->query_property("bilbo:" + mode + ":" + verb) == "run") {
	return obj;
     }
  }
}

static run_create ( object thing, object dest, varargs int silently ) {
  object clone;

  clone = spawn_thing(thing);
  clone->action("teleport", ([ "dest" : dest ]), silently ? ACTION_SILENT : 0);
}

static 
mixed run_action( mixed code, object obj, object other, string oname, string msg, string mode, string detail, string verb, mapping params ) {
  mixed o, m;
  string s, s1, s2, name, msg1, msg2, msg3;
  object *aud, env;
  int i;

  /* -------  
  s1 = resolve_ref( obj, other, oname, mode, detail, params );
  o = resolve_obj( code, s1, obj, other, mode, detail, params );
  -------- */

  o = resolve_obj( code, oname, obj, other, mode, detail, params, verb );
  if ( o==nil ) {
    o = resolve_ref( obj, other, oname, mode, detail, params );
    o = resolve_variables( code, o, detail, mode, obj, other,
      verb, params );
    o = resolve_property( code, o, obj, other, mode, detail,
      params, verb );
  }
  dbg( "action object is " + dump_value(o) );
  dbg( "msg = " + dump_value(msg) ); 


  m = resolve_ref( obj, other, msg, mode, detail, params );
  dbg( "a m = " + dump_value(m) );
  m = resolve_variables( code, m, detail, mode, obj, other,
    verb, params );
  dbg( "b m = " + dump_value(m) );
  m = resolve_property( code, m, obj, other, mode, detail,
    params, verb );
  dbg( "c m = " + dump_value(m) );
  msg = m;

/*   m = resolve_variables( code, msg, detail, mode, obj, other, verb, params ); */

  dbg( "msg = " + dump_value(msg) );
   
  if ( sscanf( msg, "%s|%s", msg1, msg2 ) < 2 ) {
    dbg( "parse error in run_action for " + msg );
    return nil;
  }
  dbg( "msg1=" + dump_value(msg1) );
  dbg( "msg2=" + dump_value(msg2) );
  if ( o==nil )
    return nil;

  env = o->query_environment();   
  aud = env->query_inventory();
  aud -= ({ nil });

  for( i=0; i<sizeof(aud); i++ ) {
    if( aud[i]==o ) {
      if ( msg1 && strlen(msg1) )
	aud[i]->act_emote( msg1 );
    } else {
      if ( msg2 && strlen(msg2) )
	aud[i]->act_emote( msg2 );
    }
  }
  return TRUE;
}

static
mixed run_fill ( string arrname, mixed dest, mixed from, mixed objtype, mixed show_debug, mixed code, object obj, object other, string mode, string detail, mapping params, string verb ) {
  object *obs, tempo;
  int count, i, pass;
  string temps, parent, testmode, op;
  mixed m1, m2, testparent, testx, testy;

  obs = from->query_inventory();
   
  dbg( "objtype = " + dump_value(objtype) );

  if ( sscanf( objtype, "property %s %s %s", m1, op, m2 )==3 ) {
    testmode = "property";
    testx = "this."+m1;
    testy = m2;
  } else if ( objtype=="" )  {
    testmode = "all";
  } else if ( sscanf( objtype, "%s", parent )==1 ) {
    testmode = "parent";
    testparent = find_object(parent);
  }

  for( count=i=0; i<sizeof(obs); i++ ) {
    if ( obs[i] ) {
      pass = FALSE;
      switch( testmode ) {
        case "all":
          pass = TRUE;
          break;
        case "property":
          dbg( "testing "+dump_value(testx)+" v "+dump_value(m2) );

          m1 = resolve_ref( obs[i], other, testx, mode, detail, 
            params );
          dbg( "parval " + dump_value(m1) );
          m1 = resolve_variables( code, m1, detail, mode, obs[i],
            other, verb, params );
          m1 = resolve_property( code, m1, obs[i], other, mode,
            detail, params, verb );
          dbg( "parval " + dump_value(m1) );
          if ( m1 )
            m1 = ""+m1;
          else
            m1 = "nil";

          m2 = resolve_ref( obs[i], other, testy, mode, detail,
            params );
          dbg( "val2 " + dump_value(m2) );
          m2 = resolve_variables( code, m2, detail, mode, obs[i],
            other, verb, params );
          m2 = resolve_property( code, m2, obs[i], other, mode,
            detail, params, verb );
          dbg( "val2 " + dump_value(m2) );
          if ( m2 )
            m2 = ""+m2;
          else
            m2 = "nil";


          dbg( "testing m1 = " + dump_value(m1) + " m2 = " + dump_value(m2) );

          pass = resolve_test( code, obj, other, m1, m2, op,
            mode, detail, params, verb, show_debug );
          dbg( "test is " + pass + " for " + describe_one(obs[i]) );
          break;
        default:
          tempo = obs[i]->query_ur_object();
          dbg( "parent test: " + dump_value(tempo)+" and " + dump_value(testparent) );
          if ( tempo == testparent )
            pass = TRUE;
          break;
      }
      tempo = obs[i]->query_ur_object();
      if ( pass ) {
        dbg( "Found one!" );
	count++;
        dest->set_property( arrname+"["+count+"]", obs[i] );
      }
    }
  }
  dest->set_property( arrname+"[0]", count );
}

static
mixed run_empty( string arrname, mixed obj ) {
  int count;
  for( count=0; obj->query_property(arrname+"["+count+"]"); count++ ) {
    obj->set_property( arrname+"["+count+"]", nil );
    dbg( "Emptying " + count + " in " + arrname );
  }
}

static
mixed run_send( mixed dest, mixed msg, string emitmode ) {
  object *ppl, user;
  mixed *souls, m1;
  int i, j;

  dbg( "run_send to " + dump_value(dest) );

  if ( emitmode=="object" ) {
    ppl = ({ dest });
  } else {
    ppl = dest->query_inventory();
    ppl -= ({ dest });
  }
  
  for( i=0; i<sizeof(ppl); i++ ) {
    if ( ppl[i] ) {
      souls = ppl[i]->query_souls();
      for( j=0; j<sizeof(souls); j++ ) {
        user = souls[j];
        user->message( msg + "\n" );
      }
    }
  }
}



static
mixed fetch_array ( mixed code, object obj, mixed other, mixed dest, string mode, string detail, string verb, mapping params ) {
  mixed m1, m2;
  string s1, arname, index, next, s, a1, a2, prefix, text, msg;
  int count;

  if ( dest ) {
    text = " " + dest + " ";

    if( sscanf( text, "%s#%s", dest, arname )<2 ) {
      dbg( "Bad array reference!" );
      return nil;
    }
    arname = strip(arname);

    dbg( "dest: " + dump_value(dest) );
    dbg( "arname: " + dump_value(arname) );
    m2 = nil;
    prefix = "bilbo:"+mode+":"+verb+":#";

    if ( sscanf( dest, " %s.", a1 )==1 ) {
      dbg( "a1 " + dump_value(a1) );
      m2 = resolve_variables( code, a1, detail, mode, obj, other,
        verb, params );
      m2 = resolve_obj( code, m2, obj, other, mode, detail, params, verb );
      dbg( "m2 " + dump_value(m2) );
      if ( m2 )
        dest = "  ";
      prefix = "#";
    }

    dbg("res_array s1 = "+dest+" m2 = " + dump_value(m2) );
    if ( !m2 )
      m2 = obj;
 
    if ( m2==nil ) {
      dbg( "m2 is nil" );
      return nil;
    }
  }

  return ([ "prefix":prefix, "obj":m2, "name":arname,
    "fullname":prefix+arname ]);
}


static
void mod_array ( string modmode, mixed code, object obj, mixed other, mixed element, mixed dest, string mode, string detail, string verb, mapping params ) {
  mixed test;
  int i, num, arcnt;
  mixed arobj;
  string arstrsize;

  arobj = fetch_array( code, obj, other, dest, mode, detail, verb,
    params );
  dbg( "arobj = " + dump_value(arobj) );
  dbg( "element = " + dump_value(element) );

  arstrsize = arobj["obj"]->query_property( arobj["fullname"]+"[0]" );
  if ( arstrsize ) {
    sscanf( arstrsize, "%d", arcnt );
    if ( !arcnt ) {
      dbg( "array count of nil!" );
      return;
    }
  } else {
    arcnt = 0;
    arstrsize = "0";
  }

  if ( modmode=="put" ) {

    for( i=1; i<=arcnt; i++ ) {
      test = arobj["obj"]->query_property( arobj["fullname"]+"["+
        i+"]" );
      dbg( "testing "+arobj["fullname"]+i+"="+dump_value(test) );
      if ( test ) {
        dbg( "test = " + dump_value(test) );
        if ( test == element ) {
          dbg( "already in there" );
          return;
        }
      }
    }
    arcnt++;
    arstrsize = "" + arcnt; 
    arobj["obj"]->set_property(arobj["fullname"]+"[0]", arstrsize);
    arobj["obj"]->set_property(arobj["fullname"]+"["+arstrsize+"]",
      element );

  } else {
    dbg( "arcnt = " + arcnt );
    for( i=1; i<=arcnt; i++ ) {
      test = arobj["obj"]->query_property( arobj["fullname"]+"["+
        i+"]" );
      dbg( "testing "+arobj["fullname"]+i+"="+dump_value(test) );
      if ( test ) {
        dbg( "test "+i+" = " + dump_value(test) );
        dbg( "element = " + dump_value(element) );
        if ( test == element ) {
          dbg( "killing element" );
          if ( i==arcnt ) {
            dbg( "resetting last" );
            arobj["obj"]->set_property( arobj["fullname"]+"["+i+"]",
              nil );
          } else {
            dbg( "moving last" );
            arobj["obj"]->set_property( arobj["fullname"]+"["+i+"]",
              arobj["obj"]->query_property( 
                arobj["fullname"]+"["+ arcnt + "]") );
            arobj["obj"]->set_property( 
              arobj["fullname"]+"["+arcnt+"]",nil );
          }
          arcnt--;
          arstrsize = "" + arcnt;
          arobj["obj"]->set_property( arobj["fullname"]+"[0]",
            arstrsize );

          return;
        }
      }
      dbg( "element not found" );
    }
 
  } 
}



static
void run_emitx ( string emitmode, mixed code, object obj, mixed other, mixed dest, mixed newval, string mode, string detail, string verb, mapping params ) {
  mixed m1, m2;
  string s1, s, a1, a2, text, msg;
  int count;
  mixed arobj;

  dbg( "fetching s2 from " + newval );
  m2 = resolve_ref( obj, other, newval, mode, detail, params );
  dbg( "m2 = " + dump_value(m2) );
  m2 = resolve_variables( code, m2, detail, mode, obj, other,
    verb, params );
  msg = run_sam( m2, obj, other, mode );

  if ( !msg ) {
    dbg( "Message unresolved!" );
  }
  dbg( "msg=" + msg );

  if ( emitmode=="array" ) {

    arobj = fetch_array( code, obj, other, dest, mode, detail, verb,
      params );
    dbg( "arobj = " + dump_value(arobj) );

    for( count=1; 
      m1 = arobj["obj"]->query_property( arobj["fullname"]+
        "["+count+"]" );
      count++ ) {
      dbg( "m1 = " + dump_value(m1) );
      run_send( m1, msg, "object" );
      run_send( m1->query_environment(), msg, "object" );
    }

  } else {
    if ( typeof(dest)==T_STRING ) {

      m1 = resolve_ref( obj, other, dest, mode, detail, params );
      dbg( "A m1 = " + dump_value(m1) );
      m1 = resolve_variables( code, m1, detail, mode, obj, other, 
        verb, params );
      m1 = resolve_property( code, m1, obj, other, mode, detail, 
        params, verb );

      dbg( "B m1 = " + dump_value(m1) );
      m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
      m1 = resolve_property( code, m1, obj, other, mode, detail, 
        params, verb );
    } else {
      m1 = dest;
    }

    dbg( "C m1 = " + dump_value(m1) );
    if ( m1 && msg ) {
      run_send( m1, msg, emitmode );
    }
  }

}

mapping find_adjacent_rooms( int dist, mapping roomdata, mapping seen ) {
  string *details, *newdetails;
  int i, j, k, choice;
  mixed dest, back, route;
  object *rooms, env;

  rooms = roomdata[dist-1];
  for( i=0; i<sizeof(rooms); i++ ) {
    env = rooms[i];    
    details = ur_details(env);
    details -= ({ nil });

    for( j=0; j<sizeof(details); j++ ) {
      if ( dest = ur_exit_dest(env, details[j]) ) {
	 if (!NRefOb(dest)) {
	    SysLog("ERK: null object in exit nref: [" + name(env) + "/" + details[j]);
	    continue;
	 }
	if (seen[NRefOb(dest)] != nil) {
	  continue;
	}
        if ( roomdata[NRefOb(dest)]==nil ) {
	  newdetails = ur_details(NRefOb(dest));
          route = nil;
          for( k=0; k<sizeof(newdetails) && route==nil; k++ ) {
            if ( back = ur_exit_dest( NRefOb(dest), newdetails[k] ) ) {
              if (NRefOb(back) == env) {
                route = newdetails[k];
                continue;
              }
            }
          }
          if ( route ) {
            roomdata[NRefOb(dest)] = route;
	  }
	  dbg( "adding " + describe_one(NRefOb(dest)) + " to level " +  dist );
	  roomdata[dist] += ({ NRefOb(dest) });
	  seen[NRefOb(dest)] = dist;
        }
      }
    }
  }

  return roomdata;
}

static void delayed_emitdistant(object **rooms, string *msgs, mapping source,
				int total, int total_inv, mixed code,
				object obj, mixed other, string mode,
				string detail, string verb, mapping params);

static
void run_emitdistant ( mixed code, object obj, mixed other, mixed startroom, mixed msg, string mode, string detail, string verb, mapping params ) {
  mixed sr;
  mapping roomdata, exits, finddata, seen;
  object **rooms;
  string *splitmsg;
  int i, j, distance, total, total_inv;

  if (!startroom || !msg) {
     return;
  }

  sr = resolve_ref(obj, other, startroom, mode, detail, params);
  sr = resolve_variables(code, sr, detail, mode, obj, other, verb, params);
  Debug("run_emitdistant: startroom = " + dump_value(sr) + " (" + dump_value(startroom) + ")");
  if (!sr || typeof(sr) != T_OBJECT) {
     sr = obj;
  }

  rooms = ({ });
  roomdata = ([ ]);

  splitmsg = explode(msg, "|");
  distance = sizeof(splitmsg)-1;

  if (sr->query_environment()) {
    roomdata[sr->query_environment()] = "";
    roomdata[0] = ({ sr->query_environment() });
  } else {
    roomdata[sr] = "";
    roomdata[0] = ({ sr });
  }

  for( i=1; i<=distance+1; i++ )
    roomdata[i] = ({ });

  seen = ([ ]);
  roomdata = find_adjacent_rooms( 1, roomdata, seen );

  for( i=1; i<=distance; i++ ) {
    roomdata = find_adjacent_rooms( i+1, roomdata, seen );
  }

  rooms = allocate(distance + 1);
  for (i = 0; i <= distance; i++) {
     rooms[i] = roomdata[i];
     if (!rooms[i] || !sizeof(rooms[i])) {
	rooms[i] = nil;
	splitmsg[i] = nil;
     }
  }
  delayed_emitdistant(rooms - ({ nil }), splitmsg - ({ nil }), roomdata, 0, 0,
		      code, obj, other, mode, detail, verb, params);
}

/*
 * Delayed emit.distant implementation.  It can be called directly the first
 * time or start from a 0 call_out, whichever people seem happier with.
 */
static void
delayed_emitdistant(object **rooms, string *msgs, mapping source,
		    int total, int total_inv,
		    mixed code, object obj, mixed other, string mode,
		    string detail, string verb, mapping params)
{
   int count, local_inv;

   while (count < 32 && sizeof(rooms)) {
      string msg;
      object room;

      /* Pick room and message */
      room = rooms[0][0];
      msg  = msgs[0];

      /* Shift stuff around */
      rooms[0] = rooms[0][1..];
      if (!sizeof(rooms[0])) {
	 rooms = rooms[1..];
	 msgs  = msgs[1..];
      }
      count++;

      /* Do actual emitting */
      params["source"] = source[room];
      local_inv += sizeof(room->query_inventory());
      run_emitx("room", code, obj, other, room, msg, mode, detail, verb,
		params);
      
   }
   total     += count;
   total_inv += local_inv;

   Debug("delayed_emitdistant() - " + total + " rooms; " + total_inv +
	 " objects.");
   Debug("strings: " + dump_value(msgs));
   if (sizeof(rooms)) {
      call_out("delayed_emitdistant", 0.1, rooms, msgs, source,
	       total, total_inv, code, obj, other, mode, detail, verb, params);
      return;
   }
   Debug("delayed_emitdistant() finished!");
}

mixed push_params( mapping params, string verb, string mode, int line, object code ) {
  mixed depth;
  mapping chunk;

  dbg( "push params = " +dump_value(params) );

  depth = params["stackdepth"];
  if ( depth==nil )
    depth = 0;
  params["stackdepth"] = ++depth;

  chunk = ([ ]);
  chunk["verb"] = verb;
  chunk["mode"] = mode;
  chunk["line"] = line;
  chunk["code"] = code;

  dbg( "pushing chunk "+dump_value(chunk)+" at depth "+depth );

  params["CHUNK"+depth] = chunk;
  return params;
}

mixed add_params ( mapping params, string text, mixed obj, mixed
other, mixed code, string mode, string verb, string detail ) {
  int count;
  mixed parval;
  string parname, *parlist;

  text += ", .";
  count = 0;
  parlist = ({ });

  dbg( "current params " + dump_value(params) );

  while( sscanf( text, "%s as $%s, %s", parval, parname, text ) ) {

    parval = resolve_ref( obj, other, parval, mode, detail, params );
    dbg( "parval " + dump_value(parval) );
    parval = resolve_variables( code, parval, detail, mode, obj, 
      other, verb, params );
    parval = resolve_property( code, parval, obj, other, mode, 
      detail, params, verb );
    dbg( "parval " + dump_value(parval) );

    params[parname] = parval;
    parlist += ({ parname });
    count++;
  }

  if ( count ) 
    params["stackparlist"+params["stackdepth"]] = parlist;
  
  dbg( "with added params " + dump_value(params) );

  return params;
}

mixed find_stance ( string prep ) {
  mixed map;
  dbg( "trying to find " + dump_value(prep) );
  map = ([ 
    "closeto": PREP_CLOSE_TO,
    "against": PREP_AGAINST,
    "under": PREP_UNDER,
    "on": PREP_ON,
    "inside": PREP_INSIDE,
    "near": PREP_NEAR,
    "over": PREP_OVER,
    "behind": PREP_BEHIND,
    "before": PREP_BEFORE,
    "beside": PREP_BESIDE
    ]);
  dbg( "is " + dump_value(map[prep]) );
  return map[prep];
}


static
mixed parse_bilbo ( mixed code, int startline, object obj, mixed other, string verb, string mode, string detail, varargs mapping params ) {
  int line, x, result, count, curcmd, i;
  string s1, s2, s3, s4, text, sline, *commands, *sar;
  mixed ret, m1, m2, m3, m4, show_debug, *mar, user, souls, dtl, stancemap;

  if (!code) {
      DEBUG("parse_bilbo() code containing object was slain during delay?");
      return nil;
  }
  if (!obj) {
      DEBUG("parse_bilbo() target object was slain during delay?");
      return nil;
  }

  dbg( "m1 = " + dump_value(m1) );

  dbg( "parse_bilbo detail: " + dump_value(detail) );
  dbg( "code: " + dump_value(code) );
   
  dbg( "PARSE code="+describe_one(code)+" obj="+describe_one(obj)+" other="+describe_one(other) );
   
  ret = nil;
  show_debug = FALSE;   
  if ( mode=="react" || mode=="auto" ) {
    if ( other->query_property("bilbo:debug") ) 
      show_debug = other;
  } else {
    if ( obj->query_property("bilbo:debug") ) 
      show_debug = obj;
  }

  {
     /*
      * Skip the $evoke, $actor and $target parameters since those conflict
      * with Bilbo's own ideas. Note that this DESTROYS ELEMENTS IN THE
      * MAPPING PASSED TO BILBO. Added: $this.
      */
     int    i, sz;
     string *parlist;

     parlist = ({ "this", "evoke", "actor", "target" });
     sz = sizeof(parlist);
     for (i = 0; i < sz; i++) {
	params[parlist[i]] = nil;
     }
  }

  if ( show_debug ) {

    run_emit( show_debug, code, obj, other,
      "\n** SCRIPT = bilbo:"+ mode+":"+verb,
      mode, detail, verb, params );

    run_emit( show_debug, code, obj, other,
      "\n** CODE = " +
	      (typeof(code) == T_OBJECT ? name(code) :
	       dump_value(code)),
      mode, detail, verb, params );

    run_emit( show_debug, code, obj, other,
      "** THIS = " + describe_one(obj)+" "+
	      (typeof(obj) == T_OBJECT ?
	       "[" + name(obj) + "]" : dump_value(obj)),
      mode, detail, verb, params );

    run_emit( show_debug, code, obj, other,
      "** OTHER = " + describe_one(other)+" "+
	      (typeof(other) == T_OBJECT ?
	       "[" + name(other) +"]" : dump_value(other)) +
      "\n*********************************************************",
      mode, detail, verb, params );
  }

  for( count=0, line=startline; line; line+=10, count++ ) {
    sline = "0" + line;
    if ( line < 999 ) {
      while( strlen(sline) < 4 )
	sline = "0"+sline;
    } else {
      sline=sline[1..];
    }

    if ( count > 1000 ) {  /* may need to up this */
      do_fail( code, "001", obj, other, "Infinite loop line" +sline, 
        mode, detail, verb );
      return ret;
    }
	 
    text = code->query_property( "bilbo:" + mode + ":" + verb + ":" 
      + sline );
    commands = ({ });
    if ( text && sscanf( text, "%s :: %s", s1, s2 )==2 ) {
      commands += ({ s1 });
      while( sscanf( s2, "%s :: %s", s1, s2 )==2 ) {
        commands += ({ s1 });
      }
      commands += ({ s2 });
    } else {
      commands += ({ text });
    }
    commands -= ({ nil });

    dbg( "Commands: " + dump_value(commands) );
    dbg( "sizeof="+sizeof(commands) );

    if ( sizeof(commands) < 1 ) {
      /* dbg( "BILBO lines executed: " + count ); */

      if ( show_debug ) {
        run_emit( show_debug, code, obj, other,
          "** -- (EXIT " + (ret ? "TRUE)" : "FALSE)"),
          mode, detail, verb, params );
      }
      return ret;
    }

    for( curcmd=0; curcmd < sizeof(commands); curcmd++ ) {
      text = strip( commands[curcmd] );

      if ( sscanf( text, "%*s//%*s" )==2 )
	sscanf( text, "%s//%*s", text );

      dbg( "BILBO "+mode+":"+verb+":"+sline+"  "+text );

      if ( show_debug ) {
        if ( sizeof(commands) > 1 )
          run_emit( show_debug, code, obj, other, "** " + sline
            +"-"+(curcmd+1)+": " + text, mode, detail, verb, params );
        else
          run_emit( show_debug, code, obj, other, "** " + sline 
            + ": " + text, mode, detail, verb, params );      
      }

      text = strip_left(text);
      if (!strlen(text)) {
	 continue;
      }

      while ( sscanf( text, "if %s then %s", s1, s2 )==2 ) {
	s3 = nil;
	if ( sscanf( s2, "%*s else %*s" )==2 )
	  sscanf( s2, "%s else %s", s2, s3 );
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	result = run_if( code, obj, other, s1, params, detail, mode, verb, show_debug );

        if ( show_debug ) {
          run_emit( show_debug, code, obj, other, 
            "** -- " + (result ? "(TRUE)" : "(FALSE)"), 
            mode, detail, verb, params );
        }

        dbg( "result " + dump_value(result) );
	
	switch( result ) {
	case -2:
	  err( "Fail at M1" );
	  do_fail( code, "002", obj, other, "Error line "+sline+": " + text, mode, detail, verb );
	  return ret;
	case FALSE:
	  dbg( "this if is false: " + s1 );
	  text = s3 ? strip(s3) : "rem  ";
	  if ( s3 )
	    dbg( "else is executed: " + s3 );
	  continue;
	default:
	  dbg( "this if is true: " + s1 );
	  dbg( "if statement is true, then = " + s2 );
	  text = strip(s2);
	  dbg( "text = " + dump_value(text) );
          continue;
	}			
      }

      if (sscanf(text, "delay %s returning %s", m1, s1) == 2 && strlen(s1)) {
	  int exit_value;

	  switch (lower_case(s1)) {
	  case "true":
	  case "continue":
	     exit_value = TRUE;
	      break;
	  case "false":
	  case "nil":
	      exit_value = FALSE;
	      break;
	  default:
	      /*
	       * Non comprendo, fall back to original parser behaviour,
	       * which will most likely mean it does a 'delay %d'.
	       */
	      exit_value = -1;
	      break;
	  }
	  if (exit_value != -1) {
	      m1 = resolve_ref(obj, other, m1, mode, detail, params);
	      m1 = resolve_variables(code, m1, detail, mode, obj, other, verb,
				     params);
	      call_out("parse_bilbo", (int)m1, code, line+10, obj, other, verb,
		       mode, detail, params);
	      return exit_value;
	  }
      }
      if ( sscanf( text, "delay %s", m1 )==1 ) {
	 m1 = resolve_ref(obj, other, m1, mode, detail, params);
	 m1 = resolve_variables(code, m1, detail, mode, obj, other, verb,
				params);
	 call_out( "parse_bilbo", (int)m1, code, line+10, obj, other, 
		   verb, mode, detail, params );
	 return TRUE;
      }
      if ( sscanf( text, "emit.actor %s", s1 ) ) {
	if ( mode=="react" || mode=="auto" )
	  run_emit1( code, obj, other, s1, mode, detail, verb, params );
	else
	  run_emit2( code, obj, other, s1, mode, detail, verb, params );
	continue;
      }
      if ( sscanf( text, "emit.target %s", s1 ) ) {
	if ( mode=="react" || mode=="auto" )
	  run_emit2( code, obj, other, s1, mode, detail, verb, params );
	else
	  run_emit1( code, obj, other, s1, mode, detail, verb, params );
	continue;
      }
      if ( sscanf( text, "emit.audience %s", s1 ) ) {
	run_emit3( code, obj, other, s1, mode, detail, verb, params );
	continue;
      }

      if ( sscanf( text, "emit.room %s %s", s1, s2 )==2 ) {
        run_emitx( "room", code, obj, other, s1, s2, mode, detail, verb, params );
        continue;
      }

      if ( sscanf( text, "emit.distant %s %s", s1, s2 )==2 ) {
        run_emitdistant( code, obj, other, s1, s2, mode, detail, 
			 verb, params );
        continue;
      }

      if ( sscanf( text, "emit.object %s %s", s1, s2 )==2 ) {
        run_emitx( "object", code, obj, other, s1, s2, mode, detail, verb, params );
        continue;
      }
      if ( sscanf( text, "emit.array %s %s", s1, s2 )==2 ) {
        run_emitx( "array", code, obj, other, s1, s2, mode, detail,
          verb, params );
        continue;
      }

      if ( sscanf( text, "put %s in %s", s1, s2 )==2 ) {

        m1 = resolve_obj( code, s1, obj, other, mode, detail, params, verb );
        if ( m1==nil ) {
          m1 = resolve_ref( obj, other, s1, mode, detail, params );
          m1 = resolve_variables( code, m1, detail, mode, obj, other, 
            verb, params );
          m1 = resolve_property( code, m1, obj, other, mode, detail,
            params, verb );
        }

        if ( m1==nil ) {
          dbg( "bad element reference" );
          continue;
        }

        mod_array( "put", code, obj, other, m1, s2, mode, detail, 
          verb, params );
        continue;
      }
      if ( sscanf( text, "remove %s from %s", s1, s2 )==2 ) {

        m1 = resolve_obj( code, s1, obj, other, mode, detail, params , verb);        if ( m1==nil ) {
          m1 = resolve_ref( obj, other, s1, mode, detail, params );
          m1 = resolve_variables( code, m1, detail, mode, obj, other, 
            verb, params );
          m1 = resolve_property( code, m1, obj, other, mode, detail,
            params, verb );
        }

        if ( m1==nil ) {
          dbg( "bad element reference" );
          continue;
        }

        mod_array( "remove", code, obj, other, m1, s2, mode, detail,
          verb, params );
        continue;
      }

      if ( sscanf( text, "action for %s %s", s1, s2 )==2 ) {
	if ( !run_action( code, obj, other, s1, s2, mode, detail, verb, params ) ) {
	  dbg( "Error in run_action" );
	  return ret;
	}
	continue;
      }		 
      if ( text=="exit" ) {
	return ret;
      }
      if ( sscanf( text, "exit %s", s1 )==1 ) {
	switch( s1 ) {
	case "continue":
	case "true":
	  ret = TRUE;
	  break;
	case "false":
	case "nil":
	  ret = nil;
	  break;
	}
	dbg( "BILBO returning " + dump_value(ret) );

        if ( show_debug ) {
          run_emit( show_debug, code, obj, other,
            "** -- (EXIT " + (ret ? "TRUE)" : "FALSE)"),
            mode, detail, verb, params );
        }

	return ret;
      }
      if ( sscanf( text, "goto %d", line )==1 ) {
        dbg( "setting line to " + dump_value(line) );
	line -= 10;
	curcmd = sizeof(commands);
	continue;
      }
      if ( sscanf( text, "rem%*s" )==1 ) {
	continue;
      }
      if ( sscanf( text, "create silently %s in %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	m1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
	if ( typeof(m1)!=T_OBJECT )
	  m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
	m2 = resolve_obj( code, s2, obj, other, mode, detail, params , verb);
	if ( m1 && m2 )
	  run_create( m1, m2, TRUE );
	continue;
      }

      if ( sscanf( text, "create %s in %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	m1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
	if ( typeof(m1)!=T_OBJECT )
	  m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
	m2 = resolve_obj( code, s2, obj, other, mode, detail, params , verb);
	if ( m1 && m2 )
	  run_create( m1, m2 );
	continue;
      }

      if ( sscanf( text, "slay %s", s1 ) == 1 ) {
	 dbg("parse_bilbo() s1: " + dump_value(s1));
	 m1 = resolve_obj( code, s1, obj, other, mode, detail, params , verb);
         dbg("parse_bilbo() m1: " + dump_value(m1));
	 if (m1)
	  m1->suicide();
	 continue;
      }
      if ( sscanf( text, "empty %s in %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	m2 = resolve_obj( code, s2, obj, other, mode, detail, params , verb);
	if ( s1 && m2 )
	  run_empty( s1, m2 );
	continue;
      }
      if ( sscanf( text, "fill %s in %s with %s(%s)", s1, s2, s3, s4 )==4 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	m2 = resolve_obj( code, s2, obj, other, mode, detail, params, verb );
	m3 = resolve_obj( code, s3, obj, other, mode, detail, params, verb );
	dbg( "Fill s1 = " + dump_value(s1) );
	dbg( "Fill m2 = " + dump_value(m2) );
	dbg( "Fill m3 = " + dump_value(m3) );

        /* -----
	m1 = find_object(s4);
	dbg( "Fill m1 = " + dump_value(m1) );
        ------ */

	if ( s1 && m2 && m3 ) {
	  run_fill( s1, m2, m3, s4, show_debug, code, obj, other,
            mode, detail, params, verb );
	}
	continue;
      }


      if ( sscanf( text, "delete detail %s in %s", s1, s2) == 2) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	m2 = resolve_obj( code, s2, obj, other, mode, detail, params, verb );

	if (s1 && m2) {
	   m2->remove_detail(s1);
	}
	continue;
      }
      if ( sscanf( text, "delete description %s in detail %s in %s", s1, s2, s3) == 3) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	s2 = resolve_ref( obj, other, s2, mode, detail, params );
	m3 = resolve_obj( code, s3, obj, other, mode, detail, params, verb );

	if (s1 && s2 && m3) {
	   m3->remove_description(s2, s1);
	}
	continue;
      }


      if ( sscanf( text, "wear %s", s1 )== 1 ) {
        DEBUG( "Bilbo:wear:s1 = " + dump_value(s1) );
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        DEBUG( "Bilbo:wear:m1 = " + dump_value(m1) );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
        DEBUG( "Bilbo:wear:m1 = " + dump_value(m1) );
	m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        DEBUG( "Bilbo:wear:m1 = " + dump_value(m1) );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );
        DEBUG( "Bilbo:wear:m1 = " + dump_value(m1) );
        if (m1 && m1->query_environment()) {
           m1->query_environment()->action("wear",
					   ([ "articles": ({ m1 }) ]),
					   ACTION_FORCED | ACTION_SILENT);
	}
        continue;
      }
      if ( sscanf( text, "wield %s", s1 )== 1 ) {
        dbg( "m1a in wield is " + dump_value(m1) );
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        dbg( "m1b in wield is " + dump_value(m1) ); 
        
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
        dbg( "m1c in wield is " + dump_value(m1) );

        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );

        dbg( "wielding " + dump_value(m1));
        if (m1 && m1->query_environment()) {
           m1->query_environment()->action("wield",
					   ([ "article": m1 ]),
					   ACTION_FORCED | ACTION_SILENT);
	}
        continue;
      }
      if ( sscanf( text, "unwear %s", s1 )== 1 ) {
        dbg( "m1a in unwear is " + dump_value(m1) );
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        dbg( "m1b in unwear is " + dump_value(m1) ); 
        
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
        dbg( "m1c in unwear is " + dump_value(m1) );

        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );

        dbg( "unwearing " + dump_value(m1));
        if (m1 && m1->query_environment()) {
           m1->query_environment()->action("unwear",
					   ([ "articles": ({ m1 }) ]),
					   ACTION_SILENT | ACTION_FORCED);
	}
        continue;
      }
      if ( sscanf( text, "unwield %s", s1 )== 1 ) {
        dbg( "m1a in unwield is " + dump_value(m1) );
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        dbg( "m1b in unwield is " + dump_value(m1) ); 
        
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
        dbg( "m1c in unwield is " + dump_value(m1) );

        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );

        dbg( "unwielding " + dump_value(m1));
        if (m1 && m1->query_environment()) {
           m1->query_environment()->action("unwield",
					   ([ "article": m1 ]),
					   ACTION_SILENT | ACTION_FORCED);
	}
        continue;
      }

      if ( sscanf( text, "teleport silently %s to %s %s %s", s1, s2, s3, s4 ) >=2 ||
        sscanf( text, "teleport silently %s to %s", s1, s2 )==2 ) {
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );

        m2 = resolve_ref( obj, other, s2, mode, detail, params );
        m2 = resolve_property( code, m2, obj, other, mode, detail, params, verb );
        m2 = resolve_variables( code, m2, detail, mode, obj, other, verb, params);

        if ( m1 && m2 )
           m1->action("teleport", ([ "dest": m2 ]), ACTION_SILENT);

	if ( s3 && s4 ) {
          souls = m1->query_souls();
          if ( souls )
            user = souls[0];
	  if ( user ) {
            m3 = ({ ({ nil, 0, 0, s4 }) });
            dtl = locate_one( user, FALSE, m3, m1, m2 );
	    dbg( "teleport dtl " + dump_value(dtl) );
            m4 = find_stance(s3);
	    m1->action( "approach", 
              ([ "what" : NRefOb(dtl)->query_proximity() ]) );
	  }
	}
          
        continue;
      }
      
      if ( sscanf( text, "teleport %s to %s %s %s", s1, s2, s3, s4 ) >=2 ||
        sscanf( text, "teleport %s to %s", s1, s2 )==2 ) {

        dbg( "s1 " + dump_value(s1) );
        dbg( "s2 " + dump_value(s2) );
	      
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );

        m2 = resolve_ref( obj, other, s2, mode, detail, params );
        m2 = resolve_property( code, m2, obj, other, mode, detail, params, verb );
        m2 = resolve_variables( code, m2, detail, mode, obj, other, verb, params);
        dbg( "m1 " + dump_value(m1) );
        dbg( "m2 " + dump_value(m2) );

        if ( m1 && m2 ) {
           m1->act_teleport( m2 );  
	}

	if ( s3 && s4 ) {
          souls = m1->query_souls();
          if ( souls )
            user = souls[0];
	  if ( user ) {
            m3 = ({ ({ nil, 0, 0, s4 }) });
            dtl = locate_one( user, FALSE, m3, m1, m2 );
	    dbg( "teleport dtl " + dump_value(m1) );
	   
	    /* ---
	    m1->action( "approach", 
              ([ "target" : NRefOb(dtl)->query_proximity() ]) );
	    --- */

            m4 = find_stance(s3);
	    dbg( "STANCE " + dump_value(m4) );
	    m1->action( "stance", ([
	       "stance": STANCE_STAND,
	       "prep": m4,
	       "target": dtl
	       ]) );
	      
	  }
	}

	
        continue;
      }

      if ( sscanf( text, "this.teleport %s", s1 ) ) {
	obj->act_teleport( find_object(s1) );
	continue;
      }
      if ( sscanf( text, "message to %s %s", s1, s2 )==2 ) {
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        dbg( "A m1 = " + dump_value(m1) );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

        dbg( "B m1 = " + dump_value(m1) );
        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

        dbg( "C m1 = " + dump_value(m1) );
        if ( m1 && s2 ) {
           dbg( "fetching s2 from " + s2 );
           m2 = resolve_ref( obj, other, s2, mode, detail, params );
           dbg( "m2 = " + dump_value(m2) );
           m2 = resolve_variables( code, m2, detail, mode, obj, other, verb, params );
           dbg( "and = " + dump_value(m2) );
           if ( m2 ) {
             m2 = run_sam( m2, obj, other, mode );
             run_send( m1, m2, "object" );
           }
        }
      }
      /* The 'force' functionality really needs modular Bilbo */
      /* ~SkotOS does not depend on ~TextIF -- badness */
      if ( sscanf( text, "force %s to %s", s1, s2 )==2 ) {
        m1 = resolve_ref( obj, other, s1, mode, detail, params );
        dbg( "A m1 = " + dump_value(m1) );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

        dbg( "B m1 = " + dump_value(m1) );
        m1 = resolve_obj( code, m1, obj, other, mode, detail, params, verb );
        m1 = resolve_property( code, m1, obj, other, mode, detail, params, verb );

        dbg( "C m1 = " + dump_value(m1) );
	/* DEBUG("BILBO force: s1 = " + dump_value(s1) + "; s2 = " + dump_value(s2) + "; m1 = " + dump_value(m1)); */
        if ( m1 && s2 ) {
           dbg( "fetching s2 from " + s2 );
           m2 = resolve_ref( obj, other, s2, mode, detail, params );
           dbg( "m2 = " + dump_value(m2) );
           m2 = resolve_variables( code, m2, detail, mode, obj, other, verb, params );
           dbg( "and = " + dump_value(m2) );
	   if (m2) {
	      object soul;

	      soul = sizeof(m1->query_souls()) ? m1->query_souls()[0] :
		 this_object();
	      /* DEBUG("BILBO force: " + dump_value(m1) + " -> " + dump_value(m2)); */
	      "/usr/TextIF/main"->command(soul, m1, m2, TRUE);
           }
        }
	continue;
      }
      if ( sscanf( text, "other.teleport %s", s1 ) ) {
	other->act_teleport( find_object(s1) );
	continue;
      }
      if ( sscanf( text, "add %s to %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	s1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
							
	s2 = resolve_ref( obj, other, s2, mode, detail, params ); 
	s2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
	if ( run_modval( code, obj, other, s1, s2, "add", mode, detail, params, verb )==FALSE ) {
	  do_fail( code, "005", obj, other, "Error line "+sline+": "+text, mode, detail, verb );
	  return ret;
	}
	continue;
      }
      if ( sscanf( text, "sub %s from %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	s1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
			
	s2 = resolve_ref( obj, other, s2, mode, detail, params ); 
	s2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
	if ( run_modval( code, obj, other, s1, s2, "sub", mode, detail, params, verb )==FALSE ) {
	  do_fail( code, "006", obj, other, "Error line "+sline+": "+text, mode, detail, verb );
	  return ret;
	}
	continue;
      }
      if ( sscanf( text, "mul %s by %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	s1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
			
	s2 = resolve_ref( obj, other, s2, mode, detail, params ); 
	s2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
	if ( run_modval( code, obj, other, s2, s1, "mul", mode, detail, params, verb )==FALSE ) {
	  do_fail( code, "007", obj, other, "Error line "+sline+": "+text, mode, detail, verb );
	  return ret;
	}
	continue;
      }
      if ( sscanf( text, "div %s by %s", s1, s2 )==2 ) {
	s1 = resolve_ref( obj, other, s1, mode, detail, params );
	s1 = resolve_variables( code, s1, detail, mode, obj, other, verb, params );
			
	s2 = resolve_ref( obj, other, s2, mode, detail, params );
	s2 = resolve_variables( code, s2, detail, mode, obj, other, verb, params );
	if ( run_modval( code, obj, other, s2, s1, "div", mode, detail, params, verb )==FALSE ) {
	  do_fail( code, "008", obj, other, "Error line "+sline+": "+text, mode, detail, verb );
	  return ret;
	}
	continue;
      }

      if ( sscanf( text, "set %s to %s", s1, s2 )==2 ) {
	/* DEBUG("set <" + s1 + "> to <" + s2 + ">"); */
	m1 = resolve_ref( obj, other, s1, mode, detail, params );
	m1 = resolve_variables( code, m1, detail, mode, obj, other, verb, params );
			
	m2 = resolve_ref( obj, other, s2, mode, detail, params );
	m2 = resolve_variables( code, m2, detail, mode, obj, other, verb, params );
	dbg( "run_set m1="+dump_value(m1) + " m2="+dump_value(m2) );
	/* DEBUG("run_set(" + dump_value(m1) + ", " + dump_value(m2) + ")"); */

	if ( !run_set( code, obj, other, m1, m2, mode, detail, params, verb, show_debug ) ) {
	  do_fail( code, "009", obj, other, "Error line "+sline+": "+text, mode, detail, verb );
	  return ret;
	}
	continue;
      }

      if ( sscanf( text, "function %s:%s:%s with %s returning %s", 
        m1, m2, m3, s1, s2 )==5 ) {
        m1 = "/usr/"+ m1 + "/bilbofuns/"+m2;
        dbg( "m1 = " + dump_value(m1) );
        
        sar = explode(s1," ");
        mar = ({ });
        dbg( "sar = " + dump_value(sar) );

        for( i=0; i<sizeof(sar); i++ ) {
          /* --------
          m4 = resolve_ref( obj, other, sar[i], mode, detail, params );
          m4 = resolve_variables( code, m4, detail, mode, obj, 
            other, verb, params );
          ----------- */

          m4 = resolve_obj( code, sar[i], obj, other, mode, detail, 
            params, verb );
          if ( m4==nil ) {
            m4 = resolve_ref( obj, other, sar[i], mode, detail, 
              params );
            m4 = resolve_variables( code, m4, detail, mode, obj, other,
              verb, params );
            m4 = resolve_property( code, m4, obj, other, mode, detail,
              params, verb );
          }

          dbg( "m4 = " + dump_value(m4) + " s = " + dump_value(sar[i]) );
          mar += ({ m4 ? m4 : sar[i] });
        }

        switch( sizeof(sar) ) { /* surely there's a better way */
          case 1:
            m2 = call_other( m1, m3, mar[0] );
            break;
          case 2:
            m2 = call_other( m1, m3, mar[0], mar[1] );
            break;
          case 3:
            m2 = call_other( m1, m3, mar[0], mar[1], mar[2] );
            break;
          case 4:
            m2 = call_other( m1, m3, mar[0], mar[1], mar[2], mar[3] );
            break;
        }

        dbg( "return value = " + dump_value(m2) );

        m1 = resolve_ref( obj, other, s2, mode, detail, params );
        m1 = resolve_variables( code, m1, detail, mode, obj, other,
          verb, params );
        run_set( code, obj, other, m1, m2, mode, detail, params,
          verb, show_debug );

        continue;
      }
      if ( sscanf( text, "function %s:%s:%s returning %s", m1, m2,
        m3, s2 )==4 ) {
        m1 = "/usr/"+ m1 + "/bilbofuns/"+m2;
        dbg( "m1 = " + dump_value(m1) );

        m2 = call_other( m1, m3 );
        dbg( "return value = " + dump_value(m2) );

        m1 = resolve_ref( obj, other, s2, mode, detail, params );
        m1 = resolve_variables( code, m1, detail, mode, obj, other, 
          verb, params );
        run_set( code, obj, other, m1, m2, mode, detail, params, 
          verb, show_debug );

        continue; 
      }


      if ( sscanf( text, "call %s:%s:%s with %s", s1,s2,s3,s4 )==4 ) {
        params = push_params( params, verb, mode, line, code );
        params = add_params( params, s4, obj, other, code, mode, verb,
          detail );

        if ( m1 = find_bilbo( obj, s3, s2 ) ) {
          line = 0;
          verb = s3;
          mode = s2;
          code = m1;
        } else {
          dbg( "error calling " + s1 + ":" + s2 + ":" + s3 );
          return FALSE;
        }
        continue;
      }

      if ( sscanf( text, "call %s:%s:%s", s1, s2, s3 )==3 ) {
        params = push_params( params, verb, mode, line, code );
        if ( m1 = find_bilbo( obj, s3, s2 ) ) {
          line = 0;
          verb = s3;
          mode = s2;
          code = m1;
        } else {
          dbg( "error calling " + s1 + ":" + s2 + ":" + s3 );
          return FALSE;
        }
        continue;
      }

      if ( text=="return" ) {
	 mapping chunk;
	 mixed parlist;
	 int depth, i;

	 if (!params["stackdepth"]) {
	    /* we were not called a subroutine: exit */

	    if (show_debug) {
	       run_emit(show_debug, code, obj, other,
			"** -- (RETURN)",
			mode, detail, verb, params);
	    }
	    return FALSE;
	 }

	 depth = params["stackdepth"];

	 if (params["stackparlist"] && depth) {
	    parlist = params["stackparlist"+depth];
	    for( i=0; i<sizeof(parlist); i++ )
	       params[parlist[i]] = nil;
	 }

	 chunk = params["CHUNK"+depth];

	 params["CHUNK"+depth] = nil;
	 params["stackdepth"]--;

	 code = chunk["code"];
	 mode = chunk["mode"];
	 line = chunk["line"];
	 verb = chunk["verb"];

	 continue;
      }

/* -----------------
        m4 = resolve_ref( obj, other, s4, mode, detail, params );
        m4 = resolve_variables( code, m4, detail, mode, obj, other, 
          verb, params );
        m4 = resolve_property( code, m4, obj, other, mode, detail, 
          params );
        m4 = resolve_obj( code, m4, obj, other, mode, detail, params );
 
        m1 = find_bilbo( m4, s3, s2 );                
        if ( m1 ) {
          call_out( "parse_bilbo", 0, m1, 10, m4, obj, 
            s3, s2, detail, params );
          return TRUE;
        } else {
        }
        return TRUE;
      }
------------------ */


      if ( show_debug ) {
        run_emit( show_debug, code, obj, other, 
          "** SYNTAX ERROR LINE " + sline + ": " + text, mode, detail, 
          verb, params );
        return FALSE;
      }

    } 

/* ---------
    if ( !curcmd ) {
      dbg( "BILBO lines executed: " + count ); 

      if ( show_debug ) {
        run_emit( show_debug, code, obj, other,
          "** -- (EXIT " + (ret ? "TRUE)" : "FALSE)"),
          mode, detail, verb, params );
      }
      return ret;
    }
--------- */

  }
}


/*
 * Run bilbo script in the given object, if it has one that matches the
 * verb and mode.
 */
static
mixed run_bilbo ( object obj, mixed other, string verb, string mode, string detail, varargs mapping params ) {
  mixed ret, code;
  string s1;

  if ( params==nil )
    params = ([ ]);

  dbg( "run_bilbo obj = " + describe_one(obj) + " other = " + describe_one(other) + "  mode = " + mode );
  dbg( "params = " + dump_value(params) );  

  code = find_bilbo( obj, verb, mode );

  if ( code==nil ) {
    dbg( "Hey, there's no BILBO script here." );
    return TRUE;
  }
 
  dbg( "Okay, mode = " + mode + " code = " + dump_value(code) );
 
  if ( typeof(code)==T_STRING ) {
    sscanf( code, "%s in %s", verb, code );
    dbg( "code? " + dump_value(code) + " mode? " + mode );
    s1 = resolve_ref( obj, other, code, mode, detail, nil );
    dbg( "s1 = " + s1 ); 
    code = resolve_property( obj, s1, obj, other, mode, detail, params, verb );  /* find_object(code); */
    dbg( "code = " + dump_value(code) );
  } 
  ret = parse_bilbo( code, 10, obj, other, verb, mode, detail, params );

  dbg( "returning " + dump_value(ret) );		
  return ret;
}		


/*
 * Run bilbo script in one object with another object as the target
 * and possibly yet another object as the 'other'. Zell 010822
 */
static
mixed run_bilbo_other (object code_object, object target, mixed other, string verb, string mode, string detail, varargs mapping params ) {
  mixed ret, code;
  string s1;

  if ( params==nil )
    params = ([ ]);

  dbg( "run_bilbo target = " + describe_one(target) + " other = " + describe_one(other) + "  mode = " + mode );
  dbg( "params = " + dump_value(params) );  

  code = find_bilbo( code_object, verb, mode );
  if ( code==nil ) {
    dbg( "Hey, there's no BILBO script here." );
    return TRUE;
  }

  dbg( "Okay, mode = " + mode + " code = " + dump_value(code) );
 
  if ( typeof(code)==T_STRING ) {
    sscanf( code, "%s in %s", verb, code );
    dbg( "code? " + dump_value(code) + " mode? " + mode );
    s1 = resolve_ref( target, other, code, mode, detail, nil );
    dbg( "s1 = " + s1 ); 
    code = resolve_property( target, s1, target, other, mode, detail, params, verb );  /* find_object(code); */
    dbg( "code = " + dump_value(code) );
  } 
  ret = parse_bilbo( code, 10, target, other, verb, mode, detail, params );

  SysLog( "returning " + dump_value(ret) );		
  return ret;
}		
