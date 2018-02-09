#pragma once

#include "nic/include/base.h"
#include "nic/fte/acl/ref.hpp"

namespace acl {

/**
 * Type of ACL key field
 *
 * Prefix - IP prefix (value/prefixlen)
 * Range - IP or port range (val1-val2)
 * Exact - exact match (value and bitmask)
 */
typedef enum {
    ACL_FIELD_TYPE_PREFIX = 0,
    ACL_FIELD_TYPE_RANGE,
    ACL_FIELD_TYPE_EXACT,
} acl_field_type_t;

union acl_field_val_t {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
};

/**
 * ACL Field definition.
 * Each field in the ACL rule defines the type of field, its size,
 * its offset in the input buffer.
 */
struct acl_field_def_t {
	acl_field_type_t  type;        /**< type - RTE_ACL_FIELD_TYPE_*. */
	uint8_t	 size;                 /**< size of field 1,2,4, or 8. */
	uint32_t offset;               /**< offset to start of field. */
};

/**
 * Defines the value of a field for a rule.
 */
struct acl_field_t {
	union acl_field_val_t value;
	/**< a 1,2,4, or 8 byte value of the field. */
	union acl_field_val_t mask_range;
	/**<
	 * depending on field type:
	 * prefix -> 1.2.3.4/32 value=0x1020304, mask_range=32,
	 * range -> 0 : 65535 value=0, mask_range=65535,
	 * exact -> 0x06/0xff value=6, mask_range=0xff.
	 */
};

/**
 * Miscellaneous data for ACL rule.
 */
struct acl_rule_data_t {
	uint32_t  category_mask; /**< Mask of categories for that rule. */
	uint32_t  priority;      /**< Priority for that rule. */
	const void  *userdata;     /**< Associated with the rule user data. */
};

/**
 * Defines single ACL rule.
 * data - miscellaneous data for the rule.
 * field[] - value and mask or range for each field.
 */
#define	ACL_RULE_DEF(name, fld_num)	struct name {               \
        ref_t                  ref_count;     /** ref count */  \
        struct acl_rule_data_t data;                            \
        struct acl_field_t     field[fld_num];                  \
    };

ACL_RULE_DEF(acl_rule_t,);

static inline const ref_t *acl_rule_to_ref(const acl_rule_t *rule) {
    return &rule->ref_count;
}

static inline const acl_rule_t *acl_rule_from_ref(const ref_t *ref_count) {
    return container_of(ref_count, acl_rule_t, ref_count);
}

static inline const acl_rule_t *acl_rule_clone(const acl_rule_t *rule) {
    ref_inc(&rule->ref_count);
    return rule;
}

static inline void acl_rule_deref(const acl_rule_t *rule) {
    ref_dec(&rule->ref_count);
}

/** Max number of characters in name.*/
#define	ACL_NAMESIZE		64
#define ACL_MAX_FIELDS      32
#define ACL_MAX_CATAGORIES  32

/**
 * Parameters used when creating the ACL context.
 */
struct acl_config_t {
	const char *name;       /**< Name of the ACL context. */
    uint8_t num_categories; /**< Number of categories to build with. */
	uint8_t num_fields;     /**< Number of field definitions. */
	struct acl_field_def_t defs[ACL_MAX_FIELDS];
};

class acl_ctx_t;

/**
 * Create a new ACL context.
 *
 *  This also inserts the ctx into global list, which can be looked up
 *  by other thread using name.
 *
 * @param param
 *   Parameters used to create and initialise the ACL context.
 * @return
 *   Pointer to ACL context structure that is used in future ACL
 *   operations. This should be released by user using
 *   acl_deref()/acl_delete()
 */
const acl_ctx_t *
acl_create(const acl_config_t *cfg);

/**
 * Remove the ctx from the global list and release the reference.
 *
 * @param ctx
 *   ACL context to free
 */
void
acl_delete(const acl_ctx_t *ctx);

/**
 * Lookup an existing ACL context by name
 *
 * Should be released by user using acl_deref()
 **/
const acl_ctx_t *
acl_get(const char *name);

/**
 * Deref the ctx pointer
 */
void
acl_deref(const acl_ctx_t *ctx);

/**
 * Callback invoked for all effected rules during an update
 */
typedef void (*acl_update_cb_t)(const acl_rule_t *rule);

/**
 * Commit the updated ctx
 *
 * acl_update_cb_t is invoked for all the effected rules.
 */
hal_ret_t
acl_commit(const acl_ctx_t *ctx, acl_update_cb_t cb = NULL);

/**
 * Add a rule
 */
hal_ret_t
acl_add_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule);

/**
 * Delete an exsiting rule
 */
hal_ret_t
acl_del_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule);

/**
 * Perform search for a matching ACL rule for input data buffer.
 * Input data buffer can have up to *categories* matches.
 * That implies that results array should be big enough to hold
 * (categories) elements.
 * Also categories parameter should be either one or multiple of
 * ACL_RESULTS_MULTIPLIER and can't be bigger than ACL_MAX_CATEGORIES.
 * If more than one rule is applicable for given input buffer and
 * given category, then rule with highest priority will be returned as a match.
 * Note, that it is a caller's responsibility to ensure that input parameters
 * are valid and point to correct memory locations.
 *
 * @param ctx
 *   ACL context to search with.
 * @param data
 *   pointer to input data buffers to perform search.
 * @param results
 *   Array of search results
 * @param categories
 *   Number of maximum possible matches for input buffer, one possible
 *   match per category.
 * @return
 *   zero on successful completion.
 *   -EINVAL for incorrect arguments.
 */
hal_ret_t
acl_classify(const acl_ctx_t *ctx, const uint8_t *key,
             const acl_rule_t *results[],  uint32_t categories);

} // namespace acl
