/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.cc
 *
 * @brief   This file deals with OCI VCN API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/api/vcn.hpp"

using namespace sdk;

namespace api {

static inline void oci_int_vcn_db_init (void);

/**
 * @defgroup OCI_INT_VCN - Internal VCN
 * @{
 */

/**
 * @defgroup OCI_INT_VCN_GSTATE - Internal VCN global state functionality
 * @ingroup OCI_INT_VCN
 * @{
 */

#define  OCI_INT_VCN_ID_MAX  1024

/**
 * @brief oci_int_gstate
 */
typedef struct oci_int_vcn_gstate_s
{
    //slab *slab[MAX];   /**< Memory slab */
    sdk::lib::ht *ht;              /**< Hash table root */
    sdk::lib::indexer *id_idxr;    /**< Indexer to allocate internal id */

} oci_int_vcn_gstate_t;

oci_int_vcn_gstate_t g_int_vcn_state;

/**
 * @brief Initialize global internal VCN state
 */
void
oci_g_int_vcn_state_init (void)
{
    //TODO: slab init

    oci_int_vcn_db_init();

    g_int_vcn_state.id_idxr = sdk::lib::indexer::factory(OCI_INT_VCN_ID_MAX);
    SDK_ASSERT_RETURN_VOID(g_int_vcn_state.id_idxr != NULL);
}

/** @} */ // end of OCI_INT_VCN_GSTATE

/**
 * @defgroup OCI_INT_VCN_DB - Internal VCN database functionality
 * @ingroup OCI_INT_VCN
 * @{
 */


static void *
oci_int_vcn_key_func_get (void *entry)
{
    oci_int_vcn_t *int_vcn = (oci_int_vcn_t *)entry;
    return (void *)&(int_vcn->key);
}

static uint32_t
oci_int_vcn_hash_func_compute (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(oci_vcn_key_t)) % ht_size;
}

static bool
oci_int_vcn_key_func_compare (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(oci_vcn_key_t)))
        return true;

    return false;
}

/**
 * @brief Initialize internal VCN database
 */
static inline void
oci_int_vcn_db_init (void)
{
    g_int_vcn_state.ht = sdk::lib::ht::factory(
        OCI_INT_VCN_ID_MAX >> 1, oci_int_vcn_key_func_get,
        oci_int_vcn_hash_func_compute, oci_int_vcn_key_func_compare);
    SDK_ASSERT_RETURN_VOID(g_int_vcn_state.ht != NULL);
}

/**
 * @brief Add internal VCN to database
 *
 * @param[in] vcn Internal VCN  
 */ 
static inline sdk_ret_t
oci_int_vcn_db_add (oci_int_vcn_t *int_vcn)
{
    return (g_int_vcn_state.ht->insert_with_key(&int_vcn->key, int_vcn,
                                                &int_vcn->ht_ctxt));
}

/**
 * @brief Delete internal VCN from database
 *
 * @param[in] vcn_key VCN key
 */
static inline oci_int_vcn_t *
oci_int_vcn_db_del (oci_vcn_key_t *vcn_key)
{
    return (oci_int_vcn_t *)(g_int_vcn_state.ht->remove(vcn_key));
}

/**
 * @brief Lookup internal VCN in database
 *
 * @param[in] vcn_key Internal VCN key
 */
static inline oci_int_vcn_t *
oci_int_vcn_db_lookup (oci_vcn_key_t *vcn_key)
{
    return (oci_int_vcn_t *)(g_int_vcn_state.ht->lookup(vcn_key));
}

/** @} */ // end of OCI_INT_VCN_DB

/**
 * @defgroup OCI_INT_VCN_INIT - Internal VCN setup/teardown functionality
 * @ingroup OCI_INT_VCN
 * @{
 */

/**
 * @brief Allocate internal VCN structure
 *
 * @return Pointer to the allocated internal VCN, NULL if no memory
 */
static inline oci_int_vcn_t *
oci_int_vcn_alloc (void)
{
//    return ((oci_int_vcn_t *)g_int_vcn_state->int_vcn_slab()->alloc());
    return NULL;
}

/**
 * @brief Free internal VCN structure
 *
 * @param[in] int_vcn Internal VCN
 */
static inline void
oci_int_vcn_free (oci_int_vcn_t *int_vcn)
{
//    sdk::delay_delete_to_slab(OCI_SLAB_INT_VCN, int_vcn);
}

/**
 * @brief Initialize internal VCN structure
 *
 * @param[in] int_vcn Internal VCN
 * @param[in] vcn VCN
 */
static inline sdk_ret_t
oci_int_vcn_init (oci_int_vcn_t *int_vcn, oci_vcn_t *vcn)
{
    //SDK_SPINLOCK_INIT(&int_vcn->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&int_vcn->key, &vcn->key, sizeof(oci_vcn_key_t));

    int_vcn->ht_ctxt.reset();

    if (g_int_vcn_state.id_idxr->alloc(&int_vcn->id) !=
        sdk::lib::indexer::SUCCESS)
        return SDK_RET_NO_RESOURCE;

    return SDK_RET_OK;
}

/**
 * @brief Uninitialize internal VCN structure
 *
 * @param[in] vcn Internal VCN
 */
static inline void
oci_int_vcn_uninit (oci_int_vcn_t *int_vcn)
{
    //SDK_SPINLOCK_DESTROY(&int_vcn->slock);
    g_int_vcn_state.id_idxr->free(int_vcn->id);
}

/**
 * @brief Allocate and initialize internal VCN structure
 *
 * @return Pointer to the allocated and initialized internal VCN,
 *         NULL if no memory
 */
static inline oci_int_vcn_t *
oci_int_vcn_alloc_init (oci_vcn_t *vcn)
{
    oci_int_vcn_t *int_vcn;

    if ((int_vcn = oci_int_vcn_alloc()) == NULL)
        return NULL;

    oci_int_vcn_init(int_vcn, vcn);
    return int_vcn;
}

/**
 * @brief Uninitialize and free internal VCN structure
 *
 * @param[in] vcn Internal VCN
 */
static inline void
oci_int_vcn_uninit_free (_In_ oci_int_vcn_t *vcn)
{
    if (vcn) {
        oci_int_vcn_uninit(vcn);
        oci_int_vcn_free(vcn);
    }
}

/** @} */ // end of OCI_INT_VCN_DB

/**
 * @defgroup OCI_INT_VCN_API - First level of VCN API handling
 * @ingroup OCI_INT_VCN
 * @{
 */

/**
 * @brief Handle VCN create message
 *
 * @param[in] vcn VCN information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_vcn_create_handle (_In_ oci_vcn_t *vcn)
{
    oci_int_vcn_t *int_vcn;

    if ((int_vcn = oci_int_vcn_alloc_init(vcn)) == NULL)
        return SDK_RET_OOM;

    return SDK_RET_OK;
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
    //TODO: validate the data ranges of vcn fields
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
    sdk_ret_t rv;

    if ((rv = oci_vcn_create_validate(vcn)) != SDK_RET_OK)
        return rv;

    if ((rv = oci_vcn_create_handle(vcn)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/**
 * @brief Handle VCN delete API
 *
 * @param[in] vcn_key VCN key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_vcn_delete_handle (_In_ oci_vcn_key_t *vcn_key)
{
    oci_int_vcn_t *int_vcn;

    if ((int_vcn = oci_int_vcn_db_del(vcn_key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    oci_int_vcn_uninit_free(int_vcn);

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
    //TODO: validate the data ranges of vcn_key fields
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

    if ((rv = oci_vcn_delete_validate(vcn_key)) != SDK_RET_OK)
        return rv;
    
    if ((rv = oci_vcn_delete_handle(vcn_key)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/** @} */ // end of OCI_INT_VCN_API

}  // namespace api 
