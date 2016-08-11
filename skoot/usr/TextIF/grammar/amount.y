# ---- begin ~TextIF/grammar/amount.y ----

Amount: 'all' {{ return ({ nil }) }}
Amount: VolumeAmount {{ return ({ ({ "V", tree[0] }) }) }}
Amount: WeightAmount {{ return ({ ({ "W", tree[0] }) }) }}

VolumeAmount: _VAmount {{ return ({ tree[0] * 1E-6 }) }}
VolumeAmount: _VAmount 'and' VolumeAmount {{
   return ({ tree[0] * 1E-6 + tree[2] });
}}

_VAmount: __VAmount
_VAmount: __ACardinal __VAmount {{
   return ({ (float) tree[0] * tree[1] });
}}
   
__VAmount:    'l' {{
   return ({ 1000.0 });
}}
   
__VAmount:    'liter' {{
   return ({ 1000.0 });
}}
   
__VAmount:    'liters' {{
   return ({ 1000.0 });
}}
   
__VAmount:    'dl' {{
   return ({ 100.0 });
}}
   
__VAmount:    'deciliter' {{
   return ({ 100.0 });
}}
   
__VAmount:    'deciliters' {{
   return ({ 100.0 });
}}

__VAmount:    'cl' {{
   return ({ 10.0 });
}}

__VAmount:    'centiliter' {{
   return ({ 10.0 });
}}

__VAmount:    'centiliters' {{
   return ({ 10.0 });
}}

__VAmount:    'ml' {{
   return ({ 1.0 });
}}


__VAmount:    'milliliter' {{
   return ({ 1.0 });
}}

__VAmount:    'milliliters' {{
   return ({ 1.0 });
}}


__VAmount:    'barrel' {{
   return ({ 119240.0 });
}}

__VAmount:    'barrels' {{
   return ({ 119240.0 });
}}

__VAmount:    'gallon' {{
   return ({ 3785.0 });
}}

__VAmount:    'gallons' {{
   return ({ 3785.0 });
}}

__VAmount:    'quart' {{
   return ({ 950.0 });
}}

__VAmount:    'quarts' {{
   return ({ 950.0 });
}}

__VAmount:    'pint' {{
   return ({ 473.0 });
}}

__VAmount:    'pints' {{
   return ({ 473.0 });
}}

__VAmount:    'cup' {{
   return ({ 225.0 });
}}

__VAmount:    'cups' {{
   return ({ 225.0 });
}}


WeightAmount: _WAmount {{ return ({ 1E-3 * tree[0] }) }}
WeightAmount: _WAmount 'and' WeightAmount {{
   return ({ 1E-3 * tree[0] + tree[2] });
}}

_WAmount: __WAmount
_WAmount: __ACardinal __WAmount {{
   return ({ (float) tree[0] * tree[1] });
}}

__WAmount:    'kg' {{
   return ({ 1000.0 });
}}

__WAmount:    'kilogram' {{
   return ({ 1000.0 });
}}

__WAmount:    'kilograms' {{
   return ({ 1000.0 });
}}

__WAmount:    'hg' {{
   return ({ 100.0 });
}}

__WAmount:    'hektogram' {{
   return ({ 100.0 });
}}

__WAmount:    'hektograms' {{
   return ({ 100.0 });
}}

__WAmount:    'hectogram' {{
   return ({ 100.0 });
}}

__WAmount:    'hectograms' {{
   return ({ 100.0 });
}}

__WAmount:    'g' {{
   return ({ 1.0 });
}}

__WAmount:    'gram' {{
   return ({ 1.0 });
}}

__WAmount:    'grams' {{
   return ({ 1.0 });
}}


__WAmount:    'ton' {{
   return ({ 907000.0 });
}}

__WAmount:    'tons' {{
   return ({ 907000.0 });
}}

__WAmount:    'pound' {{
   return ({ 454.0 });
}}

__WAmount:    'pounds' {{
   return ({ 454.0 });
}}

__WAmount:    'ounce' {{
   return ({ 28.0 });
}}

__WAmount:    'ounces' {{
   return ({ 28.0 });
}}

# Keywords used for amounts:

_Amount: 'l'
_Amount: 'liter'
_Amount: 'liters'
_Amount: 'dl'
_Amount: 'deciliter'
_Amount: 'deciliters'
_Amount: 'cl'
_Amount: 'centiliter'  
_Amount: 'centiliters'
_Amount: 'ml'
_Amount: 'milliliter'
_Amount: 'milliliters'  
_Amount: 'barrel'
_Amount: 'barrels'
_Amount: 'gallon'
_Amount: 'gallons'
_Amount: 'quart'
_Amount: 'quarts'
_Amount: 'pint'
_Amount: 'pints'  
_Amount: 'cup' 
_Amount: 'cups'

_Amount: 'kg'
_Amount: 'kilogram'
_Amount: 'kilograms'
_Amount: 'hg'
_Amount: 'hektogram'
_Amount: 'hektograms'
_Amount: 'hectogram'
_Amount: 'hectograms'
_Amount: 'g'
_Amount: 'gram'
_Amount: 'grams'
_Amount: 'ton'
_Amount: 'tons'
_Amount: 'pound'
_Amount: 'pounds'
_Amount: 'ounce'
_Amount: 'ounces'

__ACardinal: 'a'  {{ return ({ 1 }); }}
__ACardinal: 'an' {{ return ({ 1 }); }}
__ACardinal: Cardinal

# ---- end ~TextIF/grammar/amount.y ----
