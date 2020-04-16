//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "nic/apollo/core/trace.hpp"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE - vpc database functionality
/// \ingroup PDS_VPC
/// @{

vpc_impl_state::vpc_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;
    sdk_table_factory_params_t tparams;

    p4pd_global_table_properties_get(P4TBL_ID_VPC, &tinfo);
    // create indexer for vpc hw id allocation and reserve 0th entry
    vpc_idxr_ = rte_indexer::factory(tinfo.tabledepth, false, true);
    SDK_ASSERT(vpc_idxr_ != NULL);

    // instantiate P4 tables for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.entry_trace_en = false;
    tparams.table_id = P4TBL_ID_VNI;
    vni_tbl_ = slhash::factory(&tparams);
    SDK_ASSERT(vni_tbl_ != NULL);

    // create ht for vpc id to key mapping
    impl_ht_ = ht::factory(PDS_MAX_VPC >> 2,
                           vpc_impl::key_get,
                           sizeof(uint16_t));
    SDK_ASSERT(impl_ht_ != NULL);
}

vpc_impl_state::~vpc_impl_state() {
    rte_indexer::destroy(vpc_idxr_);
    slhash::destroy(vni_tbl_);
    ht::destroy(impl_ht_);
}

vpc_impl *
vpc_impl_state::alloc(void) {
    return (vpc_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VPC_IMPL, sizeof(vpc_impl));
}

void
vpc_impl_state::free(vpc_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_VPC_IMPL, impl);
}

sdk_ret_t
vpc_impl_state::table_transaction_begin(void) {
    vni_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_transaction_end(void) {
    vni_tbl_->txn_end();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t tinfo;

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_global_table_properties_get(P4TBL_ID_VNI_OTCAM, &tinfo);
    stats.table_name = tinfo.tablename;
    vni_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    return SDK_RET_OK;
}

vpc_impl *
vpc_impl_state::find(uint16_t hw_id) {
    return (vpc_impl *)impl_ht_->lookup(&hw_id);
}

sdk_ret_t
vpc_impl_state::insert(uint16_t hw_id, vpc_impl *impl) {
    impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::update(uint16_t hw_id, vpc_impl *impl) {
    if (impl_ht_->remove(&hw_id)) {
        return impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
vpc_impl_state::remove(uint16_t hw_id) {
    vpc_impl *vpc = NULL;

    vpc = (vpc_impl *)impl_ht_->remove(&hw_id);
    if (!vpc) {
        PDS_TRACE_ERR("Failed to find vpc impl for hw id %u", hw_id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
