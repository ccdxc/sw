//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif implementation state
///
//----------------------------------------------------------------------------

#ifndef __LIF_IMPL_STATE_HPP__
#define __LIF_IMPL_STATE_HPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_LIF_IMPL_STATE - lif state functionality
/// \ingroup PDS_LIF
/// \@{

// forward declarion
class lif_impl;

///< \brief    state maintained for lifs
class lif_impl_state : public state_base {
public:
    ///< \brief    constructor
    lif_impl_state(pds_state *state);

    ///< \@brief destructor
    ~lif_impl_state();

    /// \brief    allocate memory required for a lif instance
    /// \return pointer to the allocated lif, NULL if no memory
    lif_impl *alloc(void);

    /// \brief    insert given lif instance into the lif db
    /// \param[in] impl    lif to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(lif_impl *impl);

    /// \brief     remove the given instance of lif object from db
    /// \param[in] impl    lif entry to be deleted from the db
    /// \return    pointer to the removed lif instance or NULL,
    ///            if not found
    lif_impl *remove(lif_impl *impl);

    /// \brief      free lif impl instance
    /// \param[in]  impl   pointer to the allocated lif impl instance
    void free(lif_impl *impl);

    /// \brief     lookup a lif in database given the key
    /// \param[in] key    lif key
    lif_impl *find(pds_lif_key_t *key) const;

private:
    directmap *tx_rate_limiter_tbl(void) { return tx_rate_limiter_tbl_; }
    friend class lif_impl;    // lif_impl class is friend of lif_impl_state

private:
    ht           *lif_ht_;
    directmap    *tx_rate_limiter_tbl_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __LIF_IMPL_STATE_HPP__
