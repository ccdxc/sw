/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vcn.cc
 *
 * @brief   This file deals with OCI VCN API handling
 */

namespace api {

using sdk::lib::slab;
using sdk::lib::ht;
using sdk::lib::indexer;

/**
 * @brief Global internal VCN state
 */
typedef struct oci_int_vcn_gstate_s
{
    slab *int_vcn_slab;    /**< Memory slab */
    ht *ht;                /**< Hash table root */

} oci_int_vcn_gstate_t;

/**
 * @brief Internal VCN structure
 */
typedef struct oci_int_vcn_s
{
    oci_vcn_key_t key;    /**< VCN Key */
    uint16_t id;          /**< Internal ID */

} PACKED oci_vcn_t;

oci_int_vcn_gstate_t g_int_vcn_state;

/**
 * @brief Initialize global internal VCN state
 */
void
oci_g_int_vcn_state_init (void)
{
}


/**
 * @brief Add internal VCN to database
 *
 * @param[in] vcn Internal VCN  
 */ 
static inline sdk_ret_t
oci_int_vcn_db_add (oci_int_vcn_t *vcn)
{
    return (g_int_vcn_state->ht()->insert_with_key(&vcn->key, vcn,
                                                   vcn->ht_ctxt);
}

/**
 * @brief Delete internal VCN from database
 *
 * @param[in] vcn_key Internal VCN key
 */
static inline oci_int_vcn_t *
oci_int_vcn_db_del (oci_int_vnc_key_t *vcn_key)
{
    return (oci_int_vcn_t *)(g_int_vcn_state->ht()->remove(vcn_key));
}

/**
 * @brief Lookup internal VCN in database
 *
 * @param[in] vcn_key Internal VCN key
 */
static inline oci_int_vnc_t *
oci_int_vcn_db_lookup (oci_int_vcn_key_t *vcn_key)
{
    return (oci_int_vcn_t *)(g_int_vcn_state->ht()->lookup(vcn_key));
}

/**
 * @brief Allocate internal VCN structure
 *
 * @return Pointer to the allocated internal VCN, NULL if no memory
 */
static inline oci_int_vcn_t *
oci_int_vcn_alloc (void)
{
    return ((oci_int_vcn_t *)g_int_vcn_state->int_vcn_slab()->alloc());
}

/**
 * @brief Free internal VCN structure
 *
 * @param[in] vcn Internal VCN  
 */
static inline void
oci_int_vcn_free (oci_int_vcn_t *vcn)
{
    sdk::delay_delete_to_slab(OCI_SLAB_INT_VCN, int_vcn);
}

/**
 * @brief Initialize internal VCN structure
 *
 * @param[in] vcn Internal VCN
 */
static inline void
oci_int_vcn_init (oci_int_vnc_t *vcn)
{
    SDK_SPINLOCK_INIT(&vcn->slock, PTHREAD_PROCESS_SHARED);
}

/**
 * @brief Uninitialize internal VCN structure
 *
 * @param[in] vcn Internal VCN
 */
static inline void
oci_int_vcn_uninit (oci_int_vcn_t *vcn)
{
    SDK_SPINLOCK_DESTROY(&vcn->slock);
}

/**
 * @brief Allocate and initialize internal VCN structure
 *
 * @return Pointer to the allocated and initialized internal VCN,
 *         NULL if no memory
 */
static inline oci_int_vcn_t *
oci_int_vcn_alloc_init (void)
{
    oci_int_vcn_t *vcn;

    if ((vcn = oci_int_vcn_alloc()) == NULL)
        return NULL;

    oci_int_vcn_init(vcn);
    return vcn;
}

/**
 * @brief Uninitialize and free internal VCN structure
 *
 * @param[in] vcn Internal VCN
 */
static inline void
oci_int_vcn_uninit_free (_In_ oci_int_vcn_t *vcn)
{
    oci_int_vcn_uninit(vcn);
    oci_int_vcn_free(vcn);
}

/**
 * @brief Allocate and initialize internal VCN structure
 *
 * @param[in] vcn VCN information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_vcn_create_handle (_In_ oci_vcn_t *vcn)
{
    oci_int_vcn_t *int_vcn;

    return SDK_STATUS_SUCCESS;
}

/**
 * @brief Validate VNC API data
 *
 * @param[in] vcn VCN information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_vcn_create_validate (_In_ oci_vcn_t *vcn)
{
    return SDK_RET_OK;
}

/**
 * @brief Create VCN
 *
 * @param[in] vcn VCN information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vcn_create (_In_ oci_vcn_t *vcn)
{
    sdk_ret_t st;

    if ((rv = oci_vcn_create_validate(vcn) != SDK_RET_OK))
        return st;

    if ((rv = oci_vcn_create_handle(vcn)) != SDK_RET_OK)
        return st;

    return SDK_RET_OK;
}

/**
 * @brief Handle VCN delete API
 *
 * @param[in] vcn VCN information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_vcn_delete_handle (_In_ oci_vcn_t *vcn)
{
    oci_int_vcn_t *int_vcn;

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC delete API
 *
 * @param[in] vcn_key VCN key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_vcn_delete_validate (_In_ oci_vcn_key_t *vcn_key)
{
    return SDK_RET_OK;
}

/**
 * @brief Delete VCN
 *
 * @param[in] vcn_key VCN key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vcn_delete (_In_ oci_vcn_key_t *vcn_key)
{
    sdk_ret_t rv;
    oci_vcn_t *vcn;

    if ((rv = oci_vcn_delete_validate(vcn_key) != SDK_RET_OK)
        return rv;
    
    if ((int_vcn = oci_int_vcn_lookup(vcn_key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    if ((rv = oci_vcn_delete_handle(vcn_key) != OCIS_TAoci_vcn
    return SDK_RET_OK;
}

}  // namespace api 
