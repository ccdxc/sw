/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep.hpp
 *
 * @brief   This file deals with OCI TEP functionality required internally by
 *          other modules
 */

#if !defined (__TEP_HPP__)
#define __TEP_HPP__

#include "nic/sdk/include/sdk/slab.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/include/api/oci_tep.hpp"

using sdk::lib::ht_ctxt_t;

namespace api {

/**
 * @defgroup OCI_TEP_ENTRY - tep functionality
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    tep entry
 */
class tep_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize a tep entry
     * @param[in] oci_tep    tep information
     * @return    new instance of tep or NULL, in case of error
     */
    static tep_entry *factory(oci_tep_t *oci_tep);

    /**
     * @brief    release all the s/w state associate with the given tep, if any,
     *           and free the memory
     * @param[in] tep     tep to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(tep_entry *tep);

    /**
     * @brief     initialize tep entry with the given config
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
     * @brief     add given tep to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete given tep from the database
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
     * @brief     helper function to get key given tep entry
     * @param[in] entry    pointer to tep instance
     * @return    pointer to the tep instance's key
     */
    static void *tep_key_func_get(void *entry) {
        tep_entry *tep = (tep_entry *)entry;
        return (void *)&(tep->key_);
    }

    /**
     * @brief     helper function to compute hash value for given tep id
     * @param[in] key        tep's key
     * @param[in] ht_size    hash table size
     * @return    hash value
     */
    static uint32_t tep_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_tep_key_t)) % ht_size;
    }

    /**
     * @brief     helper function to compare two tep keys
     * @param[in] key1        pointer to tep's key
     * @param[in] key2        pointer to tep's key
     * @return    0 if keys are same or else non-zero value
     */
    static bool tep_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_tep_key_t))) {
            return true;
        }
        return false;
    }

    // TODO: this is impl API
    uint16_t hwid(void) { return hw_id_; }

private:
    /**< @brief    constructor */
    tep_entry();

    /**< @brief    destructor */
    ~tep_entry();

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
    /**
     * NOTE: the mac address for this TEP will be learnt in the datapath from
     *       ARP traffic in the substrate, but we don't need to store it in s/w,
     *       we will directly refresh the TEP_TX table
     */
    oci_tep_key_t    key_;        /**< tep key */
    ht_ctxt_t        ht_ctxt_;    /**< hash table context */

    /**< P4 datapath specific state */
    uint16_t         hw_id_;      /**< hardware id for this tep */
} __PACK__;

/** @} */    // end of OCI_TEP_ENTRY

/**
 * @defgroup OCI_TEP_STATE - tep state functionality
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    state maintained for teps
 */
class tep_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    tep_state();

    /**
     * @brief    destructor
     */
    ~tep_state();

    /**
     * @brief    allocate memory required for a tep
     * @return pointer to the allocated tep, NULL if no memory
     */
    tep_entry *tep_alloc(void);

    /**
     * @brief      free tep instance back to slab
     * @param[in]  tep   pointer to the allocated tep
     */
    void tep_free(tep_entry *tep);

    /**
     * @brief     lookup a tep in database given the key
     * @param[in] tep_key tep key
     */
    tep_entry *tep_find(oci_tep_key_t *tep_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *tep_ht(void) { return tep_ht_; }
    directmap *tep_tx_tbl(void) { return tep_tx_tbl_; }
    indexer *tep_idxr(void) { return tep_idxr_; }
    slab *tep_slab(void) { return tep_slab_; }
    friend class tep_entry;   /**< tep_entry class is friend of tep_state */

private:
    ht           *tep_ht_;      /**< Hash table root */
    slab         *tep_slab_;    /**< slab for allocating tep entry */
    indexer      *tep_idxr_;    /**< indexer to allocate hw ids for TEPs */
    directmap    *tep_tx_tbl_;  /**< directmap table for TEP_TX */
};

}  /** end namespace api */

#endif    /** __TEP_HPP__ */
