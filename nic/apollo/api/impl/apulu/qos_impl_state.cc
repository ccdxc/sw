//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// qos implementation state maintenance
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/api/impl/apulu/qos_impl_state.hpp"
#include "nic/sdk/platform/capri/capri_qos.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

/// \defgroup PDS_QOS_IMPL_STATE - qos state functionality
/// \ingroup PDS_QOS
/// \@{

namespace api {
namespace impl {

qos_impl_state::qos_impl_state(pds_state *state) {
    // create indexers for iqs and oqs
    qos_txdma_iq_idxr_ = rte_indexer::factory(
                             sdk::asic::pd::asicpd_qos_max_txdma_iqs(), true,
                             false);
    SDK_ASSERT(qos_txdma_iq_idxr_ != NULL);

    qos_uplink_iq_idxr_ = rte_indexer::factory(
                              sdk::asic::pd::asicpd_qos_max_uplink_iqs(), true,
                              false);
    SDK_ASSERT(qos_uplink_iq_idxr_ != NULL);

    qos_common_oq_idxr_ = rte_indexer::factory(
                              sdk::asic::pd::asicpd_qos_max_common_oqs(), true,
                              false);
    SDK_ASSERT(qos_common_oq_idxr_ != NULL);

    qos_rxdma_oq_idxr_ = rte_indexer::factory(
                             sdk::asic::pd::asicpd_qos_max_rxdma_oqs(), true,
                             false);
    SDK_ASSERT(qos_rxdma_oq_idxr_ != NULL);
}

qos_impl_state::~qos_impl_state() {
    rte_indexer::destroy(qos_txdma_iq_idxr_);
    rte_indexer::destroy(qos_uplink_iq_idxr_);
    rte_indexer::destroy(qos_common_oq_idxr_);
    rte_indexer::destroy(qos_rxdma_oq_idxr_);
}

/// \@}

}    // namespace impl
}    // namespace api
