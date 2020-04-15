//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __RDMAMGR_IRIS_HPP__
#define __RDMAMGR_IRIS_HPP__

#include <memory>
#include "nic/sdk/platform/rdmamgr/rdmamgr.hpp"
#include "nic/sdk/lib/bm_allocator/bm_allocator.hpp"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "nic/sdk/lib/pal/pal.hpp"

#define READ_MEM        sdk::lib::pal_mem_read
#define WRITE_MEM       sdk::lib::pal_mem_write

namespace iris {

class rdmamgr_iris : public rdmamgr {
private:
    mpartition *mp_;
    lif_mgr *lm_;
    uint64_t rdma_hbm_base_;
    std::unique_ptr<sdk::lib::BMAllocator> rdma_hbm_allocator_;
    std::map<uint64_t, uint64_t> rdma_allocation_sizes_;

private:
    sdk_ret_t init_(mpartition *mp, lif_mgr *lm);
    uint32_t roundup_to_pow_2_(uint32_t x);
    uint64_t rdma_mem_alloc_(uint32_t size);
    sdk_ret_t p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add_(
                                                      uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint32_t ah_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint8_t log_num_kt_entries,
                                                      uint8_t log_num_dcqcn_profiles,
                                                      uint8_t log_num_ah_entries,
                                                      uint32_t cqcb_base_addr_hi,
                                                      uint32_t sqcb_base_addr_hi,
                                                      uint32_t rqcb_base_addr_hi,
                                                      uint8_t log_num_cq_entries,
                                                      uint8_t log_num_sq_entries,
                                                      uint8_t log_num_rq_entries,
                                                      uint32_t prefetch_base_addr_page_id,
                                                      uint8_t log_prefetch_buf_size,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype,
                                                      uint8_t aq_qtype,
                                                      uint64_t barmap_base_addr,
                                                      uint32_t barmap_size,
                                                      uint8_t log_num_eq_entries);
    sdk_ret_t p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add_(
                                                      uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint8_t log_num_kt_entries,
                                                      uint8_t log_num_dcqcn_profiles,
                                                      uint8_t log_num_ah_entries,
                                                      uint32_t cqcb_base_addr_hi,
                                                      uint32_t sqcb_base_addr_hi,
                                                      uint32_t rqcb_base_addr_hi,
                                                      uint8_t log_num_cq_entries,
                                                      uint8_t log_num_sq_entries,
                                                      uint8_t log_num_rq_entries,
                                                      uint32_t prefetch_base_addr_page_id,
                                                      uint8_t log_prefetch_buf_size,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype,
                                                      uint8_t aq_qtype);
    sdk_ret_t p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get_(
                                                      uint32_t idx,
                                                      rx_stage0_load_rdma_params_actiondata_t *data);
public:
    static rdmamgr_iris *factory(mpartition *mp, lif_mgr *lm_);
    static void destroy(rdmamgr *rmgr);

    sdk_ret_t lif_init(uint32_t lif, uint32_t max_keys,
                       uint32_t max_ahs, uint32_t max_ptes,
                       uint64_t mem_bar_addr, uint32_t mem_bar_size,
                       uint32_t max_prefetch_wqes);

    uint64_t rdma_get_pt_base_addr (uint32_t lif);
    uint64_t rdma_get_kt_base_addr (uint32_t lif);

protected:
    rdmamgr_iris() {}
    ~rdmamgr_iris() {}

};

}    // namespace iris

using iris::rdmamgr_iris;

#endif    // __RDMAMGR_IRIS_HPP__
