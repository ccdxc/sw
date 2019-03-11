//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vcn state handling
///
//----------------------------------------------------------------------------

#if !defined(__API_VCN_STATE_HPP__)
#define __API_VCN_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/vcn.hpp"

namespace api {

/// \defgroup PDS_VCN_STATE - vcn state functionality
/// \ingroup PDS_VCN
/// @{

/// \brief    state maintained for vcns
class vcn_state : public obj_base {
public:
    /// \brief constructor
    vcn_state();

    /// \brief destructor
    ~vcn_state();

    /// \brief      allocate memory required for a vcn instance
    /// \return     pointer to the allocated vcn, NULL if no memory
    vcn_entry *alloc(void);

    /// \brief    insert given vcn instance into the vcn db
    /// \param[in] vcn    vcn entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(vcn_entry *vcn);

    /// \brief     remove the given instance of vcn object from db
    /// \param[in] vcn    vcn entry to be deleted from the db
    /// \return    pointer to the removed vcn instance or NULL, if not found
    vcn_entry *remove(vcn_entry *vcn);

    /// \brief      free vcn instance back to slab
    /// \param[in]  vcn   pointer to the allocated vcn
    void free(vcn_entry *vcn);

    /// \brief      lookup a vcn in database given the key
    /// \param[in]  vcn  key for the VCN object
    /// \return     pointer to the vcn instance found or NULL
    vcn_entry *find(pds_vcn_key_t *key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *vcn_ht(void) { return vcn_ht_; }
    indexer *vcn_idxr(void) { return vcn_idxr_; }
    slab *vcn_slab(void) { return vcn_slab_; }
    friend class vcn_entry;    ///< vcn_entry class is friend of vcn_state

private:
    ht *vcn_ht_;           ///< hash table root
    indexer *vcn_idxr_;    ///< indexer to allocate hw vcn id
    slab *vcn_slab_;       ///< slab for allocating vcn entry
};

/// \@}    // end of PDS_VCN_ENTRY

}    // namespace api

using api::vcn_state;

#endif    // __API_VCN_STATE_HPP__
