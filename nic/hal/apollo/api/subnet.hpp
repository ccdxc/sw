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
 * @defgroup OCI_SUBNET_ENTRY - subnet functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief    subnet entry
 */
class subnet_entry : public oci_base {
 public:
    /**
     * @brief    factory method to allocate and initialize a subnet entry
     * @param[in] oci_subnet    subnet information
     * @return    new instance of subnet or NULL, in case of error
     */
    static subnet_entry *factory(oci_subnet_t *oci_subnet);

    /**
     * @brief    release all the s/w state associate with the given subnet, if
     *           any, and free the memory
     * @param[in] subnet     subnet to be freed
     * NOTE: h/w entries should have been cleaned up (by calling cleanup_hw()
     * before calling this
     */
    static void destroy(subnet_entry *subnet);

    /**
     * @brief    process a create/delete/update/get operation on a subnet
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t process_api(api_ctxt_t *api_ctxt);

    /**
     * @brief    commit() is invokved during commit phase of the API processing
     *           and is not expected to fail as all required resources are
     *           already allocated by now. Based on the API operation, this API
     *           is expected to process either create/retrieve/update/delete. If
     *           any temporary state was stashed in the api_ctxt while
     *           processing this API, it should be freed here
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     *
     * NOTE:     commit() is not expected to fail
     */
    sdk_ret_t commit(api_ctxt_t *api_ctxt);

    /**
     * @brief     abort() is invoked during abort phase of the API processing
     *            and is not expected to fail. During this phase, all associated
     *            resources must be freed and global DBs need to be restored
     *            back to their original state and any transient state stashed
     *            in api_ctxt while processing this API should also be freed
     *            here
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t abort(api_ctxt_t *api_ctxt);

    /**
     * @brief     helper function to get key given subnet entry
     * @param[in] entry    pointer to subnet instance
     * @return    pointer to the subnet instance's key
     */
    static void *subnet_key_func_get(void *entry) {
        subnet_entry *subnet = (subnet_entry *)entry;
        return (void *)&(subnet->key_);
    }

    /**
     * @brief     helper function to compute hash value for given subnet id
     * @param[in] key        subnet's key
     * @param[in] ht_size    hash table size
     * @return    hash value
     */
    static uint32_t subnet_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_subnet_key_t)) % ht_size;
    }

    /**
     * @brief     helper function to compare two subnet keys
     * @param[in] key1        pointer to subnet's key
     * @param[in] key2        pointer to subnet's key
     * @return    0 if keys are same or else non-zero value
     */
    static bool subnet_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_subnet_key_t)))
            return true;

        return false;
    }

private:
    /**
     * @brief    constructor
     */
    subnet_entry() {}
    /**
     * @brief    destructor
     */
    ~subnet_entry() {}
    /**
     * @brief     initialize subnet entry with the given config
     * @param[in] oci_subnet    subnet information
     * @return    SDK_RET_OK on success, failure status code on error
     *
     * NOTE:     allocate all h/w resources (i.e., table indices as well here, we
     *           can always release them in abort phase if something goes wrong
     */
    sdk_ret_t init(oci_subnet_t *oci_subnet);
    /**
     * @brief     handle a subnet create by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t process_create(api_ctxt_t *api_ctxt);
    /**
     * @brief     handle a subnet update by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t process_update(api_ctxt_t *api_ctxt);
    /**
     * @brief     handle a subnet delete by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t process_delete(api_ctxt_t *api_ctxt);
    /**
     * @brief     handle a subnet get by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t process_get(api_ctxt_t *api_ctxt);

    /**
     * @brief     add given subnet to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t add_to_db(void);

    /**
     * @brief     delete given subnet from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t del_from_db(void);

 private:
    oci_subnet_key_t    key_;               /**< subnet Key */
    mac_addr_t          vr_mac_;            /**< virtual router MAC */
    ht_ctxt_t           ht_ctxt_;           /**< Hash table context */

    /**< P4 datapath specific state */
    uint16_t            hw_id_;             /**< Internal ID */
    mem_addr_t          lpm_base_addr_;     /**< LPM base address for current
                                                 epoch */
    mem_addr_t          policy_base_addr_;  /**< security policy rules base
                                                 address for current epoch */
} __PACK__;

/** @} */ // end of OCI_SUBNET_ENTRY

/**
 * @defgroup OCI_SUBNET_STATE - subnet state functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief    state maintained for subnets
 */
class subnet_state {
public:
    /**
     * @brief    constructor
     */
    subnet_state();

    /**
     * @brief    destructor
     */
    ~subnet_state();

    /**
     * @brief    allocate memory required for a subnet
     * @return pointer to the allocated subnet, NULL if no memory
     */
    subnet_entry *subnet_alloc(void);

    /**
     * @brief     destroy subnet and free the memory back to the slab
     * @param[in] subnet subnet
     */
    void subnet_free(subnet_entry *subnet);

    /**
     * @brief     lookup a subnet in database given the key
     * @param[in] subnet_key subnet key
     */
    subnet_entry *subnet_find(oci_subnet_key_t *subnet_key) const;

private:
    ht *subnet_ht(void) { return subnet_ht_; }
    indexer *subnet_idxr(void) { return subnet_idxr_; }
    slab *subnet_slab(void) { return subnet_slab_; }
    friend class subnet_entry;   /**< subnet_entry class is friend of subnet_state */

private:
    ht      *subnet_ht_;      /**< Hash table root */
    indexer *subnet_idxr_;    /**< Indexer to allocate hw subnet id */
    slab    *subnet_slab_;    /**< slab for allocating subnet state */
};

/** * @} */ // end of OCI_SUBNET_STATE

}    // namespace api

#endif    /** __SUBNET_HPP__ */
