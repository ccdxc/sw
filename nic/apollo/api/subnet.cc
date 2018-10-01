/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.cc
 *
 * @brief   This file deals with OCI subnet API handling
 */

namespace api {

using sdk::lib::slab;
using sdk::lib::ht;
using sdk::lib::indexer;

/**
 * @defgroup OCI_INT_SUBNET - Internal subnet 
 * @{
 */

/**
 * @defgroup OCI_INT_SUBNET_GSTATE - Internal subnet global state functionality
 * @ingroup OCI_INT_SUBNET
 * @{
 */

/**
 * @brief oci_int_gstate
 */
typedef struct oci_int_subnet_gstate_s
{
    slab *slab[MAX];    /**< Memory slab */
    ht *ht;             /**< Hash table root */

} oci_int_subnet_gstate_t;

oci_int_subnet_gstate_t g_int_subnet_state;

/**
 * @brief Initialize global internal subnet state
 */
void
oci_g_int_subnet_state_init (void)
{
    //TODO: Indexer, slab and ht init
}

/** @} */ // end of OCI_INT_SUBNET_GSTATE

/**
 * @defgroup OCI_INT_SUBNET_DB - Internal subnet database functionality
 * @ingroup OCI_INT_SUBNET
 * @{
 */

/**
 * @brief Add internal subnet to database
 *
 * @param[in] subnet Internal subnet 
 */ 
static inline sdk_ret_t
oci_int_subnet_db_add (oci_int_subnet_t *subnet)
{
    return (g_int_subnet_state->ht()->insert_with_key(&subnet->key, subnet,
                                                   subnet->ht_ctxt);
}

/**
 * @brief Delete internal subnet from database
 *
 * @param[in] subnet_key Internal subnet key
 */
static inline oci_int_subnet_t *
oci_int_subnet_db_del (oci_int_vnc_key_t *subnet_key)
{
    return (oci_int_subnet_t *)(g_int_subnet_state->ht()->remove(subnet_key));
}

/**
 * @brief Lookup internal subnet in database
 *
 * @param[in] subnet_key Internal subnet key
 */
static inline oci_int_vnc_t *
oci_int_subnet_db_lookup (oci_int_subnet_key_t *subnet_key)
{
    return (oci_int_subnet_t *)(g_int_subnet_state->ht()->lookup(subnet_key));
}

/** @} */ // end of OCI_INT_SUBNET_DB

/**
 * @defgroup OCI_INT_SUBNET_INIT - Internal subnet setup/teardown functionality
 * @ingroup OCI_INT_SUBNET
 * @{
 */

/**
 * @brief Allocate internal subnet structure
 *
 * @return Pointer to the allocated internal subnet, NULL if no memory
 */
static inline oci_int_subnet_t *
oci_int_subnet_alloc (void)
{
    return ((oci_int_subnet_t *)g_int_subnet_state->int_subnet_slab()->alloc());
}

/**
 * @brief Free internal subnet structure
 *
 * @param[in] subnet Internal subnet 
 */
static inline void
oci_int_subnet_free (oci_int_subnet_t *subnet)
{
    sdk::delay_delete_to_slab(OCI_SLAB_INT_SUBNET, int_subnet);
}

/**
 * @brief Initialize internal subnet structure
 *
 * @param[in] subnet Internal subnet 
 */
static inline void
oci_int_subnet_init (oci_int_vnc_t *int_subnet, oci_subnet_t *subnet)
{
    SDK_SPINLOCK_INIT(&int_subnet->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&int_subnet->key, &subnet->key, sizeof(oci_subnet_key_t));
    //TODO: Get a index from indexer
}

/**
 * @brief Uninitialize internal subnet structure
 *
 * @param[in] subnet Internal subnet 
 */
static inline void
oci_int_subnet_uninit (oci_int_subnet_t *int_subnet)
{
    SDK_SPINLOCK_DESTROY(&int_subnet->slock);
    //TODO: Release a index to indexer
}

/**
 * @brief Allocate and initialize internal subnet structure
 *
 * @return Pointer to the allocated and initialized internal subnet,
 *         NULL if no memory
 */
static inline oci_int_subnet_t *
oci_int_subnet_alloc_init (oci_subnet_t *subnet)
{
    oci_int_subnet_t *int_subnet;

    if ((int_subnet = oci_int_subnet_alloc()) == NULL)
        return NULL;

    oci_int_subnet_init(int_subnet, subnet);
    return subnet;
}

/**
 * @brief Uninitialize and free internal subnet structure
 *
 * @param[in] subnet Internal subnet 
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
 * @defgroup OCI_INT_SUBNET_API - First level of subnet API handling
 * @ingroup OCI_INT_SUBNET
 * @{
 */

/**
 * @brief Handle subnet create message
 *
 * @param[in] subnet Subnet information
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_create_handle (_In_ oci_subnet_t *subnet)
{
    oci_int_subnet_t *int_subnet;

    if ((int_subnet = oci_int_subnet_alloc_init(subnet)) == NULL)
        return SDK_RET_NO_MEM;

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC API data
 *
 * @param[in] subnet Subnet
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_create_validate (_In_ oci_subnet_t *subnet)
{
    //TODO: validate the data ranges of subnet fields
    return SDK_RET_OK;
}

/**
 * @brief Create subnet 
 *
 * @param[in] subnet Subnet
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_create (_In_ oci_subnet_t *subnet)
{
    sdk_ret_t rv;

    if ((rv = oci_subnet_create_validate(subnet) != SDK_RET_OK))
        return rv;

    if ((rv = oci_subnet_create_handle(subnet)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/**
 * @brief Handle subnet delete API
 *
 * @param[in] subnet_key Subnet key
 * @return #SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
oci_subnet_delete_handle (_In_ oci_subnet_key_t *subnet_key)
{
    oci_int_subnet_t *int_subnet;

    if ((int_subnet = oci_int_subnet_db_del(subnet_key)) == NULL)
        return SDK_RET_NOT_FOUND;

    oci_int_subnet_uninit_free(int_subnet);

    return SDK_RET_OK;
}

/**
 * @brief Validate VNC delete API
 *
 * @param[in] subnet_key Subnet key
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
 * @param[in] subnet_key Subnet key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_delete (_In_ oci_subnet_key_t *subnet_key)
{
    sdk_ret_t rv;
    oci_subnet_t *subnet;

    if ((rv = oci_subnet_delete_validate(subnet_key)) != SDK_RET_OK)
        return rv;
    
    if ((rv = oci_subnet_delete_handle(subnet_key)) != SDK_RET_OK)
        return rv;

    return SDK_RET_OK;
}

/** @} */ // end of OCI_INT_SUBNET_API

}  // namespace api 
