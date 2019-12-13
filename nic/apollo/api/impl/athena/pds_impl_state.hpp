//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// captures all the state maintained in impl layer
///
//----------------------------------------------------------------------------

#ifndef __PDS_IMPL_STATE_HPP__
#define __PDS_IMPL_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/impl/athena/athena_impl_state.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"

namespace api {
namespace impl {

/// \brief slab types
enum {
    PDS_SLAB_ID_IMPL_MIN     = 8192,
};

/// \defgroup PDS_IMPL_STATE - internal state
/// \ingroup PDS_IMPL_STATE
/// @{

/// \brief state implementation at impl layer
class pds_impl_state {
public:
    sdk_ret_t init(pds_state *state);
    static void destroy(pds_impl_state *impl_state);
    pds_impl_state();
    ~pds_impl_state();
    athena_impl_state *athena_impl_db(void) const { return athena_impl_db_; }
    lif_impl_state *lif_impl_db(void) const { return lif_impl_db_; }

private:
    athena_impl_state          *athena_impl_db_;
    lif_impl_state             *lif_impl_db_;
};
extern pds_impl_state g_pds_impl_state;

static inline athena_impl_state *
athena_impl_db (void)
{
    return  g_pds_impl_state.athena_impl_db();
}

/// \@}

}    // namespace  impl
}    // namespace api

#endif    // __PDS_IMPL_STATE_HPP__
