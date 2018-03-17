#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/rdma.hpp"
#include "nic/utils/host_mem/host_mem.hpp"
#include "nic/p4/include/common_defines.h"
#include "nic/include/oif_list_api.hpp"

namespace hal {

const static char *kHBMLabel = "rdma";
const static uint32_t kHBMSizeKB = 128 * 1024;  // 128 MB
const static uint32_t kAllocUnit = 4096;

#define MAX_LIFS 2048                  // TODO: why are there multiple definitions of this ?? why not in some global file ??
uint32_t g_pt_base[MAX_LIFS] = {0};

RDMAManager *g_rdma_manager = nullptr;
extern LIFManager *g_lif_manager;

RDMAManager::RDMAManager() {
  pd::pd_get_start_offset_args_t off_args = {0};
  pd::pd_get_size_kb_args_t size_args = {0};

  off_args.reg_name = kHBMLabel;
  pd::hal_pd_call(pd::PD_FUNC_ID_GET_START_OFFSET, (void *)&off_args);
  uint64_t hbm_addr = off_args.offset;

  size_args.reg_name = kHBMLabel;
  pd::hal_pd_call(pd::PD_FUNC_ID_GET_REG_SIZE, (void *)&size_args);
  assert(size_args.size == kHBMSizeKB);




  uint32_t num_units = (kHBMSizeKB * 1024) / kAllocUnit;
  if (hbm_addr & 0xFFF) {
    // Not 4K aligned.
    hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
    num_units--;
  }
  hbm_base_ = hbm_addr;
  hbm_allocator_.reset(new BMAllocator(num_units));
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

	hal::pd::pd_rxdma_table_entry_add_args_s rx_args;
	rx_args.idx = lif;
    rx_args.rdma_en_qtype_mask = entry_p->rdma_en_qtype_mask;
    rx_args.pt_base_addr_page_id = entry_p->pt_base_addr_page_id;
    rx_args.log_num_pt_entries = entry_p->log_num_pt_entries;
    rx_args.cqcb_base_addr_page_id = entry_p->cqcb_base_addr_page_id;
    rx_args.log_num_cq_entries = entry_p->log_num_cq_entries;
    rx_args.prefetch_pool_base_addr_page_id = entry_p->prefetch_pool_base_addr_page_id;
    rx_args.log_num_prefetch_pool_entries = entry_p->log_num_prefetch_pool_entries;
    rx_args.sq_qtype = entry_p->sq_qtype;
    rx_args.rq_qtype = entry_p->rq_qtype;
	ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_RXDMA_TABLE_ADD, (void *)&rx_args);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for rxdma, idx : {}, err : {}",
                      lif, ret);
        HAL_ASSERT(0);
        return ret;
    }

	hal::pd::pd_txdma_table_entry_add_args_s tx_args;
	tx_args.idx = lif;
    tx_args.rdma_en_qtype_mask = entry_p->rdma_en_qtype_mask;
    tx_args.pt_base_addr_page_id = entry_p->pt_base_addr_page_id;
    tx_args.log_num_pt_entries = entry_p->log_num_pt_entries;
    tx_args.cqcb_base_addr_page_id = entry_p->cqcb_base_addr_page_id;
    tx_args.log_num_cq_entries = entry_p->log_num_cq_entries;
    tx_args.prefetch_pool_base_addr_page_id = entry_p->prefetch_pool_base_addr_page_id;
    tx_args.log_num_prefetch_pool_entries = entry_p->log_num_prefetch_pool_entries;
    tx_args.sq_qtype = entry_p->sq_qtype;
    tx_args.rq_qtype = entry_p->rq_qtype;
	ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TXDMA_TABLE_ADD, (void *)&tx_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for txdma, idx : {}, err : {}",
                      lif, ret);
        HAL_ASSERT(0);
        return ret;
    }

    memcpy(&g_sram_lif_entry[lif], entry_p, sizeof(sram_lif_entry_t));

    return HAL_RET_OK;
}

hal_ret_t
rdma_rx_sram_lif_entry_get (uint16_t lif, sram_lif_entry_t *entry_p)
{
    memcpy(entry_p, &g_sram_lif_entry[lif], sizeof(sram_lif_entry_t));
#if 0
    hal_ret_t                    ret;
    rx_stage0_rdma_params_table_actiondata data = {0};

    ret = hal::pd::p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get(lif, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for rxdma, idx : {}, err : {}",
                      lif, ret);
        return ret;
    }
    entry_p->rdma_en_qtype_mask = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.rdma_en_qtype_mask;
    entry_p->pt_base_addr_page_id = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.pt_base_addr_page_id;
    entry_p->log_num_pt_entries = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.log_num_pt_entries;
    entry_p->cqcb_base_addr_page_id = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.cqcb_base_addr_page_id;
    entry_p->log_num_cq_entries = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.log_num_cq_entries;
    entry_p->prefetch_pool_base_addr_page_id = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.prefetch_pool_base_addr_page_id;
    entry_p->log_num_prefetch_pool_entries = data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.log_num_prefetch_pool_entries;
#endif

    return HAL_RET_OK;
}


hal_ret_t
rdma_tx_sram_lif_entry_get (uint16_t lif, sram_lif_entry_t *entry_p)
{
    memcpy(entry_p, &g_sram_lif_entry[lif], sizeof(sram_lif_entry_t));
#if 0
    hal_ret_t                    ret;
    tx_stage0_rdma_params_table_actiondata data = {0};

    ret = hal::pd::p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get(lif, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for txdma, idx : {}, err : {}",
                      lif, ret);
        return ret;
    }
    entry_p->rdma_en_qtype_mask = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.rdma_en_qtype_mask;
    entry_p->pt_base_addr_page_id = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.pt_base_addr_page_id;
    entry_p->log_num_pt_entries = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.log_num_pt_entries;
    entry_p->cqcb_base_addr_page_id = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.cqcb_base_addr_page_id;
    entry_p->log_num_cq_entries = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.log_num_cq_entries;
    entry_p->prefetch_pool_base_addr_page_id = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.prefetch_pool_base_addr_page_id;
    entry_p->log_num_prefetch_pool_entries = data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.log_num_prefetch_pool_entries;
#endif

    return HAL_RET_OK;
}

uint64_t rdma_lif_pt_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;
    hal_ret_t           rc;  

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} rdma_en_qtype_mask {}\n", 
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.pt_base_addr_page_id, 
                    sram_lif_entry.log_num_pt_entries,
                    sram_lif_entry.rdma_en_qtype_mask);

    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    return(pt_table_base_addr);
}

uint64_t rdma_lif_kt_base_addr(uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;
    uint64_t            key_table_base_addr;
    hal_ret_t           rc;  

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} rdma_en_qtype_mask {}\n", 
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.pt_base_addr_page_id, 
                    sram_lif_entry.log_num_pt_entries,
                    sram_lif_entry.rdma_en_qtype_mask);

    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    key_table_base_addr = pt_table_base_addr + (sizeof(uint64_t) << sram_lif_entry.log_num_pt_entries);
    return(key_table_base_addr);
}

hal_ret_t
rdma_lif_init (intf::LifSpec& spec, uint32_t lif)
{
    sram_lif_entry_t    sram_lif_entry;
    uint32_t            pt_size, key_table_size;
    uint32_t            total_size;
    uint64_t            base_addr;
    uint32_t            max_pt_entries;
    uint32_t            max_keys;
    uint32_t            max_cqs, max_eqs;
    uint64_t            cq_base_addr; //address in HBM memory
    hal_ret_t           rc;

    LIFQState *qstate = g_lif_manager->GetLIFQState(lif);
    if (qstate == nullptr)
        return HAL_RET_ERR;

    max_cqs  = qstate->type[Q_TYPE_RDMA_CQ].num_queues;
    max_eqs  = qstate->type[Q_TYPE_RDMA_EQ].num_queues;
    max_keys = spec.rdma_max_keys();
    max_pt_entries  = spec.rdma_max_pt_entries();


    HAL_TRACE_DEBUG("({},{}): LIF {}: {}, max_CQ: {}, max_EQ: {}, "
           "max_keys: {}, max_pt: {} g_pt_base: {}",
           __FUNCTION__, __LINE__, lif, spec.key_or_handle().lif_id(),
           max_cqs, max_eqs,
           max_keys, max_pt_entries, g_pt_base[lif]);

    memset(&sram_lif_entry, 0, sizeof(sram_lif_entry_t));
    
    // Some one in HAL is corruptimg and causing writting junk value to this address
    // so rest the address to zero.
    g_pt_base[lif] = 0;

    // Fill the CQ info in sram_lif_entry
    cq_base_addr = g_lif_manager->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_CQ);
    HAL_TRACE_DEBUG("({},{}): Lif {} cq_base_addr: {:#x}, max_cqs: {} log_num_cq_entries: {}",
           __FUNCTION__, __LINE__, lif, cq_base_addr,
           max_cqs, log2(roundup_to_pow_2(max_cqs)));
    HAL_ASSERT((cq_base_addr & ((1 << HBM_PAGE_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.cqcb_base_addr_page_id = cq_base_addr >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_cq_entries = log2(roundup_to_pow_2(max_cqs));


    // Setup page table and key table entries
    max_pt_entries = roundup_to_pow_2(max_pt_entries);

    pt_size = sizeof(uint64_t) * max_pt_entries;
    //adjust to page boundary
    if (pt_size & (HBM_PAGE_SIZE - 1)) {
        pt_size = ((pt_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    max_keys = roundup_to_pow_2(max_keys);

    key_table_size = sizeof(key_entry_t) * max_keys;
    //adjust to page boundary
    if (key_table_size & (HBM_PAGE_SIZE - 1)) {
        key_table_size = ((key_table_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    total_size = pt_size + key_table_size + HBM_PAGE_SIZE;

    base_addr = g_rdma_manager->HbmAlloc(total_size);
    
    HAL_TRACE_DEBUG("{}: pt_size: {}, key_table_size: {}, total_size: {}, base_addr: {:#x}\n",
           __FUNCTION__, pt_size, key_table_size, total_size, base_addr);

    sram_lif_entry.pt_base_addr_page_id = base_addr >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_pt_entries = log2(max_pt_entries);

    // TODO: Fill prefetch data and add corresponding code

    sram_lif_entry.rdma_en_qtype_mask = 
        ((1 << Q_TYPE_RDMA_SQ) | (1 << Q_TYPE_RDMA_RQ) | (1 << Q_TYPE_RDMA_CQ) | (1 << Q_TYPE_RDMA_EQ));
    sram_lif_entry.sq_qtype = Q_TYPE_RDMA_SQ;
    sram_lif_entry.rq_qtype = Q_TYPE_RDMA_RQ;

    HAL_TRACE_DEBUG("({},{}): pt_base_addr_page_id: {}, log_num_pt: {}, rdma_en_qtype_mask: {} "
                    "sq_qtype: {} rq_qtype: {}\n",
           __FUNCTION__, __LINE__,
           sram_lif_entry.pt_base_addr_page_id,
           sram_lif_entry.log_num_pt_entries,
           sram_lif_entry.rdma_en_qtype_mask, 
           sram_lif_entry.sq_qtype,
           sram_lif_entry.rq_qtype);
           
    rc = rdma_sram_lif_init(lif, &sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: SRAM LIF INIT successful\n", __FUNCTION__, __LINE__, lif);

    HAL_TRACE_DEBUG("({},{}): Lif: {}: LIF Init successful\n", __FUNCTION__, __LINE__, lif);

    return HAL_RET_OK;
}

void
rdma_key_entry_read (uint16_t lif, uint32_t key, key_entry_t *entry_p)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    sram_lif_entry_t    tx_sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;
    uint64_t            key_table_base_addr;
    hal_ret_t           rc;  

    rc = rdma_tx_sram_lif_entry_get(lif, &tx_sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: Tx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} rdma_en_qtype_mask {}\n", 
                    __FUNCTION__, __LINE__, lif,
                    tx_sram_lif_entry.pt_base_addr_page_id, 
                    tx_sram_lif_entry.log_num_pt_entries,
                    tx_sram_lif_entry.rdma_en_qtype_mask);

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} rdma_en_qtype_mask {}\n", 
                    __FUNCTION__, __LINE__, lif,
                    sram_lif_entry.pt_base_addr_page_id, 
                    sram_lif_entry.log_num_pt_entries,
                    sram_lif_entry.rdma_en_qtype_mask);

    //Make sure Tx and Rx are always programmed the same
    HAL_ASSERT(memcmp(&tx_sram_lif_entry, &sram_lif_entry, sizeof(sram_lif_entry_t)) == 0);

    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    key_table_base_addr = pt_table_base_addr + (sizeof(uint64_t) << sram_lif_entry.log_num_pt_entries);

#if 0
    capri_hbm_read_mem((uint64_t)(((key_entry_t *) key_table_base_addr) + key),
                       (uint8_t*)entry_p, sizeof(key_entry_t));
#endif
    pd::pd_capri_hbm_read_mem_args_t args = {0};
    args.addr = (uint64_t)(((key_entry_t *) key_table_base_addr) + key);
    args.buf = (uint8_t*)entry_p;
    args.size = sizeof(key_entry_t);
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_READ, (void *)&args);
    // Convert data before reading from HBM
    memrev((uint8_t*)entry_p, sizeof(key_entry_t));
}

void
rdma_key_entry_write (uint16_t lif, uint32_t key, key_entry_t *entry_p)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;
    uint64_t            key_table_base_addr;
    key_entry_t	        tmp_key_entry = {0};

    rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    pt_table_base_addr = sram_lif_entry.pt_base_addr_page_id;
    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    key_table_base_addr = pt_table_base_addr + (sizeof(uint64_t) << sram_lif_entry.log_num_pt_entries);

    memcpy(&tmp_key_entry, entry_p, sizeof(key_entry_t));
    memrev((uint8_t *)&tmp_key_entry, sizeof(key_entry_t));
#if 0
    capri_hbm_write_mem((uint64_t)(((key_entry_t *) key_table_base_addr) + key),
                        (uint8_t*)&tmp_key_entry, sizeof(key_entry_t));
#endif
    pd::pd_capri_hbm_write_mem_args_t args = {0};
    args.addr = (uint64_t)(((key_entry_t *) key_table_base_addr) + key);
    args.buf = (uint8_t*)&tmp_key_entry;
    args.size = sizeof(key_entry_t);
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, (void *)&args);
}

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
    args.addr = (uint64_t)(pt_table_base_addr + (offset * sizeof(uint64_t)));
    args.buf = (uint8_t*)&pg_ptr;
    args.size = sizeof(pg_ptr);
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, (void *)&args);
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
    args.addr = (uint64_t)(pt_table_base_addr + (offset * sizeof(uint64_t)));
    args.buf = (uint8_t*)pg_ptr;
    args.size = sizeof(uint64_t);
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_READ, (void *)&args);
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

uint32_t
rdma_mr_pt_base_get (uint16_t lif, uint32_t key)
{
    key_entry_t key_entry = {0};
    uint32_t         pt_seg_size, pt_seg_offset;
    uint32_t         page_size;
    uint8_t          pt_start_page_id;

    rdma_key_entry_read(lif, key, &key_entry);
    page_size = pow(2, key_entry.log_page_size);
    pt_seg_size = page_size * HBM_NUM_PT_ENTRIES_PER_CACHE_LINE;
    pt_seg_offset = key_entry.base_va % pt_seg_size;
    pt_start_page_id = pt_seg_offset / HOSTMEM_PAGE_SIZE;
    //HAL_TRACE_DEBUG("{}: pt_start_page_id: {}\n", __FUNCTION__, pt_start_page_id);
    
    return (key_entry.pt_base + pt_start_page_id);
}

int
is_rkey_valid (uint16_t lif, uint32_t rkey)
{
    key_entry_t rkey_entry = {0};

    if (rkey == INVALID_KEY) {
        return (FALSE);
    }   

    rdma_key_entry_read(lif, rkey, &rkey_entry);

    return (rkey_entry.state == KEY_STATE_VALID);
}

int
is_lkey_valid (uint16_t lif, uint32_t lkey)
{
    key_entry_t lkey_entry = {0};

    if (lkey == INVALID_KEY) {
        return (FALSE);
    }   

    rdma_key_entry_read(lif, lkey, &lkey_entry);

    return (lkey_entry.state == KEY_STATE_VALID);
}

hal_ret_t
rdma_memory_register (RdmaMemRegSpec& spec, RdmaMemRegResponse *rsp)
{
    hal_ret_t        ret = HAL_RET_OK;
    uint32_t         lif = spec.hw_lif_id(); 
    uint32_t         num_pages, num_pt_entries;
    uint32_t         lkey, rkey;
    key_entry_t      lkey_entry = {0} , rkey_entry = {0};
    key_entry_t      *lkey_entry_p = &lkey_entry , *rkey_entry_p = &rkey_entry; 
    uint32_t         pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE *
                              spec.hostmem_pg_size();
    uint32_t         pt_seg_offset, pt_page_offset;
    uint32_t         pt_start_page_id, pt_end_page_id;
    uint32_t         transfer_bytes, pt_page_offset2;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: RdmaMemReg for HW LIF {} PD {} LKEY {} RKEY {} VA {} LEN {}", 
                    __FUNCTION__, 
                    spec.hw_lif_id(),
                    spec.pd(),
                    spec.lkey(),
                    spec.rkey(),
                    spec.va(),
                    spec.len());

    if (spec.ac_remote_wr() && !spec.ac_local_wr()) {
        HAL_TRACE_DEBUG("requesting remote write without requesting "
               "local write permission is not allowed !!\n");
        return (HAL_RET_ERR);
    }   

    if (spec.ac_remote_atomic() && !spec.ac_local_wr()) {
        HAL_TRACE_DEBUG("requesting remote atomic without requesting "
               "local write permission is not allowed !!\n");
        return (HAL_RET_ERR);
    }
    
#if 0
    //TODO: enable this check later
    if (attr_p->flags & MR_FLAG_INV_EN) {
        HAL_TRACE_DEBUG("memory region created thru register_mr() "
               "cannot be invalidated using local/remote "
               "invalidate option and hence MR_FLAG_INV_EN "
               "should not be set !!\n");
        return (NULL);
    }
#endif

    lkey = spec.lkey();

    rdma_key_entry_read(lif, lkey, lkey_entry_p);
    memset(lkey_entry_p, 0, sizeof(key_entry_t));
    lkey_entry_p->state = KEY_STATE_VALID;
    lkey_entry_p->acc_ctrl = (spec.ac_local_wr() ? ACC_CTRL_LOCAL_WRITE : 0);
    lkey_entry_p->log_page_size = log2(spec.hostmem_pg_size());
    lkey_entry_p->base_va = spec.va();
    lkey_entry_p->len = spec.len();
    lkey_entry_p->pt_base = g_pt_base[lif];
    lkey_entry_p->pd = spec.pd();
    // disable user key
    lkey_entry_p->flags = (MR_FLAG_INV_EN | MR_FLAG_UKEY_EN);
    if (spec.override_lif_vld()) {
        lkey_entry_p->override_lif_vld = 1;
        lkey_entry_p->override_lif = spec.override_lif();
    }

    lkey_entry_p->type = MR_TYPE_MR;
    rdma_key_entry_write(lif, lkey, lkey_entry_p);
    HAL_TRACE_DEBUG("{}: lif_id: {} lkey: {}  acc_ctrl: {:#x}, flags: {:#x}, override_lif: {}", 
                    __FUNCTION__, lif, lkey, lkey_entry_p->acc_ctrl, 
                    lkey_entry_p->flags, lkey_entry_p->override_lif);

    if (spec.ac_remote_rd() || spec.ac_remote_wr() || spec.ac_remote_atomic()) {
        // rkey requested
        rkey = spec.rkey();

        rdma_key_entry_read(lif, rkey, rkey_entry_p);
        memcpy(rkey_entry_p, lkey_entry_p, sizeof(key_entry_t));
        rkey_entry_p->acc_ctrl &= ~ACC_CTRL_LOCAL_WRITE;
        if (spec.ac_remote_wr())
            rkey_entry_p->acc_ctrl |= ACC_CTRL_REMOTE_WRITE;
        if (spec.ac_remote_rd())
            rkey_entry_p->acc_ctrl |= ACC_CTRL_REMOTE_READ;
        if (spec.ac_remote_atomic())
            rkey_entry_p->acc_ctrl |= ACC_CTRL_REMOTE_ATOMIC;
        rdma_key_entry_write(lif, rkey, rkey_entry_p);
        HAL_TRACE_DEBUG("{}: lif_id: {} rkey: {}  acc_ctrl: {:#x}", 
                    __FUNCTION__, lif, rkey, rkey_entry_p->acc_ctrl);
    } else {
        rkey = INVALID_KEY;
    }
    
    HAL_TRACE_DEBUG("{}: lif_id: {}  g_pt_base: {:#x}, lkey_entry_p->pt_base: {:#x}\n", 
                    __FUNCTION__, lif, g_pt_base[lif], lkey_entry_p->pt_base);
    HAL_ASSERT(lkey_entry_p->pt_base % HBM_NUM_PT_ENTRIES_PER_CACHE_LINE == 0);

    pt_seg_offset = lkey_entry_p->base_va % pt_seg_size;
    pt_page_offset = pt_seg_offset % spec.hostmem_pg_size();

    num_pages = 0;
    transfer_bytes = lkey_entry_p->len;
    if (pt_page_offset) {
        num_pages++;
        transfer_bytes -= (spec.hostmem_pg_size()-pt_page_offset);
    }
    pt_page_offset2 = 
        (pt_page_offset + lkey_entry_p->len) % spec.hostmem_pg_size();
    if (pt_page_offset2) {
        num_pages++;
        transfer_bytes -= pt_page_offset2;
    }

    HAL_ASSERT(transfer_bytes % spec.hostmem_pg_size() == 0);
    num_pages += transfer_bytes / spec.hostmem_pg_size();


    pt_start_page_id = pt_seg_offset / spec.hostmem_pg_size();
    pt_end_page_id = pt_start_page_id + num_pages - 1;

    HAL_TRACE_DEBUG("{}: base_va: {:#x} len: {} "
           " pt_base: {} pt_seg_offset: {} "
           "pt_start_page_id: {} pt_end_page_id: {} "
           "pt_page_offset: {} num_pages: {} \n",
            __FUNCTION__, lkey_entry_p->base_va, lkey_entry_p->len,
            lkey_entry_p->pt_base, pt_seg_offset,
            pt_start_page_id, pt_end_page_id, pt_page_offset,
            num_pages);

    // Make sure that received the expected number of Pages physical addresses for
    // the VA being registered
    HAL_ASSERT(((uint32_t)spec.va_pages_phy_addr_size()) == num_pages);

    // Fill the PT with the physical addresses
    for (uint32_t i=pt_start_page_id; i<=pt_end_page_id; i++) {
        // write the physical page pointer address into pt entry
        rdma_pt_entry_write(lif, g_pt_base[lif]+i, (uint64_t) spec.va_pages_phy_addr(i-pt_start_page_id));
        HAL_TRACE_DEBUG("PT Entry Write: Lif {}: PT Idx: {} PhyAddr: {:#x}",
                       lif, g_pt_base[lif]+i, spec.va_pages_phy_addr(i-pt_start_page_id));
    }

    //g_pt_base[lif] += num_pages;
    num_pt_entries = ((pt_end_page_id / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE)+1) * HBM_NUM_PT_ENTRIES_PER_CACHE_LINE;
    g_pt_base[lif] += num_pt_entries;
    HAL_TRACE_DEBUG("{}: Enf of MR PT index: {}", __FUNCTION__, g_pt_base[lif]);

#if 0
    lkey_entry_p->pt_size = num_pt_entries;
    if (rkey != INVALID_KEY) {
        rkey_entry_p->pt_size = num_pt_entries;
    }
#endif

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("--------------------- API End ------------------------");
    return ret;
}


hal_ret_t
stage0_resp_rx_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "rdma_resp_rx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, 
                              (void *)&args);
    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_resp_tx_prog_addr(uint64_t* offset)
{
    // Can't access capri apis from PI
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rdma_resp_tx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, 
                                    (void *)&args);
    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_req_rx_prog_addr(uint64_t* offset)
{
    //  Can't access capri apis from PI

    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "rdma_req_rx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, 
                                    (void *)&args);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    // if(ret < 0) {
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_req_tx_prog_addr(uint64_t* offset)
{
    // Can't access capri apis from PI
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rdma_req_tx_stage0";

    pd::pd_capri_program_label_to_offset_args_t args = {0};
    args.handle = "p4plus";
    args.prog_name = progname;
    args.label_name = labelname;
    args.offset = offset;
    hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, 
                                    (void *)&args);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
rdma_qp_create (RdmaQpSpec& spec, RdmaQpResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint32_t      num_sq_wqes, num_rq_wqes;
    uint32_t      num_rrq_wqes, num_rsq_wqes;
    uint32_t     sqwqe_size, rqwqe_size;
    uint32_t     sq_size, rq_size;
    sqcb_t       sqcb;
    sqcb_t       *sqcb_p = &sqcb;
    rqcb_t       rqcb;
    rqcb_t       *rqcb_p = &rqcb;
    uint64_t     header_template_addr, rrq_base_addr, rsq_base_addr;
    uint64_t     hbm_sq_base_addr, hbm_rq_base_addr;
    uint64_t     offset;
    uint64_t     offset_verify;
    hal_ret_t    ret;
    if_t         *hal_if = NULL;
    l2seg_t      *l2seg = NULL;
    oif_t        oif;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA QP Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: qp_num: {} pd: {} svc: {} pmtu: {}",
                     __FUNCTION__, spec.qp_num(), spec.pd(), spec.svc(), spec.pmtu());
    HAL_TRACE_DEBUG("{}: Inputs: sq_wqe_size: {} num_sq_wqes: {} rq_wqe_size: {} "
                    "num_rq_wqes: {} hostmem_pg_size: {} num_rrq_wqes: {} "
                    "num_rsq_wqes: {} ", __FUNCTION__,
                    spec.sq_wqe_size(), spec.num_sq_wqes(), spec.rq_wqe_size(),
                    spec.num_rq_wqes( ), spec.hostmem_pg_size(), spec.num_rrq_wqes(),
                    spec.num_rsq_wqes());
    HAL_TRACE_DEBUG("{}: Inputs: sq_lkey: {} rq_lkey: {}", __FUNCTION__,
                    spec.sq_lkey(), spec.rq_lkey());
    HAL_TRACE_DEBUG("{}: Inputs: sq_cq_id: {} rq_cq_id: {}", __FUNCTION__,
                    spec.sq_cq_num(), spec.rq_cq_num());
    HAL_TRACE_DEBUG("{}: Inputs: atomic_enabled: {} immdt_as_dbell: {}, "
                    "sq_in_nic: {}, rq_in_nic: {}", __FUNCTION__,
                    spec.atomic_enabled(), spec.immdt_as_dbell(), 
                    spec.sq_in_nic_memory(), spec.rq_in_nic_memory());

    // allocate sq and rq
    sqwqe_size = roundup_to_pow_2(spec.sq_wqe_size());
    num_sq_wqes = roundup_to_pow_2(spec.num_sq_wqes());

    rqwqe_size = roundup_to_pow_2(spec.rq_wqe_size());
    num_rq_wqes = roundup_to_pow_2(spec.num_rq_wqes());

    num_rrq_wqes = roundup_to_pow_2(spec.num_rrq_wqes());

    num_rsq_wqes = roundup_to_pow_2(spec.num_rsq_wqes());

    HAL_TRACE_DEBUG("sqwqe_size: {} rqwqe_size: {}",
                    sqwqe_size, rqwqe_size);
   
    // DCQCN related info is stored after header-template info 
    header_template_addr = g_rdma_manager->HbmAlloc(sizeof(header_template_t) + sizeof(dcqcn_cb_t));
    HAL_ASSERT(header_template_addr);
    HAL_ASSERT(header_template_addr != (uint32_t)-ENOMEM);
    // Make sure header_template_addr is 8 byte aligned
    HAL_ASSERT(header_template_addr % 8 == 0);

    HAL_TRACE_DEBUG("{}: header_template_addr: {}",
                     __FUNCTION__, header_template_addr);
    // Fill sqcb and write to HW
    memset(sqcb_p, 0, sizeof(sqcb_t));
    // RRQ is defined as last ring in the SQCB ring array
    // We should skip scheduling for the RRQ, so set total
    //  rings as one less than max/total
    sqcb_p->sqcb0.ring_header.total_rings = MAX_SQ_RINGS - 1;
    sqcb_p->sqcb0.ring_header.host_rings = MAX_SQ_RINGS - 1;
    sqcb_p->sqcb0.poll_for_work = 0;
    sqcb_p->sqcb0.color = 1;
    if (spec.sq_in_nic_memory()) {
        sqcb_p->sqcb0.sq_in_hbm = 1;
        sq_size = num_sq_wqes * sqwqe_size;
        hbm_sq_base_addr = g_rdma_manager->HbmAlloc(sq_size);
        HAL_ASSERT(hbm_sq_base_addr);
        HAL_ASSERT(hbm_sq_base_addr != (uint32_t)-ENOMEM);
        // Make sure hbm_sq_base_addr is 8 byte aligned
        HAL_ASSERT(hbm_sq_base_addr % 8 == 0);
        sqcb_p->sqcb0.hbm_sq_base_addr = hbm_sq_base_addr >> HBM_SQ_BASE_ADDR_SHIFT;
    } else {
        sqcb_p->sqcb0.sq_in_hbm = 0;
        hbm_sq_base_addr = 0;
        sqcb_p->sqcb0.pt_base_addr =
            rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.sq_lkey())) >> PT_BASE_ADDR_SHIFT;
    }

    HAL_TRACE_DEBUG("{}: lif: {}, lkey: {}, rdma_mr_pt_base: {}, rdma_pt_addr: {}, shifted rdma_pt_addr: {} sqcb0_pt_base_addr: {}",
                     __FUNCTION__, lif, spec.sq_lkey(),
                    rdma_mr_pt_base_get(lif, spec.sq_lkey()),
                    rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.sq_lkey())),
                    rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.sq_lkey())) >> PT_BASE_ADDR_SHIFT,
                    sqcb_p->sqcb0.pt_base_addr);
    sqcb_p->sqcb0.header_template_addr = 
                            header_template_addr >> HDR_TEMP_ADDR_SHIFT;
    sqcb_p->sqcb1.header_template_addr = sqcb_p->sqcb0.header_template_addr;
    sqcb_p->sqcb2.header_template_addr = sqcb_p->sqcb0.header_template_addr;

    sqcb_p->sqcb1.header_template_size = sizeof(header_template_v4_t);
    sqcb_p->sqcb2.header_template_size = sizeof(header_template_v4_t);
    sqcb_p->sqcb1.log_rrq_size = log2(num_rrq_wqes);
    sqcb_p->sqcb2.log_rrq_size = sqcb_p->sqcb1.log_rrq_size;

    rrq_base_addr = 
        g_rdma_manager->HbmAlloc(sizeof(rrqwqe_t) << sqcb_p->sqcb1.log_rrq_size);
    HAL_ASSERT(rrq_base_addr);
    HAL_ASSERT(rrq_base_addr != (uint32_t)-ENOMEM);
    // Make sure rrq_base_addr is 8 byte aligned
    HAL_ASSERT(rrq_base_addr % 8 == 0);
    sqcb_p->sqcb1.rrq_base_addr = rrq_base_addr >> RRQ_BASE_ADDR_SHIFT;
    sqcb_p->sqcb2.rrq_base_addr = sqcb_p->sqcb1.rrq_base_addr;
    sqcb_p->sqcb0.log_sq_page_size = log2(spec.hostmem_pg_size());
    sqcb_p->sqcb0.log_wqe_size = log2(sqwqe_size);
    sqcb_p->sqcb0.log_num_wqes = log2(num_sq_wqes);
    sqcb_p->sqcb2.log_sq_size = sqcb_p->sqcb0.log_num_wqes;
    sqcb_p->sqcb0.log_pmtu = log2(spec.pmtu());
    sqcb_p->sqcb1.log_pmtu = sqcb_p->sqcb0.log_pmtu;
    sqcb_p->sqcb0.congestion_mgmt_enable = FALSE;  
    sqcb_p->sqcb1.congestion_mgmt_enable = sqcb_p->sqcb0.congestion_mgmt_enable;  
    sqcb_p->sqcb1.cq_id = spec.sq_cq_num();
    sqcb_p->sqcb0.service = spec.svc();
    sqcb_p->sqcb1.service = sqcb_p->sqcb0.service;
    sqcb_p->sqcb2.service = sqcb_p->sqcb0.service;
    sqcb_p->sqcb2.lsn = 128; // FOR now allowing 128 sq send/write_imm requests
    sqcb_p->sqcb1.lsn = sqcb_p->sqcb2.lsn;
    sqcb_p->sqcb2.ssn = 1;
    sqcb_p->sqcb1.ssn = sqcb_p->sqcb2.ssn;
    sqcb_p->sqcb1.msn = 0;
    sqcb_p->sqcb2.msn = sqcb_p->sqcb1.msn;
    sqcb_p->sqcb1.credits = 0xe; // 0x01110 - 128
    sqcb_p->sqcb2.credits = sqcb_p->sqcb1.credits;
    sqcb_p->sqcb2.p4plus_to_p4_flags = 0xA;
    //sqcb_p->sqcb1.p4plus_to_p4_flags = (P4PLUS_TO_P4_UPDATE_UDP_LEN |
    //                                    P4PLUS_TO_P4_UPDATE_IP_LEN);
    sqcb_p->sqcb0.pd = spec.pd();

    stage0_req_rx_prog_addr(&offset);
    sqcb_p->sqcb0.ring_header.pc = offset >> 6;
    sqcb_p->sqcb1.pc = sqcb_p->sqcb0.ring_header.pc;

    stage0_req_tx_prog_addr(&offset_verify);
    HAL_ASSERT(offset == offset_verify);

    HAL_TRACE_DEBUG("SQCB Header Template Size: {}\n", sqcb_p->sqcb1.header_template_size);
    HAL_TRACE_DEBUG("SQCB Log num WQEs: {}\n", sqcb_p->sqcb0.log_num_wqes);
    HAL_TRACE_DEBUG("SQCB Log WQE size: {}\n", sqcb_p->sqcb0.log_wqe_size);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial SQCB State, SQCB->PT: {}", 
                    __FUNCTION__, lif, sqcb_p->sqcb0.pt_base_addr);
    // Convert data before writting to HBM
    memrev((uint8_t*)&sqcb_p->sqcb0, sizeof(sqcb0_t));
    memrev((uint8_t*)&sqcb_p->sqcb1, sizeof(sqcb1_t));
    memrev((uint8_t*)&sqcb_p->sqcb2, sizeof(sqcb2_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_SQ, spec.qp_num(), (uint8_t *)sqcb_p, sizeof(sqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__, g_lif_manager->GetLIFQStateAddr(lif, Q_TYPE_SQ, spec.qp_num()));

    // allocate rqcb
    memset(rqcb_p, 0, sizeof(rqcb_t));
    rqcb.rqcb0.ring_header.total_rings = MAX_RQ_RINGS;
    rqcb.rqcb0.ring_header.host_rings = MAX_RQ_RINGS;

    if (spec.rq_in_nic_memory()) {
        rqcb_p->rqcb0.rq_in_hbm = 1;
        rqcb_p->rqcb1.rq_in_hbm = 1;
        rq_size = num_rq_wqes * rqwqe_size;
        hbm_rq_base_addr = g_rdma_manager->HbmAlloc(rq_size);
        HAL_ASSERT(hbm_rq_base_addr);
        HAL_ASSERT(hbm_rq_base_addr != (uint32_t)-ENOMEM);
        // Make sure hbm_rq_base_addr is 8 byte aligned
        HAL_ASSERT(hbm_rq_base_addr % 8 == 0);
        rqcb_p->rqcb0.hbm_rq_base_addr = hbm_rq_base_addr >> HBM_RQ_BASE_ADDR_SHIFT;
        rqcb_p->rqcb1.hbm_rq_base_addr = rqcb_p->rqcb0.hbm_rq_base_addr;
    } else {
        rqcb_p->rqcb0.rq_in_hbm = 0;
        rqcb_p->rqcb1.rq_in_hbm = 0;
        hbm_rq_base_addr = 0;
        rqcb_p->rqcb0.pt_base_addr =
            rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.rq_lkey())) >> PT_BASE_ADDR_SHIFT;
        rqcb_p->rqcb1.pt_base_addr = rqcb_p->rqcb0.pt_base_addr;
    }

    HAL_ASSERT(rqcb.rqcb0.pt_base_addr);
    rqcb.rqcb0.log_rsq_size = log2(num_rsq_wqes);
    rqcb.rqcb1.log_rsq_size = rqcb.rqcb0.log_rsq_size;

    rsq_base_addr = 
        g_rdma_manager->HbmAlloc(sizeof(rsqwqe_t) << rqcb.rqcb0.log_rsq_size);
    HAL_ASSERT(rsq_base_addr);
    HAL_ASSERT(rsq_base_addr  != (uint32_t)-ENOMEM);
    // Make sure rsq_base_addr is 8 byte aligned
    HAL_ASSERT(rsq_base_addr % 8 == 0);
    rqcb.rqcb0.rsq_base_addr = rsq_base_addr >> RSQ_BASE_ADDR_SHIFT;
    rqcb.rqcb1.rsq_base_addr = rqcb.rqcb0.rsq_base_addr;

    rqcb.rqcb0.serv_type = spec.svc();
    rqcb.rqcb0.log_rq_page_size = log2(spec.hostmem_pg_size());
    rqcb.rqcb0.log_wqe_size = log2(rqwqe_size);
    rqcb.rqcb0.log_num_wqes = log2(num_rq_wqes);
    rqcb.rqcb0.log_pmtu = log2(spec.pmtu());
    rqcb.rqcb1.serv_type = rqcb.rqcb0.serv_type;
    rqcb.rqcb1.log_rq_page_size = rqcb.rqcb0.log_rq_page_size;
    rqcb.rqcb1.log_wqe_size = rqcb.rqcb0.log_wqe_size;
    rqcb.rqcb1.log_num_wqes = rqcb.rqcb0.log_num_wqes;
    rqcb.rqcb1.log_pmtu = rqcb.rqcb0.log_pmtu;

    rqcb.rqcb1.cache = FALSE;
    rqcb.rqcb1.immdt_as_dbell = spec.immdt_as_dbell();
    rqcb.rqcb0.congestion_mgmt_enable = FALSE;
    rqcb.rqcb1.congestion_mgmt_enable = rqcb.rqcb0.congestion_mgmt_enable;
    rqcb.rqcb0.pd = spec.pd();
    rqcb.rqcb1.pd = rqcb.rqcb0.pd;
    rqcb.rqcb1.cq_id = spec.rq_cq_num();
    rqcb.rqcb0.header_template_addr = 
                            header_template_addr >> HDR_TEMP_ADDR_SHIFT;
    rqcb.rqcb0.header_template_size = sizeof(header_template_v4_t);
    rqcb.rqcb1.header_template_addr = rqcb.rqcb0.header_template_addr;
    rqcb.rqcb1.header_template_size = rqcb.rqcb0.header_template_size;
    rqcb.rqcb0.p4plus_to_p4_flags = 0xA;
    //rqcb.rqcb1.p4plus_to_p4_flags = (P4PLUS_TO_P4_UPDATE_UDP_LEN |
    //                                 P4PLUS_TO_P4_UPDATE_IP_LEN);

    stage0_resp_rx_prog_addr(&offset);
    rqcb.rqcb0.ring_header.pc = offset >> 6;
    rqcb.rqcb1.pc = offset >> 6;

    stage0_resp_tx_prog_addr(&offset_verify);
    HAL_ASSERT(offset == offset_verify);

    HAL_TRACE_DEBUG("RQCB Header Template Size: {}\n", rqcb.rqcb1.header_template_size);

    HAL_TRACE_DEBUG("{}: Create QP successful for LIF: {}  RQCB->PT: {}", 
                    __FUNCTION__, lif, rqcb.rqcb0.pt_base_addr);

    // Convert data before writting to HBM
    memrev((uint8_t*)&rqcb.rqcb0, sizeof(rqcb0_t));
    memrev((uint8_t*)&rqcb.rqcb1, sizeof(rqcb1_t));
    //TODO: rqcb2

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial RQCB State", __FUNCTION__, lif);
    g_lif_manager->WriteQState(lif, Q_TYPE_RQ, spec.qp_num(), (uint8_t *)rqcb_p, sizeof(rqcb_t));

 
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->set_rsq_base_addr(rsq_base_addr);
    rsp->set_rrq_base_addr(rrq_base_addr);
    rsp->set_nic_sq_base_addr(hbm_sq_base_addr);
    rsp->set_nic_rq_base_addr(hbm_rq_base_addr);
    rsp->set_header_temp_addr(header_template_addr);

    // For UD QPs, please add it to the Segment's Broadcast OIFs list
    // For testing purpose, please add only Queuepairs with QID less than or equal to 6
    // In final implementation, we will be registering QPs to a Mcast group(ID'ed by IP address)
    // but for now, we are testing this with Bcast/Flood functionality.

    HAL_TRACE_DEBUG("Check if this QP to be added to oif_list for LIF:{} PD:{} QP:{}, If_hdl: {}",
                    lif, spec.pd(), spec.qp_num(), spec.if_handle());
    if (((uint8_t)spec.svc() == RDMA_SERV_TYPE_UD) && (spec.qp_num() <= 6)) {
        hal_if = find_if_by_handle(spec.if_handle());
        HAL_ASSERT(hal_if != NULL);
        l2seg = l2seg_lookup_by_handle(hal_if->l2seg_handle);
        HAL_ASSERT(l2seg != NULL);
        oif.intf = hal_if;
        oif.l2seg = l2seg;
        oif.qid = spec.qp_num();
        oif.purpose = intf::LIF_QUEUE_PURPOSE_RDMA_RECV;
        ret = oif_list_add_qp_oif(l2seg->bcast_oif_list, &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Add QP oif to oif_list failed for LIF:{} QP:{}, err : {}", lif, spec.qp_num(), ret);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
        }
        HAL_TRACE_DEBUG("Added QP oif to oif_list for LIF:{} PD: {} QP:{}, If_hdl: {}",
                        lif, spec.pd(), spec.qp_num(), spec.if_handle());
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_ah_create (RdmaAhSpec& spec, RdmaAhResponse *rsp)
{
    uint64_t     header_template_addr;
    uint32_t     header_template_size;
    uint8_t      smac[ETH_ADDR_LEN], dmac[ETH_ADDR_LEN];
    header_template_t temp;
    ip_addr_t    ip_addr;
    
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA AH Create", __FUNCTION__);

    memcpy(smac, spec.smac().c_str(), spec.smac().size());
    memcpy(dmac, spec.dmac().c_str(), spec.dmac().size());
    
    // HAL_TRACE_DEBUG("{}: Inputs: smac: {} dmac: {} ethtype: {} vlan: {} "
    //                 "vlan_pri: {} vlan_cfi: {} ip_ver: {} ip_tos: {} "
    //                 "ip_ttl: {} ip_saddr: {} ip_daddr: {} "
    //                 "udp_sport: {} udp_dport: {}",
    //                 __FUNCTION__, (struct ether_addr*)smac,
    //                 (struct ether_addr*)spec.dmac().c_str(),
    //                 (struct ether_addr*)spec.smac().c_str(), 
    //                 spec.vlan(), spec.vlan_pri(), spec.vlan_cfi(),
    //                 spec.ip_ver(), spec.ip_tos(), spec.ip_ttl(),
    //                 spec.ip_saddr(), spec.ip_daddr(),
    //                 spec.udp_sport(), spec.udp_dport());

    HAL_TRACE_DEBUG("{}: Inputs: ethtype: {} vlan: {} "
                    "vlan_pri: {} vlan_cfi: {} ip_ver: {} ip_tos: {} "
                    "ip_ttl: {} ip_saddr: {} ip_daddr: {} "
                    "udp_sport: {} udp_dport: {}",
                    __FUNCTION__, spec.ethtype(),
                    spec.vlan(), spec.vlan_pri(), spec.vlan_cfi(),
                    spec.ip_ver(), spec.ip_tos(), spec.ip_ttl(), 0, 0,
                    //spec.ip_saddr(), spec.ip_daddr(),
                    spec.udp_sport(), spec.udp_dport());

    memset(&temp, 0, sizeof(temp));

    if (spec.ip_ver() == 6) {
        memcpy(&temp.v6.eth.dmac, dmac, MAC_SIZE);
        memcpy(&temp.v6.eth.smac, smac, MAC_SIZE);
        temp.v6.eth.ethertype = 0x8100;
        temp.v6.vlan.pri = spec.vlan_pri();
        temp.v6.vlan.vlan = spec.vlan();
        temp.v6.vlan.ethertype = spec.ethtype();
        temp.v6.ip.version = spec.ip_ver();
        temp.v6.ip.tc = spec.ip_tos();
        temp.v6.ip.hop_limit = spec.ip_ttl();
        temp.v6.ip.nh = 17;
        ip_addr_spec_to_ip_addr(&ip_addr, spec.ip_saddr()); 
        memrev((uint8_t*)&ip_addr.addr.v6_addr, sizeof(ip_addr.addr.v6_addr));
        temp.v6.ip.saddr = ip_addr.addr.v6_addr;
        ip_addr_spec_to_ip_addr(&ip_addr, spec.ip_daddr()); 
        memrev((uint8_t*)&ip_addr.addr.v6_addr, sizeof(ip_addr.addr.v6_addr));
        temp.v6.ip.daddr = ip_addr.addr.v6_addr;
        temp.v6.ip.flow_label = 0;
        temp.v6.udp.sport = spec.udp_sport();
        temp.v6.udp.dport = spec.udp_dport();
        header_template_size = sizeof(temp.v6);

        HAL_TRACE_DEBUG("SIP6 : {}  DIP6: {} \n", ipv6addr2str(temp.v6.ip.saddr), ipv6addr2str(temp.v6.ip.daddr));

    } else {
        memcpy(&temp.v4.eth.dmac, dmac, MAC_SIZE);
        memcpy(&temp.v4.eth.smac, smac, MAC_SIZE);
        temp.v4.eth.ethertype = 0x8100;
        temp.v4.vlan.pri = spec.vlan_pri();
        temp.v4.vlan.vlan = spec.vlan();
        temp.v4.vlan.ethertype = spec.ethtype();
        temp.v4.ip.version = spec.ip_ver();
        temp.v4.ip.ihl = 5;
        temp.v4.ip.tos = spec.ip_tos();
        temp.v4.ip.ttl = spec.ip_ttl();
        temp.v4.ip.protocol = 17;
        ip_addr_spec_to_ip_addr(&ip_addr, spec.ip_saddr()); 
        temp.v4.ip.saddr = ip_addr.addr.v4_addr;
        ip_addr_spec_to_ip_addr(&ip_addr, spec.ip_daddr()); 
        temp.v4.ip.daddr = ip_addr.addr.v4_addr;
        temp.v4.ip.id = 1;
        temp.v4.udp.sport = spec.udp_sport();
        temp.v4.udp.dport = spec.udp_dport();
        header_template_size = sizeof(temp.v4);
        HAL_TRACE_DEBUG("SIP4 : {}  DIP4: {} \n", temp.v4.ip.saddr, temp.v4.ip.daddr);

    }
    header_template_addr = g_rdma_manager->HbmAlloc(header_template_size);
    HAL_TRACE_DEBUG("{} header_template_addr: {} Header Template Size: {}\n", 
                    __FUNCTION__, header_template_addr, header_template_size);

    HAL_ASSERT(header_template_addr);
    HAL_ASSERT(header_template_addr != (uint32_t)-ENOMEM);
    // Make sure header_template_addr is 8 byte aligned
    HAL_ASSERT(header_template_addr % 8 == 0);

    memrev((uint8_t*)&temp, header_template_size);
    pd::pd_capri_hbm_write_mem_args_t args = {0};
    args.addr = (uint64_t)header_template_addr;
    args.buf = (uint8_t*)&temp;
    args.size = header_template_size;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, (void *)&args);

    rsp->set_api_status(types::API_STATUS_OK);
    rsp->set_ah_handle(header_template_addr);
    rsp->set_ah_size(header_template_size);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_qp_update (RdmaQpUpdateSpec& spec, RdmaQpUpdateResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint32_t     qp_num = spec.qp_num();
    uint32_t     oper = spec.oper();
    sqcb_t       sqcb;
    sqcb_t       *sqcb_p = &sqcb;
    rqcb_t       rqcb;
    rqcb_t       *rqcb_p = &rqcb;
    uint64_t     header_template_addr;
    pd::pd_capri_hbm_write_mem_args_t args = {0};

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA QP Update for lif {} QID {} oper type {}", 
                    __FUNCTION__, lif, qp_num, oper);

    HAL_TRACE_DEBUG("{}: Inputs: dst_qp: {}", __FUNCTION__,
                    spec.dst_qp_num());
    HAL_TRACE_DEBUG("{}: Inputs: header_template: {}", __FUNCTION__,
                    spec.header_template());
    HAL_TRACE_DEBUG("{}: Inputs: e_psn: {}", __FUNCTION__,
                    spec.e_psn());
    HAL_TRACE_DEBUG("{}: Inputs: tx_psn: {}", __FUNCTION__,
                    spec.tx_psn());

    // Read sqcb from HW
    memset(sqcb_p, 0, sizeof(sqcb_t));
    g_lif_manager->ReadQState(lif, Q_TYPE_SQ, qp_num, (uint8_t *)sqcb_p, sizeof(sqcb_t));
    memrev((uint8_t*)&sqcb_p->sqcb0, sizeof(sqcb0_t));
    memrev((uint8_t*)&sqcb_p->sqcb1, sizeof(sqcb1_t));
    memrev((uint8_t*)&sqcb_p->sqcb2, sizeof(sqcb2_t));

    // Read rqcb from HW
    memset(rqcb_p, 0, sizeof(rqcb_t));
    g_lif_manager->ReadQState(lif, Q_TYPE_RQ, qp_num, (uint8_t *)rqcb_p, sizeof(rqcb_t));
    memrev((uint8_t*)&rqcb_p->rqcb0, sizeof(rqcb0_t));
    memrev((uint8_t*)&rqcb_p->rqcb1, sizeof(rqcb1_t));

    switch (oper) {

        case rdma::RDMA_UPDATE_QP_OPER_SET_DEST_QP:
            rqcb_p->rqcb0.dst_qp = spec.dst_qp_num();
            sqcb_p->sqcb2.dst_qp = spec.dst_qp_num();
 
            HAL_TRACE_DEBUG("{}: Update: Setting dst_qp to: {}", __FUNCTION__,
                    spec.dst_qp_num());
        break;
        
        case rdma::RDMA_UPDATE_QP_OPER_SET_E_PSN:
            rqcb_p->rqcb1.e_psn = spec.e_psn();
            HAL_TRACE_DEBUG("{}: Update: Setting e_psn to: {}", __FUNCTION__,
                            spec.e_psn());
            break;
        
        case rdma::RDMA_UPDATE_QP_OPER_SET_TX_PSN:
            sqcb_p->sqcb2.tx_psn = spec.tx_psn();
            sqcb_p->sqcb1.tx_psn = sqcb_p->sqcb2.tx_psn;
            sqcb_p->sqcb1.rexmit_psn = spec.tx_psn();
            HAL_TRACE_DEBUG("{}: Update: Setting tx_psn to: {}", __FUNCTION__,
                            spec.tx_psn());
            break;
        
        case rdma::RDMA_UPDATE_QP_OPER_SET_Q_KEY:
            rqcb_p->rqcb0.q_key = spec.q_key();
            rqcb_p->rqcb1.q_key = rqcb_p->rqcb0.q_key;
            sqcb_p->sqcb2.q_key = spec.q_key();
            HAL_TRACE_DEBUG("{}: Update: Setting q_key to: {}", __FUNCTION__,
                            spec.q_key());
            break;
        
        case rdma::RDMA_UPDATE_QP_OPER_SET_HEADER_TEMPLATE:

            sqcb_p->sqcb1.header_template_size = spec.header_template().size();
            sqcb_p->sqcb2.header_template_size = sqcb_p->sqcb1.header_template_size;
            rqcb_p->rqcb0.header_template_size = spec.header_template().size();
            rqcb_p->rqcb1.header_template_size = rqcb_p->rqcb0.header_template_size;

            header_template_addr = sqcb_p->sqcb1.header_template_addr;
            header_template_addr <<= HDR_TEMP_ADDR_SHIFT;
            header_template_t header_template;

            memcpy(&header_template, (uint8_t *)spec.header_template().c_str(),
                   std::min(sizeof(header_template_t), spec.header_template().size()));
        
            args.addr = (uint64_t)header_template_addr;
            args.buf = (uint8_t *)&header_template;
            args.size =  sizeof(header_template_t);
            pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, (void *)&args);

            HAL_TRACE_DEBUG("{}: Update: Setting header_template content @addr: {} to: {}", 
                            __FUNCTION__, header_template_addr, spec.header_template());

            HAL_TRACE_DEBUG("{}: Update: Setting rqcb/sqcb header_template_size to {}, {}",
                            __FUNCTION__, rqcb_p->rqcb1.header_template_size, sqcb_p->sqcb1.header_template_size);  
        break;

        default:
        break;
    }
    
    // Convert and write SQCB to HBM
    memrev((uint8_t*)&sqcb_p->sqcb0, sizeof(sqcb0_t));
    memrev((uint8_t*)&sqcb_p->sqcb1, sizeof(sqcb1_t));
    memrev((uint8_t*)&sqcb_p->sqcb2, sizeof(sqcb2_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_SQ, spec.qp_num(), (uint8_t *)sqcb_p, sizeof(sqcb_t));

    // Convert and write RQCB to HBM
    memrev((uint8_t*)&rqcb_p->rqcb0, sizeof(rqcb0_t));
    memrev((uint8_t*)&rqcb_p->rqcb1, sizeof(rqcb1_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_RQ, spec.qp_num(), (uint8_t *)rqcb_p, sizeof(rqcb_t));
    
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_cq_create (RdmaCqSpec& spec, RdmaCqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint32_t      num_cq_wqes, cqwqe_size;
    cqcb_t       cqcb;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA CQ Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} eq_id: {}"
                    " hostmem_pg_size: {} cq_lkey: {} wakeup_dpath: {}", __FUNCTION__, spec.cq_num(),
                    spec.cq_wqe_size(), spec.num_cq_wqes(), spec.eq_id(), spec.hostmem_pg_size(),
                    spec.cq_lkey(), spec.wakeup_dpath());

    if (spec.wakeup_dpath()) {
        HAL_TRACE_DEBUG("{}: Inputs - Wakeup LIF: {}, QTYPE: {}, QID: {}, RING_ID: {}", 
                        spec.wakeup_lif(), spec.wakeup_qtype(), spec.wakeup_qid(), spec.wakeup_ring_id());
    }

    cqwqe_size = roundup_to_pow_2(spec.cq_wqe_size());
    num_cq_wqes = roundup_to_pow_2(spec.num_cq_wqes());

    HAL_TRACE_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);
    
    memset(&cqcb, 0, sizeof(cqcb_t));
    // EQ does not need scheduling, so set one less (meaning #rings as zero)
    cqcb.ring_header.total_rings = MAX_CQ_RINGS - 1;
    int32_t cq_pt_base = rdma_mr_pt_base_get(lif, spec.cq_lkey());
    cqcb.pt_base_addr =
        rdma_pt_addr_get(lif, cq_pt_base) >> PT_BASE_ADDR_SHIFT;
    cqcb.log_cq_page_size = log2(spec.hostmem_pg_size());
    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.cq_num = spec.cq_num();
    cqcb.eq_id = spec.eq_id();
    cqcb.color = 0;
    cqcb.arm = 0;   // Dont arm by default, only Arm it for tests which post/validate EQ

    cqcb.wakeup_dpath
        = spec.wakeup_dpath();
    cqcb.wakeup_lif = spec.wakeup_lif();
    cqcb.wakeup_qtype = spec.wakeup_qtype();
    cqcb.wakeup_qid = spec.wakeup_qid();
    cqcb.wakeup_ring_id = spec.wakeup_ring_id();

    rdma_pt_entry_read(lif, cq_pt_base, &cqcb.pt_pa);
    rdma_pt_entry_read(lif, cq_pt_base+1, &cqcb.pt_next_pa);    
    cqcb.pt_pa_index = 1 << (cqcb.log_cq_page_size - cqcb.log_wqe_size);
    cqcb.pt_next_pa_index = 2 << (cqcb.log_cq_page_size - cqcb.log_wqe_size);

    HAL_TRACE_DEBUG("{}: LIF: {}: pt_pa: {:#x}: pt_next_pa: {:#x}: pt_pa_index: {}: pt_next_pa_index: {}:", __FUNCTION__, lif, cqcb.pt_pa, cqcb.pt_next_pa, cqcb.pt_pa_index, cqcb.pt_next_pa_index);
    
    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial CQCB State, CQCB->PT: {:#x} cqcb_size: {}", 
                    __FUNCTION__, lif, cqcb.pt_base_addr, sizeof(cqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&cqcb, sizeof(cqcb_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_RDMA_CQ, spec.cq_num(), (uint8_t *)&cqcb, sizeof(cqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__, g_lif_manager->GetLIFQStateAddr(lif, Q_TYPE_CQ, spec.cq_num()));

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}


hal_ret_t
rdma_eq_create (RdmaEqSpec& spec, RdmaEqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id(), num_eq_wqes;
    uint8_t      eqwqe_size;
    eqcb_t       eqcb;
    uint64_t     hbm_eq_intr_table_base;

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
    eqcb.eqe_base_addr = spec.eqe_base_addr_phy();
    eqcb.log_wqe_size = log2(eqwqe_size);
    eqcb.log_num_wqes = log2(num_eq_wqes);
    eqcb.int_enabled = 1;
    eqcb.int_num = spec.int_num();
    eqcb.eq_id = spec.eq_id();
    eqcb.color = 0;

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial EQCB State, eqcb_size: {}", 
                    __FUNCTION__, lif, sizeof(eqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&eqcb, sizeof(eqcb_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_RDMA_EQ, spec.eq_id(), (uint8_t *)&eqcb, sizeof(eqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__, g_lif_manager->GetLIFQStateAddr(lif, Q_TYPE_EQ, spec.eq_id()));

    rsp->set_api_status(types::API_STATUS_OK);
    // Fill the EQ Interrupt address = Intr_table base + 8 bytes for each intr_num
    pd::pd_get_start_offset_args_t off_args = {0};
    off_args.reg_name = "rdma-eq-intr-table";
    pd::hal_pd_call(pd::PD_FUNC_ID_GET_START_OFFSET, (void *)&off_args);
    hbm_eq_intr_table_base = off_args.offset;
    HAL_ASSERT(hbm_eq_intr_table_base > 0);
    rsp->set_eq_intr_tbl_addr(hbm_eq_intr_table_base + spec.int_num() * sizeof(uint8_t));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_hal_init()
{
    HAL_TRACE_DEBUG("{}: Entered\n", __FUNCTION__);
    g_rdma_manager = new RDMAManager();
    HAL_TRACE_DEBUG("{}: Leaving\n", __FUNCTION__);
    return HAL_RET_OK;
}


}    // namespace hal
