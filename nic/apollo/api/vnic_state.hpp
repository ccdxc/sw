/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    vnic_state.hpp
 *
 * @brief   vnic database handling
 */

#if !defined (__VNIC_STATE_HPP__)
#define __VNIC_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/vnic.hpp"

namespace api {

/**
 * @defgroup PDS_VNIC_STATE - vnic state functionality
 * @ingroup PDS_VNIC
 * @{
 */

/**
 * @brief    state maintained for VNICs
 */
class vnic_state : public state_base {
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
    vnic_entry *alloc(void);

    /// \brief    insert given vnic instance into the vnic db
    /// \param[in] vnic     vnic to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(vnic_entry *vnic);

    /// \brief     remove the given instance of vnic object from db
    /// \param[in] vnic    vnic entry to be deleted from the db
    /// \return    pointer to the removed vnic instance or NULL,
    ///            if not found
    vnic_entry *remove(vnic_entry *vnic);

    /// \brief    remove current object from the databse(s) and swap it with the
    ///           new instance of the obj (with same key)
    /// \param[in] curr_vnic     current instance of the vnic
    /// \param[in] new_vnic      new instance of the vnic
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(vnic_entry *curr_vnic, vnic_entry *new_vnic);

    /**
     * @brief      free vnic instance back to slab
     * @param[in]  vnic   pointer to the allocated vnic
     */
    void free(vnic_entry *vnic);

    /**
     * @brief     lookup a vnic in database given the key
     * @param[in] vnic_key vnic key
     */
    vnic_entry *find(pds_vnic_key_t *vnic_key) const;

    /// \brief API to walk all the db elements
    /// \param[in] walk_cb    callback to be invoked for every node
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *vnic_ht(void) const { return vnic_ht_; }
    slab *vnic_slab(void) const { return vnic_slab_; }
    friend class vnic_entry;   /**< vnic_entry class is friend of vnic_state */

private:
    ht              *vnic_ht_;      /**< vnic database
                                         NOTE: even though VNIC scale is 1K, ids
                                               can be in the range [0, 4095], so
                                               to save memory, instead of 4k
                                               index table, we use hash table */
    slab            *vnic_slab_;    /**< slab to allocate vnic entry */
};

/** @} */    // end of PDS_VNIC_STATE

}    // namespace api

using api::vnic_state;

#endif    /** __VNIC_STATE_HPP__ */
