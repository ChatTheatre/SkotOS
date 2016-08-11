/* bulk system error types */

# define GENERIC_SUCCESS	0	/* plain old success */
# define GENERIC_FAILURE	-1	/* plain old failure */
# define FAIL_MASS_CHECK	-2	/* mass check failed */
# define FAIL_FA_CHECK		-3	/* frontal area check failed */
# define FAIL_DEPTH_CHECK	-4	/* depth check failed */
# define FAIL_CAP_CHECK		-5	/* capacity check failed */
# define FAIL_VOL_CHECK         -6      /* volume check failed */
# define FAIL_WORN_CHECK	-7	/* worn_by check failed */
# define FAIL_IMMOBILE		-8	/* cannot be moved */

/* other move return values (rename this file) */

# define RES_CYCLIC_CONT	-20	/* would create cyclic containment */
# define RES_CYCLIC_PROX	-21	/* would create cyclic prox chain */

# define FAIL_WOULD_SPILL	-30
# define FAIL_WOULD_MIX		-31
