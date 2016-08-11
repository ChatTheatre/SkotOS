/*
**	/lib/string.c
**
**	Useful functions for manipulating strings.
**
**	Copyright Skotos Tech Inc 1999
*/

/* this can be much optimized */

float decayed_value(float old_value, float half_time, float lapse) {
   return old_value * exp(-log(2.0) * (lapse / half_time));
}
