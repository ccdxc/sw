//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// LIF state handling
///
//----------------------------------------------------------------------------

#ifndef __API_LIF_STATE_HPP__
#define __API_LIF_STATE_HPP__

#include "nic/apollo/framework/state_base.hpp"

namespace api {

/// \defgroup PDS_LIF_STATE - LIF state functionality
/// \ingroup PDS_LIF
/// @{

/// \brief state maintained for LIFs
class lif_state : public state_base {
public:
    /// \brief constructor
    lif_state();

    /// \brief destructor
    ~lif_state();

    /// \brief     set lif_impl_state_ pointer
    /// \param[in] impl_state    pointer to lif_impl_state_ instance
    void impl_state_set(void *impl_state);

    /// \brief     find lif_impl object
    /// \param[in] key    internal lif key for the LIF object
    /// \return    pointer to the LIF instance found or NULL
    void *find(pds_lif_id_t *key) const;

    /// \brief     find lif_impl object
    /// \param[in] key    lif key for the LIF object
    /// \return    pointer to the LIF instance found or NULL
    void *find(pds_obj_key_t *key) const;

    /// \brief     walk lif_impl object
    /// \param[in] walk_cb    callback function
    /// \param[in] ctxt       context for callback function
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt);

private:
    void *lif_impl_state_;
};

/// \@}

}    // namespace api

using api::lif_state;

#endif    // __API_LIF_STATE_HPP__
