//---------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// Metaswitch internal MIB index management helper
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/mgmt/pds_ms_mib_idx_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"

namespace pds_ms {

// ----------------------------------------
//  MIB Index Guard class implementation
// -----------------------------------------  
void mib_idx_gen_guard_t::alloc() {
    // Allocate index
    // Enter thread-safe context to access/modify global state
    auto mgmt_ctxt = pds_ms::mgmt_state_t::thread_context();
    auto rs = mgmt_ctxt.state()->mib_indexer().alloc(tbl_type_, &mib_idx_);
    if (rs != SDK_RET_OK) {
        throw Error(std::string("Allocating internal MIB index failed for table ")
                                .append(std::to_string(tbl_type_))
                                .append(" err ").append(std::to_string(rs)));
    }
}

mib_idx_gen_guard_t::~mib_idx_gen_guard_t() {
    if(mib_idx_ == 0) return;
    // Free index
    // Enter thread-safe context to access/modify global state
    auto mgmt_ctxt = pds_ms::mgmt_state_t::thread_context();
    auto rs = mgmt_ctxt.state()->mib_indexer().free(tbl_type_, mib_idx_);
    if (rs != SDK_RET_OK) {
        PDS_TRACE_ERR("MIB Table %d Index %d free failed with err %d",
                      tbl_type_, mib_idx_, rs);
        return;
    }
    PDS_TRACE_VERBOSE("Freed MIB Table %d Index %d", tbl_type_, mib_idx_);
}

// ----------------------------------------
//  MIB Indexer Table class implementation
// -----------------------------------------  
mib_idx_gen_indexer_t::mib_idx_gen_indexer_t(void) {

    mib_indexers_[MIB_IDX_GEN_TBL_VRF]
        = sdk::lib::rte_indexer::factory(0xFFFF-1, /* 16 bit index */
                                         /* skip zero */
                                         true, true);

    if (mib_indexers_[MIB_IDX_GEN_TBL_VRF] == nullptr) {
        throw Error("VRF MIB Indexer allocation failed");
    }
    // Pre-alloc index 1 for default VRF 
    mib_idx_t idx;
    mib_indexers_[MIB_IDX_GEN_TBL_VRF]->alloc(&idx);
    SDK_ASSERT(idx == PDS_MS_RTM_DEF_ENT_INDEX);

    mib_indexers_[MIB_IDX_GEN_TBL_BD]
        = sdk::lib::rte_indexer::factory(0xFFFF-1, /* 16 bit index */
                                         /* skip zero */
                                         true, true);
    if (mib_indexers_[MIB_IDX_GEN_TBL_BD] == nullptr) {
        throw Error("BD MIB Indexer allocation failed");
    }
}

mib_idx_gen_indexer_t::~mib_idx_gen_indexer_t(void) {
    for (auto tbl = MIB_IDX_GEN_TBL_START+1;
         tbl < MIB_IDX_GEN_TBL_MAX; ++tbl) {
        sdk::lib::rte_indexer::destroy(mib_indexers_[tbl]);
    }
}

} // End namespace



