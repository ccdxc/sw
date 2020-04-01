//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mapping database maintenance
///
//----------------------------------------------------------------------------

#ifndef __MAPPING_STATE_HPP__
#define __MAPPING_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/kvstore/kvstore.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/mapping.hpp"

namespace api {

typedef void (*mapping_state_cb_t)(mapping_entry *entry, void *ctxt);

/// \defgroup PDS_MAPPING_STATE - mapping state handling
/// \ingroup PDS_MAPPING
/// @{

/// \brief    state maintained for MAPPINGs
class mapping_state : public state_base {
public:
    /// \brief    constructor
    /// \param[in] kvs pointer to key-value store instance
    mapping_state(sdk::lib::kvstore *kvs);

    /// \brief    destructor
    ~mapping_state();

    /// \brief    allocate memory required for a mapping
    /// \return pointer to the allocated mapping, NULL if no memory
    mapping_entry *alloc(void);

    /// \brief     insert given mapping instance into the mapping db
    /// \param[in] mapping mapping entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(mapping_entry *mapping);

    /// \brief     remove the given instance of mapping object from db
    /// \param[in] mapping mapping entry to be deleted from the db
    /// \return    pointer to the removed mapping instance or NULL, if not found
    mapping_entry *remove(mapping_entry *mapping);

    /// \brief      free mapping instance back to slab
    /// \param[in]  mapping   pointer to the allocated mapping
    void free(mapping_entry *mapping);

    /// \brief     lookup a mapping in database given the key
    /// \param[in] key key for the mapping object
    /// \return    pointer to the mapping instance found or NULL
    mapping_entry *find(pds_obj_key_t *key) const;

    /// \brief     lookup a mapping in database given the 2nd-ary key of mapping
    /// \param[in] skey    2nd-ary key of the mapping object
    /// \return    pointer to the mapping instance found or NULL
    mapping_entry *find(pds_mapping_key_t *skey) const;

    /// \brief     iterate through mapping entries persisted in kvstore
    /// \param[in] cb      callback called on each entry
    /// \param[in] ctxt    opaque context passed to callback
    void kvstore_iterate(mapping_state_cb_t cb, void *ctxt);

    /// \brief      given the primary key, return the 2nd-ary key of mapping
    /// \param[in]  key primary key of the mapping object
    /// \param[out] 2nd-ary key of the mapping object, if found
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t skey(_In_ pds_obj_key_t *key,
                   _Out_ pds_mapping_key_t *skey) const;

    /// \brief      persist given mapping entry (portions of it that need to be)
    /// \param[in]  mapping    instance of the mapping entry
    /// \param[in]  spec       mapping configuration
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t persist(mapping_entry *mapping, pds_mapping_spec_t *spec);

    /// \brief      destroy any persisted state of the given mapping entry
    /// \param[in] key    key of mapping to be removed from persistent db
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t perish(const pds_obj_key_t& key);

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

    /// \brief API to begin database/remote transactions, if any
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t transaction_begin(void) {
        //return kvstore_->txn_start(sdk::lib::kvstore::TXN_TYPE_READ_WRITE);
        return SDK_RET_OK;
    }

    /// \brief API to end database/remote transactions, if any
    /// \param[in] abort    true if transaction needs to be aborted, else false
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t transaction_end(bool abort) {
        //if (abort) {
            //return kvstore_->txn_abort();
        //}
        //return kvstore_->txn_commit();
        return SDK_RET_OK;
    }

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *mapping_ht(void) const { return mapping_ht_; }
    ht *mapping_skey_ht(void) const { return mapping_skey_ht_; }
    slab *mapping_slab(void) const { return mapping_slab_; }

private:
    ht *mapping_ht_;                ///< mapping databse based on primary key
    ht *mapping_skey_ht_;           ///< mapping database based on 2nd-ary key
    slab *mapping_slab_;            ///< slab for allocating mapping entry
    sdk::lib::kvstore *kvstore_;    ///< key-value store instance
};

static inline mapping_entry *
mapping_find (pds_mapping_key_t *skey)
{
    return (mapping_entry *)api_base::find_obj(OBJ_ID_MAPPING, skey);
}

/// \@}

}    // namespace api

using api::mapping_state;

#endif    // __MAPPING_STATE_HPP__
