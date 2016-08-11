/*
**	/lib/version.c
**
**	Calculate a numerical version identifier for comparisons.
**	1.1.96 -> 11096
**
**	Copyright Skotos Tech Inc 1999
*/


# include <status.h>

int dgd_version() {
   string v;
   int a, b, c;

   v = status()[ST_VERSION];
   if (v[.. 3] == "DGD ") {
      v = v[4 ..];
   }
   if (sscanf(v, "%d.%d.%d", a, b, c)) {
      return a * 10000 + b * 1000 + c;
   }
}

