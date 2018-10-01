/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep.cc
 *
 * @brief   This file deals with OCI TEP API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/apollo/include/api/oci_tep.hpp"
#include "nic/apollo/api/tep.hpp"

using namespace sdk;

namespace api {

static inline void oci_int_tep_db_init (void);

/**
 * @defgroup OCI_INT_TEP - Internal TEP
 * @{
 */

/**
 * @defgroup OCI_INT_TEP_GSTATE - Internal TEP global state functionality
 * @ingroup OCI_INT_TEP
 * @{
 */

#define  OCI_INT_TEP_ID_MAX  1024

/**
 * @brief oci_int_gstate
 */
typedef struct oci_int_tep_gstate_s
{
    //slab *slab[MAX];   /**< Memory slab */
    sdk::lib::ht *ht;              /**< Hash table root */
    sdk::lib::indexer *id_idxr;    /**< Indexer to allocate internal id */

} oci_int_tep_gstate_t;

oci_int_tep_gstate_t g_int_tep_state;

/**
 * @brief Initialize global internal TEP state
 */
void
oci_g_int_tep_state_init (void)
{
    //TODO: slab init

    oci_int_tep_db_init();

    g_int_tep_state.id_idxr = sdk::lib::indexer::factory(OCI_INT_TEP_ID_MAX);
    SDK_ASSERT_RETURN_VOID(g_int_tep_state.id_idxr != NULL);
}

/** @} */ // end of OCI_INT_TEP_GSTATE

/**
 * @defgroup OCI_INT_TEP_DB - Internal TEP database functionality
 * @ingroup OCI_INT_TEP
 * @{
 */


static void *
oci_int_tep_key_func_get (void *entry)
{
    oci_int_tep_t *int_tep = (oci_int_tep_t *)entry;
    return (void *)&(int_tep->key);
}

static uint32_t
oci_int_tep_hash_func_compute (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(oci_tep_key_t)) % ht_size;
}

static bool
oci_int_tep_key_func_compare (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(oci_tep_key_t)))
        return true;

    return false;
}

/**
 * @brief Initialize internal TEP database
 */
static inline void
oci_int_tep_db_init (void)
{
    g_int_tep_state.ht = sdk::lib::ht::factory(
        OCI_INT_TEP_ID_MAX >> 1, oci_int_tep_key_func_get,
        oci_int_tep_hash_func_compute, oci_int_tep_key_func_compare);
    SDK_ASSERT_RETURN_VOID(g_int_tep_state.ht != NULL);
}

/**
 * @brief Add internal TEP to database
 *
 * @param[in] tep Internal TEP  
 */ 
static inline sdk_ret_t
oci_int_tep_db_add (oci_int_tep_t *int_tep)
{
    return (g_int_tep_state.ht->insert_with_key(&int_tep->key, int_tep,
                                                &int_tep->ht_ctxt));
}

/**
 * @brief Delete internal TEP from database
 *
 * @param[in] tep_key TEP key
 */
static inline oci_int_tep_t *
oci_int_tep_db_del (oci_tep_key_t *tep_key)
{
    return (oci_int_tep_t *)(g_int_tep_state.ht->remove(tep_key));
}

/**
 * @brief Lookup internal TEP in database
 *
 * @param[in] tep_key Internal TEP key
 */
static inline oci_int_tep_t *
oci_int_tep_db_lookup (oci_tep_key_t *tep_key)
{
    return (oci_int_tep_t *)(g_int_tep_state.ht->lookup(tep_key));
}

/** @} */ // end of OCI_INT_TEP_DB

/**
 * @defgroup OCI_INT_TEP_INIT - Internal TEP setup/teardown functionality
 * @ingroup OCI_INT_TEP
 * @{
 */

/**
 * @brief Allocate internal TEP structure
 *
 * @return Pointer to the allocated internal TEP, NULL if no memory
 */
static inline oci_int_tep_t *
oci_int_tep_alloc (void)
{
//    return ((oci_int_tep_t *)g_int_tep_state->int_tep_slab()->alloc());
    return NULL;
}

/**
 * @brief Free internal TEP structure
 *
 * @param[in] int_tep Internal TEP
 */
static inline void
oci_int_tep_free (oci_int_tep_t *int_tep)
{
//    sdk::delay_delete_to_slab(OCI_SLAB_INT_TEP, int_tep);
}

/**
 * @brief Initialize internal TEP structure
 *
 * @param[in] int_tep Internal TEP
 * @param[in] tep TEP
 */
static inline sdk_ret_t
oci_int_tep_init (oci_int_tep_t *int_tep, oci_tep_t *tep)
{
    //SDK_SPINLOCK_INIT(&int_tep->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&int_tep->key, &tep->key, sizeof(oci_tep_key_t));

    int_tep->ht_ctxt.reset();

    if (g_int_tep_state.id_idxr->alloc(&int_tep->id) !=
        sdk::lib::indexer::SUCCESS)
        return SDK_RET_NO_RESOURCE;

    return SDK_RET_OK;
}

/**
 * @brief Uninitialize internal TEP structure
 *
 * @param[in] tep Internal TEP
 */
static inline void
oci_int_tep_uninit (oci_int_tep_t *int_tep)
{
    //SDK_SPINLOCK_DESTROY(&int_tep->slock);
    g_int_tep_state.id_idxr->free(int_tep->id);
}

/**
 * @brief Allocate and initialize internal TEP structure
 *
 * @return Pointer to the allocated and initialized internal TEP,
 *         NULL if no memory
 */
static inline oci_int_tep_t *
oci_int_tep_alloc_init (oci_tep_t *tep)
{
    oci_int_tep_t *int_tep;

    if ((int_tep = oci_int_tep_alloc()) == NULL)
        return NULL;

    oci_int_tep_init(int_tep, tep);
    return int_tep;
}

/**
 * @brief Uninitialize and free internal TEP structure
 *
 * @param[in] tep Internal TEP
 */
static inline void
oci_int_tep_uninit_free (_In_ oci_int_tep_t *tep)
{
    if (tep) {
        oci_int_tep_uninit(tep);
        oci_int_tep_free(tep);
    }
}

/** @} */ // end of OCI_INT_TEP_DB

/**
 * @defgroup OCI_INT_TEP_API - First level of TEP API handling
 * @ingroup OCI_INT_TEP
 * @{
 */

/**
 * @brief Handle TEP create message
 *
 * @param[in] tep TEP information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_tep_create_handle (_In_ oci_tep_t *tep)
{
    oci_int_tep_t *int_tep;

    if ((int_tep = oci_int_tep_alloc_init(tep)) == NULL)
        return SDK_RET_OOM;

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC API data
 *
 * @param[in] tep TEP information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_tep_create_validate (_In_ oci_tep_t *tep)
{
    //TODO: validate the data ranges of tep fields
    return SDK_RET_OK;
}

/**
 * @brief Create TEP
 *
 * @param[in] tep TEP information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_tep_create (_In_ oci_tep_t *tep)
{
    sdk_ret_t rv;

    if ((rv = oci_tep_create_validate(tep)) != SDK_RET_OK)
        return rv;

    if ((rv = oci_tep_create_handle(tep)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/**
 * @brief Handle TEP delete API
 *
 * @param[in] tep_key TEP key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_tep_delete_handle (_In_ oci_tep_key_t *tep_key)
{
    oci_int_tep_t *int_tep;

    if ((int_tep = oci_int_tep_db_del(tep_key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    oci_int_tep_uninit_free(int_tep);

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC delete API
 *
 * @param[in] tep_key TEP key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_tep_delete_validate (_In_ oci_tep_key_t *tep_key)
{
    //TODO: validate the data ranges of tep_key fields
    return SDK_RET_OK;
}

/**
 * @brief Delete TEP
 *
 * @param[in] tep_key TEP key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_tep_delete (_In_ oci_tep_key_t *tep_key)
{
    sdk_ret_t rv;

    if ((rv = oci_tep_delete_validate(tep_key)) != SDK_RET_OK)
        return rv;
    
    if ((rv = oci_tep_delete_handle(tep_key)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/** @} */ // end of OCI_INT_TEP_API

}  // namespace api 
