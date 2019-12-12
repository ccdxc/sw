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
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/subnet.hpp"

namespace api {

/// \defgroup PDS_SUBNET_STATE - subnet state functionality
/// \ingroup PDS_SUBNET
/// @{

/// \brief    state maintained for subnets
class subnet_state : public state_base {
public:
    /// \brief constructor
    subnet_state();

    /// \brief destructor
    ~subnet_state();

    /// \brief      allocate memory required for a subnet
    /// \return     pointer to the allocated subnet, NULL if no memory
    subnet_entry *alloc(void);

    /// \brief    insert given subnet instance into the subnet db
    /// \param[in] subnet    subnet entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(subnet_entry *subnet);

    /// \brief     remove the (singleton) instance of device object from db
    /// \param[in] subnet    subnet entry to be deleted from the db
    /// \return    pointer to the removed subnet instance or NULL, if not found
    subnet_entry *remove(subnet_entry *subnet);

    /// \brief      free subnet instance back to slab
    /// \param[in]  subnet   pointer to the allocated subnet
    void free(subnet_entry *subnet);

    /// \brief      lookup a subnet in database given the key
    /// \param[in]  subnet_key subnet key
    /// \return     pointer to the subnet instance found or NULL
    subnet_entry *find(pds_subnet_key_t *subnet_key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *subnet_ht(void) const { return subnet_ht_; }
    indexer *subnet_idxr(void) const { return subnet_idxr_; }
    slab *subnet_slab(void) const { return subnet_slab_; }
    friend class subnet_entry;    ///< subnet_entry class is friend of
                                  ///< subnet_state

private:
    ht *subnet_ht_;               ///< Hash table root
    indexer *subnet_idxr_;        ///< Indexer to allocate hw subnet id
    slab *subnet_slab_;           ///< slab for allocating subnet entry
};

static inline subnet_entry *
subnet_find (pds_subnet_key_t *key) {
    return (subnet_entry *)api_base::find_obj(OBJ_ID_SUBNET, key);
}

/// \@}    // end of PDS_SUBNET_ENTRY

}    // namespace api

using api::subnet_state;

#endif    // __API_SUBNET_STATE_HPP__
