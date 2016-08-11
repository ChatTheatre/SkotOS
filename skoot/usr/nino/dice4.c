inherit "/usr/SkotOS/lib/merrynode";
# include "/usr/SkotOS/include/merrynode.h"

mixed merry(string mode, string signal, string label) {
    switch(label) { case "virgin": {
float *vals, roll;
int i, amount;
string err;

if( !(args["actor"]) )
  error( "mortalis::dice(): $actor argument missing" );

vals = allocate_float( 3 );

for( i = 0; i < 3; i++ )
  if( typeof( (args["roll"])[i] ) != T_ARRAY )
    vals[i] = (float) (args["roll"])[i];
  else
    vals[i] = (float) (args["roll"])[i][0] + ( (float) (args["roll"])[i][1] * ( Flt( (GetVar( (args["roll"])[i][2] ) ? GetVar( (args["roll"])[i][2] ) : Get( (args["actor"]), (args["roll"])[i][2] ))) - (float) (args["roll"])[i][3] ));
roll = (float) random( Int( 1.0 + (fabs(vals[1]) * (vals[2]-1.0) )));
if (vals[1] < 0.0)
  roll = -roll;

return Int( ((float) (int) vals[0]) + vals[1] + roll);}
    }
}
