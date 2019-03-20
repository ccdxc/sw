//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif datapath implementation
///
//----------------------------------------------------------------------------

#ifndef __LIF_IMPL_HPP__
#define __LIF_IMPL_HPP__

#include "nic/sdk/include/sdk/qos.hpp"
#include "nic/apollo/framework/impl_base.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL - vnic entry datapath implementation
/// \ingroup PDS_VNIC
/// \@{
/// \brief LIF implementation

class lif_impl : public impl_base {
public:
    ///< \brief    program lif tx policer for given lif 
    ///< param[in] lif_id     h/w lif id
    ///< param[in] policer    policer parameters
    static sdk_ret_t program_tx_policer(uint32_t lif_id,
                                        sdk::policer_t *policer);
} __PACK__;

/// \@}

}    // namespace impl
}    // namespace api

#endif    /** __LIF_IMPL_HPP__ */
