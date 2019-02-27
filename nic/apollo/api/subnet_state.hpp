//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet state handling
///
//----------------------------------------------------------------------------

#if !defined(__API_SUBNET_STATE_HPP__)
#define __API_SUBNET_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/subnet.hpp"

namespace api {

/// \defgroup PDS_SUBNET_STATE - subnet state functionality
/// \ingroup PDS_SUBNET
/// @{

/// \brief    state maintained for subnets
class subnet_state : public obj_base {
public:
    /// \brief constructor
    subnet_state();

    /// \brief destructor
    ~subnet_state();

    /// \brief      allocate memory required for a subnet
    /// \return     pointer to the allocated subnet, NULL if no memory
    subnet_entry *subnet_alloc(void);

    /// \brief      free subnet instance back to slab
    /// \param[in]  subnet   pointer to the allocated subnet
    void subnet_free(subnet_entry *subnet);

    /// \brief      lookup a subnet in database given the key
    /// \param[in]  subnet_key subnet key
    /// \return     pointer to the subnet instance found or NULL
    subnet_entry *subnet_find(pds_subnet_key_t *subnet_key) const;
    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *subnet_ht(void) { return subnet_ht_; }
    indexer *subnet_idxr(void) { return subnet_idxr_; }
    slab *subnet_slab(void) { return subnet_slab_; }
    friend class subnet_entry;    ///< subnet_entry class is friend of
                                  ///< subnet_state

private:
    ht *subnet_ht_;               ///< Hash table root
    indexer *subnet_idxr_;        ///< Indexer to allocate hw subnet id
    slab *subnet_slab_;           ///< slab for allocating subnet entry
};

/// \@}    // end of PDS_SUBNET_ENTRY

}    // namespace api

using api::subnet_state;

#endif    // __API_SUBNET_STATE_HPP__
