/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.hpp
 *
 * @brief   subnet entry handlnig
 */

#if !defined (__SUBNET_HPP__)
#define __SUBNET_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/include/api/oci_subnet.hpp"

namespace api {

/**
 * @defgroup OCI_SUBNET_ENTRY - subnet functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief    subnet entry
 */
class subnet_entry : public api_base {
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
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(subnet_entry *subnet);

    /**
     * @brief     initialize subnet entry with the given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_config(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_config(oci_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     add given subnet to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete given subnet from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] old         old version of the object being swapped out
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

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

    mac_addr_t& vr_mac(void) { return vr_mac_; }

    uint16_t hw_id(void) const { return hw_id_; }
    oci_route_table_key_t route_table(void) const { return route_table_; }
    mem_addr_t policy_tree_root(void) const { return policy_base_addr_; }

private:
    /**< @brief    constructor */
    subnet_entry();

    /**< @brief    destructor */
    ~subnet_entry();

    /**
     * @brief    allocate h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t reserve_resources_(void);

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t release_resources_(void);

 private:
    oci_subnet_key_t         key_;               /**< subnet Key */
    oci_route_table_key_t    route_table_;       /**< route table id */
    mac_addr_t               vr_mac_;            /**< virtual router MAC */
    ht_ctxt_t                ht_ctxt_;           /**< hash table context */

    /**< P4 datapath specific state */
    uint16_t                 hw_id_;             /**< hardware id */
    mem_addr_t               policy_base_addr_;  /**< security policy rules base
                                                      address for current epoch */
} __PACK__;

/** @} */    // end of OCI_SUBNET_ENTRY

}    // namespace api

using api::subnet_entry;

#endif    /** __SUBNET_HPP__ */
