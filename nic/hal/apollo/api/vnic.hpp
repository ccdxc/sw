/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.hpp
 *
 * @brief   This file deals with OCI VNIC functionality
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
    /**
     * @brief    constructor
     */
    vnic_entry() {}

    /**
     * @brief    destructor
     */
    ~vnic_entry() {}

    /**
     * @brief     initialize vnic entry with the given config
     * @param[in] oci_vnic    vnic information
     * @return    SDK_RET_OK on success, failure status code on error
     *
     * NOTE:     allocate all h/w resources (i.e., table indices as well here,
     *           we can always release them in abort phase if something goes
     *           wrong
     */
    sdk_ret_t init(oci_vnic_t *oci_vnic);

    /**
     * @brief     add given vnic to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t add_to_db(void);

    /**
     * @brief     delete given vnic from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t del_from_db(void);

private:
    oci_vnic_key_t    key_;        /**< vnic key */
    ht_ctxt_t         ht_ctxt_;    /**< hash table context */

    /**< P4 datapath specific state */
    uint32_t          hw_id_;      /**< hardware id */
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
class vnic_state {
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
    directmap       *local_vnic_by_vlan_tx_;        /**< indexer for LOCAL_VNIC_BY_VLAN_TX table */
    sdk_hash        *local_vnic_by_slot_rx_;        /**< hash table for LOCAL_VNIC_BY_SLOT_RX table */
    directmap       *egress_local_vnic_info_rx_;    /**< indexer for EGRESS_LOCAL_VNIC_INFO_RX table */
};

/** @} */    // end of OCI_VNIC_STATE

}    // namespace api

#endif    /** __VNIC_HPP__ */
