inherit "/usr/SkotOS/lib/merrynode";
# include "/usr/SkotOS/include/merrynode.h"

mixed merry(string mode, string signal, string label) {
    switch(label) { case "virgin": {
/*

  D=Emulate a dice-roll.

  Required arguments:
    $roll (array):   The dice-map

  Semi-required arguments:
    $actor (object): The acting subject.

  Note -    read about dice-maps,
    http://troll.skotos.net/twiki/bin/view/Mortalis/DiceSystem

  Note #2 - the +dice tool can be used to generate dice-maps.

*/

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
    /* DEBUG:
    if( err = catch( vals[i] = (float) $roll[i][0] + ( (float) $roll[i][1] * ( Flt( (GetVar( $roll[i][2] ) ? GetVar( $roll[i][2] ) : Get( $actor, $roll[i][2] ))) - (float) $roll[i][3] ))))
    {
      error( "[Dice error in "+dump_value($actor)+" for #"+Str(i)+" -> {" + dump_value($roll[i]) + "} -> " + err + "]" );
    }
    */

/* $dice_callback = Str( Int( vals[1] )) + "D" + Str( Int( vals[2] )) + "+" + Str( Int( vals[0] )); */

/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */
/* EmitTo( $actor, Str( vals[0] ) +"+"+ Str( vals[1] ) +"+random( Int( 1.0 + (" + Str( vals[1] ) + " * " + Str(vals[2]-1.0) + " )))" ); */

roll = (float) random( Int( 1.0 + (fabs(vals[1]) * (vals[2]-1.0) )));
if (vals[1] < 0.0)
  roll = -roll;

return Int( ((float) (int) vals[0]) + vals[1] + roll);}
    }
}
