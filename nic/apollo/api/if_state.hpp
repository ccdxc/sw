//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface state handling
///
//----------------------------------------------------------------------------

#ifndef __API_IF_STATE_HPP__
#define __API_IF_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/if.hpp"

namespace api {

/// \defgroup PDS_IF_STATE - if state functionality
/// \ingroup PDS_IF
/// @{

/// \brief    state maintained for interface
class if_state : public state_base {
public:
    /// \brief constructor
    if_state();

    /// \brief destructor
    ~if_state();

    /// \brief      allocate memory required for an interface instance
    /// \return     pointer to the allocated interface, NULL if no memory
    if_entry *alloc(void);

    /// \brief    insert given interface instance into the interface db
    /// \param[in] intf    interface entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(if_entry *intf);

    /// \brief     remove the given instance of interface object from db
    /// \param[in] intf interface entry to be deleted from the db
    /// \return    pointer to the removed interface instance or NULL,
    ///            if not found
    if_entry *remove(if_entry *intf);

    /// \brief      free interface instance back to slab
    /// \param[in]  intf    pointer to the allocated interface
    void free(if_entry *intf);

    /// \brief      lookup a interface in database given the ifindex
    /// \param[in]  key  ifindex for the interface object
    /// \return     pointer to the interface instance found or NULL
    if_entry *find(pds_ifindex_t *key) const;

    /// \brief      lookup a interface in database given the key
    /// \param[in]  key  key for the interface object
    /// \return     pointer to the interface instance found or NULL
    if_entry *find(pds_if_key_t *key) const;

    /// \brief      walk the interface database and invoke the callback per
    ///             interface
    /// \param[in]  if_type    type of the interface
    /// \param[in]  walk_cb    callback invoked per interface
    /// \param[in]  ctxt       opaque context passed back to the callback
    /// \return     pointer to the interface instance found or NULL
    sdk_ret_t walk(uint32_t if_type, sdk::lib::ht::ht_walk_cb_t walk_cb,
                   void *ctxt);

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

private:
    typedef struct if_walk_ctxt_s {
        uint32_t if_type;
        sdk::lib::ht::ht_walk_cb_t *walk_cb;
        void *ctxt;
    } if_walk_ctxt_t;

    ht *if_ht(void) const { return if_ht_; }
    ht *ifindex_ht(void) const { return ifindex_ht_; }
    slab *if_slab(void) const { return if_slab_; }
    static bool walk_cb_(void *entry, void *ctxt);

    friend class if_entry;

private:
    ht *ifindex_ht_;    ///< interface hash table based on ifindex
    ht *if_ht_;         ///< interface hash table based on the key
    slab *if_slab_;     ///< slab for allocating interface entry
};

static inline if_entry *
if_find (void)
{
    return (if_entry *)api_base::find_obj(OBJ_ID_IF, NULL);
}

/// \@}

}    // namespace api

using api::if_state;

#endif    // __API_IF_STATE_HPP__
