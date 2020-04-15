//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "rdmamgr_iris.hpp"
#include "rdmamgr_iris_mem.hpp"
#include "platform/src/lib/nicmgr/include/rdma_dev.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/p4/common/defines.h"
#include "common_rxdma_actions_p4pd_table.h"
#include "common_txdma_actions_p4pd.h"
#include "common_txdma_actions_p4pd_table.h"

namespace iris {

const static char *kRdmaHBMLabel = "rdma";
const static uint32_t kRdmaAllocUnit = 4096;

// Memory bar should be multiple of 8 MB
#define MEM_BARMAP_SIZE_SHIFT               (23)

rdmamgr_iris *
rdmamgr_iris::factory(mpartition *mp, lif_mgr *lm)
{
    sdk_ret_t ret      = SDK_RET_OK;
    void *mem          = NULL;
    rdmamgr_iris      *riris = NULL;

    mem = (rdmamgr_iris *)RDMAMGR_CALLOC(RDMAMGR_MEM_ALLOC_RDMA_MGR_IRIS,
                                         sizeof(rdmamgr_iris));
    if (mem) {
        riris = new (mem) rdmamgr_iris();
        ret = riris->init_(mp, lm);
        if (ret != SDK_RET_OK) {
            riris->~rdmamgr_iris();
            RDMAMGR_FREE(RDMAMGR_MEM_ALLOC_RDMA_MGR_IRIS, mem);
            riris = NULL;
        }
    }
    return riris;
}

void
rdmamgr_iris::destroy(rdmamgr *rmgr)
{
    NIC_LOG_DEBUG("Destroy rdmamgr_iris");
    rdmamgr_iris *rmgr_iris = dynamic_cast<rdmamgr_iris*>(rmgr);
    rmgr_iris->~rdmamgr_iris();
    RDMAMGR_FREE(RDMAMGR_MEM_ALLOC_RDMA_MGR_IRIS, rmgr_iris);
}


sdk_ret_t
rdmamgr_iris::init_(mpartition *mp, lif_mgr *lm)
{
    uint64_t hbm_addr = 0;
    uint32_t size = 0;
    uint32_t num_units = 0;

    mp_       = mp;
    lm_       = lm;
    hbm_addr  = mp_->start_addr(kRdmaHBMLabel);
    size      = mp_->size(kRdmaHBMLabel);
    num_units = size / kRdmaAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
        num_units--;
    }

    rdma_hbm_base_ = hbm_addr;
    rdma_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("rdma_hbm_base : {:#x}", rdma_hbm_base_);

    return SDK_RET_OK;
}

sdk_ret_t
rdmamgr_iris::lif_init(uint32_t lif, uint32_t max_keys,
                       uint32_t max_ahs, uint32_t max_ptes,
                       uint64_t mem_bar_addr, uint32_t mem_bar_size,
                       uint32_t max_prefetch_wqes)
{
    sdk_ret_t           ret = SDK_RET_OK;
    sram_lif_entry_t    sram_lif_entry;
    uint32_t            pt_size, key_table_size, ah_table_size;
    uint32_t            prefetch_cb_size = 0;
    uint32_t            prefetch_ring_size = 0;
    uint32_t            prefetch_buf_size = 0;
    uint32_t            total_size;
    uint64_t            base_addr;
    uint64_t            size;
    uint32_t            max_cqs, max_eqs, max_sqs, max_rqs;
    // TODO: num dcqcn profiles param from json
    uint32_t            num_dcqcn_profiles = 8;
    uint64_t            cq_base_addr; //address in HBM memory
    uint64_t            sq_base_addr; //address in HBM memory
    uint64_t            rq_base_addr; //address in HBM memory
    uint64_t            prefetch_cb_base_addr = 0; //address in HBM memory

    NIC_FUNC_DEBUG("lif-{}: RDMA lif init", lif);

    lif_qstate_t *qstate = lm_->get_lif_qstate(lif);
    if (qstate == nullptr) {
        NIC_FUNC_ERR("lif-{}: GetLIFQState failed", lif);
        return SDK_RET_ERR;
    }

    max_cqs  = qstate->type[Q_TYPE_RDMA_CQ].num_queues;
    max_sqs  = qstate->type[Q_TYPE_RDMA_SQ].num_queues;
    max_rqs  = qstate->type[Q_TYPE_RDMA_RQ].num_queues;
    max_eqs  = qstate->type[Q_TYPE_RDMA_EQ].num_queues;

    memset(&sram_lif_entry, 0, sizeof(sram_lif_entry_t));

    // Fill the CQ info in sram_lif_entry
    // cq_base_addr = lm_->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_CQ);
    cq_base_addr = lm_->get_lif_qstate_base_addr(lif, Q_TYPE_RDMA_CQ);
    NIC_FUNC_DEBUG("lif-{}: cq_base_addr: {:#x}, max_cqs: {} ",
                  lif, cq_base_addr, roundup_to_pow_2_(max_cqs));
    SDK_ASSERT((cq_base_addr & ((1 << SQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.cqcb_base_addr_hi = cq_base_addr >> CQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_cq_entries = log2(roundup_to_pow_2_(max_cqs));

    // Fill the SQ info in sram_lif_entry
    // sq_base_addr = lm_->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_SQ);
    sq_base_addr = lm_->get_lif_qstate_base_addr(lif, Q_TYPE_RDMA_SQ);
    NIC_FUNC_DEBUG("lif-{}: sq_base_addr: {:#x}",
                    lif, sq_base_addr);
    SDK_ASSERT((sq_base_addr & ((1 << SQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.sqcb_base_addr_hi = sq_base_addr >> SQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_sq_entries = log2(roundup_to_pow_2_(max_sqs));

    // Fill the RQ info in sram_lif_entry
    // rq_base_addr = lm_->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_RQ);
    rq_base_addr = lm_->get_lif_qstate_base_addr(lif, Q_TYPE_RDMA_RQ);
    NIC_FUNC_DEBUG("lif-{}: rq_base_addr: {:#x}",
                    lif, rq_base_addr);
    SDK_ASSERT((rq_base_addr & ((1 << RQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.rqcb_base_addr_hi = rq_base_addr >> RQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_rq_entries = log2(roundup_to_pow_2_(max_rqs));

    // Setup page table and key table entries
    max_ptes = roundup_to_pow_2_(max_ptes);

    pt_size = sizeof(uint64_t) * max_ptes;
    pt_size = HBM_PAGE_ALIGN(pt_size);

    max_keys = roundup_to_pow_2_(max_keys);

    key_table_size = sizeof(key_entry_t) * max_keys;
    key_table_size += sizeof(dcqcn_config_cb_t) * num_dcqcn_profiles;
    key_table_size = HBM_PAGE_ALIGN(key_table_size);

    if (max_prefetch_wqes > 0) {
        prefetch_cb_size = sizeof(rq_prefetch_cb_t); // 64 bytes for LIF level cb
        //adjust to page boundary
        if (prefetch_cb_size & (HBM_PAGE_SIZE - 1)) {
            prefetch_cb_size = ((prefetch_cb_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
        }

        prefetch_ring_size = sizeof(rq_prefetch_ring_t) * MAX_RQ_PREFETCH_ACTIVE_QPS;
        //adjust to page boundary
        if (prefetch_ring_size & (HBM_PAGE_SIZE - 1)) {
            prefetch_ring_size = ((prefetch_ring_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
        }

        // max_prefetch_wqes is the total number of minimum-size WQEs to preallocate in HBM
        prefetch_buf_size = RQ_PREFETCH_WQE_SIZE * max_prefetch_wqes;
        //adjust to page boundary
        if (prefetch_buf_size & (HBM_PAGE_SIZE - 1)) {
            prefetch_buf_size = ((prefetch_buf_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
        }

        /* asm code makes assumptions about the memory layout */
        SDK_ASSERT(prefetch_cb_size ==
                   RQ_PREFETCH_RING_PG_OFFSET << HBM_PAGE_SIZE_SHIFT);
        SDK_ASSERT(prefetch_cb_size + prefetch_ring_size ==
                   RQ_PREFETCH_BUF_PG_OFFSET << HBM_PAGE_SIZE_SHIFT);
    }

    max_ahs = roundup_to_pow_2_(max_ahs);

    ah_table_size = AT_ENTRY_SIZE_BYTES * max_ahs;
    ah_table_size = HBM_PAGE_ALIGN(ah_table_size);

    total_size = pt_size + key_table_size + prefetch_cb_size + prefetch_ring_size + prefetch_buf_size + ah_table_size + HBM_PAGE_SIZE;

    base_addr = rdma_mem_alloc_(total_size);

    NIC_FUNC_DEBUG("lif-{}: pt_size: {}, key_table_size: {}, "
                  "ah_table_size: {}, base_addr: {:#x}, "
                  "prefetch_cb_size: {}, prefetch_ring_size: {}, prefetch_buf_size: {}",
                  lif,
                  pt_size, key_table_size,
                  ah_table_size, base_addr,
                  prefetch_cb_size, prefetch_ring_size, prefetch_buf_size);

    size = base_addr;
    sram_lif_entry.pt_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
    size += pt_size + key_table_size;

    if (max_prefetch_wqes > 0) {
        // init LIF level prefetch CB
        prefetch_cb_base_addr = size;
        uint16_t p_index = MAX_RQ_PREFETCH_ACTIVE_QPS - 1;
        WRITE_MEM(prefetch_cb_base_addr, (uint8_t *)&p_index, sizeof(p_index), 0);
        size += prefetch_cb_size;

        // init prefetch ring
        uint64_t addr = size;
        for (int i = 0; i < p_index; i++) {
            WRITE_MEM(addr, (uint8_t *)&i, sizeof(uint16_t), 0);
            addr += sizeof(uint16_t);
        }
        size += prefetch_ring_size;

        // populate prefetch related fields
        sram_lif_entry.prefetch_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
        sram_lif_entry.log_prefetch_buf_size = log2(prefetch_buf_size / MAX_RQ_PREFETCH_ACTIVE_QPS);
        size += prefetch_buf_size;
    }

    sram_lif_entry.ah_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_pt_entries = log2(max_ptes);
    sram_lif_entry.log_num_kt_entries = log2(max_keys);
    sram_lif_entry.log_num_dcqcn_profiles = log2(num_dcqcn_profiles);
    sram_lif_entry.log_num_ah_entries = log2(max_ahs);
    size += ah_table_size;

    sram_lif_entry.rdma_en_qtype_mask =
        ((1 << Q_TYPE_RDMA_SQ) | (1 << Q_TYPE_RDMA_RQ) | (1 << Q_TYPE_RDMA_CQ) | (1 << Q_TYPE_RDMA_EQ) | (1 << Q_TYPE_ADMINQ));
    sram_lif_entry.sq_qtype = Q_TYPE_RDMA_SQ;
    sram_lif_entry.rq_qtype = Q_TYPE_RDMA_RQ;
    sram_lif_entry.aq_qtype = Q_TYPE_ADMINQ;

    sram_lif_entry.log_num_eq_entries = log2(max_eqs);

    NIC_FUNC_DEBUG("lif-{}: pt_base_addr_page_id: {}, log_num_pt: {}, "
                   "log_num_kt: {}, log_num_dcqcn: {}, "
                   "ah_base_addr_page_id: {}, log_num_ah: {}, log_num_eq: {}, rdma_en_qtype_mask: {} "
                   "sq_qtype: {} rq_qtype: {} aq_qtype: {}, "
                   "prefetch_pool_base_addr : {:#x} log_prefetch_buf_size: {} "
                   "prefetch_cb_base_addr: {:#x}",
                    lif,
                    sram_lif_entry.pt_base_addr_page_id,
                    sram_lif_entry.log_num_pt_entries,
                    sram_lif_entry.log_num_kt_entries,
                    sram_lif_entry.log_num_dcqcn_profiles,
                    sram_lif_entry.ah_base_addr_page_id,
                    sram_lif_entry.log_num_ah_entries,
                    sram_lif_entry.log_num_eq_entries,
                    sram_lif_entry.rdma_en_qtype_mask,
                    sram_lif_entry.sq_qtype,
                    sram_lif_entry.rq_qtype,
                    sram_lif_entry.aq_qtype,
                    sram_lif_entry.prefetch_base_addr_page_id,
                    sram_lif_entry.log_prefetch_buf_size,
                    prefetch_cb_base_addr);

    //Controller Memory Buffer
    //meant for SQ/RQ in HBM for good performance
    //Allocated in units of 8MB
    if (mem_bar_size != 0) {
        sram_lif_entry.barmap_base_addr = mem_bar_addr >> MEM_BARMAP_SIZE_SHIFT;
        sram_lif_entry.barmap_size = mem_bar_size >> MEM_BARMAP_SIZE_SHIFT;
    } else {
        sram_lif_entry.barmap_base_addr = 0;
        sram_lif_entry.barmap_size = 0;
    }

    ret = p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add_(
                            lif,
                            sram_lif_entry.rdma_en_qtype_mask,
                            sram_lif_entry.pt_base_addr_page_id,
                            sram_lif_entry.log_num_pt_entries,
                            sram_lif_entry.log_num_kt_entries,
                            sram_lif_entry.log_num_dcqcn_profiles,
                            sram_lif_entry.log_num_ah_entries,
                            sram_lif_entry.cqcb_base_addr_hi,
                            sram_lif_entry.sqcb_base_addr_hi,
                            sram_lif_entry.rqcb_base_addr_hi,
                            sram_lif_entry.log_num_cq_entries,
                            sram_lif_entry.log_num_sq_entries,
                            sram_lif_entry.log_num_rq_entries,
                            sram_lif_entry.prefetch_base_addr_page_id,
                            sram_lif_entry.log_prefetch_buf_size,
                            sram_lif_entry.sq_qtype,
                            sram_lif_entry.rq_qtype,
                            sram_lif_entry.aq_qtype);

    ret = p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add_(
                            lif,
                            sram_lif_entry.rdma_en_qtype_mask,
                            sram_lif_entry.pt_base_addr_page_id,
                            sram_lif_entry.ah_base_addr_page_id,
                            sram_lif_entry.log_num_pt_entries,
                            sram_lif_entry.log_num_kt_entries,
                            sram_lif_entry.log_num_dcqcn_profiles,
                            sram_lif_entry.log_num_ah_entries,
                            sram_lif_entry.cqcb_base_addr_hi,
                            sram_lif_entry.sqcb_base_addr_hi,
                            sram_lif_entry.rqcb_base_addr_hi,
                            sram_lif_entry.log_num_cq_entries,
                            sram_lif_entry.log_num_sq_entries,
                            sram_lif_entry.log_num_rq_entries,
                            sram_lif_entry.prefetch_base_addr_page_id,
                            sram_lif_entry.log_prefetch_buf_size,
                            sram_lif_entry.sq_qtype,
                            sram_lif_entry.rq_qtype,
                            sram_lif_entry.aq_qtype,
                            sram_lif_entry.barmap_base_addr,
                            sram_lif_entry.barmap_size,
                            sram_lif_entry.log_num_eq_entries);

    NIC_FUNC_DEBUG("lif-{}: rdma_params_table init successful", lif);
    return ret;
}

uint64_t
rdmamgr_iris::rdma_mem_alloc_(uint32_t size)
{
    uint32_t alloc_units;

    alloc_units = (size + kRdmaAllocUnit - 1) & ~(kRdmaAllocUnit-1);
    alloc_units /= kRdmaAllocUnit;
    int alloc_offset = rdma_hbm_allocator_->Alloc(alloc_units);

    if (alloc_offset < 0) {
        NIC_FUNC_ERR("Invalid alloc_offset {}", alloc_offset);
        return 0;
    }

    rdma_allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kRdmaAllocUnit;
    NIC_FUNC_DEBUG("size: {} alloc_offset: {} hbm_addr: {}",
                    size, alloc_offset, rdma_hbm_base_ + alloc_offset);
    return rdma_hbm_base_ + alloc_offset;
}

sdk_ret_t
rdmamgr_iris::
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add_(
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
                                                             uint8_t aq_qtype)
{
    p4pd_error_t        pd_err;
    //directmap                    *dm;
    rx_stage0_load_rdma_params_actiondata_t data = { 0 };

    data.action_id = RX_STAGE0_LOAD_RDMA_PARAMS_RX_STAGE0_LOAD_RDMA_PARAMS_ID;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_kt_entries = log_num_kt_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_dcqcn_profiles = log_num_dcqcn_profiles;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_ah_entries = log_num_ah_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.cqcb_base_addr_hi = cqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sqcb_base_addr_hi = sqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rqcb_base_addr_hi = rqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_sq_entries = log_num_sq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_rq_entries = log_num_rq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.prefetch_base_addr_page_id = prefetch_base_addr_page_id;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_prefetch_buf_size = log_prefetch_buf_size;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sq_qtype = sq_qtype;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rq_qtype = rq_qtype;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.aq_qtype = aq_qtype;

    /* TODO: Do we need memrev */
    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS,
                                     idx, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table write failure for rxdma, "
                    "idx : {}, err : {}",
                    idx, pd_err);
        assert(0);
    }
    NIC_LOG_DEBUG("stage0 rdma LIF table entry add successful for rxdma, "
                 "idx : {}, err : {}",
                 idx, pd_err);
    return SDK_RET_OK;
}

sdk_ret_t
rdmamgr_iris::
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add_(
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
                                                             uint8_t log_num_eq_entries)
{
    p4pd_error_t                  pd_err;
    tx_stage0_lif_params_table_actiondata_t data = { 0 };

    data.action_id = TX_STAGE0_LIF_PARAMS_TABLE_TX_STAGE0_LIF_RDMA_PARAMS_ID;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.ah_base_addr_page_id = ah_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_kt_entries = log_num_kt_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_dcqcn_profiles = log_num_dcqcn_profiles;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_ah_entries = log_num_ah_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.cqcb_base_addr_hi = cqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sqcb_base_addr_hi = sqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rqcb_base_addr_hi = rqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_sq_entries = log_num_sq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_rq_entries = log_num_rq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.prefetch_base_addr_page_id = prefetch_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_prefetch_buf_size = log_prefetch_buf_size;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sq_qtype = sq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rq_qtype = rq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.aq_qtype = aq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.barmap_base_addr = barmap_base_addr;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.barmap_size = barmap_size;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_eq_entries = log_num_eq_entries;

    pd_err = p4pd_global_entry_write(
        P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE,
        idx, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table write failure for txdma, "
                    "idx : {}, err : {}", idx, pd_err);
        assert(0);
    }
    NIC_LOG_DEBUG("stage0 rdma LIF table entry add successful for "
                 "txdma, idx : {}, err : {}", idx, pd_err);
    return SDK_RET_OK;
}

sdk_ret_t
rdmamgr_iris::p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get_(
                                                    uint32_t idx,
                                                    rx_stage0_load_rdma_params_actiondata_t *data)
{
    p4pd_error_t        pd_err;

    assert(data != NULL);

    pd_err = p4pd_global_entry_read(
        P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS,
        idx, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "rxdma, idx : {}, err : {}",
                    idx, pd_err);
        assert(0);
    }

    NIC_LOG_DEBUG("stage0 rdma LIF table entry get successful for "
                 "rxdma, idx : {}, err : {}",
                 idx, pd_err);
    return SDK_RET_OK;
}


uint64_t
rdmamgr_iris::rdma_get_pt_base_addr(uint32_t lif)
{
    uint64_t            pt_table_base_addr;
    int                 rc;
    rx_stage0_load_rdma_params_actiondata_t data = {0};

    rc = p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get_(lif, &data);
    if (rc) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "rxdma, idx : {}, err : {}",
                    lif, rc);
        return rc;
    }

    pt_table_base_addr = data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id;

    NIC_FUNC_DEBUG("lif-{}: Rx LIF params - pt_base_addr_page_id {}",
                    lif, pt_table_base_addr);

    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    return(pt_table_base_addr);
}

uint64_t
rdmamgr_iris::rdma_get_kt_base_addr(uint32_t lif)
{
    uint64_t            pt_table_base_addr;
    uint64_t            key_table_base_addr;
    uint32_t            log_num_pt_entries;
    int                 rc;
    rx_stage0_load_rdma_params_actiondata_t data = {0};

    rc = p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get_(lif, &data);
    if (rc) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "rxdma, idx : {}, err : {}",
                    lif, rc);
        return rc;
    }

    pt_table_base_addr = data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id;
    log_num_pt_entries = data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_pt_entries;

    key_table_base_addr = (pt_table_base_addr << HBM_PAGE_SIZE_SHIFT) +
        (sizeof(uint64_t) << log_num_pt_entries);

    NIC_FUNC_DEBUG("lif-{}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} key_table_base_addr {}",
                    lif,
                    pt_table_base_addr, log_num_pt_entries,
                    key_table_base_addr);

    return key_table_base_addr;
}

uint32_t
rdmamgr_iris::roundup_to_pow_2_(uint32_t x)
{
    uint32_t power = 1;

    if (x == 1)
        return (power << 1);

    while(power < x)
        power*=2;
    return power;
}

}    // namespace nicmgr
