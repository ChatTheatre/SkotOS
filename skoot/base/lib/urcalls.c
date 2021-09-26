/*
**	/base/lib/urcalls.c
**
**	Define the ur-calls for /base.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <urcalls.h>

# include <nref.h>

/* properties */

R1(mixed, ur_property, query_property, string)
O0(string *, ur_property_indices, query_property_indices)
A0(mapping, ur_properties, query_properties)

/* thing */
R0(int, ur_weapon, query_weapon)
R0(mixed, ur_die_message_first, query_die_message_first)
R0(mixed, ur_die_message_third, query_die_message_third)

R0(string, ur_integration_property, query_integration_property)

R0(int, ur_combinable, query_combinable)
R0(int, ur_discrete, query_discrete)
R0(int, ur_by_weight, query_by_weight)

/* details */

O0(string *, ur_details, query_details)

R1(string, ur_prime_id, get_prime_id, string)

R1(int, ur_hush, query_hush, string)
R1(int, ur_prime, query_prime, string)
R1(int, ur_def, query_def, string)
R1(int, ur_abstract, query_abstract, string)
R1(int, ur_plural, query_plural, string)
R1(int, ur_is_door, query_is_door, string)
R1(int, ur_hidden, query_hidden, string)
R1(int, ur_never_obvious, query_never_obvious, string)
R1(int, ur_obvious_when_open, query_obvious_when_open, string)
R1(int, ur_hidden_when_closed, query_hidden_when_closed, string)
R1(int, ur_separate, query_separate, string)
R1(int, ur_self_locking, query_self_locking, string)
R1(int, ur_deadbolt, query_deadbolt, string)
R1(int, ur_targetable_in_dark, query_targetable_in_dark, string)

R2(int, ur_id_prep, test_id_prep, string, int)
O1(int, ur_id_preps, query_id_preps, string)

R2(int, ur_sname, test_sname, string, string)
R2(int, ur_pname, test_pname, string, string)

O1(string *, ur_sname_ids, query_sname_ids, string)
O1(string *, ur_pname_ids, query_pname_ids, string)

R2(mixed, ur_description, query_description, string, string)
A1(mapping, ur_description_mapping, query_description_mapping, string)

R1(mixed, ur_exit_dest, query_exit_dest, string)
O1(string *, ur_details_by_dir, query_details_by_dir, string)
R2(int, ur_exit_dir, test_exit_dir, string, string)
O1(string *, ur_exit_dirs, query_exit_dirs, string)
R1(string, ur_exit_keycode, query_exit_keycode, string)
R1(NRef, ur_climbable_dest, query_climbable_dest, string)
O0(string *, ur_dirs, query_dirs)

O1(string *, ur_description_types, query_description_types, string)
O1(string *, ur_snames, query_snames, string)
O1(string *, ur_pnames, query_pnames, string)
R2(int, ur_test_adjective, test_adjective, string, string)
O1(string *, ur_adjectives, query_adjectives, string)
R1(string, ur_face, query_face, string)
R1(string, ur_pname_of_sname, query_pname_of_sname, string)

R2(string *, ur_match_exit_details, match_exit_details, string, int)

/* clothing */

O0(string *, ur_parts_covered, query_parts_covered)
R0(int, ur_transparency, query_transparency)
R0(int, ur_single_wear, query_single_wear)

/* environment but not really */

R0(int, ur_unsafe, query_unsafe)
R0(int, ur_safe, query_safe)
R0(int, ur_tight, query_tight)
R0(int, ur_public_container, query_public_container)
R0(int, ur_transparent_container, query_transparent_container)
R0(string, ur_container_verb_second, query_container_verb_second)
R0(string, ur_container_verb_third, query_container_verb_third)
R0(int, ur_clothes_expected, query_clothes_expected)

M0(float, ur_max_weight, query_max_weight)

/* events */
A0(string *, ur_coded_events, query_coded_events)
R1(mixed, ur_event_code, query_event_code, string)

/* living */
R0(int, ur_gender, query_gender)

R0(int, ur_volition, query_volition)

M0(float, ur_strength, query_strength)

/* misc */

R0(string, ur_gait, query_gait)
R0(int, ur_default_stance, query_default_stance)
R0(int, ur_edible, query_edible)
R0(int, ur_potable, query_potable)

R0(int, ur_cached_motion, query_cached_motion)

/* bulk */

R0(int, ur_immobile, query_immobile)
M0(float, ur_intrinsic_mass, query_intrinsic_mass)
M0(float, ur_original_mass, query_original_mass)
M0(float, ur_intrinsic_density, query_intrinsic_density)
M0(float, ur_intrinsic_capacity, query_intrinsic_capacity)
R0(int, ur_flexible, query_flexible)

/* content groups */

O0(string *, ur_content_groups, query_content_groups)
A1(mapping, ur_content_group, query_content_group, string)

/* light */

A1(float, ur_luminosity, query_luminosity, string)

/* crafting */

O0(string *, ur_ingredients, query_ingredients)
R1(float, ur_ingredient_amount, query_ingredient_amount, string)
R1(string, ur_ingredient_description, query_ingredient_description, string)

O0(string *, ur_tools, query_tools)
R1(string, ur_tool_description, query_tool_description, string)

O0(string *, ur_crafting_verbs, query_crafting_verbs)

R0(string, ur_see_skill, query_see_skill)
R0(int, ur_see_level, query_see_level)

R0(string, ur_do_skill, query_do_skill)
R0(int, ur_do_level, query_do_level)

R0(int, ur_crafting_time, query_crafting_time)
R0(object, ur_crafting_intermediate, query_crafting_intermediate)
R0(object, ur_crafting_failure, query_crafting_failure)

R0(int, ur_crafting_attention, query_crafting_attention)
R0(int, ur_crafting_held, query_crafting_held)
