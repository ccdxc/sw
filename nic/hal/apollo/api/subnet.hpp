/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.hpp
 *
 * @brief   This file deals with OCI SUBNET functionality required internally by
 *          other modules
 */

#if !defined (__SUBNET_HPP__)
#define __SUBNET_HPP__

#include "nic/hal/apollo/api/api.hpp"
#include "nic/hal/apollo/include/api/oci_subnet.hpp"

namespace api {

/**
 * @defgroup OCI_INT_SUBNET - Internal subnet
 * @{
 */

/**
 * @brief Internal subnet structure
 */
typedef struct subnet_s {
    oci_subnet_key_t    key;               /**< subnet Key */
    mac_addr_t          vr_mac;            /**< virtual router MAC */
    ht_ctxt_t           ht_ctxt;           /**< Hash table context */

    /**< P4 datapath specific state */
    uint16_t            hw_id;             /**< Internal ID */
    mem_addr_t          lpm_base_addr;     /**< LPM base address for current
                                                epoch */
    mem_addr_t          policy_base_addr;  /**< security policy rules base
                                                address for current epoch */
} __PACK__ subnet_t;

/**
 * @brief state maintained for subnets
 */
class subnet_state {
public:
    /**
     * constructor
     */
    subnet_state();

    /**
     * destructor
     */
    ~subnet_state();

    /**
     * @brief handle subnet create message
     *
     * @param[in] subnet subnet information
     * @return #SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t subnet_create(_In_ oci_subnet_t *oci_subnet);

    /**
     * @brief handle subnet delete API
     *
     * @param[in] subnet_key subnet key information
     * @return #SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t subnet_delete(_In_ oci_subnet_key_t *key);

    subnet_t *subnet_get(_In_ oci_subnet_key_t *key);

    /**
     * @brief   return subnet slab
     */
    slab *subnet_slab(void) const { return subnet_slab_; }

private:
    /**
     * @brief add subnet to database
     *
     * @param[in] subnet subnet
     */ 
    sdk_ret_t subnet_add_to_db(subnet_t *subnet);

    /**
     * @brief delete subnet from database
     *
     * @param[in] subnet_key subnet key
     */
    subnet_t *subnet_del_from_db(oci_subnet_key_t *subnet_key);

    /**
     * @brief Lookup subnet in database
     *
     * @param[in] subnet_key subnet key
     */
    subnet_t *subnet_find(oci_subnet_key_t *subnet_key) const;

    /**
     * @brief Allocate subnet structure
     *
     * @return Pointer to the allocated subnet, NULL if no memory
     */
    subnet_t *subnet_alloc(void);

    /**
     * @brief Initialize subnet structure
     *
     * @param[in] subnet subnet structure to store the state
     * @param[in] oci_subnet subnet specific information
     */
    sdk_ret_t subnet_init(subnet_t *subnet, oci_subnet_t *oci_subnet);

    /**
     * @brief Allocate and initialize subnet structure
     *
     * @return Pointer to the allocated and initialized subnet,
     *         NULL if no memory
     */
    subnet_t *subnet_alloc_init(oci_subnet_t *oci_subnet);

    /**
     * @brief cleanup state maintained for given subnet
     *
     * @param[in] subnet subnet
     */
    void subnet_cleanup(subnet_t *subnet);

    /**
     * @brief Free subnet structure
     *
     * @param[in] subnet subnet
     */
    void subnet_free(subnet_t *subnet);

    /**
     * @brief Uninitialize and free subnet structure
     *
     * @param[in] subnet subnet
     */
    void subnet_delete(_In_ subnet_t *subnet);

    /**
     * @brief helper function to get key given subnet entry
     */
    static void *subnet_key_func_get(void *entry) {
        subnet_t *subnet = (subnet_t *)entry;
        return (void *)&(subnet->key);
    }

    /**
     * @brief helper function to compute hash value for given subnet id
     */
    static uint32_t subnet_hash_func_compute (void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_subnet_key_t)) % ht_size;
    }

    /**
     * @brief helper function to compare two subnet keys
     */
    static bool subnet_key_func_compare (void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_subnet_key_t)))
            return true;

        return false;
    }

private:
    ht      *subnet_ht_;      /**< Hash table root */
    indexer *subnet_idxr_;    /**< Indexer to allocate hw subnet id */
    slab    *subnet_slab_;    /**< slab for allocating subnet state */
};
extern subnet_state g_subnet_state;

/** * @} */ // end of OCI_INT_SUBNET

}    // namespace api

#endif /** __SUBNET_HPP__ */
