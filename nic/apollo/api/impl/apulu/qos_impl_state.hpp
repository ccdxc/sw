//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// qos implementation state
///
//----------------------------------------------------------------------------

#ifndef __QOS_IMPL_STATE_HPP__
#define __QOS_IMPL_STATEHPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_QOS_IMPL_STATE interface state functionality
/// \ingroup PDS_QOS
/// @{

/// \brief  state maintained for interfaces
class qos_impl_state : public state_base {
public:
    /// \brief  constructor
    qos_impl_state(pds_state *state);

    /// \brief  destructor
    ~qos_impl_state();

    rte_indexer *qos_txdma_iq_idxr(void) { return qos_txdma_iq_idxr_; }
    rte_indexer *qos_uplink_iq_idxr(void) { return qos_uplink_iq_idxr_; }
    rte_indexer *qos_common_oq_idxr(void) { return qos_common_oq_idxr_; }
    rte_indexer *qos_rxdma_oq_idxr(void) { return qos_rxdma_oq_idxr_; }

private:
    ///< indexer for iqs and oqs
    rte_indexer *qos_txdma_iq_idxr_;
    rte_indexer *qos_uplink_iq_idxr_;
    rte_indexer *qos_common_oq_idxr_;
    rte_indexer *qos_rxdma_oq_idxr_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __QOS_IMPL_STATE_HPP__
