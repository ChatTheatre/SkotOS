/*
**	/base/sys/materials.c
**
**	Keep track of helpful material properties.
**
**	Copyright Skotos Tech Inc 2000
*/

# include <type.h>

mapping material_to_density;
mapping density_to_material;

void clear_materials();

string query_state_root() { return "Base:Materials"; }

static
void create() {
   clear_materials();
   set_object_name("Base:Materials");
}

void clear_materials() {
   material_to_density = ([ ]);
   density_to_material = ([ ]);
}

void set_density(string material, float density) {
   mixed val;

   val = material_to_density[material];
   if (typeof(val) == T_FLOAT && density_to_material[val] == material) {
      density_to_material[val] = nil;
   }
   material_to_density[material] = density;
   density_to_material[density] = material;	/* may overwrite, shrug */
}

float query_density(string material) {
   return material_to_density[material];
}

void clear_material(string material) {
   mixed val;

   val = material_to_density[material];
   if (typeof(val) == T_FLOAT && density_to_material[val] == material) {
      density_to_material[val] = nil;
   }
   material_to_density[material] = nil;
}

void new_material() {
   set_density("NEW", 0.0);
}

string **find_material_bracket(float density) {
   string *materials, last;
   float *densities;
   int i;

   densities = map_indices(density_to_material);
   materials = map_values(density_to_material);

   while (i < sizeof(densities) && density > densities[i]) {
      i ++;
   }
   return ({ materials[.. i-1], materials[i ..] });
}


string *query_materials() {
   return map_values(density_to_material);
}
