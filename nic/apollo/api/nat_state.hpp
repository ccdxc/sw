//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// NAT state handling
///
//----------------------------------------------------------------------------

#ifndef __API_NAT_STATE_HPP__
#define __API_NAT_STATE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/nat.hpp"

namespace api {

/// \defgroup PDS_NAT_STATE - NAT state functionality
/// \ingroup PDS_NAT
/// @{

/// \brief    state maintained for NAT policies
class nat_state : public state_base {
public:
    /// \brief constructor
    nat_state();

    /// \brief destructor
    ~nat_state();

    /// \brief      allocate memory required for a NAT port block
    /// \return     pointer to the allocated NAT port block, NULL if no memory
    nat_port_block *alloc(void);

    /// \brief    insert given NAT port block instance into the db
    /// \param[in] port_block    NAT port block to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(nat_port_block *port_block);

    /// \brief     remove the NAT port block object from db
    /// \param[in] port_block    NAT port block to be deleted from the db
    /// \return    pointer to the removed NAT port block or NULL, if not found
    nat_port_block *remove(nat_port_block *port_block);

    /// \brief      free NAT port block back to slab
    /// \param[in]  port_block    pointer to the allocated NAT port block
    void free(nat_port_block *port_block);

    /// \brief      lookup a NAT port block in database given the key
    /// \param[in]  key NAT port block key
    /// \return     pointer to the NAT port block found or NULL
    nat_port_block *find(pds_nat_port_block_key_t *key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *nat_port_block_ht(void) const { return nat_port_block_ht_; }
    slab *nat_port_block_slab(void) const { return nat_port_block_slab_; }
    friend class nat_port_block;    ///< nat_port_block class is friend of
                                    ///< nat_state

private:
    ht *nat_port_block_ht_;        ///< NAT port block hash table root
    slab *nat_port_block_slab_;    ///< slab for allocating NAT port block
};

static inline nat_port_block *
nat_port_block_find (pds_nat_port_block_key_t *key) {
    return (nat_port_block *)api_base::find_obj(OBJ_ID_NAT_PORT_BLOCK, key);
}

/// \@}    // end of PDS_NAT

}    // namespace api

using api::nat_state;

#endif    // __API_NAT_STATE_HPP__
