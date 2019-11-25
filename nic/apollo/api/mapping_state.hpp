/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    mapping_state.hpp
 *
 * @brief   mapping database maintenance
 */

#if !defined (__MAPPING_STATE_HPP__)
#define __MAPPING_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/mapping.hpp"

namespace api {

/**
 * @defgroup PDS_MAPPING_STATE - mapping state handling
 * @ingroup PDS_MAPPING
 * @{
 */

/**
 * @brief    state maintained for MAPPINGs
 */
class mapping_state : public state_base {
public:
    /**
     * @brief    constructor
     */
    mapping_state();

    /**
     * @brief    destructor
     */
    ~mapping_state();

    /**
     * @brief    allocate memory required for a mapping
     * @return pointer to the allocated mapping, NULL if no memory
     */
    mapping_entry *alloc(void);

    /// \brief     insert given mapping instance into the mapping db
    /// \param[in] mapping mapping entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(mapping_entry *mapping);

    /// \brief     remove the given instance of mapping object from db
    /// \param[in] mapping mapping entry to be deleted from the db
    /// \return    pointer to the removed mapping instance or NULL, if not found
    mapping_entry *remove(mapping_entry *mapping);

    /**
     * @brief      free mapping instance back to slab
     * @param[in]  mapping   pointer to the allocated mapping
     */
    void free(mapping_entry *mapping);

    /// \brief     lookup a mapping in database given the key
    /// \param[in] key key for the mapping object
    /// \return    pointer to the mapping instance found or NULL
    mapping_entry *find(pds_mapping_key_t *key) const;

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
    ht *mapping_ht(void) const { return mapping_ht_; }
    slab *mapping_slab(void) const { return mapping_slab_; }

private:
    ht *mapping_ht_;        /**< mapping databse */
    slab *mapping_slab_;    /**< slab for allocating mapping entry */
};

/** @} */    // end of PDS_MAPPING_STATE

}    // namespace api

using api::mapping_state;

#endif    /** __MAPPING_STATE_HPP__ */
