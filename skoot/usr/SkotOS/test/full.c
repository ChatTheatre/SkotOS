/*
 * D=Synchronize an item in a virtual area -- void viral::sync_item($area: <object>, $item: <object>)
 *
 * $item will have these properties in the "sys:viral" namespace:
 *   "xpos"       <float> The X position in the virtual area.
 *   "ypos"       <float> The Y position -'-
 *   "xsize"      <int>   The X size (W->E) in kilometers.
 *   "ysize"      <int>   The Y size (N->S) in kilometers.
 *   "visibility" <float> The visibility of the object.
 *
 * These properties may optionally existed in the "sys:viral:sync" namespace,
 * and will then be properly cleaned out.
 */

/* Property short-cuts and the like. */
constant _xlength = ((int)ceil((float)$item."sys:viral:xsize" / 2.0));
constant _ylength = ((int)ceil((float)$item."sys:viral:ysize" / 2.0));
constant _wside = ($item."sys:viral:xpos" - _xlength);
constant _eside = ($item."sys:viral:xpos" + _xlength);
constant _nside = ($item."sys:viral:ypos" - _ylength);
constant _sside = ($item."sys:viral:ypos" + _ylength);
constant _radius = ($area."sys:viral:condition" * $item."sys:viral:visibility");
constant _wedge = (_wside - _radius);
constant _nedge = (_nside - _radius);
constant _eedge = (_eside + _radius);
constant _sedge = (_sside + _radius);

/* Synchronized versions... */
constant sync_xlength = ((int)ceil((float)$item."sys:viral:sync:xsize" / 2.0));
constant sync_ylength = ((int)ceil((float)$item."sys:viral:sync:ysize" / 2.0));
constant sync_wside = ($item."sys:viral:sync:xpos" - sync_xlength);
constant sync_eside = ($item."sys:viral:sync:xpos" + sync_xlength);
constant sync_nside = ($item."sys:viral:sync:ypos" - sync_ylength);
constant sync_sside = ($item."sys:viral:sync:ypos" + sync_ylength);
constant sync_radius = ($item."sys:viral:sync:condition" * $item."sys:viral:sync:visibility");
constant sync_wedge = (sync_wside - sync_radius);
constant sync_nedge = (sync_nside - sync_radius);
constant sync_eedge = (sync_eside + sync_radius);
constant sync_sedge = (sync_sside + sync_radius);

/* Functionality. */
constant bigmap_set_x = set_by_int($area."sys:viral:xmap", i, map_indices(arr_to_set(Arr(get_by_int($area."sys:viral:xmap", i)) + ({ $item }) - ({ nil }))));
constant bigmap_set_y = set_by_int($area."sys:viral:ymap", i, map_indices(arr_to_set(Arr(get_by_int($area."sys:viral:ymap", i)) + ({ $item }) - ({ nil }))));
constant bigmap_clear_x = set_by_int($area."sys:viral:xmap", i, get_by_int($area."sys:viral:xmap", i) - ({ $item }));
constant bigmap_clear_y = set_by_int($area."sys:viral:ymap", i, get_by_int($area."sys:viral:ymap", i) - ({ $item }));

/* Variables. */
int x, x2, y, y2, i;

::log($log: "Synchronizing " + dump_value($item) + " in " + dump_value($area) + ":");

/* Declarations. */
x  = _wedge > 0 ? _wedge : 0;
x2 = _eedge < $area."sys:viral:xsize" ? _eedge : $area."sys:viral:xsize";
y  = _nedge > 0 ? _nedge : 0;
y2 = _sedge < $area."sys:viral:ysize" ? _sedge : $area."sys:viral:ysize";

::log($log: "Checking previous synchronized state.");

/* Check if a synchronized state exists. */
if (typeof($item."sys:viral:sync:xpos") != T_NIL) {
   /* A sync'd state exists; we need to clean it up. */
   int s_x, s_x2, s_y, s_y2;

   ::log($log: "Item has sync data.");

   s_x  = sync_wedge > 0 ? sync_wedge : 0;
   s_x2 = sync_eedge < $area."sys:viral:xsize" ? sync_eedge : $area."sys:viral:xsize";
   s_y  = sync_nedge > 0 ? sync_nedge : 0;
   s_y2 = sync_sedge < $area."sys:viral:ysize" ? sync_sedge : $area."sys:viral:ysize";

   /*
    * Now we have a rectangular visibility range for the new area,
    * and one for the old (synchronized) area, something like:
    **************************************************************

     sync
0     +------------------+
1     |         new      |
2     |         +------------------+
3     |         |        |         |
4     +---------|--------+         |
5               |                  |
6               +------------------+
               1         2         3
     01234567890123456789012345678901234
     **************************************************************
     * So we need to cut:
     * the synchronized X1 -> new X1 [0->10],
     * the new X2 -> synchronized X2 [30->20 = not required],
     * the synchronized Y1 -> new Y1 [0->2],
     * the new Y2 -> synchronized Y2 [6->4 = not required]
     */

   /* This for loop will never execute if the movement was north/south exclusively, or westward. */
   for (i = s_x; i < x; i++) {
      bigmap_clear_x;
   }
   /* This for loop will never execute if the movement was north/south exclusively, or eastward. */
   for (i = x2; i < s_x2; i++) {
      bigmap_clear_x;
   }
   /* This for loop will never execute if the movement was west/east exclusively, or northward. */
   for (i = s_y; i < y; i++) {
      bigmap_clear_y;
   }
   /* This for loop will never execute if the movement was west/east exclusively, or southward. */
   for (i = y2; i < s_y2; i++) {
      bigmap_clear_y;
   }
   ::log($log: "Synchronized data cropped.");
}

::log($log: "Filling bigmaps.");

/* Fill in the bigmaps. */
for (i = x; i <= x2; i++) {
   bigmap_set_x;
}
for (i = y; i < y2; i++) {
   bigmap_set_y;
}

::log($log: "Finalizing.");

/* Store the synchronized values. */
$item."sys:viral:sync:xsize" = $item."sys:viral:xsize";
$item."sys:viral:sync:ysize" = $item."sys:viral:ysize";
$item."sys:viral:sync:xpos" = $item."sys:viral:xpos";
$item."sys:viral:sync:ypos" = $item."sys:viral:xpos";
$item."sys:viral:sync:ypos" = $item."sys:viral:ypos";
$item."sys:viral:sync:visibility" = $item."syss:viral:visibility";
$item."sys:viral:sync:condition" = $area."sys:viral:condition";

::log($log: "Synchronization completed successfully.");
