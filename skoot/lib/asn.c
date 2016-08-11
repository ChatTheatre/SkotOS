# include <type.h>

# define INT64_MAX "x7f\xff\xff\xff\xff\xff\xff\xff"

static string int32_to_int64(int i32)
{
   string i64;

   i64 = "\0\0\0\0";
   i64[0] = (i32 & 0xff000000) >> 24;
   i64[1] = (i32 & 0x00ff0000) >> 16;
   i64[2] = (i32 & 0x0000ff00) >>  8;
   i64[3] =  i32 & 0x000000ff;

   return i64;
}

static int int64_to_int32(mixed i64)
{
   int i32;
   int i;

   if (typeof(i64) == T_INT) {
      return i64;
   }
   for (i = 0; i < strlen(i64); i++) {
      i32 = (i32 << 8) | i64[i];
   }
   return i32;
}

static string add_int64_int32(mixed i64, int i32)
{
   if (typeof(i64) == T_INT) {
      i64 = int32_to_int64(i64);
   }
   return asn_add(i64, int32_to_int64(i32), INT64_MAX);
}

static string add_int64_int64(mixed i1, mixed i2)
{
   if (typeof(i1) == T_INT) {
      i1 = int32_to_int64(i1);
   }
   if (typeof(i2) == T_INT) {
      i2 = int32_to_int64(i2);
   }
   return asn_add(i1, i2, INT64_MAX);
}

static int div_int64_int32(mixed i64, int i32)
{
   if (typeof(i64) == T_INT) {
      i64 = int32_to_int64(i64);
   }
   return int64_to_int32(asn_div(i64, int32_to_int64(i32), INT64_MAX));
}
