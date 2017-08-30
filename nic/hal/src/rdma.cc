#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <interface_svc.hpp>
#include <interface.hpp>
#include <lif_manager.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <rdma.hpp>
#include <rdma_pd.hpp>
#include <host_mem.hpp>
#include <pd/capri/capri_hbm.hpp>
#include <pd/iris/if_pd_utils.hpp>
#include <hal_state_pd.hpp>
#include <capri_loader.h>

namespace hal {

const static char *kHBMLabel = "rdma";
const static uint32_t kHBMSizeKB = 128 * 1024;  // 128 MB
const static uint32_t kAllocUnit = 4096;

uint32_t g_qp_num[MAX_LIFS];
uint32_t g_cq_num[MAX_LIFS];
uint32_t g_eq_num[MAX_LIFS];
uint32_t g_lkey_num[MAX_LIFS];
uint32_t g_rkey_num[MAX_LIFS];
uint32_t g_pt_base[MAX_LIFS];

RDMAManager *g_rdma_manager = nullptr;
::utils::HostMem  *g_host_mem = nullptr;
extern LIFManager *g_lif_manager;

#define CHECKOUT_LKEY(__lif) (++g_lkey_num[(__lif)])
#define CHECKOUT_RKEY(__lif) (R_KEY_BASE + ++g_rkey_num[(__lif)])

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

  alloc_units = (size + kAllocUnit - 1) & ~kAllocUnit;
  alloc_units /= kAllocUnit;
  int alloc_offset = hbm_allocator_->Alloc(alloc_units);
  if (alloc_offset < 0)
    return -ENOMEM;
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

hal_ret_t
rdma_sram_lif_init (uint16_t lif, sram_lif_entry_t *entry_p)
{
    hal_ret_t                    ret;

    ret = hal::pd::p4pd_common_p4plus_rxdma_stage0_lif_table1_entry_add(lif,
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
#if 0
    // TODO: Check for txdma routine
    ret = hal::pd::p4pd_common_p4plus_txdma_stage0_lif_table1_entry_add(lif,
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
#endif

    return HAL_RET_OK;
}

hal_ret_t
rdma_rx_sram_lif_entry_get (uint16_t lif, sram_lif_entry_t *entry_p)
{
    hal_ret_t                    ret;
    common_p4plus_stage0_lif_table1_actiondata data = {0};

    ret = hal::pd::p4pd_common_p4plus_rxdma_stage0_lif_table1_entry_get(lif, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for rxdma, idx : {}, err : {}",
                      lif, ret);
        return ret;
    }
    entry_p->pt_base_addr_page_id = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.pt_base_addr_page_id;
    entry_p->log_num_pt_entries = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_pt_entries;
    entry_p->cqcb_base_addr_page_id = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.cqcb_base_addr_page_id;
    entry_p->log_num_cq_entries = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_cq_entries;
    entry_p->prefetch_pool_base_addr_page_id = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.prefetch_pool_base_addr_page_id;
    entry_p->log_num_prefetch_pool_entries = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_prefetch_pool_entries;

    return HAL_RET_OK;
}

hal_ret_t
rdma_tx_sram_lif_entry_get (uint16_t lif, sram_lif_entry_t *entry_p)
{
    hal_ret_t                    ret;
    common_p4plus_stage0_lif_table1_actiondata data = {0};

    ret = hal::pd::p4pd_common_p4plus_txdma_stage0_lif_table1_entry_get(lif, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for txdma, idx : {}, err : {}",
                      lif, ret);
        return ret;
    }
    entry_p->pt_base_addr_page_id = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.pt_base_addr_page_id;
    entry_p->log_num_pt_entries = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_pt_entries;
    entry_p->cqcb_base_addr_page_id = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.cqcb_base_addr_page_id;
    entry_p->log_num_cq_entries = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_cq_entries;
    entry_p->prefetch_pool_base_addr_page_id = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.prefetch_pool_base_addr_page_id;
    entry_p->log_num_prefetch_pool_entries = data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_prefetch_pool_entries;

    return HAL_RET_OK;
}

uint32_t
rdma_lif_init (uint16_t lif, lif_init_attr_t *attr_p)
{
    LIFQStateParams params;

    sram_lif_entry_t    sram_lif_entry;
    uint32_t                 pt_size, key_table_size;
    uint32_t                 total_size;
    uint64_t                 base_addr;
    uint32_t                 max_pt_entries;
    uint32_t                 max_keys;
    uint32_t                 cq_base_addr; //address in HBM memory
    //uint32_t                 eq_base_addr; //address in HBM memory
    //uint32_t                 cq_eq_cb_size;
    uint32_t                 log_roundup_max_qps;
    hal_ret_t                rc;

    if (attr_p == NULL) {
        return -EINVAL;
    }

    HAL_TRACE_DEBUG("({},{}): LIF: {}, max_SQ: {}, max_RQ: {}, max_CQ: {}, max_EQ: {}, "
           "max_keys: {}, max_pt: {}\n",
           __FUNCTION__, __LINE__, lif,
           attr_p->max_qps, attr_p->max_qps, attr_p->max_cqs, attr_p->max_eqs,
           attr_p->max_keys, attr_p->max_pt_entries);

    if (g_lif_manager->LIFRangeAlloc(lif, 1) != lif) {
        HAL_TRACE_DEBUG("{}: LIF range alloc failed for lif: {}\n", __FUNCTION__, lif);
        return -ENOMEM;
    }

    memset(&params, 0, sizeof(LIFQStateParams));
    log_roundup_max_qps = log2(roundup_to_pow_2(attr_p->max_qps));

    // RDMA SQ
    params.type[Q_TYPE_SQ].size = log2(32);  // in multiples of 32 bytes: total 1024B
    params.type[Q_TYPE_SQ].entries = log_roundup_max_qps;
    // RDMA RQ
    params.type[Q_TYPE_RQ].size = log2(32);  // in multiples of 32 bytes: total 1024B
    params.type[Q_TYPE_RQ].entries = log_roundup_max_qps;
    // ETH TxQ
    params.type[Q_TYPE_TXQ].size = log2(2);  // in multiples of 32 bytes: total 64B
    params.type[Q_TYPE_TXQ].entries = log_roundup_max_qps;
    // AdminQ
    params.type[Q_TYPE_ADMINQ].size = log2(32);  // in multiples of 32 bytes: total 1024B
    params.type[Q_TYPE_ADMINQ].entries = log_roundup_max_qps;
    // ETH RxQ
    params.type[Q_TYPE_RXQ].size = log2(2);  // in multiples of 32 bytes: total 64B
    params.type[Q_TYPE_RXQ].entries = log_roundup_max_qps;
    // CQ
    params.type[Q_TYPE_CQ].size = log2(1);  // in multiples of 32 bytes: total 32B
    params.type[Q_TYPE_CQ].entries = log2(roundup_to_pow_2(attr_p->max_cqs));
    // EQ
    params.type[Q_TYPE_EQ].size = log2(1);  // in multiples of 32 bytes: total 32B
    params.type[Q_TYPE_EQ].entries = log2(roundup_to_pow_2(attr_p->max_eqs));
    // NOT USED
    params.type[Q_TYPE_MAX].size = log2(1);  // in multiples of 32 bytes: total 1024B
    params.type[Q_TYPE_MAX].entries = log_roundup_max_qps;

    if (g_lif_manager->InitLIFQState(lif, &params) != 0) {
        HAL_TRACE_DEBUG("{}: Init LIF QState failed for lif: {}\n", __FUNCTION__, lif);
        return -ENOMEM;
    }

    memset(&sram_lif_entry, 0, sizeof(sram_lif_entry_t));
    

    // Fill the CQ info in sram_lif_entry
    cq_base_addr = g_lif_manager->GetLIFQStateBaseAddr(lif, Q_TYPE_CQ);
    sram_lif_entry.cqcb_base_addr_page_id = cq_base_addr >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_cq_entries = log2(roundup_to_pow_2(attr_p->max_cqs));


    // Setup page table and key table entries
    max_pt_entries = roundup_to_pow_2(attr_p->max_pt_entries);

    pt_size = sizeof(uint64_t) * max_pt_entries;
    //adjust to page boundary
    if (pt_size & (HBM_PAGE_SIZE - 1)) {
        pt_size = ((pt_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    max_keys = roundup_to_pow_2(attr_p->max_keys);

    key_table_size = sizeof(key_entry_t) * attr_p->max_keys;
    //adjust to page boundary
    if (key_table_size & (HBM_PAGE_SIZE - 1)) {
        key_table_size = ((key_table_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    total_size = pt_size + key_table_size + HBM_PAGE_SIZE;

    base_addr = g_rdma_manager->HbmAlloc(total_size);
    
    HAL_TRACE_DEBUG("{}: pt_size: {}, key_table_size: {}, total_size: {}, base_addr: {}\n",
           __FUNCTION__, pt_size, key_table_size, total_size, base_addr);

    sram_lif_entry.pt_base_addr_page_id = base_addr >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_pt_entries = log2(max_keys);

    // TODO: Fill prefetch data and add corresponding code

    HAL_TRACE_DEBUG("({},{}): pt_base_addr_page_id: {}, log_num_pt: {}\n",
           __FUNCTION__, __LINE__,
           sram_lif_entry.pt_base_addr_page_id,
           sram_lif_entry.log_num_pt_entries);
           
    rc = rdma_sram_lif_init(lif, &sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
    HAL_TRACE_DEBUG("({},{}): Lif: {}: SRAM LIF INIT successful\n", __FUNCTION__, __LINE__, lif);

    HAL_TRACE_DEBUG("({},{}): Lif: {}: LIF Init successful\n", __FUNCTION__, __LINE__, lif);

    return 0;
}

void
rdma_key_entry_read (uint16_t lif, uint32_t key, key_entry_t *entry_p)
{
    sram_lif_entry_t    sram_lif_entry = {0};
    uint64_t            pt_table_base_addr;
    uint64_t            key_table_base_addr;
    hal_ret_t           rc;  

    rc = rdma_rx_sram_lif_entry_get(lif, &sram_lif_entry);
    HAL_ASSERT(rc == HAL_RET_OK);
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

void
rdma_post_send_wr (void *handle_p, sqwqe_base_t *wr_p)
{
    qp_handle_t *qp_handle_p = (qp_handle_t*)handle_p;
    uint8_t     p_index;
    sqcb_t      sqcb;

    g_lif_manager->ReadQState(qp_handle_p->lif, Q_TYPE_SQ,
        qp_handle_p->qp, (uint8_t*)&sqcb, sizeof(sqcb_t));
    // Convert data after reading from HBM
    pd::memrev((uint8_t*)&sqcb, sizeof(sqcb0_t));

    p_index = RING_P_INDEX_GET(&sqcb, SQ_RING_ID);
    memcpy((((char*)qp_handle_p->sq_p) + (p_index * qp_handle_p->sqwqe_size)),
                          wr_p, qp_handle_p->sqwqe_size);
    // TODO: Integrate doorbell request to Capri
#if 0
    doorbell_write_pindex(qp_handle_p->lif, Q_TYPE_SQ,
                          qp_handle_p->qp, SQ_RING_ID,
                          p_index + 1);
#endif
    return;
}

void
rdma_post_recv_wr (void *handle_p, void *wr_p)
{
    qp_handle_t *qp_handle_p = (qp_handle_t*)handle_p;
    uint8_t     p_index;
    rqcb_t      rqcb;

    g_lif_manager->ReadQState(qp_handle_p->lif, Q_TYPE_RQ, qp_handle_p->qp, (uint8_t*)&rqcb, sizeof(rqcb_t));
    // Convert data after reading from HBM
    pd::memrev((uint8_t*)&rqcb, sizeof(rqcb0_t));

    p_index = RING_P_INDEX_GET(&rqcb, RQ_RING_ID);
    memcpy(((char*)qp_handle_p->rq_p) + (p_index * qp_handle_p->rqwqe_size),
           wr_p, qp_handle_p->rqwqe_size);

// TODO: TEMP: For now increment the pindex so RXDMA will see the posted wqes.
// Remove this code once the following door bell code is hooked in
    RING_P_INDEX_INCREMENT(&rqcb, RQ_RING_ID);

    // Convert data before writting to HBM
    pd::memrev((uint8_t*)&rqcb, sizeof(rqcb0_t));
    g_lif_manager->WriteQState(qp_handle_p->lif, Q_TYPE_RQ, qp_handle_p->qp, (uint8_t*)&rqcb, sizeof(rqcb_t));



    // TODO: Integrate doorbell request to Capri
#if 0
    doorbell_write_pindex(qp_handle_p->lif, Q_TYPE_RQ,
                          qp_handle_p->qp, RQ_RING_ID,
                          p_index + 1);
#endif
    return;
}

void *
rdma_register_mr (uint16_t lif, mr_attr_t *attr_p)
{
    uint32_t         num_pages, num_pt_entries;
    uint32_t         lkey, rkey;
    key_entry_t lkey_entry = {0} , rkey_entry = {0};
    key_entry_t *lkey_entry_p = &lkey_entry , *rkey_entry_p = &rkey_entry; 
    uint64_t         ptr;
    uint32_t         pt_seg_size = HBM_NUM_PT_ENTRIES_PER_CACHE_LINE *
                              HOSTMEM_PAGE_SIZE;
    uint32_t         pt_seg_offset, pt_page_offset;
    uint32_t         pt_start_page_id, pt_end_page_id;
    uint32_t         transfer_bytes, pt_page_offset2;
    mr_handle_t *mr_handle_p;
            
    if (attr_p == NULL) {
        return (NULL);
    }

    if ((attr_p->acc_ctrl & ACC_CTRL_REMOTE_WRITE) &&
        !(attr_p->acc_ctrl & ACC_CTRL_LOCAL_WRITE)) {
        HAL_TRACE_DEBUG("requesting remote write without requesting "
               "local write permission is not allowed !!\n");
        return (NULL);
    }   

    if ((attr_p->acc_ctrl & ACC_CTRL_REMOTE_ATOMIC) &&
        !(attr_p->acc_ctrl & ACC_CTRL_LOCAL_WRITE)) {
        HAL_TRACE_DEBUG("requesting remote atomic without requesting "
               "local write permission is not allowed !!\n");
        return (NULL);
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

    mr_handle_p = (mr_handle_t *) malloc(sizeof(mr_handle_t));
    memset(mr_handle_p, 0, sizeof(mr_handle_t));
    
    lkey = CHECKOUT_LKEY(lif);

    rdma_key_entry_read(lif, lkey, lkey_entry_p);
    memset(lkey_entry_p, 0, sizeof(key_entry_t));
    lkey_entry_p->state = KEY_STATE_VALID;
    lkey_entry_p->acc_ctrl = (attr_p->acc_ctrl & 
                                ACC_CTRL_LOCAL_WRITE);
    lkey_entry_p->log_page_size = log2(HOSTMEM_PAGE_SIZE);
    lkey_entry_p->base_va = attr_p->va;
    lkey_entry_p->len = attr_p->len;
    lkey_entry_p->pt_base = g_pt_base[lif];
    lkey_entry_p->pd = attr_p->pd;
    // disable user key
    lkey_entry_p->flags = attr_p->flags & ~MR_FLAG_UKEY_EN;
    lkey_entry_p->type = MR_TYPE_MR;
    rdma_key_entry_write(lif, lkey, lkey_entry_p);

    if (attr_p->acc_ctrl & (ACC_CTRL_REMOTE_READ |
                            ACC_CTRL_REMOTE_WRITE |
                            ACC_CTRL_REMOTE_ATOMIC)) {
        // rkey requested
        rkey = CHECKOUT_RKEY(lif);

        rdma_key_entry_read(lif, rkey, rkey_entry_p);
        memcpy(rkey_entry_p, lkey_entry_p, sizeof(key_entry_t));
        rkey_entry_p->acc_ctrl = (attr_p->acc_ctrl & ~ACC_CTRL_LOCAL_WRITE);
        rdma_key_entry_write(lif, rkey, rkey_entry_p);
    } else {
        rkey = INVALID_KEY;
    }
    
    HAL_ASSERT(lkey_entry_p->pt_base % HBM_NUM_PT_ENTRIES_PER_CACHE_LINE == 0);

    pt_seg_offset = lkey_entry_p->base_va % pt_seg_size;
    pt_page_offset = pt_seg_offset % HOSTMEM_PAGE_SIZE;

    num_pages = 0;
    transfer_bytes = lkey_entry_p->len;
    if (pt_page_offset) {
        num_pages++;
        transfer_bytes -= (HOSTMEM_PAGE_SIZE-pt_page_offset);
    }
    pt_page_offset2 = 
        (pt_page_offset + lkey_entry_p->len)%HOSTMEM_PAGE_SIZE;
    if (pt_page_offset2) {
        num_pages++;
        transfer_bytes -= pt_page_offset2;
    }

    HAL_ASSERT(transfer_bytes % HOSTMEM_PAGE_SIZE == 0);
    num_pages += transfer_bytes / HOSTMEM_PAGE_SIZE;


    pt_start_page_id = pt_seg_offset / HOSTMEM_PAGE_SIZE;
    pt_end_page_id = pt_start_page_id + num_pages - 1;

    ptr = (uint64_t)attr_p->va & ~(HOSTMEM_PAGE_SIZE-1);
#if 0
    HAL_TRACE_DEBUG("{}: base_va: {} len: {} "
           " pt_base: {} pt_seg_offset: {} "
           "pt_start_page_id: {} pt_end_page_id: {} "
           "pt_page_offset: {} num_pages: {} ptr: {}\n",
            __FUNCTION__, lkey_entry_p->base_va, lkey_entry_p->len,
            lkey_entry_p->pt_base, pt_seg_offset,
            pt_start_page_id, pt_end_page_id, pt_page_offset,
            num_pages, ptr);
#endif

    for (uint32_t i=pt_start_page_id; i<=pt_end_page_id; i++) {
        // write the physical page pointer address into pt entry
        rdma_pt_entry_write(lif, g_pt_base[lif]+i, (uint64_t) g_host_mem->VirtToPhys((void*)ptr));
        ptr += HOSTMEM_PAGE_SIZE;
    }

    attr_p->lkey = lkey;
    attr_p->rkey = rkey;
    
    mr_handle_p->lkey_entry_p = lkey_entry_p;
    mr_handle_p->rkey_entry_p = rkey_entry_p;

    //g_pt_base[lif] += num_pages;
    num_pt_entries = ((pt_end_page_id / HBM_NUM_PT_ENTRIES_PER_CACHE_LINE)+1) * HBM_NUM_PT_ENTRIES_PER_CACHE_LINE;
    g_pt_base[lif] += num_pt_entries;

    lkey_entry_p->pt_size = num_pt_entries;
    if (rkey != INVALID_KEY) {
        rkey_entry_p->pt_size = num_pt_entries;
    }

    return (mr_handle_p);
}

#if 0
void *
rdma_qpcb_get (uint16_t lif, uint8_t q_type, uint32_t qid)
{
    hw_lif_entry_t *hw_lif_entry_p = &(hw_lif_table[lif][q_type]);
    return ((void *)hbm_addr_get(hw_lif_entry_p->qtable_base_addr) + 
            ((hw_lif_entry_p->qpcb_size) * qid));
}
#endif

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


qp_handle_t*
rdma_create_qp (uint16_t lif, qp_attr_t *attr_p)
{
    void         *sq_p, *rq_p;
    uint8_t      num_sq_wqes, num_rq_wqes;
    uint32_t     sqwqe_size, rqwqe_size = sizeof(rqwqe_base_t);
    uint32_t     sq_size, rq_size, qp_num;
    sqcb_t  sqcb;
    sqcb_t  *sqcb_p = &sqcb;
    rqcb_t  rqcb;
    rqcb_t  *rqcb_p = &rqcb;
    mr_attr_t mr_attr;
    qp_handle_t *qp_handle_p;
    uint32_t header_template_addr;
    uint64_t offset;

    HAL_TRACE_DEBUG("{}:\n", __FUNCTION__);

    qp_handle_p = (qp_handle_t *) malloc(sizeof(qp_handle_t));
    memset(qp_handle_p, 0, sizeof(qp_handle_t));

    qp_handle_p->lif = lif;
    qp_handle_p->pd = attr_p->pd;

    // allocate a qp num
    g_qp_num[lif]++;
    qp_num = g_qp_num[lif];
    qp_handle_p->qp = qp_num;

    // allocate sq and rq
    sqwqe_size = sizeof(sqwqe_base_t) + 
                 sizeof(sqwqe_send_t);
    sqwqe_size += (attr_p->num_sq_sges * sizeof(sge_t));
    sqwqe_size = roundup_to_pow_2(sqwqe_size);
    num_sq_wqes = roundup_to_pow_2(attr_p->num_sq_wqes);
    rqwqe_size += (attr_p->num_rq_sges * sizeof(sge_t));
    rqwqe_size = roundup_to_pow_2(rqwqe_size);
    num_rq_wqes = roundup_to_pow_2(attr_p->num_rq_wqes);
    qp_handle_p->sqwqe_size = sqwqe_size;
    qp_handle_p->rqwqe_size = rqwqe_size;

    HAL_TRACE_DEBUG("sqwqe_size: {} rqwqe_size: {}\n",
            qp_handle_p->sqwqe_size, 
            qp_handle_p->rqwqe_size);
    
    uint32_t sq_num_pages;
    uint32_t sq_num_wqe_per_page;
    uint32_t rq_num_pages;
    uint32_t rq_num_wqe_per_page;

    sq_num_wqe_per_page = HOSTMEM_PAGE_SIZE / sqwqe_size;
    sq_num_pages = num_sq_wqes / sq_num_wqe_per_page;
    sq_size = sq_num_pages * HOSTMEM_PAGE_SIZE;

    rq_num_wqe_per_page = HOSTMEM_PAGE_SIZE / rqwqe_size;
    rq_num_pages = num_rq_wqes / rq_num_wqe_per_page;
    rq_size = rq_num_pages * HOSTMEM_PAGE_SIZE;

    HAL_TRACE_DEBUG("sq_size: {} rq_size: {}\n",
            sq_size,
            rq_size);

    // HostMem Alloc API takes care of alignment
    HAL_TRACE_DEBUG("Before allocating Hostmem for sq_p\n");
    sq_p = g_host_mem->Alloc(sq_size);
    HAL_TRACE_DEBUG("After allocating Hostmem for sq_p\n");
    HAL_ASSERT((uint64_t)sq_p % HOSTMEM_PAGE_SIZE == 0);
    HAL_TRACE_DEBUG("Before clearing Hostmem for sq_p\n");
    memset(sq_p, 0, sq_size);
    HAL_TRACE_DEBUG("After clearing Hostmem for sq_p\n");

    rq_p = g_host_mem->Alloc(rq_size);
    HAL_ASSERT((uint64_t)rq_p % HOSTMEM_PAGE_SIZE == 0);
    memset(rq_p, 0, rq_size);
    HAL_TRACE_DEBUG("After clearing Hostmem for rq_p\n");

    qp_handle_p->sq_p = sq_p;
    qp_handle_p->rq_p = rq_p;

    HAL_TRACE_DEBUG("{}: sq_p: {} rq_p: {}\n", __FUNCTION__, sq_p, rq_p);



    // register sq
    memset(&mr_attr, 0, sizeof(mr_attr_t));
    mr_attr.pd = attr_p->pd;
    mr_attr.va = (uint64_t)sq_p;
    mr_attr.len = sq_size;
    HAL_TRACE_DEBUG("{}: LIF: {}: Register MR for SQ\n", __FUNCTION__, lif);
    rdma_register_mr(lif, &mr_attr);
    header_template_addr = g_rdma_manager->HbmAlloc(sizeof(header_template_t));
    // Fill sqcb and write to HW
    memset(sqcb_p, 0, sizeof(sqcb_t));
    // RRQ is defined as last ring in the SQCB ring array
    // We should skip scheduling for the RRQ, so set total
    //  rings as one less than max/total
    sqcb_p->sqcb0.ring_header.total_rings = MAX_SQ_RINGS - 1;
    sqcb_p->sqcb0.pt_base_addr =
        rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, mr_attr.lkey));
    sqcb_p->sqcb1.header_template_addr = header_template_addr;
    sqcb_p->sqcb1.rrq_size = attr_p->num_rrq_wqes;
    sqcb_p->sqcb1.rrq_base_addr =
        g_rdma_manager->HbmAlloc(sizeof(rrqwqe_t) * sqcb_p->sqcb1.rrq_size);
    sqcb_p->sqcb0.log_sq_page_size = log2(HOSTMEM_PAGE_SIZE);
    sqcb_p->sqcb0.log_wqe_size = log2(sqwqe_size);
    sqcb_p->sqcb0.log_num_wqes = log2(num_sq_wqes);
    sqcb_p->sqcb0.log_pmtu = log2(MAX_PMTU);
    //sqcb_p->sqcb1.cq_id = get_cqid(attr_p->sq_cq);
    sqcb_p->sqcb0.service = attr_p->service;
    sqcb_p->sqcb1.service = attr_p->service;
    sqcb_p->sqcb1.lsn = 32; // FOR now allowing 32 sq send/write_imm requests
    sqcb_p->sqcb1.ssn = 1;
    sqcb_p->sqcb1.msn = 0;
    sqcb_p->sqcb1.credits = 0xa;
    sqcb_p->sqcb0.pd = attr_p->pd;

    stage0_req_rx_prog_addr(&offset);
    sqcb_p->sqcb0.ring_header.pc = offset >> 6;

    qp_handle_p->sqcb_p = sqcb_p;
    qp_handle_p->header_template_addr = header_template_addr;
    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial SQCB State\n", __FUNCTION__, lif);
    // Convert data before writting to HBM
    pd::memrev((uint8_t*)sqcb_p, sizeof(sqcb0_t));
    g_lif_manager->WriteQState(lif, Q_TYPE_SQ, qp_num, (uint8_t *)sqcb_p, sizeof(sqcb_t));

    // register rq
    memset(&mr_attr, 0, sizeof(mr_attr_t));
    mr_attr.pd = attr_p->pd;
    mr_attr.va = (uint64_t) rq_p;
    mr_attr.len = rq_size;
    HAL_TRACE_DEBUG("{}: LIF: {}: Register MR for RQ\n", __FUNCTION__, lif);
    rdma_register_mr(lif, &mr_attr);
    // allocate rqcb
    memset(rqcb_p, 0, sizeof(rqcb_t));
    rqcb.rqcb0.ring_header.total_rings = MAX_RQ_RINGS;
    rqcb.rqcb0.pt_base_addr =
        rdma_pt_addr_get(lif, rdma_mr_pt_base_get(lif, mr_attr.lkey));
    rqcb.rqcb0.rsq_size = attr_p->num_rsq_wqes;
    rqcb.rqcb0.rsq_base_addr = g_rdma_manager->HbmAlloc(sizeof(rsqwqe_t) * rqcb.rqcb0.rsq_size);
    rqcb.rqcb0.serv_type = attr_p->service;
    rqcb.rqcb0.log_rq_page_size = log2(HOSTMEM_PAGE_SIZE);
    rqcb.rqcb0.log_wqe_size = log2(rqwqe_size);
    rqcb.rqcb0.log_num_wqes = log2(num_rq_wqes);
    rqcb.rqcb0.log_pmtu = log2(MAX_PMTU);
    rqcb.rqcb0.cache = FALSE;
    rqcb.rqcb0.pd = attr_p->pd;
    //rqcb.rqcb1.cq_id = get_cqid(attr_p->rq_cq);
    rqcb.rqcb1.header_template_addr = header_template_addr;
    rqcb.rqcb2.num_rqwqes_per_cpage = HBM_PAGE_SIZE / rqwqe_size;

    stage0_resp_rx_prog_addr(&offset);
    rqcb.rqcb0.ring_header.pc = offset >> 6;

    qp_handle_p->rqcb_p = &rqcb;

    // Convert data before writting to HBM
    pd::memrev((uint8_t*)rqcb_p, sizeof(rqcb0_t));

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writting initial RQCB State\n", __FUNCTION__, lif);
    g_lif_manager->WriteQState(lif, Q_TYPE_RQ, qp_num, (uint8_t *)rqcb_p, sizeof(rqcb_t));

 
    attr_p->qp = qp_num;
    HAL_TRACE_DEBUG("{}: Create QP successful for LIF: {}\n", __FUNCTION__, lif);
    return (qp_handle_p);
}

hal_ret_t
rdma_modify_qp (qp_handle_t *qp_handle_p, modify_qp_attr_t *attr_p)
{
    rqcb_t  rqcb = {0};
    sqcb_t  sqcb = {0};

    g_lif_manager->ReadQState(qp_handle_p->lif, Q_TYPE_RQ, qp_handle_p->qp,
                              (uint8_t*)&rqcb, sizeof(rqcb_t));
    // Convert data before reading from HBM
    pd::memrev((uint8_t*)&rqcb, sizeof(rqcb0_t));

    rqcb.rqcb1.dst_qp = attr_p->dst_qp;

    // Convert data before writting to HBM
    pd::memrev((uint8_t*)&rqcb, sizeof(rqcb0_t));
    g_lif_manager->WriteQState(qp_handle_p->lif, Q_TYPE_RQ, qp_handle_p->qp,
                               (uint8_t*)&rqcb, sizeof(rqcb_t));

    g_lif_manager->ReadQState(qp_handle_p->lif, Q_TYPE_SQ, qp_handle_p->qp,
                              (uint8_t*)&sqcb, sizeof(sqcb_t));
    // Convert data before reading from HBM
    pd::memrev((uint8_t*)&sqcb, sizeof(sqcb0_t));

    sqcb.sqcb1.dst_qp = attr_p->dst_qp;

    // Convert data before writting to HBM
    pd::memrev((uint8_t*)&sqcb, sizeof(sqcb0_t));
    g_lif_manager->WriteQState(qp_handle_p->lif, Q_TYPE_SQ, qp_handle_p->qp,
                               (uint8_t*)&sqcb, sizeof(sqcb_t));
    return HAL_RET_OK;
}


hal_ret_t
rdma_configure ()
{
    lif_init_attr_t lif_init_attr;
    qp_attr_t       qp_attr;
    //eq_attr_t       eq_attr;
    //cq_attr_t       cq1_attr;
    //cq_attr_t       cq2_attr;
    qp_handle_t     *qp_handle_p;
    //void            *cq1_handle_p;
    //void            *cq2_handle_p;
    //void            *eq_handle_p;
    rqwqe_t           wr;
    rqwqe_base_t      *rcv_wr_p = &(wr.base);
    sge_t             *sge_p;
    //uint8_t              rr[128];
    mr_attr_t       mr_attr;
    void            *buff, *buff2, *buff3, *buff4;
    mr_attr_t       mr_attr2;
    mr_attr_t       mr_attr3;
    mr_attr_t       mr_attr4;
    //rdma_reth_t     *reth_p;
    //rdma_aeth_t     *aeth_p;
    //rdma_immeth_t   *immeth_p;
    //rdma_ieth_t     *ieth_p;
    //rdma_atomicaeth_t *atomic_aeth_hdr_p;
    //rrqwqe_t        *rrqwqe_p;
    //uint32_t             e_psn = 100;
    //uint32_t             tx_psn = 2222;
    //rqcb_t          *rqcb_p;
    //cqcb_t          *cqcb_p;
    //sge_t           *sge_p;
    //sqcb_t          *sqcb_p;
    //sqwqe_base_t    *rr_base_p;
    //sqwqe_read_t    *read_p;
    //sqwqe_write_t   *write_p;
    //sqwqe_atomic_t  *atomic_p;
    //uint32_t             cq_id;
    //void            *d_p;
    //uint64_t             cmp_data = 0x1020304050607080;
    //uint64_t             swap_or_add_data = 0x0100010001000100;
    //uint64_t             orig_data;
    //rdma_atomiceth_t *atomic_hdr_p;
    //rqwqe_base_t     *rqwr_base_p;
    //sqwqe_base_t     *sqwr_base_p;
    //rsqwqe_t        *rsqwqe_p;
    //key_entry_t     *lkey_entry_p, *rkey_entry_p;
    modify_qp_attr_t modify_qp_attr;
    //sqwqe_write_t   *wr_write_p = NULL;
    //sqwqe_local_inv_t *local_inv_p;
    alloc_mw_attr_t mw_attr;
    //uint8_t              user_key;
    //sqwqe_bind_mw_t *bind_mw_p;
    //sqwqe_send_t    *wr_send_p;
    uint32_t          lif_id = 0x2;

    HAL_TRACE_DEBUG("sq: {} sqcb0: {}, sqcb1: {}, rqcb: {} rq1: {} rq2: {} cqcb: {} eqcb:{} srqcb: {} sqwqe_base: {} rqwqe_base: {} rsqwqe: {} rrqwqe: {}\n "
           "header_template: {}\n",
            sizeof(sqcb_t), sizeof(sqcb0_t), sizeof(sqcb1_t), sizeof(rqcb_t), sizeof(rqcb1_t),
            sizeof(rqcb2_t), sizeof(cqcb_t), sizeof(eqcb_t), sizeof(srqcb_t),
            sizeof(sqwqe_base_t), sizeof(rqwqe_base_t),
            sizeof(rsqwqe_t), sizeof(rrqwqe_t),
            sizeof(header_template_t));
    HAL_TRACE_DEBUG("send: {} read: {} write: {} atomic: {}\n",
            sizeof(sqwqe_send_t),
            sizeof(sqwqe_read_t),
            sizeof(sqwqe_write_t),
            sizeof(sqwqe_atomic_t));

    // Do some sanity checks
    assert(sizeof(rqcb0_t) == 64);
    assert(sizeof(rqcb1_t) == 64);
    assert(sizeof(rqcb2_t) == 64);
    assert(sizeof(sqcb0_t) == 64);
    assert(sizeof(sqcb1_t) == 64);

    memset(&lif_init_attr, 0, sizeof(lif_init_attr_t));
    lif_init_attr.max_pt_entries = MAX_PT_ENTRIES_PER_LIF;
    lif_init_attr.max_qps = 1024;
    lif_init_attr.max_cqs = 512;
    lif_init_attr.max_eqs = 64;
    lif_init_attr.max_keys = MAX_KEYS_PER_LIF;
    rdma_lif_init(lif_id, &lif_init_attr);

      
    buff = g_host_mem->Alloc(3072);
    HAL_ASSERT(buff != nullptr);
    buff2 = g_host_mem->Alloc(3072);
    HAL_ASSERT(buff2 != nullptr);
    buff3 = g_host_mem->Alloc(3072);
    HAL_ASSERT(buff3 != nullptr);
    buff4 = g_host_mem->Alloc(3072);
    HAL_ASSERT(buff4 != nullptr);

    HAL_TRACE_DEBUG("buff: {}\n", buff);

    memset(&mr_attr, 0, sizeof(mr_attr_t));
    mr_attr.pd = 1;
    mr_attr.va = (uint64_t) buff;
    mr_attr.len = 3072;
    mr_attr.acc_ctrl = ACC_CTRL_LOCAL_WRITE | ACC_CTRL_REMOTE_ATOMIC | ACC_CTRL_REMOTE_READ;
    

    rdma_register_mr(lif_id, &mr_attr);
    HAL_TRACE_DEBUG("lkey: {} rkey: {}\n", mr_attr.lkey, mr_attr.rkey);
    HAL_ASSERT(is_rkey_valid(lif_id, mr_attr.rkey) == TRUE);

    memset(&mr_attr2, 0, sizeof(mr_attr_t));
    mr_attr2.pd = 1;
    mr_attr2.va = (uint64_t) buff2;
    mr_attr2.len = sizeof(buff2);
    mr_attr2.acc_ctrl = ACC_CTRL_LOCAL_WRITE |
                        ACC_CTRL_REMOTE_WRITE |
                        ACC_CTRL_REMOTE_READ;
    mr_attr2.flags = MR_FLAG_INV_EN; // enable invalidation
    
    rdma_register_mr(lif_id, &mr_attr2);
    HAL_TRACE_DEBUG("MR2 lkey: {} rkey: {}\n", mr_attr2.lkey, mr_attr2.rkey);
    assert(is_rkey_valid(lif_id, mr_attr2.rkey) == TRUE);

    memset(&mr_attr3, 0, sizeof(mr_attr_t));
    mr_attr3.pd = 1;
    mr_attr3.va = (uint64_t) buff3;
    mr_attr3.len = sizeof(buff3);
    mr_attr3.acc_ctrl = ACC_CTRL_LOCAL_WRITE | ACC_CTRL_REMOTE_ATOMIC | ACC_CTRL_REMOTE_READ;
    mr_attr3.flags = MR_FLAG_INV_EN; // enable invalidation
    
    rdma_register_mr(lif_id, &mr_attr3);
    HAL_TRACE_DEBUG("MR3 lkey: {} rkey: {}\n", mr_attr3.lkey, mr_attr3.rkey);
    assert(is_rkey_valid(lif_id, mr_attr3.rkey) == TRUE);

    memset(&mr_attr4, 0, sizeof(mr_attr_t));
    mr_attr4.pd = 1;
    mr_attr4.va = (uint64_t) buff4;
    mr_attr4.len = sizeof(buff4);
    mr_attr4.acc_ctrl = ACC_CTRL_LOCAL_WRITE;
    mr_attr4.flags = MR_FLAG_MW_EN; // enable invalidation
    
    rdma_register_mr(lif_id, &mr_attr4);
    HAL_TRACE_DEBUG("MR4 lkey: {} rkey: {}\n", mr_attr4.lkey, mr_attr4.rkey);
    assert(is_rkey_valid(lif_id, mr_attr4.rkey) == FALSE);

    memset(&mw_attr, 0, sizeof(alloc_mw_attr_t));
    mw_attr.pd = 1;
    mw_attr.type = MW_TYPE_2B;
   
#if 0 
    alloc_mw(lif_id, &mw_attr);
    HAL_TRACE_DEBUG("MW rkey: {}\n", mw_attr.rkey);
    rkey_entry_p = key_entry_get(lif_id, mw_attr.rkey);
    assert(rkey_entry_p->state == KEY_STATE_FREE);

    // Create Interrupt array for all EQs
	// HW has 4K interrupts, allocate 4K interrupt array
    create_eq_int();
    HAL_TRACE_DEBUG("EQ Interrupt table ptr: {}\n", eq_int_table);

    // Create EQ and store EQ handle in CQs
    memset(&eq_attr, 0, sizeof(eq_attr_t));
    eq_attr.pd = 1;
    eq_attr.num_eq_entries = 64;
    eq_handle_p = create_eq(lif_id, &eq_attr);
    HAL_TRACE_DEBUG("EQ handle ptr: {}\n", eq_handle_p);


    memset(&cq1_attr, 0, sizeof(cq_attr_t));
    cq1_attr.pd = 1;
    cq1_attr.num_cq_entries = 64;
    cq1_attr.eq_enabled = 0;
    cq1_attr.eq = ((eq_handle_t*)eq_handle_p)->eq;
    cq1_attr.eq_handle_p = eq_handle_p;
    cq1_attr.arm = 0;
    cq1_handle_p = create_cq(lif_id, &cq1_attr);

    memset(&cq2_attr, 0, sizeof(cq_attr_t));
    cq2_attr.pd = 1;
    cq2_attr.num_cq_entries = 64;
    cq2_attr.eq_enabled = 0;
    cq2_attr.eq = ((eq_handle_t*)eq_handle_p)->eq;
    cq2_attr.eq_handle_p = eq_handle_p;
    cq2_attr.arm = 0;
    cq2_handle_p = create_cq(lif_id, &cq2_attr);
#endif

    memset(&qp_attr, 0, sizeof(qp_attr_t));
    qp_attr.num_sq_sges = 2;
    qp_attr.num_rq_sges = 2;
    qp_attr.atomic_enabled = 1;
    qp_attr.service = RDMA_SERV_TYPE_RC;
    qp_attr.pd = 1;
    qp_attr.num_sq_wqes = 64;
    qp_attr.num_rq_wqes = 64;
    qp_attr.num_rsq_wqes = 16;
    qp_attr.num_rrq_wqes = 16;
    //qp_attr.sq_cq = cq1_handle_p;
    //qp_attr.rq_cq = cq2_handle_p;
    qp_attr.service = RDMA_SERV_TYPE_RC;
    HAL_TRACE_DEBUG("{}: create_qp call: qp_num: {}\n", qp_attr.qp, __FUNCTION__);
    qp_handle_p = rdma_create_qp(lif_id, &qp_attr);
    HAL_TRACE_DEBUG("post create_qp call: qp_num: {}\n", qp_attr.qp);

    memset(&modify_qp_attr, 0, sizeof(modify_qp_attr_t));
    modify_qp_attr.dst_qp = 555;
    rdma_modify_qp(qp_handle_p, &modify_qp_attr);

#if 0
    header_template_t *template_p = 
        (header_template_t *)QP_HEADER_TEMPLATE_GET(qp_handle_p);
    HAL_TRACE_DEBUG("template: 0x{}\n", template_p);
    uint8_t smac[MAC_SIZE]={1,1,1,1,1,1};
    uint8_t dmac[MAC_SIZE]={2,2,2,2,2,2};
    memcpy(&template_p->eth.dmac, dmac, MAC_SIZE);
    memcpy(&template_p->eth.smac, smac, MAC_SIZE);
    template_p->eth.ethertype = 0x8100;
    template_p->vlan.pri = 5;
    template_p->vlan.vlan = 1234;
    template_p->vlan.ethertype = 0x800;
    template_p->ip.version = 4;
    template_p->ip.ihl = 5;
    template_p->ip.tos = 13 << 2;
    template_p->ip.ttl = 255;
    template_p->ip.protocol = 17;
    template_p->ip.saddr = 0x01020304;
    template_p->ip.daddr = 0x05060708;
    template_p->udp.sport = 1111;
    template_p->udp.dport = 2222;
#endif

    HAL_TRACE_DEBUG("==========================================================\n");
    HAL_TRACE_DEBUG("END OF INIT\n");
    HAL_TRACE_DEBUG("==========================================================\n\n");

    //SEND_ONLY
    HAL_TRACE_DEBUG("\n\n\nTrying SEND_ONLY\n");
    //wr_base_p->wrid++;
    memset(&wr, 0, sizeof(wr));
    rcv_wr_p->wrid = 0x0102030405060708;
    rcv_wr_p->num_sges = 1;

    sge_p = &(wr.sge0);
    sge_p->va = mr_attr.va;
    sge_p->len = 3072;
    sge_p->l_key = mr_attr.lkey;
    HAL_TRACE_DEBUG("Posting RQ WR: sge va: {} len: {} l_key: {}\n", sge_p->va, sge_p->len, sge_p->l_key);
    

    pd::memrev((uint8_t*)&wr, sizeof(wr)); 
    rdma_post_recv_wr(qp_handle_p, (void *) &wr);


    // Inject packet into model

    // Verify the data received in the rqwqe posted above

#if 0
    rocev2_pkt_p->bth.opcode = RDMA_PKT_OPC_SEND_ONLY;
    rocev2_pkt_p->bth.dst_qp = qp_attr.qp;
    rocev2_pkt_p->bth.psn = e_psn++;
    memset(rocev2_pkt_p->rest, 3, 1024);

    rx_pipeline((uint8_t *)rocev2_pkt_p, 1024);
    pkt_list_p = tx_pipeline();
    print_pkt_list(pkt_list_p);
    
    if (memcmp(rocev2_pkt_p->rest, buff, 1024) == 0) {
        HAL_TRACE_DEBUG("DMA contents verification successful !!\n");
    } else {
        HAL_TRACE_DEBUG("DMA contents verification failed !!\n");
        assert(0);
    }
#endif
    return HAL_RET_OK;
}

hal_ret_t
rdma_hal_init()
{
    HAL_TRACE_DEBUG("{}: Entered\n", __FUNCTION__);
    g_rdma_manager = new RDMAManager();
    g_host_mem = ::utils::HostMem::New();
    HAL_ASSERT(g_host_mem != nullptr);

    if (rdma_configure() != HAL_RET_OK) {
        HAL_ASSERT(0);
    }
    HAL_TRACE_DEBUG("{}: Leaving\n", __FUNCTION__);
    return HAL_RET_OK;
}


}    // namespace hal
