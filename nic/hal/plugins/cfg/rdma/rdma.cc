//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cstdlib>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/rdma/rdma.hpp"
#include "nic/utils/host_mem/host_mem.hpp"
#include "nic/p4/common/defines.h"
#include "nic/hal/plugins/cfg/mcast/oif_list_api.hpp"

namespace hal {

const static uint32_t kAllocUnit = 4096;

//RDMAManager *g_rdma_manager = nullptr;
// extern sdk::platform::capri::LIFManager *lif_manager();
extern lif_mgr *lif_manager();

RDMAManager::RDMAManager() {
  sdk::platform::utils::mpartition *mp = lif_manager()->get_mpartition();
  uint64_t hbm_addr = mp->start_addr("rdma");
  uint32_t kHBMSizeKB = mp->size("rdma") >> 10;
  uint32_t num_units = (kHBMSizeKB * 1024) / kAllocUnit;

  // Minimum 128 MB
  SDK_ASSERT(kHBMSizeKB >= (128 * 1024));

  if (hbm_addr & 0xFFF) {
    // Not 4K aligned.
    hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
    num_units--;
  }
  hbm_base_ = hbm_addr;
  hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));
  HAL_TRACE_DEBUG("{}: hbm_base_ : {}\n", __FUNCTION__, hbm_base_);
}

uint64_t RDMAManager::HbmAlloc(uint32_t size) {
  uint32_t alloc_units;
  alloc_units = (size + kAllocUnit - 1) & ~(kAllocUnit-1);
  alloc_units /= kAllocUnit;
  uint64_t alloc_offset = hbm_allocator_->Alloc(alloc_units);
  if (alloc_offset < 0) {
    HAL_TRACE_DEBUG("{}: Invalid alloc_offset {}", __FUNCTION__, alloc_offset);
    return -ENOMEM;
  }
  allocation_sizes_[alloc_offset] = alloc_units;
  alloc_offset *= kAllocUnit;
    HAL_TRACE_DEBUG("{}: size: {} alloc_offset: {} hbm_addr: {}\n",
                     __FUNCTION__, size, alloc_offset, hbm_base_ + alloc_offset);
  return hbm_base_ + alloc_offset;
}

//------------------------------------------------------------------------------
// TODO:
// Looks like this file has all kinds of hacks at this point
// 1. directly including iris header files
// 2. directly managing HBM memory
// 3. directly calling capri functions
// 4. table management is being done from this layer ??????????
// None of these work if asic changes of P4 program changes
// we are trying to make HAL run on another P4 program and this file seems
// be beyond repair at this point, the only way to proceed is to hack further!!
//------------------------------------------------------------------------------
#ifdef __x86_64__
static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
     uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
     return block;
}
#endif

uint32_t
roundup_to_pow_2(uint32_t x)
{
    uint32_t power = 1;

    if (x == 1)
        return (power << 1);

    while(power < x)
        power*=2;
    return power;
}

static sram_lif_entry_t g_sram_lif_entry[MAX_LIFS];

hal_ret_t
rdma_sram_lif_init (uint16_t lif, sram_lif_entry_t *entry_p)
{
    hal_ret_t   ret;
    pd::pd_func_args_t pd_func_args = {0};

    hal::pd::pd_rxdma_table_entry_add_args_s rx_args;
    rx_args.idx = lif;
    rx_args.rdma_en_qtype_mask = entry_p->rdma_en_qtype_mask;
    rx_args.pt_base_addr_page_id = entry_p->pt_base_addr_page_id;
    rx_args.log_num_pt_entries = entry_p->log_num_pt_entries;
    rx_args.log_num_kt_entries = entry_p->log_num_kt_entries;
    rx_args.log_num_dcqcn_profiles = entry_p->log_num_dcqcn_profiles;
    rx_args.log_num_ah_entries = entry_p->log_num_ah_entries;
    rx_args.cqcb_base_addr_hi = entry_p->cqcb_base_addr_hi;
    rx_args.sqcb_base_addr_hi = entry_p->sqcb_base_addr_hi;
    rx_args.rqcb_base_addr_hi = entry_p->rqcb_base_addr_hi;
    rx_args.log_num_cq_entries = entry_p->log_num_cq_entries;
    rx_args.log_num_sq_entries = entry_p->log_num_sq_entries;
    rx_args.log_num_rq_entries = entry_p->log_num_rq_entries;
    rx_args.prefetch_base_addr_page_id = entry_p->prefetch_base_addr_page_id;
    rx_args.log_prefetch_buf_size = entry_p->log_prefetch_buf_size;
    rx_args.sq_qtype = entry_p->sq_qtype;
    rx_args.rq_qtype = entry_p->rq_qtype;
    rx_args.aq_qtype = entry_p->aq_qtype;
    pd_func_args.pd_rxdma_table_entry_add = &rx_args;

    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_RXDMA_TABLE_ADD, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for rxdma, idx : {}, err : {}",
                      lif, ret);
        SDK_ASSERT(0);
        return ret;
    }

    hal::pd::pd_txdma_table_entry_add_args_s tx_args;
    tx_args.idx = lif;
    tx_args.rdma_en_qtype_mask = entry_p->rdma_en_qtype_mask;
    tx_args.pt_base_addr_page_id = entry_p->pt_base_addr_page_id;
    tx_args.ah_base_addr_page_id = entry_p->ah_base_addr_page_id;
    tx_args.log_num_pt_entries = entry_p->log_num_pt_entries;
    tx_args.log_num_kt_entries = entry_p->log_num_kt_entries;
    tx_args.log_num_dcqcn_profiles = entry_p->log_num_dcqcn_profiles;
    tx_args.log_num_ah_entries = entry_p->log_num_ah_entries;
    tx_args.cqcb_base_addr_hi = entry_p->cqcb_base_addr_hi;
    tx_args.sqcb_base_addr_hi = entry_p->sqcb_base_addr_hi;
    tx_args.rqcb_base_addr_hi = entry_p->rqcb_base_addr_hi;
    tx_args.log_num_cq_entries = entry_p->log_num_cq_entries;
    tx_args.log_num_sq_entries = entry_p->log_num_sq_entries;
    tx_args.log_num_rq_entries = entry_p->log_num_rq_entries;
    tx_args.prefetch_base_addr_page_id = entry_p->prefetch_base_addr_page_id;
    tx_args.log_prefetch_buf_size = entry_p->log_prefetch_buf_size;
    tx_args.sq_qtype = entry_p->sq_qtype;
    tx_args.rq_qtype = entry_p->rq_qtype;
    tx_args.aq_qtype = entry_p->aq_qtype;
    tx_args.barmap_base_addr = entry_p->barmap_base_addr;
    tx_args.barmap_size = entry_p->barmap_size;
    pd_func_args.pd_txdma_table_entry_add = &tx_args;

    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TXDMA_TABLE_ADD, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for txdma, idx : {}, err : {}",
                      lif, ret);
        SDK_ASSERT(0);
        return ret;
    }

    memcpy(&g_sram_lif_entry[lif], entry_p, sizeof(sram_lif_entry_t));

    return HAL_RET_OK;
}

hal_ret_t
rdma_rx_sram_lif_entry_get (uint16_t lif, sram_lif_entry_t *entry_p)
{
    memcpy(entry_p, &g_sram_lif_entry[lif], sizeof(sram_lif_entry_t));

    return HAL_RET_OK;
}


hal_ret_t
rdma_tx_sram_lif_entry_get (uint16_t lif, sram_lif_entry_t *entry_p)
{
    memcpy(entry_p, &g_sram_lif_entry[lif], sizeof(sram_lif_entry_t));

    return HAL_RET_OK;
}

uint64_t rdma_lif_pt_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            base_addr;
    hal_ret_t           rc;

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    SDK_ASSERT(rc == HAL_RET_OK);

    // from pt_base_addr_page_id, first table

    base_addr = sram_lif_entry.pt_base_addr_page_id;
    base_addr <<= HBM_PAGE_SIZE_SHIFT;

    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} rdma_en_qtype_mask {} base_addr {}\n",
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.pt_base_addr_page_id,
                    sram_lif_entry.log_num_pt_entries,
                    sram_lif_entry.rdma_en_qtype_mask, base_addr);

    return(base_addr);
}

uint64_t rdma_lif_kt_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            base_addr;
    hal_ret_t           rc;

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    SDK_ASSERT(rc == HAL_RET_OK);

    // from pt_base_addr_page_id, after pt

    base_addr = sram_lif_entry.pt_base_addr_page_id;
    base_addr <<= HBM_PAGE_SIZE_SHIFT;

    base_addr += sizeof(uint64_t) << sram_lif_entry.log_num_pt_entries;

    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} size_pt_entry{} "
                    "rdma_en_qtype_mask {} base_addr {}\n",
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.pt_base_addr_page_id,
                    sram_lif_entry.log_num_pt_entries, sizeof(uint64_t),
                    sram_lif_entry.rdma_en_qtype_mask, base_addr);

    return(base_addr);
}

uint64_t rdma_lif_dcqcn_profile_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            base_addr;
    hal_ret_t           rc;

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    SDK_ASSERT(rc == HAL_RET_OK);

    // from pt_base_addr_page_id, after pt and kt

    base_addr = sram_lif_entry.pt_base_addr_page_id;
    base_addr <<= HBM_PAGE_SIZE_SHIFT;

    base_addr += sizeof(uint64_t) << sram_lif_entry.log_num_pt_entries;
    base_addr += sizeof(key_entry_t) << sram_lif_entry.log_num_kt_entries;

    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} size_pt_entry {} "
                    "log_num_kt_entries {} size_kt_entry {} "
                    "rdma_en_qtype_mask {} base_addr {}\n",
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.pt_base_addr_page_id,
                    sram_lif_entry.log_num_pt_entries, sizeof(uint64_t),
                    sram_lif_entry.log_num_kt_entries, sizeof(key_entry_t),
                    sram_lif_entry.rdma_en_qtype_mask, base_addr);

    return(base_addr);
}

uint64_t rdma_lif_at_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            base_addr;
    hal_ret_t           rc;

    rc = rdma_tx_sram_lif_entry_get(lif, &sram_lif_entry);
    SDK_ASSERT(rc == HAL_RET_OK);

    // from ah_base_addr_page_id, first table

    base_addr = sram_lif_entry.ah_base_addr_page_id;
    base_addr <<= HBM_PAGE_SIZE_SHIFT;

    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - ah_base_addr_page_id {} "
                    "rdma_en_qtype_mask {} base_addr {}\n",
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.ah_base_addr_page_id,
                    sram_lif_entry.rdma_en_qtype_mask, base_addr);

    return(base_addr);
}

uint64_t rdma_lif_barmap_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            base_addr;
    hal_ret_t           rc;

    rc = rdma_tx_sram_lif_entry_get(lif, &sram_lif_entry);
    SDK_ASSERT(rc == HAL_RET_OK);

    // from barmap_base_addr, first table

    base_addr = sram_lif_entry.barmap_base_addr;
    base_addr <<= HBM_BARMAP_BASE_SHIFT;

    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - barmap_base_addr_id {} "
                    "rdma_en_qtype_mask {} base_addr {}\n",
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.barmap_base_addr,
                    sram_lif_entry.rdma_en_qtype_mask, base_addr);

    return(base_addr);
}

hal_ret_t
rdma_lif_init (intf::LifSpec& spec, uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry;
    uint32_t            pt_size, key_table_size, ah_table_size, hbm_bar_size;
    uint32_t            total_size;
    uint64_t            base_addr;
    uint64_t            size;
    uint32_t            max_pt_entries;
    uint32_t            max_keys, max_ahs;
    uint32_t            max_cqs, max_eqs, max_sqs, max_rqs;
    // TODO: num dcqcn profiles param from json
    uint32_t            num_dcqcn_profiles = 8;
    uint64_t            cq_base_addr; //address in HBM memory
    uint64_t            sq_base_addr; //address in HBM memory
    uint64_t            rq_base_addr; //address in HBM memory
    hal_ret_t           rc;

    // LIFQState *qstate = lif_manager()->GetLIFQState(lif);
    lif_qstate_t *qstate = lif_manager()->get_lif_qstate(lif);
    if (qstate == nullptr)
        return HAL_RET_ERR;

    max_cqs  = qstate->type[Q_TYPE_RDMA_CQ].num_queues;
    max_eqs  = qstate->type[Q_TYPE_RDMA_EQ].num_queues;
    max_sqs  = qstate->type[Q_TYPE_RDMA_SQ].num_queues;
    max_rqs  = qstate->type[Q_TYPE_RDMA_RQ].num_queues;
    max_keys = spec.rdma_max_keys();
    max_ahs  = spec.rdma_max_ahs();
    max_pt_entries  = spec.rdma_max_pt_entries();

    HAL_TRACE_DEBUG("({},{}): LIF {}: {}, max_CQ: {}, max_EQ: {}, "
           "max_keys: {}, max_ahs: {}, max_pt: {}",
           __FUNCTION__, __LINE__, lif, spec.key_or_handle().lif_id(),
           max_cqs, max_eqs, max_keys,
           max_ahs, max_pt_entries);

    memset(&sram_lif_entry, 0, sizeof(sram_lif_entry_t));

    // Fill the CQ info in sram_lif_entry
    // cq_base_addr = lif_manager()->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_CQ);
    cq_base_addr = lif_manager()->get_lif_qstate_base_addr(lif, Q_TYPE_RDMA_CQ);
    HAL_TRACE_DEBUG("({},{}): Lif {} cq_base_addr: {:#x}, max_cqs: {} log_num_cq_entries: {}",
           __FUNCTION__, __LINE__, lif, cq_base_addr,
           max_cqs, log2(roundup_to_pow_2(max_cqs)));
    SDK_ASSERT((cq_base_addr & ((1 << CQCB_ADDR_HI_SHIFT) - 1)) == 0);
    sram_lif_entry.cqcb_base_addr_hi = cq_base_addr >> CQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_cq_entries = log2(roundup_to_pow_2(max_cqs));

    // Fill the SQ info in sram_lif_entry
    // sq_base_addr = lif_manager()->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_SQ);
    sq_base_addr = lif_manager()->get_lif_qstate_base_addr(lif, Q_TYPE_RDMA_SQ);
    HAL_TRACE_DEBUG("({},{}): Lif {} sq_base_addr: {:#x}",
                    __FUNCTION__, __LINE__, lif, sq_base_addr);
    SDK_ASSERT((sq_base_addr & ((1 << SQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.sqcb_base_addr_hi = sq_base_addr >> SQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_sq_entries = log2(roundup_to_pow_2(max_sqs));

    // Fill the RQ info in sram_lif_entry
    // rq_base_addr = lif_manager()->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_RQ);
    rq_base_addr = lif_manager()->get_lif_qstate_base_addr(lif, Q_TYPE_RDMA_RQ);
    HAL_TRACE_DEBUG("({},{}): Lif {} rq_base_addr: {:#x}",
                    __FUNCTION__, __LINE__, lif, rq_base_addr);
    SDK_ASSERT((rq_base_addr & ((1 << RQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.rqcb_base_addr_hi = rq_base_addr >> RQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_rq_entries = log2(roundup_to_pow_2(max_rqs));

    // Setup page table and key table entries
    max_pt_entries = roundup_to_pow_2(max_pt_entries);

    pt_size = sizeof(uint64_t) * max_pt_entries;
    pt_size = HBM_PAGE_ALIGN(pt_size);

    max_keys = roundup_to_pow_2(max_keys);

    key_table_size = sizeof(key_entry_t) * max_keys;
    key_table_size += sizeof(dcqcn_config_cb_t) * num_dcqcn_profiles;
    key_table_size = HBM_PAGE_ALIGN(key_table_size);

    max_ahs = roundup_to_pow_2(max_ahs);

    ah_table_size = AT_ENTRY_SIZE_BYTES * max_ahs;
    ah_table_size = HBM_PAGE_ALIGN(ah_table_size);

    // For DOL, hardcode 8MB space as a makeshift barmap
    hbm_bar_size = 8 * 1024 * 1024;

    total_size = pt_size + key_table_size + ah_table_size + hbm_bar_size + HBM_PAGE_SIZE;

    base_addr = rdma_manager()->HbmAlloc(total_size);

    HAL_TRACE_DEBUG("{}: pt_size: {}, key_table_size: {}, ah_table_size: {}, base_addr: {:#x}\n",
           __FUNCTION__, pt_size, key_table_size, ah_table_size, base_addr);

    size = base_addr;
    sram_lif_entry.pt_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
    size += pt_size + key_table_size;
    sram_lif_entry.ah_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_pt_entries = log2(max_pt_entries);
    sram_lif_entry.log_num_kt_entries = log2(max_keys);
    sram_lif_entry.log_num_dcqcn_profiles = log2(num_dcqcn_profiles);
    sram_lif_entry.log_num_ah_entries = log2(max_ahs);
    size += ah_table_size;
    sram_lif_entry.barmap_base_addr = size >> HBM_BARMAP_BASE_SHIFT;
    size += hbm_bar_size;
    sram_lif_entry.barmap_size = hbm_bar_size >> HBM_BARMAP_SIZE_SHIFT;

    sram_lif_entry.rdma_en_qtype_mask =
        ((1 << Q_TYPE_RDMA_SQ) | (1 << Q_TYPE_RDMA_RQ) | (1 << Q_TYPE_RDMA_CQ) | (1 << Q_TYPE_RDMA_EQ) | (1 << Q_TYPE_ADMINQ));
    sram_lif_entry.sq_qtype = Q_TYPE_RDMA_SQ;
    sram_lif_entry.rq_qtype = Q_TYPE_RDMA_RQ;
    sram_lif_entry.aq_qtype = Q_TYPE_ADMINQ;

    HAL_TRACE_DEBUG("({},{}): pt_base_addr_page_id: {}, log_num_pt: {}, log_num_kt: {}, log_num_dcqcn: {}, log_num_ah: {},  ah_base_addr_page_id: {}, "
                    "barmap_base: {} rdma_en_qtype_mask: {} sq_qtype: {} rq_qtype: {} aq_qtype: {}\n",
           __FUNCTION__, __LINE__,
           sram_lif_entry.pt_base_addr_page_id,
           sram_lif_entry.log_num_pt_entries,
           sram_lif_entry.log_num_kt_entries,
           sram_lif_entry.log_num_dcqcn_profiles,
           sram_lif_entry.log_num_ah_entries,
           sram_lif_entry.ah_base_addr_page_id,
           sram_lif_entry.barmap_base_addr,
           sram_lif_entry.rdma_en_qtype_mask,
           sram_lif_entry.sq_qtype,
           sram_lif_entry.rq_qtype,
           sram_lif_entry.aq_qtype);

    rc = rdma_sram_lif_init(lif, &sram_lif_entry);
    SDK_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: SRAM LIF INIT successful\n", __FUNCTION__, __LINE__, lif);

    HAL_TRACE_DEBUG("({},{}): Lif: {}: LIF Init successful\n", __FUNCTION__, __LINE__, lif);

    return HAL_RET_OK;
}

/*
 * Utility functions to read/write PT entries.
 * Ideally, HAL should never program PT entries.
 */
void
rdma_pt_entry_write (uint16_t lif, uint32_t offset, uint64_t pg_ptr)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;

    rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;

    // Don't use capri apis
#if 0
    capri_hbm_write_mem((uint64_t)(pt_table_base_addr +
        (offset * sizeof(uint64_t))), (uint8_t*)&pg_ptr, sizeof(pg_ptr));
#endif
    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.addr = (uint64_t)(pt_table_base_addr + (offset * sizeof(uint64_t)));
    args.buf = (uint8_t*)&pg_ptr;
    args.size = sizeof(pg_ptr);
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);
}

void
rdma_pt_entry_read (uint16_t lif, uint32_t offset, uint64_t *pg_ptr)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;

    rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;

    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.addr = (uint64_t)(pt_table_base_addr + (offset * sizeof(uint64_t)));
    args.buf = (uint8_t*)pg_ptr;
    args.size = sizeof(uint64_t);
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_READ, &pd_func_args);
}

uint64_t
rdma_pt_addr_get (uint16_t lif, uint32_t offset)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;

    rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;

    HAL_TRACE_DEBUG("{}: pt_table_base_addr: {:#x} offset: {}\n",
                    __FUNCTION__, pt_table_base_addr, offset);

    return ((uint64_t)(pt_table_base_addr + (offset * sizeof(uint64_t))));
}


hal_ret_t
stage0_rdma_cq_rx_prog_addr(uint64_t* offset)
{
    // Can't access capri apis from PI
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "rdma_cq_rx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    pd_func_args.pd_capri_program_label_to_offset = &args;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, &pd_func_args);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n",
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: {}\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}



hal_ret_t
stage0_rdma_cq_tx_prog_addr(uint64_t* offset)
{
    // Can't access capri apis from PI
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rdma_cq_tx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    pd_func_args.pd_capri_program_label_to_offset = &args;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, &pd_func_args);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n",
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: {}\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_rdma_aq_rx_prog_addr(uint64_t* offset)
{
    // Can't access capri apis from PI
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "rdma_aq_rx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    pd_func_args.pd_capri_program_label_to_offset = &args;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, &pd_func_args);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n",
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: {}\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_rdma_aq_tx_prog_addr(uint64_t* offset)
{
    // Can't access capri apis from PI
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rdma_aq_tx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    pd_func_args.pd_capri_program_label_to_offset = &args;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, &pd_func_args);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n",
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: {}\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

#ifdef __x86_64__
hal_ret_t
rdma_ah_create (RdmaAhSpec& spec, RdmaAhResponse *rsp)
{
    uint32_t            lif = spec.hw_lif_id();
    uint64_t            ah_table_base_addr;
    uint64_t            header_template_addr;
    ah_entry_t          ah_entry;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA AH Create", __FUNCTION__);

    HAL_TRACE_DEBUG("{}: Inputs: lif: {} ahid: {} header_template_size: {}",
                    __FUNCTION__, lif, spec.ahid(), spec.header_template().size());

    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};

    ah_table_base_addr = rdma_lif_at_base_addr(lif);
    header_template_addr = ah_table_base_addr + AT_ENTRY_SIZE_BYTES * spec.ahid();

    ah_entry.ah_size = std::min(sizeof(header_template_t), spec.header_template().size());
    memcpy(&ah_entry.hdr_tmp, (uint8_t *)spec.header_template().c_str(), ah_entry.ah_size);

    args.addr = (uint64_t)header_template_addr;
    args.buf = (uint8_t *)&ah_entry;
    args.size =  sizeof(ah_entry_t);
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);

    HAL_TRACE_DEBUG("{} ah_table_base_addr: {:#x}, header_template_addr: {:#x}, header_template_size: {}\n",
                    __FUNCTION__, ah_table_base_addr, args.addr, args.size);

     pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);

     rsp->set_api_status(types::API_STATUS_OK);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_cq_create (RdmaCqSpec& spec, RdmaCqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint32_t      num_cq_wqes, cqwqe_size;
    cqcb_t       cqcb;
    uint64_t     offset;
    uint64_t     offset_verify;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA CQ Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} eq_id: {}"
                    " hostmem_pg_size: {} cq_lkey: {} wakeup_dpath: {} host_addr: {}",
                    __FUNCTION__, spec.cq_num(),
                    spec.cq_wqe_size(), spec.num_cq_wqes(), spec.eq_id(), spec.hostmem_pg_size(),
                    spec.cq_lkey(), spec.wakeup_dpath(), spec.host_addr());

    if (spec.wakeup_dpath()) {
        HAL_TRACE_DEBUG("Inputs - Wakeup LIF: {}, QTYPE: {}, QID: {}, RING_ID: {}",
                        spec.wakeup_lif(), spec.wakeup_qtype(), spec.wakeup_qid(), spec.wakeup_ring_id());
    }

    cqwqe_size = roundup_to_pow_2(spec.cq_wqe_size());
    num_cq_wqes = roundup_to_pow_2(spec.num_cq_wqes());

    HAL_TRACE_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);

    memset(&cqcb, 0, sizeof(cqcb_t));
    cqcb.ring_header.total_rings = MAX_CQ_RINGS;
    cqcb.ring_header.host_rings = MAX_CQ_HOST_RINGS;

    /* Only AdminCQ is created here. No PT entry writes are necessary */
    cqcb.pt_base_addr = 0;

    cqcb.log_cq_page_size = log2(spec.hostmem_pg_size());
    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.cq_id = spec.cq_num();
    cqcb.eq_id = spec.eq_id();
    cqcb.color = 0;
    cqcb.arm = 0;   // Dont arm by default, only Arm it for tests which post/validate EQ
    cqcb.sarm = 0;

    cqcb.host_addr = spec.host_addr();

    cqcb.wakeup_dpath
        = spec.wakeup_dpath();
    cqcb.wakeup_lif = spec.wakeup_lif();
    cqcb.wakeup_qtype = spec.wakeup_qtype();
    cqcb.wakeup_qid = spec.wakeup_qid();
    cqcb.wakeup_ring_id = spec.wakeup_ring_id();

    cqcb.pt_pa = spec.cq_va_pages_phy_addr(0);
    cqcb.pt_next_pa = 0;

    if (cqcb.host_addr) {
        cqcb.pt_pa |= ((1UL << 63)  | (uint64_t)lif << 52);
        cqcb.pt_next_pa |= ((1UL << 63) | (uint64_t)lif << 52);
    }
    cqcb.pt_pg_index = 0;
    cqcb.pt_next_pg_index = 0;

    HAL_TRACE_DEBUG("{}: LIF: {}: pt_pa: {:#x}: pt_next_pa: {:#x}: pt_pa_index: {}: pt_next_pa_index: {}", __FUNCTION__, lif, cqcb.pt_pa, cqcb.pt_next_pa, cqcb.pt_pg_index, cqcb.pt_next_pg_index);

    /* store  pt_pa & pt_next_pa in little endian. So need an extra memrev */
    memrev((uint8_t*)&cqcb.pt_pa, sizeof(uint64_t));
    memrev((uint8_t*)&cqcb.pt_next_pa, sizeof(uint64_t));

    cqcb.proxy_pindex = 0;
    cqcb.proxy_s_pindex = 0;

    stage0_rdma_cq_rx_prog_addr(&offset);
    cqcb.ring_header.pc = offset >> 6;

    stage0_rdma_cq_tx_prog_addr(&offset_verify);
    SDK_ASSERT(offset == offset_verify);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial CQCB State, CQCB->PT: {:#x} cqcb_size: {}",
                    __FUNCTION__, lif, cqcb.pt_base_addr, sizeof(cqcb_t));
    // Convert data before writing to HBM
    memrev((uint8_t*)&cqcb, sizeof(cqcb_t));
    // lif_manager()->WriteQState(lif, Q_TYPE_RDMA_CQ, spec.cq_num(), (uint8_t *)&cqcb, sizeof(cqcb_t));
    lif_manager()->write_qstate(lif, Q_TYPE_RDMA_CQ, spec.cq_num(), (uint8_t *)&cqcb, sizeof(cqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__,
                    lif_manager()->get_lif_qstate_addr(lif, Q_TYPE_RDMA_CQ, spec.cq_num()));

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}
#endif

/*
 * TODO: Need to remove this hardcoded values. They will go away
 * anyway once we move the code out to nicmgr.
 */
#define CAP_ADDR_BASE_INTR_INTR_OFFSET 0x6000000
#define CAP_INTR_CSR_DHS_INTR_ASSERT_BYTE_OFFSET 0x68000
#define INTR_BASE               CAP_ADDR_BASE_INTR_INTR_OFFSET
#define INTR_ASSERT_OFFSET      CAP_INTR_CSR_DHS_INTR_ASSERT_BYTE_OFFSET
#define INTR_ASSERT_BASE        (INTR_BASE + INTR_ASSERT_OFFSET)
#define INTR_ASSERT_STRIDE      0x4

#ifdef __x86_64__
static u_int64_t
intr_assert_addr(const int intr)
{
    return INTR_ASSERT_BASE + (intr * INTR_ASSERT_STRIDE);
}

hal_ret_t
rdma_eq_create (RdmaEqSpec& spec, RdmaEqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id(), num_eq_wqes;
    uint8_t      eqwqe_size;
    eqcb_t       eqcb;
    uint64_t     hbm_eq_intr_table_base;
    sdk::platform::utils::mpartition *mp = lif_manager()->get_mpartition();

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA EQ Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: eq_id: {} eq_wqe_size: {} num_eq_wqes: {} "
                    " eqe_base_addr_phy: {} ", __FUNCTION__, spec.eq_id(),
                    spec.eq_wqe_size(), spec.num_eq_wqes(),
                    spec.eqe_base_addr_phy());

    eqwqe_size = roundup_to_pow_2(spec.eq_wqe_size());
    num_eq_wqes = roundup_to_pow_2(spec.num_eq_wqes());

    HAL_TRACE_DEBUG("eqwqe_size: {} num_eq_wqes: {}", eqwqe_size, num_eq_wqes);


    memset(&eqcb, 0, sizeof(eqcb_t));
    // EQ does not need scheduling, so set one less (meaning #rings as zero)
    eqcb.ring_header.total_rings = MAX_EQ_RINGS - 1;
    eqcb.eqe_base_addr = spec.eqe_base_addr_phy() | (1UL << 63) | ((uint64_t)lif << 52);
    eqcb.log_wqe_size = log2(eqwqe_size);
    eqcb.log_num_wqes = log2(num_eq_wqes);
    eqcb.int_enabled = 1;
    //eqcb.int_num = spec.int_num();
    eqcb.eq_id = spec.eq_id();
    eqcb.color = 0;

    rsp->set_api_status(types::API_STATUS_OK);
    // Fill the EQ Interrupt address = Intr_table base + 8 bytes for each intr_num
    hbm_eq_intr_table_base = mp->start_addr("rdma-eq-intr-table");
    SDK_ASSERT(hbm_eq_intr_table_base > 0);
    //eqcb.int_assert_addr = hbm_eq_intr_table_base + spec.int_num() * sizeof(uint8_t);

    eqcb.int_assert_addr = intr_assert_addr(spec.int_num());

    rsp->set_eq_intr_tbl_addr(eqcb.int_assert_addr);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial EQCB State, eqcb_size: {}",
                    __FUNCTION__, lif, sizeof(eqcb_t));
    // Convert data before writing to HBM
    memrev((uint8_t*)&eqcb, sizeof(eqcb_t));
    // lif_manager()->WriteQState(lif, Q_TYPE_RDMA_EQ, spec.eq_id(), (uint8_t *)&eqcb, sizeof(eqcb_t));
    lif_manager()->write_qstate(lif, Q_TYPE_RDMA_EQ, spec.eq_id(), (uint8_t *)&eqcb, sizeof(eqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__, lif_manager()->get_lif_qstate_addr(lif, Q_TYPE_RDMA_EQ, spec.eq_id()));

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_aq_create (RdmaAqSpec& spec, RdmaAqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    aqcb_t       aqcb;
    uint64_t     offset;
    uint64_t     rdma_atomic_res_addr;
    //uint64_t     offset_verify;
    sdk::platform::utils::mpartition *mp = lif_manager()->get_mpartition();

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA AQ Create for lif {}", __FUNCTION__, lif);

    HAL_TRACE_DEBUG("{}: Inputs: aq_num: {} aq_log_wqe_size: {} aq_log_num_wqes: {} "
                    "cq_num: {} phy_base_addr: {}", __FUNCTION__, spec.aq_num(),
                    spec.log_wqe_size(), spec.log_num_wqes(), spec.cq_num(),
                    spec.phy_base_addr());

    assert(sizeof(aqcb0_t) == 64);
    assert(sizeof(aqcb1_t) == 64);

    memset(&aqcb, 0, sizeof(aqcb_t));
    aqcb.aqcb0.ring_header.total_rings = MAX_AQ_RINGS;
    aqcb.aqcb0.ring_header.host_rings = MAX_AQ_HOST_RINGS;

    aqcb.aqcb0.log_wqe_size = spec.log_wqe_size();
    aqcb.aqcb0.log_num_wqes = spec.log_num_wqes();
    aqcb.aqcb0.aq_id = spec.aq_num();
    aqcb.aqcb0.phy_base_addr = spec.phy_base_addr() | (1UL << 63) | ((uint64_t)lif << 52);
    aqcb.aqcb0.cq_id = spec.cq_num();
    aqcb.aqcb0.cqcb_addr = lif_manager()->get_lif_qstate_addr(lif, Q_TYPE_RDMA_CQ, spec.cq_num());

    aqcb.aqcb0.first_pass = 1;

    //stage0_rdma_aq_rx_prog_addr(&offset);
    stage0_rdma_aq_tx_prog_addr(&offset);
    aqcb.aqcb0.ring_header.pc = offset >> 6;

    //SDK_ASSERT(offset == offset_verify);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial AQCB State, AQCB->phy_addr: {:#x} "
                    "aqcb_size: {}",
                    __FUNCTION__, lif, aqcb.aqcb0.phy_base_addr, sizeof(aqcb_t));
    // Convert data before writing to HBM
    memrev((uint8_t*)&aqcb.aqcb0, sizeof(aqcb0_t));
    memrev((uint8_t*)&aqcb.aqcb1, sizeof(aqcb1_t));
    lif_manager()->write_qstate(lif, Q_TYPE_ADMINQ, spec.aq_num(),
                               (uint8_t *)&aqcb, sizeof(aqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__,
                    lif_manager()->get_lif_qstate_addr(lif, Q_TYPE_ADMINQ, spec.aq_num()));

    rdma_atomic_res_addr = mp->start_addr("rdma-atomic-resource-addr");

    rsp->set_rdma_atomic_res_addr(rdma_atomic_res_addr);
    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}
#endif

}    // namespace hal
