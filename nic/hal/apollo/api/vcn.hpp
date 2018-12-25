/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.hpp
 *
 * @brief   APIs for vcn implementation
 */

#if !defined (__VCN_HPP__)
#define __VCN_HPP__

#include "nic/sdk/include/sdk/slab.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"

namespace api {

/**
 * @defgroup OCI_VCN_ENTRY - vcn functionality
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief    vcn entry
 */
class vcn_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize a vcn entry
     * @param[in] oci_vcn    vcn information
     * @return    new instance of vcn or NULL, in case of error
     */
    static vcn_entry *factory(oci_vcn_t *oci_vcn);

    /**
     * @brief    release all the s/w state associate with the given vcn, if any,
     *           and free the memory
     * @param[in] vcn     vcn to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(vcn_entry *vcn);

    /**
     * @brief     initialize vcn entry with the given config
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
     * @brief     add given vcn to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete given vcn from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the object being swapped out
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

    /**
     * @brief     helper function to get key given vcn entry
     * @param[in] entry    pointer to vcn instance
     * @return    pointer to the vcn instance's key
     */
    static void *vcn_key_func_get(void *entry) {
        vcn_entry *vcn = (vcn_entry *)entry;
        return (void *)&(vcn->key_);
    }

    /**
     * @brief     helper function to compute hash value for given vcn id
     * @param[in] key        vcn's key
     * @param[in] ht_size    hash table size
     * @return    hash value
     */
    static uint32_t vcn_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_vcn_key_t)) % ht_size;
    }

    /**
     * @brief     helper function to compare two vcn keys
     * @param[in] key1        pointer to vcn's key
     * @param[in] key2        pointer to vcn's key
     * @return    0 if keys are same or else non-zero value
     */
    static bool vcn_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_vcn_key_t))) {
            return true;
        }
        return false;
    }

    // TODO: this is impl API
    uint16_t hwid(void) { return hw_id_; }

private:
    /**< @brief    constructor */
    vcn_entry();

    /**< @brief    destructor */
    ~vcn_entry();

    /**
     * @brief    allocate h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t alloc_resources_(void);

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t free_resources_(void);

private:
    oci_vcn_key_t    key_;        /**< vcn key */
    ht_ctxt_t        ht_ctxt_;    /**< hash table context */

    /**< P4 datapath specific state */
    uint16_t         hw_id_;      /**< hardware id */
} __PACK__;

/** @} */    // end of OCI_VCN_ENTRY

/**
 * @defgroup OCI_VCN_STATE - vcn state functionality
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief    state maintained for vcns
 */
class vcn_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    vcn_state();

    /**
     * @brief    destructor
     */
    ~vcn_state();

    /**
     * @brief    allocate memory required for a vcn
     * @return pointer to the allocated vcn, NULL if no memory
     */
    vcn_entry *vcn_alloc(void);

    /**
     * @brief      free vcn instance back to slab
     * @param[in]  vcn   pointer to the allocated vcn
     */
    void vcn_free(vcn_entry *vcn);

    /**
     * @brief     lookup a vcn in database given the key
     * @param[in] vcn_key vcn key
     */
    vcn_entry *vcn_find(oci_vcn_key_t *vcn_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *vcn_ht(void) { return vcn_ht_; }
    indexer *vcn_idxr(void) { return vcn_idxr_; }
    slab *vcn_slab(void) { return vcn_slab_; }
    friend class vcn_entry;   /**< vcn_entry class is friend of vcn_state */

private:
    ht         *vcn_ht_;      /**< Hash table root */
    indexer    *vcn_idxr_;    /**< Indexer to allocate hw vcn id */
    slab       *vcn_slab_;    /**< slab for allocating vcn entry */
};

/** * @} */    // end of OCI_VCN_STATE

}    // namespace api

using api::vcn_entry;
using api::vcn_state;

#endif    /** __VCN_HPP__ */
