//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// meter database handling
///
//----------------------------------------------------------------------------

#ifndef __METER_STATE_HPP__
#define __METER_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/meter.hpp"

namespace api {

/// \defgroup PDS_METER__STATE - meter state/db functionality
/// \ingroup PDS_METER
/// \@{

/// \brief    state maintained for meter entries
class meter_state : public state_base {
public:
    /// \brief constructor
    meter_state();

    /// \brief destructor
    ~meter_state();

    /// \brief  allocate memory required for a meter entry
    /// \return pointer to the allocated meter entry, NULL if no memory
    meter_entry *alloc(void);

    /// \brief    insert given meter entry into the meter db
    /// \param[in] meter     meter entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(meter_entry *meter);

    /// \brief     remove the given meter entry from db
    /// \param[in] meter    meter entry to be deleted from the db
    /// \return    pointer to the removed meter entry or NULL, if not found
    meter_entry *remove(meter_entry *meter);

    /// \brief    remove current object from the databse(s) and swap it with the
    ///           new instance of the obj (with same key)
    /// \param[in] curr_meter    current instance of the meter entry
    /// \param[in] new_meter     new instance of the meter entry
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(meter_entry *curr_meter, meter_entry *new_meter);

    /// \brief      free meter entry back to slab
    /// \param[in]  meter    pointer to the allocated meter entry
    void free(meter_entry *meter);

    /// \brief     lookup a meter entry in database given the key
    /// \param[in] key meter entry key
    meter_entry *find(pds_meter_key_t *key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *meter_ht(void) const { return meter_ht_; }
    slab *meter_slab(void) const { return meter_slab_; }
    friend class meter_entry;   // meter_entry is friend of meter_state

private:
    ht      *meter_ht_;      // meter database
    slab    *meter_slab_;    // slab to allocate meter entry
};

static inline meter_entry *
meter_find (void) {
    return (meter_entry *)api_base::find_obj(OBJ_ID_METER, NULL);
}

/// \@}    // end of PDS_METER_STATE

}    // namespace api

using api::meter_state;

#endif    // __METER_STATE_HPP__
