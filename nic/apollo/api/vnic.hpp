/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.hpp
 *
 * @brief   vnic entry handling
 */

#if !defined (__VNIC_HPP__)
#define __VNIC_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/oci_vnic.hpp"

namespace api {

/**
 * @defgroup OCI_VNIC_ENTRY - vnic functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    vnic entry
 */
class vnic_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize a vnic entry
     * @param[in] oci_vnic    vnic information
     * @return    new instance of vnic or NULL, in case of error
     */
    static vnic_entry *factory(oci_vnic_t *oci_vnic);

    /**
     * @brief    release all the s/w state associate with the given vnic,
     *           if any, and free the memory
     * @param[in] vnic     vnic to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(vnic_entry *vnic);

    /**
     * @brief     initialize vnic entry with the given config
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
    virtual sdk_ret_t update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) override;

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
     * @brief     add given vnic to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete given vnic from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

    /**
     * @brief     helper function to get key given vnic entry
     * @param[in] entry    pointer to vnic instance
     * @return    pointer to the vnic instance's key
     */
    static void *vnic_key_func_get(void *entry) {
        vnic_entry *vnic = (vnic_entry *)entry;
        return (void *)&(vnic->key_);
    }

    /**
     * @brief     helper function to compute hash value for given vnic id
     * @param[in] key        vnic's key
     * @param[in] ht_size    hash table size
     * @return    hash value
     */
    static uint32_t vnic_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_vnic_key_t)) % ht_size;
    }

    /**
     * @brief     helper function to compare two vnic keys
     * @param[in] key1        pointer to vnic's key
     * @param[in] key2        pointer to vnic's key
     * @return    0 if keys are same or else non-zero value
     */
    static bool vnic_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_vnic_key_t))) {
            return true;
        }
        return false;
    }

    /**
     * @brief     return impl instance of this vnic object
     * @return    impl instance of the vnic object
     */
    impl_base *impl(void) { return impl_; }

private:
    /**< @brief    constructor */
    vnic_entry();

    /**< @brief    destructor */
    ~vnic_entry();

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
    oci_vnic_key_t    key_;        /**< vnic key */
    ht_ctxt_t         ht_ctxt_;    /**< hash table context */
    impl_base         *impl_;      /**< impl object instance */
} __PACK__;

/** @} */    // end of OCI_VNIC_ENTRY

}    // namespace api

using api::vnic_entry;

#endif    /** __VNIC_HPP__ */
