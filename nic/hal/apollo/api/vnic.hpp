/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.hpp
 *
 * @brief   This file deals with vnic functionality
 */

#if !defined (__VNIC_HPP__)
#define __VNIC_HPP__

#include "nic/sdk/include/sdk/slab.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/include/api/oci_vnic.hpp"

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
     * NOTE: h/w entries should have been cleaned up (by calling cleanup_hw()
     *       before calling this
     */
    static void destroy(vnic_entry *vnic);

#if 0
    /**
     * @brief     handle a vnic create by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_create(api_ctxt_t *api_ctxt) override;

    /**
     * @brief     handle a vnic update by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_update(api_ctxt_t *api_ctxt) override;

    /**
     * @brief     handle a vnic delete by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_delete(api_ctxt_t *api_ctxt) override;

    /**
     * @brief     handle a vnic get by allocating all required resources
     *            and keeping them ready for commit phase
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_get(api_ctxt_t *api_ctxt) override;
#endif

    /**
     * @brief     initialize vnic entry with the given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief     update/override the vnic object with given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_hw(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_hw(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_hw(api_base *orig_obj, obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_epoch(oci_epoch_t epoch, api_op_t api_op,
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

#if 0
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
    virtual sdk_ret_t commit(api_ctxt_t *api_ctxt) override;

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
    virtual sdk_ret_t abort(api_ctxt_t *api_ctxt) override;
#endif

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

private:
    /**< @brief    constructor */
    vnic_entry();

    /**< @brief    destructor */
    ~vnic_entry();

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
    oci_vnic_key_t    key_;        /**< vnic key */
    ht_ctxt_t         ht_ctxt_;    /**< hash table context */

    /**< P4 datapath specific state */
    uint16_t          hw_id_;      /**< hardware id */
    uint16_t          vnic_by_slot_hash_idx_;
} __PACK__;


/** @} */    // end of OCI_VNIC_ENTRY

/**
 * @defgroup OCI_VNIC_STATE - vnic state functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    state maintained for VNICs
 */
class vnic_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    vnic_state();

    /**
     * @brief    destructor
     */
    ~vnic_state();

    /**
     * @brief    allocate memory required for a vnic
     * @return pointer to the allocated vnic, NULL if no memory
     */
    vnic_entry *vnic_alloc(void);

    /**
     * @brief     destroy vnic and free the memory back to the slab
     * @param[in] vnic vnic instance
     */
    void vnic_free(vnic_entry *vnic);

    /**
     * @brief     lookup a vnic in database given the key
     * @param[in] vnic_key vnic key
     */
    vnic_entry *vnic_find(oci_vnic_key_t *vnic_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *vnic_ht(void) { return vnic_ht_; }
    indexer *vnic_idxr(void) { return vnic_idxr_; }
    directmap *local_vnic_by_vlan_tx_tbl(void) { return local_vnic_by_vlan_tx_tbl_; }
    sdk_hash *local_vnic_by_slot_rx_tbl(void) { return local_vnic_by_slot_rx_tbl_; }
    directmap *egress_local_vnic_info_rx_tbl(void) { return egress_local_vnic_info_rx_tbl_; }
    slab *vnic_slab(void) { return vnic_slab_; }
    friend class vnic_entry;   /**< vnic_entry class is friend of vnic_state */

private:
    ht              *vnic_ht_;                      /**< vnic database
                                                         NOTE: even though VNIC
                                                         scale is 1K, ids can be
                                                         in the range [0, 4095],
                                                         so to save memory,
                                                         instead of 4k index
                                                         table, we use hash
                                                         table */
    indexer         *vnic_idxr_;                    /**< indexer to allocate hw
                                                         vnic id */
    slab            *vnic_slab_;                    /**< slab for allocating
                                                         vnic state */

    /**< P4 datapath tables for vnic
     *   NOTE: there is no explicit table mgmt for rx and tx stats, we directly
     *         index using hw_id_ of vnic and and bzero out when we create vnic
     */
    directmap       *local_vnic_by_vlan_tx_tbl_;        /**< directmap table for LOCAL_VNIC_BY_VLAN_TX */
    sdk_hash        *local_vnic_by_slot_rx_tbl_;        /**< hash table for LOCAL_VNIC_BY_SLOT_RX */
    directmap       *egress_local_vnic_info_rx_tbl_;    /**< directmap table for EGRESS_LOCAL_VNIC_INFO_RX */
};

/** @} */    // end of OCI_VNIC_STATE

}    // namespace api

#endif    /** __VNIC_HPP__ */
