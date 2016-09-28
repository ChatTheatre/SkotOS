/*
**	This is pretty much a use-once program to go through and
**	replace array NRefs with LWO NRefs inside generic data.
**
**	It is based on /lib/array's copy() function.
*/

# include <nref.h>
# include <type.h>
# include <XML.h>

mixed womble(mixed val, varargs mapping seen) {
   mixed *arr, *ix;
   int i;

   if (!seen) {
      seen = ([ ]);
   }
   switch (typeof(val)) {
   case T_ARRAY:
      switch (::query_colour(val)) {
      case COL_NREF:
	 return CopyNRef(val);
      case COL_ELEMENT:
	 return NewXMLElement(womble(val[..]));
      case COL_PCDATA:
	 return NewXMLPCData(womble(val[..]));
      case COL_SAMREF:
	 return NewXMLSAMRef(womble(val[..]));
      default:
	 break;
      }
      arr = val;
      for (i = 0; i < sizeof(arr); i ++) {
	 arr[i] = womble(arr[i], seen);
      }
      break;
   case T_MAPPING:
      ix = map_indices(val);
      arr = map_values(val);
      val = ([ ]);
      for (i = 0; i < sizeof(arr); i ++) {
	 if (typeof(ix[i]) == T_STRING) {
	    ix[i] = ix[i][..];
	 }
	 val[ix[i]] = womble(arr[i], seen);
      }
      break;
   case T_OBJECT:
      switch (ur_name(val)) {
      case "/usr/XML/data/element#-1":
      case "/usr/XML/data/pcdata#-1":
      case "/usr/XML/data/samref#-1":
	 DEBUG("[womble] recursing into " + ur_name(val) + " ...");
	 val->set_data(womble(val->query_data()));
	 break;
      case "/usr/SkotOS/data/merry#-1":
	 val->womble_merry();
	 break;
      default:
	 break;
      }
      break;
   default:
      /* Get rid of the colour value, so it won't need to be checked again. */
      switch (::query_colour(val)) {
      case COL_ELEMENT:
	 val = NewXMLElement(val);
	 break;
      case COL_PCDATA:
	 val = NewXMLPCData(val);
	 break;
      case COL_SAMREF:
	 val = NewXMLSAMRef(val);
	 break;
      default:
	 break;
      }
      break;
   }
   return val;
}
