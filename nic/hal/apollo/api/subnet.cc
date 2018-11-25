/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.cc
 *
 * @brief   This file deals with OCI subnet API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/hal/apollo/include/api/oci_subnet.hpp"
#include "nic/hal/apollo/api/subnet.hpp"

using namespace sdk;

namespace api {

static inline void oci_int_subnet_db_init (void);

/**
 * @defgroup OCI_INT_SUBNET - Internal SUBNET
 * @{
 */

/**
 * @defgroup OCI_INT_SUBNET_GSTATE - Internal SUBNET global state functionality
 * @ingroup OCI_INT_SUBNET
 * @{
 */

#define  OCI_INT_SUBNET_ID_MAX  1024

/**
 * @brief oci_int_gstate
 */
typedef struct oci_int_subnet_gstate_s
{
    //slab *slab[MAX];   /**< Memory slab */
    sdk::lib::ht *ht;              /**< Hash table root */
    sdk::lib::indexer *id_idxr;    /**< Indexer to allocate internal id */

} oci_int_subnet_gstate_t;

oci_int_subnet_gstate_t g_int_subnet_state;

/**
 * @brief Initialize global internal SUBNET state
 */
void
oci_g_int_subnet_state_init (void)
{
    //TODO: slab init

    oci_int_subnet_db_init();

    g_int_subnet_state.id_idxr = sdk::lib::indexer::factory(
        OCI_INT_SUBNET_ID_MAX);
    SDK_ASSERT_RETURN_VOID(g_int_subnet_state.id_idxr != NULL);
}

/** @} */ // end of OCI_INT_SUBNET_GSTATE

/**
 * @defgroup OCI_INT_SUBNET_DB - Internal SUBNET database functionality
 * @ingroup OCI_INT_SUBNET
 * @{
 */

static void *
oci_int_subnet_key_func_get (void *entry)
{
    oci_int_subnet_t *int_subnet = (oci_int_subnet_t *)entry;
    return (void *)&(int_subnet->key);
}

static uint32_t
oci_int_subnet_hash_func_compute (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(oci_subnet_key_t))%ht_size;
}

static bool
oci_int_subnet_key_func_compare (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(oci_subnet_key_t)))
        return true;

    return false;
}

/**
 * @brief Initialize internal SUBNET database
 */
static inline void
oci_int_subnet_db_init (void)
{
    g_int_subnet_state.ht = sdk::lib::ht::factory(
        OCI_INT_SUBNET_ID_MAX >> 1, oci_int_subnet_key_func_get,
        oci_int_subnet_hash_func_compute, oci_int_subnet_key_func_compare);
    SDK_ASSERT_RETURN_VOID(g_int_subnet_state.ht != NULL);
}

/**
 * @brief Add internal SUBNET to database
 *
 * @param[in] subnet Internal SUBNET  
 */ 
static inline sdk_ret_t
oci_int_subnet_db_add (oci_int_subnet_t *int_subnet)
{
    return (g_int_subnet_state.ht->insert_with_key(&int_subnet->key, int_subnet,
                                                &int_subnet->ht_ctxt));
}

/**
 * @brief Delete internal SUBNET from database
 *
 * @param[in] subnet_key SUBNET key
 */
static inline oci_int_subnet_t *
oci_int_subnet_db_del (oci_subnet_key_t *subnet_key)
{
    return (oci_int_subnet_t *)(g_int_subnet_state.ht->remove(subnet_key));
}

/**
 * @brief Lookup internal SUBNET in database
 *
 * @param[in] subnet_key Internal SUBNET key
 */
static inline oci_int_subnet_t *
oci_int_subnet_db_lookup (oci_subnet_key_t *subnet_key)
{
    return (oci_int_subnet_t *)(g_int_subnet_state.ht->lookup(subnet_key));
}

/** @} */ // end of OCI_INT_SUBNET_DB

/**
 * @defgroup OCI_INT_SUBNET_INIT - Internal SUBNET setup/teardown functionality
 * @ingroup OCI_INT_SUBNET
 * @{
 */

/**
 * @brief Allocate internal SUBNET structure
 *
 * @return Pointer to the allocated internal SUBNET, NULL if no memory
 */
static inline oci_int_subnet_t *
oci_int_subnet_alloc (void)
{
//    return ((oci_int_subnet_t *)g_int_subnet_state->int_subnet_slab()->alloc());
    return NULL;
}

/**
 * @brief Free internal SUBNET structure
 *
 * @param[in] int_subnet Internal SUBNET
 */
static inline void
oci_int_subnet_free (oci_int_subnet_t *int_subnet)
{
//    sdk::delay_delete_to_slab(OCI_SLAB_INT_SUBNET, int_subnet);
}

/**
 * @brief Initialize internal SUBNET structure
 *
 * @param[in] int_subnet Internal SUBNET
 * @param[in] subnet SUBNET
 */
static inline sdk_ret_t
oci_int_subnet_init (oci_int_subnet_t *int_subnet, oci_subnet_t *subnet)
{
    //SDK_SPINLOCK_INIT(&int_subnet->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&int_subnet->key, &subnet->key, sizeof(oci_subnet_key_t));

    int_subnet->ht_ctxt.reset();

    if (g_int_subnet_state.id_idxr->alloc(&int_subnet->id) !=
        sdk::lib::indexer::SUCCESS)
        return SDK_RET_NO_RESOURCE;

    return SDK_RET_OK;
}

/**
 * @brief Uninitialize internal SUBNET structure
 *
 * @param[in] subnet Internal SUBNET
 */
static inline void
oci_int_subnet_uninit (oci_int_subnet_t *int_subnet)
{
    //SDK_SPINLOCK_DESTROY(&int_subnet->slock);
    g_int_subnet_state.id_idxr->free(int_subnet->id);
}

/**
 * @brief Allocate and initialize internal SUBNET structure
 *
 * @return Pointer to the allocated and initialized internal SUBNET,
 *         NULL if no memory
 */
static inline oci_int_subnet_t *
oci_int_subnet_alloc_init (oci_subnet_t *subnet)
{
    oci_int_subnet_t *int_subnet;

    if ((int_subnet = oci_int_subnet_alloc()) == NULL)
        return NULL;

    oci_int_subnet_init(int_subnet, subnet);
    return int_subnet;
}

/**
 * @brief Uninitialize and free internal SUBNET structure
 *
 * @param[in] subnet Internal SUBNET
 */
static inline void
oci_int_subnet_uninit_free (_In_ oci_int_subnet_t *subnet)
{
    if (subnet) {
        oci_int_subnet_uninit(subnet);
        oci_int_subnet_free(subnet);
    }
}

/** @} */ // end of OCI_INT_SUBNET_DB

/**
 * @defgroup OCI_INT_SUBNET_API - First level of SUBNET API handling
 * @ingroup OCI_INT_SUBNET
 * @{
 */

/**
 * @brief Handle SUBNET create message
 *
 * @param[in] subnet SUBNET information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_create_handle (_In_ oci_subnet_t *subnet)
{
    oci_int_subnet_t *int_subnet;

    if ((int_subnet = oci_int_subnet_alloc_init(subnet)) == NULL)
        return SDK_RET_OOM;

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC API data
 *
 * @param[in] subnet SUBNET information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_create_validate (_In_ oci_subnet_t *subnet)
{
    //TODO: validate the data ranges of subnet fields
    return SDK_RET_OK;
}

/**
 * @brief Create SUBNET
 *
 * @param[in] subnet SUBNET information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_create (_In_ oci_subnet_t *subnet)
{
    sdk_ret_t rv;

    if ((rv = oci_subnet_create_validate(subnet)) != SDK_RET_OK)
        return rv;

    if ((rv = oci_subnet_create_handle(subnet)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/**
 * @brief Handle SUBNET delete API
 *
 * @param[in] subnet_key SUBNET key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_delete_handle (_In_ oci_subnet_key_t *subnet_key)
{
    oci_int_subnet_t *int_subnet;

    if ((int_subnet = oci_int_subnet_db_del(subnet_key)) == NULL)
        return SDK_RET_ENTRY_NOT_FOUND;

    oci_int_subnet_uninit_free(int_subnet);

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC delete API
 *
 * @param[in] subnet_key SUBNET key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_delete_validate (_In_ oci_subnet_key_t *subnet_key)
{
    //TODO: validate the data ranges of subnet_key fields
    return SDK_RET_OK;
}

/**
 * @brief Delete SUBNET
 *
 * @param[in] subnet_key SUBNET key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_delete (_In_ oci_subnet_key_t *subnet_key)
{
    sdk_ret_t rv;

    if ((rv = oci_subnet_delete_validate(subnet_key)) != SDK_RET_OK)
        return rv;
    
    if ((rv = oci_subnet_delete_handle(subnet_key)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/** @} */ // end of OCI_INT_SUBNET_API

}  // namespace api 
