/*
**	/lib/random.c
**
**	Copyright Skotos Tech Inc 2000
*/

#define RNG_M 2147483647
#define RNG_A 16807
#define RNG_Q 127773
#define RNG_R 2836

int rnd_seed;

int eric_random( int max ) {
   int low, high, test, final;

   if (!max) return 0;

   high = rnd_seed / RNG_Q;
   low = rnd_seed % RNG_Q;
   test = RNG_A * low - RNG_R * high;
   if ( test > 0 )
      rnd_seed = test;
   else
      rnd_seed = test + RNG_M;
   final = rnd_seed / (RNG_M/max); 
   if (final >= max) final = max-1;
   return final;
}

void eric_randomize ( int seedval )
{
   rnd_seed = ( seedval % (RNG_M-1) )+1;
   eric_random( RNG_A );
}

static
int roll(int dice, int sides) {
   int i, total;

   for (i = 0; i < dice; i++)
      total += random(sides);

   return total;
}

/* put some fuzz on an integer */

static
int fuzzy(int val, int add) {
   int type;
   float tmp, max_mod, mod;

   tmp = (float)val;
   max_mod = tmp * 0.1;
   mod = (float)random((int)max_mod);

   if (add) {
      tmp += mod;
   } else {
      tmp -= mod;
   }

   return (int)tmp;
}


