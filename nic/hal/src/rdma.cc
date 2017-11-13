#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
// #include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/rdma.hpp"
#include "nic/hal/pd/iris/rdma_pd.hpp"
#include "nic/utils/host_mem/host_mem.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/p4/include/common_defines.h"
#include "nic/include/oif_list_api.hpp"

namespace hal {

const static char *kHBMLabel = "rdma";
const static uint32_t kHBMSizeKB = 128 * 1024;  // 128 MB
const static uint32_t kAllocUnit = 4096;

uint32_t g_pt_base[MAX_LIFS] = {0};

RDMAManager *g_rdma_manager = nullptr;
extern LIFManager *g_lif_manager;

RDMAManager::RDMAManager() {
  uint32_t hbm_addr = get_start_offset(kHBMLabel);
  HAL_ASSERT(hbm_addr > 0);
  HAL_ASSERT(get_size_kb(kHBMLabel) == kHBMSizeKB);
  uint32_t num_units = (kHBMSizeKB * 1024) / kAllocUnit;
  if (hbm_addr & 0xFFF) {
    // Not 4K aligned.
    hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFU;
    num_units--;
  }
  hbm_base_ = hbm_addr;
  hbm_allocator_.reset(new BMAllocator(num_units));
}

int32_t RDMAManager::HbmAlloc(uint32_t size) {
  uint32_t alloc_units;

  alloc_units = (size + kAllocUnit - 1) & ~(kAllocUnit-1);
  alloc_units /= kAllocUnit;
  int alloc_offset = hbm_allocator_->Alloc(alloc_units);
  if (alloc_offset < 0) {
    HAL_TRACE_DEBUG("{}: Invalid alloc_offset {}", __FUNCTION__, alloc_offset);
    return -ENOMEM;
  }
  allocation_sizes_[alloc_offset] = alloc_units;
  alloc_offset *= kAllocUnit;
  return hbm_base_ + alloc_offset;
}

uint32_t
roundup_to_pow_2(uint32_t x)
{
    uint32_t power = 1;

    while(power < x)
        power*=2;
    return power;
}

static sram_lif_entry_t g_sram_lif_entry[MAX_LIFS];

hal_ret_t
rdma_sram_lif_init (uint16_t lif, sram_lif_entry_t *entry_p)
{
    hal_ret_t   ret;

    ret = hal::pd::p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add(lif,
               entry_p->rdma_en_qtype_mask,
               entry_p->pt_base_addr_page_id,
               entry_p->log_num_pt_entries,
               entry_p->cqcb_base_addr_page_id,
               entry_p->log_num_cq_entries,
               entry_p->prefetch_pool_base_addr_page_id,
               entry_p->log_num_prefetch_pool_entries);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for rxdma, idx : {}, err : {}",
                      lif, ret);
        HAL_ASSERT(0);
        return ret;
    }

    ret = hal::pd::p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add(lif,
               entry_p->rdma_en_qtype_mask,
               entry_p->pt_base_addr_page_id,
               entry_p->log_num_pt_entries,
               entry_p->cqcb_base_addr_page_id,
               entry_p->log_num_cq_entries,
               entry_p->prefetch_pool_base_addr_page_id,
               entry_p->log_num_prefetch_pool_entries);
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
    uint32_t            cq_base_addr; //address in HBM memory
    hal_ret_t           rc;

    LIFQState *qstate = g_lif_manager->GetLIFQState(lif);
    if (qstate == nullptr)
        return HAL_RET_ERR;

    max_cqs  = qstate->type[Q_TYPE_RDMA_CQ].qsize;
    max_eqs  = qstate->type[Q_TYPE_RDMA_EQ].qsize;
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

    HAL_TRACE_DEBUG("{}: pt_size: {}, key_table_size: {}, total_size: {}, base_addr: {:#x}\n",
           __FUNCTION__, pt_size, key_table_size, total_size, base_addr);

    base_addr = g_rdma_manager->HbmAlloc(total_size);
    
    sram_lif_entry.pt_base_addr_page_id = base_addr >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_pt_entries = log2(max_pt_entries);

    // TODO: Fill prefetch data and add corresponding code

    sram_lif_entry.rdma_en_qtype_mask = 
        ((1 << Q_TYPE_RDMA_SQ) | (1 << Q_TYPE_RDMA_RQ) | (1 << Q_TYPE_RDMA_CQ) | (1 << Q_TYPE_RDMA_EQ));

    HAL_TRACE_DEBUG("({},{}): pt_base_addr_page_id: {}, log_num_pt: {}, rdma_en_qtype_mask: {}\n",
           __FUNCTION__, __LINE__,
           sram_lif_entry.pt_base_addr_page_id,
           sram_lif_entry.log_num_pt_entries,
           sram_lif_entry.rdma_en_qtype_mask);
           
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

    capri_hbm_read_mem((uint64_t)(((key_entry_t *) key_table_base_addr) + key),
                       (uint8_t*)entry_p, sizeof(key_entry_t));
    // Convert data before reading from HBM
    pd::memrev((uint8_t*)entry_p, sizeof(key_entry_t));
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
    pd::memrev((uint8_t *)&tmp_key_entry, sizeof(key_entry_t));
    capri_hbm_write_mem((uint64_t)(((key_entry_t *) key_table_base_addr) + key),
                        (uint8_t*)&tmp_key_entry, sizeof(key_entry_t));
}

void
rdma_pt_entry_write (uint16_t lif, uint32_t offset, uint64_t pg_ptr)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint32_t            pt_table_base_addr;

    rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    pt_table_base_addr = (uint32_t)(sram_lif_entry.pt_base_addr_page_id << HBM_PAGE_SIZE_SHIFT);

    capri_hbm_write_mem((uint64_t)(pt_table_base_addr + 
        (offset * sizeof(uint64_t))), (uint8_t*)&pg_ptr, sizeof(pg_ptr));
}

uint32_t
rdma_pt_addr_get (uint16_t lif, uint32_t offset)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint32_t            pt_table_base_addr;

    rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    pt_table_base_addr = (uint32_t)(sram_lif_entry.pt_base_addr_page_id << HBM_PAGE_SIZE_SHIFT);

    HAL_TRACE_DEBUG("{}: pt_table_base_addr: {:#x} offset: {}\n",
                    __FUNCTION__, pt_table_base_addr, offset);

    return ((uint32_t)(pt_table_base_addr + (offset * sizeof(uint64_t))));
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

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret < 0) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_resp_tx_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rdma_resp_tx_stage0";

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret < 0) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_req_rx_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "rdma_req_rx_stage0";

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret < 0) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
stage0_req_tx_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rdma_req_tx_stage0";

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);

    //HAL_TRACE_DEBUG("{}: ret: {}, offset: {}\n", 
    //                __FUNCTION__, ret, offset);
    if(ret < 0) {
        HAL_TRACE_ERR("{}: ret: []\n", __FUNCTION__, ret);
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
rdma_qp_create (RdmaQpSpec& spec, RdmaQpResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint8_t      num_sq_wqes, num_rq_wqes;
    uint8_t      num_rrq_wqes, num_rsq_wqes;
    uint32_t     sqwqe_size, rqwqe_size;
    sqcb_t       sqcb;
    sqcb_t       *sqcb_p = &sqcb;
    rqcb_t       rqcb;
    rqcb_t       *rqcb_p = &rqcb;
    uint32_t     header_template_addr, rrq_base_addr, rsq_base_addr;
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
    HAL_TRACE_DEBUG("{}: Inputs: atomic_enabled: {} immdt_as_dbell: {}", __FUNCTION__,
                    spec.atomic_enabled(), spec.immdt_as_dbell());

    // allocate sq and rq
    sqwqe_size = roundup_to_pow_2(spec.sq_wqe_size());
    num_sq_wqes = roundup_to_pow_2(spec.num_sq_wqes());

    rqwqe_size = roundup_to_pow_2(spec.rq_wqe_size());
    num_rq_wqes = roundup_to_pow_2(spec.num_rq_wqes());

    num_rrq_wqes = roundup_to_pow_2(spec.num_rrq_wqes());

    num_rsq_wqes = roundup_to_pow_2(spec.num_rsq_wqes());

    HAL_TRACE_DEBUG("sqwqe_size: {} rqwqe_size: {}",
                    sqwqe_size, rqwqe_size);
    
    header_template_addr = g_rdma_manager->HbmAlloc(sizeof(header_template_t));
    HAL_ASSERT(header_template_addr);
    HAL_ASSERT(header_template_addr != (uint32_t)-ENOMEM);
    // Fill sqcb and write to HW
    memset(sqcb_p, 0, sizeof(sqcb_t));
    // RRQ is defined as last ring in the SQCB ring array
    // We should skip scheduling for the RRQ, so set total
    //  rings as one less than max/total
    sqcb_p->sqcb0.ring_header.total_rings = MAX_SQ_RINGS - 1;
    sqcb_p->sqcb0.ring_header.host_rings = MAX_SQ_RINGS - 1;
    sqcb_p->sqcb0.pt_base_addr =
        rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.sq_lkey()));
    sqcb_p->sqcb1.header_template_addr = header_template_addr;
    sqcb_p->sqcb1.log_rrq_size = log2(num_rrq_wqes);
    sqcb_p->sqcb1.rrq_base_addr =
        g_rdma_manager->HbmAlloc(sizeof(rrqwqe_t) << sqcb_p->sqcb1.log_rrq_size);
    rrq_base_addr = sqcb_p->sqcb1.rrq_base_addr;
    HAL_ASSERT(rrq_base_addr);
    HAL_ASSERT(rrq_base_addr != (uint32_t)-ENOMEM);
    sqcb_p->sqcb0.log_sq_page_size = log2(spec.hostmem_pg_size());
    sqcb_p->sqcb0.log_wqe_size = log2(sqwqe_size);
    sqcb_p->sqcb0.log_num_wqes = log2(num_sq_wqes);
    sqcb_p->sqcb0.log_pmtu = log2(spec.pmtu());
    sqcb_p->sqcb1.cq_id = spec.sq_cq_num();
    sqcb_p->sqcb0.service = spec.svc();
    sqcb_p->sqcb1.service = spec.svc();
    sqcb_p->sqcb1.lsn = 128; // FOR now allowing 128 sq send/write_imm requests
    sqcb_p->sqcb1.ssn = 1;
    sqcb_p->sqcb1.msn = 0;
    sqcb_p->sqcb1.credits = 0xe; // 0x01110 - 128
    sqcb_p->sqcb1.p4plus_to_p4_flags = 0xA;
    //sqcb_p->sqcb1.p4plus_to_p4_flags = (P4PLUS_TO_P4_UPDATE_UDP_LEN |
    //                                    P4PLUS_TO_P4_UPDATE_IP_LEN);
    sqcb_p->sqcb0.pd = spec.pd();

    stage0_req_rx_prog_addr(&offset);
    sqcb_p->sqcb0.ring_header.pc = offset >> 6;

    stage0_req_tx_prog_addr(&offset_verify);
    HAL_ASSERT(offset == offset_verify);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial SQCB State, SQCB->PT: {}", 
                    __FUNCTION__, lif, sqcb_p->sqcb0.pt_base_addr);
    // Convert data before writting to HBM
    pd::memrev((uint8_t*)&sqcb_p->sqcb0, sizeof(sqcb0_t));
    pd::memrev((uint8_t*)&sqcb_p->sqcb1, sizeof(sqcb1_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_SQ, spec.qp_num(), (uint8_t *)sqcb_p, sizeof(sqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__, g_lif_manager->GetLIFQStateAddr(lif, Q_TYPE_SQ, spec.qp_num()));

    // allocate rqcb
    memset(rqcb_p, 0, sizeof(rqcb_t));
    rqcb.rqcb0.ring_header.total_rings = MAX_RQ_RINGS;
    rqcb.rqcb0.ring_header.host_rings = MAX_RQ_RINGS;
    rqcb.rqcb0.pt_base_addr =
        rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.rq_lkey()));
    HAL_ASSERT(rqcb.rqcb0.pt_base_addr);
    rqcb.rqcb0.log_rsq_size = log2(num_rsq_wqes);
    rqcb.rqcb0.rsq_base_addr = g_rdma_manager->HbmAlloc(sizeof(rsqwqe_t) << rqcb.rqcb0.log_rsq_size);
    rsq_base_addr = rqcb.rqcb0.rsq_base_addr;
    HAL_ASSERT(rsq_base_addr);
    HAL_ASSERT(rsq_base_addr  != (uint32_t)-ENOMEM);
    rqcb.rqcb0.serv_type = spec.svc();
    rqcb.rqcb0.log_rq_page_size = log2(spec.hostmem_pg_size());
    rqcb.rqcb0.log_wqe_size = log2(rqwqe_size);
    rqcb.rqcb0.log_num_wqes = log2(num_rq_wqes);
    rqcb.rqcb0.log_pmtu = log2(spec.pmtu());
    rqcb.rqcb0.cache = FALSE;
    rqcb.rqcb0.immdt_as_dbell = spec.immdt_as_dbell();
    rqcb.rqcb0.pd = spec.pd();
    rqcb.rqcb1.cq_id = spec.rq_cq_num();
    rqcb.rqcb1.header_template_addr = header_template_addr;
    rqcb.rqcb1.p4plus_to_p4_flags = 0xA;
    //rqcb.rqcb1.p4plus_to_p4_flags = (P4PLUS_TO_P4_UPDATE_UDP_LEN |
    //                                 P4PLUS_TO_P4_UPDATE_IP_LEN);
    rqcb.rqcb2.num_rqwqes_per_cpage = HBM_PAGE_SIZE / rqwqe_size;

    stage0_resp_rx_prog_addr(&offset);
    rqcb.rqcb0.ring_header.pc = offset >> 6;

    stage0_resp_tx_prog_addr(&offset_verify);
    HAL_ASSERT(offset == offset_verify);

    HAL_TRACE_DEBUG("{}: Create QP successful for LIF: {}  RQCB->PT: {}", 
                    __FUNCTION__, lif, rqcb.rqcb0.pt_base_addr);

    // Convert data before writting to HBM
    pd::memrev((uint8_t*)&rqcb.rqcb0, sizeof(rqcb0_t));
    pd::memrev((uint8_t*)&rqcb.rqcb1, sizeof(rqcb1_t));
    //TODO: rqcb2

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial RQCB State", __FUNCTION__, lif);
    g_lif_manager->WriteQState(lif, Q_TYPE_RQ, spec.qp_num(), (uint8_t *)rqcb_p, sizeof(rqcb_t));

 
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->set_rsq_base_addr(rsq_base_addr);
    rsp->set_rrq_base_addr(rrq_base_addr);
    rsp->set_header_temp_addr(header_template_addr);

    // For UD QPs, please add it to the Segment's Broadcast OIFs list
    // For testing purpose, please add only Queuepairs with QID less than or equal to 5

    HAL_TRACE_DEBUG("Check if this QP to be added to oif_list for LIF:{} PD:{} QP:{}, If_hdl: {}",
                    lif, spec.pd(), spec.qp_num(), spec.if_handle());
    if (((uint8_t)spec.svc() == RDMA_SERV_TYPE_UD) && (spec.qp_num() <= 5)) {
        hal_if = find_if_by_handle(spec.if_handle());
        HAL_ASSERT(hal_if != NULL);
        l2seg = find_l2seg_by_handle(hal_if->l2seg_handle);
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
    uint32_t     header_template_addr;
    uint8_t      smac[ETH_ADDR_LEN], dmac[ETH_ADDR_LEN];
    header_template_t temp;
    
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
                    spec.ip_ver(), spec.ip_tos(), spec.ip_ttl(),
                    spec.ip_saddr(), spec.ip_daddr(),
                    spec.udp_sport(), spec.udp_dport());

    memset(&temp, 0, sizeof(temp));
    memcpy(&temp.eth.dmac, dmac, MAC_SIZE);
    memcpy(&temp.eth.smac, smac, MAC_SIZE);
    temp.eth.ethertype = 0x8100;
    temp.vlan.pri = spec.vlan_pri();
    temp.vlan.vlan = spec.vlan();
    temp.vlan.ethertype = spec.ethtype();
    temp.ip.version = spec.ip_ver();
    temp.ip.ihl = 5;
    temp.ip.tos = spec.ip_tos();
    temp.ip.ttl = spec.ip_ttl();
    temp.ip.protocol = 17;
    temp.ip.saddr = spec.ip_saddr();
    temp.ip.daddr = spec.ip_daddr();
    temp.ip.id = 1;
    temp.udp.sport = spec.udp_sport();
    temp.udp.dport = spec.udp_dport();

    header_template_addr = g_rdma_manager->HbmAlloc(sizeof(header_template_t));
    HAL_ASSERT(header_template_addr);
    HAL_ASSERT(header_template_addr != (uint32_t)-ENOMEM);

    pd::memrev((uint8_t*)&temp, sizeof(temp));
    capri_hbm_write_mem((uint64_t)header_template_addr, (uint8_t*) &temp, sizeof(temp));

    rsp->set_api_status(types::API_STATUS_OK);
    rsp->set_ah_handle(header_template_addr);

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
    uint32_t     header_template_addr;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA QP Update for lif {} QID {} oper type {}", 
                    __FUNCTION__, lif, qp_num, oper);

    HAL_TRACE_DEBUG("{}: Inputs: dst_qp: {}", __FUNCTION__,
                    spec.dst_qp_num());
    HAL_TRACE_DEBUG("{}: Inputs: header_template: {}", __FUNCTION__,
                    spec.header_template());

    // Read sqcb from HW
    memset(sqcb_p, 0, sizeof(sqcb_t));
    g_lif_manager->ReadQState(lif, Q_TYPE_SQ, qp_num, (uint8_t *)sqcb_p, sizeof(sqcb_t));
    pd::memrev((uint8_t*)&sqcb_p->sqcb0, sizeof(sqcb0_t));
    pd::memrev((uint8_t*)&sqcb_p->sqcb1, sizeof(sqcb1_t));

    // Read rqcb from HW
    memset(rqcb_p, 0, sizeof(rqcb_t));
    g_lif_manager->ReadQState(lif, Q_TYPE_RQ, qp_num, (uint8_t *)rqcb_p, sizeof(rqcb_t));
    pd::memrev((uint8_t*)&rqcb_p->rqcb0, sizeof(rqcb0_t));
    pd::memrev((uint8_t*)&rqcb_p->rqcb1, sizeof(rqcb1_t));

    switch (oper) {

        case rdma::RDMA_UPDATE_QP_OPER_SET_DEST_QP:
            rqcb_p->rqcb1.dst_qp = spec.dst_qp_num();
            sqcb_p->sqcb1.dst_qp = spec.dst_qp_num();
            HAL_TRACE_DEBUG("{}: Update: Setting dst_qp to: {}", __FUNCTION__,
                    spec.dst_qp_num());
        break;
        
        case rdma::RDMA_UPDATE_QP_OPER_SET_Q_KEY:
            rqcb_p->rqcb0.q_key = spec.q_key();
            sqcb_p->sqcb1.q_key = spec.q_key();
            HAL_TRACE_DEBUG("{}: Update: Setting q_key to: {}", __FUNCTION__,
                            spec.q_key());
            break;
        
        case rdma::RDMA_UPDATE_QP_OPER_SET_HEADER_TEMPLATE:
            header_template_addr = sqcb_p->sqcb1.header_template_addr;
            header_template_t header_template;

            memcpy(&header_template, (uint8_t *)spec.header_template().c_str(),
                   std::min(sizeof(header_template_t), spec.header_template().size()));
        
            capri_hbm_write_mem((uint64_t)header_template_addr, (uint8_t *)&header_template, sizeof(header_template_t));

            HAL_TRACE_DEBUG("{}: Update: Setting header_template content to: {}", 
                            __FUNCTION__, spec.header_template());
        break;

        default:
        break;
    }
    
    // Convert and write SQCB to HBM
    pd::memrev((uint8_t*)&sqcb_p->sqcb0, sizeof(sqcb0_t));
    pd::memrev((uint8_t*)&sqcb_p->sqcb1, sizeof(sqcb1_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_SQ, spec.qp_num(), (uint8_t *)sqcb_p, sizeof(sqcb_t));

    // Convert and write RQCB to HBM
    pd::memrev((uint8_t*)&rqcb_p->rqcb0, sizeof(rqcb0_t));
    pd::memrev((uint8_t*)&rqcb_p->rqcb1, sizeof(rqcb1_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_RQ, spec.qp_num(), (uint8_t *)rqcb_p, sizeof(rqcb_t));
    
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
rdma_cq_create (RdmaCqSpec& spec, RdmaCqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint8_t      num_cq_wqes, cqwqe_size;
    cqcb_t       cqcb;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: RDMA CQ Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} "
                    " hostmem_pg_size: {} cq_lkey: {} ", __FUNCTION__, spec.cq_num(),
                    spec.cq_wqe_size(), spec.num_cq_wqes(), spec.hostmem_pg_size(),
                    spec.cq_lkey());

    cqwqe_size = roundup_to_pow_2(spec.cq_wqe_size());
    num_cq_wqes = roundup_to_pow_2(spec.num_cq_wqes());

    HAL_TRACE_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);
    
    memset(&cqcb, 0, sizeof(cqcb_t));
    // RRQ is defined as last ring in the SQCB ring array
    // We should skip scheduling for the RRQ, so set total
    //  rings as one less than max/total
    cqcb.ring_header.total_rings = MAX_CQ_RINGS - 1;
    cqcb.pt_base_addr =
        rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, spec.cq_lkey()));
    cqcb.log_cq_page_size = log2(spec.hostmem_pg_size());
    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.cq_num = spec.cq_num();
    cqcb.eq_num = spec.eq_num();

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial CQCB State, CQCB->PT: {:#x} cqcb_size: {}", 
                    __FUNCTION__, lif, cqcb.pt_base_addr, sizeof(cqcb_t));
    // Convert data before writting to HBM
    pd::memrev((uint8_t*)&cqcb, sizeof(cqcb_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_RDMA_CQ, spec.cq_num(), (uint8_t *)&cqcb, sizeof(cqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__, g_lif_manager->GetLIFQStateAddr(lif, Q_TYPE_CQ, spec.cq_num()));

    rsp->set_api_status(types::API_STATUS_OK);
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
