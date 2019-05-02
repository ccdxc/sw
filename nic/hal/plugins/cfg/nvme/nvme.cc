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
#include "nic/hal/plugins/cfg/nvme/nvme.hpp"
#include "nic/utils/host_mem/host_mem.hpp"
#include "nic/p4/common/defines.h"
#include "nic/hal/plugins/cfg/mcast/oif_list_api.hpp"
#include "nic/sdk/nvme/nvme_common.h"
#include "nvme_dpath.hpp"
#include "nic/hal/src/internal/wring.hpp"

namespace hal {

#define NVME_TX_SESS_XTSQ_ENTRIES 64
#define NVME_TX_SESS_XTSQ_ENTRY_SIZE 4
#define NVME_TX_SESS_XTSQ_SIZE (NVME_TX_SESS_XTSQ_ENTRY_SIZE * NVME_TX_SESS_XTSQ_ENTRIES)

#define NVME_TX_SESS_DGSTQ_ENTRIES 64
#define NVME_TX_SESS_DGSTQ_ENTRY_SIZE 4
#define NVME_TX_SESS_DGSTQ_SIZE (NVME_TX_SESS_DGSTQ_ENTRY_SIZE * NVME_TX_SESS_DGSTQ_ENTRIES)

#define NVME_RX_SESS_XTSQ_ENTRIES 64
#define NVME_RX_SESS_XTSQ_ENTRY_SIZE 4
#define NVME_RX_SESS_XTSQ_SIZE (NVME_RX_SESS_XTSQ_ENTRY_SIZE * NVME_RX_SESS_XTSQ_ENTRIES)

#define NVME_RX_SESS_DGSTQ_ENTRIES 64
#define NVME_RX_SESS_DGSTQ_ENTRY_SIZE 4
#define NVME_RX_SESS_DGSTQ_SIZE (NVME_RX_SESS_DGSTQ_ENTRY_SIZE * NVME_RX_SESS_DGSTQ_ENTRIES)

const static uint32_t kAllocUnit = 4096;

typedef struct nvme_global_info_s {
    uint32_t max_ns;
    uint32_t cur_ns;
    uint64_t nscb_base_addr;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint64_t txsessprodcb_base;
    uint64_t rxsessprodcb_base;
    uint64_t tx_sess_xtsq_base;
    uint64_t tx_sess_dgstq_base;
    uint64_t rx_sess_xtsq_base;
    uint64_t rx_sess_dgstq_base;
    uint64_t sess_bitmap_addr;
    uint32_t max_cmd_context;
    uint64_t cmd_context_page_base;
    uint64_t cmd_context_ring_base;
    uint32_t tx_max_aol;
    uint64_t tx_aol_page_base;
    uint64_t tx_aol_ring_base;
    uint64_t tx_nmdpr_ring_base;
    uint64_t tx_nmdpr_ring_size;
    uint32_t rx_max_aol;
    uint64_t rx_aol_page_base;
    uint64_t rx_aol_ring_base;
    uint64_t rx_nmdpr_ring_base;
    uint64_t rx_nmdpr_ring_size;
    uint64_t tx_resourcecb_addr;
    uint64_t rx_resourcecb_addr;

    //tuning params
    uint16_t sess_q_depth;
} nvme_global_info_t;

typedef struct nvme_lif_info_s {
    uint32_t max_ns;
    uint64_t nscb_base_addr;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint64_t sess_bitmap_addr;
    uint32_t sess_start;
    uint32_t max_cq;
    uint64_t cqcb_base_addr;
    uint32_t max_sq;
    uint64_t sqcb_base_addr;
    uint32_t log_host_page_size;
    uint32_t ns_start;
} nvme_lif_info_t;

typedef struct nvme_ns_info_s {
    uint32_t sess_start;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint32_t key_index;
    uint32_t sec_key_index;
} nvme_ns_info_t;

static nvme_global_info_t g_nvme_global_info;
static nvme_lif_info_t g_nvme_lif_info[MAX_LIFS];
static nvme_ns_info_t *g_nvme_ns_info = NULL;


//NVMEManager *g_nvme_manager = nullptr;
// extern sdk::platform::capri::LIFManager *lif_manager();
extern lif_mgr *lif_manager();

NVMEManager::NVMEManager() {
  sdk::platform::utils::mpartition *mp = lif_manager()->get_mpartition();
  uint64_t hbm_addr = mp->start_addr("nvme");
  uint32_t kHBMSizeKB = mp->size("nvme") >> 10;
  uint32_t num_units = (kHBMSizeKB * 1024) / kAllocUnit;

  // Minimum 128 MB
  //SDK_ASSERT(kHBMSizeKB >= (128 * 1024));

  if (hbm_addr & 0xFFF) {
    // Not 4K aligned.
    hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
    num_units--;
  }
  hbm_base_ = hbm_addr;
  hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));
  HAL_TRACE_DEBUG("{}: hbm_base_ : {}\n", __FUNCTION__, hbm_base_);
}

uint64_t 
NVMEManager::HbmAlloc(uint32_t size) {
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
    HAL_TRACE_DEBUG("{}: size: {:#x} alloc_offset: {:#x} hbm_addr: {:#x}\n",
                     __FUNCTION__, size, alloc_offset, hbm_base_ + alloc_offset);
  return hbm_base_ + alloc_offset;
}

hal_ret_t 
nvme_hbm_write (uint64_t dst_addr, void *src_addr, uint16_t size)
{
    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.addr = dst_addr;
    args.buf = (uint8_t *)src_addr;
    args.size = size;
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);

    HAL_TRACE_DEBUG("({}, {}): Writing from: {:#x} to: {:#x} of size: {}",
                    __FUNCTION__, __LINE__, src_addr, dst_addr, size);
    return (HAL_RET_OK);
}

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

hal_ret_t
nvme_cmd_context_ring_entry_write (uint16_t index, 
                                   uint16_t cmd_id)
{
    uint64_t            base_addr;

    SDK_ASSERT(index < g_nvme_global_info.max_cmd_context);

    base_addr = g_nvme_global_info.cmd_context_ring_base;

    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.addr = (uint64_t)(base_addr + (index * sizeof(nvme_cmd_context_ring_entry_t)));
    args.buf = (uint8_t*)&cmd_id;
    args.size = sizeof(nvme_cmd_context_ring_entry_t);
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);

    HAL_TRACE_DEBUG("({}, {}): Writing cmd_context_ring[{}] = {}",
                    __FUNCTION__, __LINE__, index, cmd_id);
    return (HAL_RET_OK);
}

hal_ret_t
nvme_enable (NvmeEnableRequest& spec, NvmeEnableResponse *rsp)
{
    uint32_t            max_ns;
    uint32_t            max_sess;
    uint32_t            max_cmd_context;
    uint32_t            tx_max_aol;
    uint32_t            rx_max_aol;
    uint32_t            total_size;
    uint64_t            base_addr;
    uint64_t            iter_addr;
    uint32_t            index;
    hal_ret_t           ret;
    wring_t             wring;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
 
    HAL_TRACE_DEBUG("({}, {}): NVME Enable Request",
                    __FUNCTION__, __LINE__);

    max_ns  = spec.max_ns();
    max_sess  = spec.max_sess();
    max_cmd_context  = spec.max_cmd_context();
    tx_max_aol  = spec.tx_max_aol();
    rx_max_aol  = spec.rx_max_aol();

    SDK_ASSERT(max_cmd_context <= (1 << 16));
    SDK_ASSERT(tx_max_aol <= (1 << 16));
    SDK_ASSERT(rx_max_aol <= (1 << 16));

    HAL_TRACE_DEBUG("{}: max_ns: {}, max_sess: {}, max_cmd_context: {}, "
                    "tx_max_aol: {}, rx_max_aol: {}\n",
                   __FUNCTION__, max_ns, max_sess, max_cmd_context, 
                     tx_max_aol, rx_max_aol);

    //1st HBM alloc
    total_size = sizeof(nvme_nscb_t) * max_ns +
                 sizeof(nvme_txsessprodcb_t) * max_sess +
                 sizeof(nvme_rxsessprodcb_t) * max_sess +
                 NVME_TX_SESS_XTSQ_SIZE * max_sess +
                 NVME_TX_SESS_DGSTQ_SIZE * max_sess +
                 NVME_RX_SESS_XTSQ_SIZE * max_sess +
                 NVME_RX_SESS_DGSTQ_SIZE * max_sess +
                 sizeof(nvme_resourcecb_t) * 2 +
                 max_sess / 8 /* bitmap */;

    base_addr = nvme_manager()->HbmAlloc(total_size);

    memset(&g_nvme_global_info, 0, sizeof(g_nvme_global_info));

    g_nvme_global_info.max_ns = max_ns;
    g_nvme_global_info.max_sess = max_sess;
    g_nvme_global_info.max_cmd_context = max_cmd_context;
    g_nvme_global_info.tx_max_aol = tx_max_aol;
    g_nvme_global_info.rx_max_aol = rx_max_aol;

    iter_addr = base_addr;

    //ns
    g_nvme_global_info.nscb_base_addr = iter_addr;
    iter_addr += sizeof(nvme_nscb_t) * max_ns;

    //txsessprodcb
    g_nvme_global_info.txsessprodcb_base = iter_addr;
    iter_addr += sizeof(nvme_txsessprodcb_t) * max_sess;

    //rxsessprodcb
    g_nvme_global_info.rxsessprodcb_base = iter_addr;
    iter_addr += sizeof(nvme_rxsessprodcb_t) * max_sess;

    //tx_sess_xtsq
    g_nvme_global_info.tx_sess_xtsq_base = iter_addr;
    iter_addr += NVME_TX_SESS_XTSQ_SIZE * max_sess;

    //tx_sess_dgstq
    g_nvme_global_info.tx_sess_dgstq_base = iter_addr;
    iter_addr += NVME_TX_SESS_DGSTQ_SIZE * max_sess;

    //rx_sess_xtsq
    g_nvme_global_info.rx_sess_xtsq_base = iter_addr;
    iter_addr += NVME_RX_SESS_XTSQ_SIZE * max_sess;

    //rx_sess_dgstq
    g_nvme_global_info.rx_sess_dgstq_base = iter_addr;
    iter_addr += NVME_RX_SESS_DGSTQ_SIZE * max_sess;

    //tx_resourcecb
    g_nvme_global_info.tx_resourcecb_addr = iter_addr;
    iter_addr += sizeof(nvme_resourcecb_t);

    //rx_resourcecb
    g_nvme_global_info.rx_resourcecb_addr = iter_addr;
    iter_addr += sizeof(nvme_resourcecb_t);

    g_nvme_global_info.sess_bitmap_addr = iter_addr;
    iter_addr += max_sess / 8;

    HAL_TRACE_DEBUG("{}: base_addr: {:#x}, nscb_base_addr: {:#x} "
                    "txsessprodcb_base: {:#x}, "
                    "tx_sess_xtsq_base: {:#x}, "
                    "tx_sess_dgstq_base: {:#x}, "
                    "tx_resourcecb_addr: {:#x}, "
                    "rxsessprodcb_base: {:#x}, "
                    "rx_sess_xtsq_base: {:#x}, "
                    "rx_sess_dgstq_base: {:#x}, "
                    "rx_resourcecb_addr: {:#x}, "
                    "sess_bitmap_addr: {:#x}\n",
                   __FUNCTION__, base_addr, g_nvme_global_info.nscb_base_addr, 
                    g_nvme_global_info.txsessprodcb_base,
                    g_nvme_global_info.tx_sess_xtsq_base,
                    g_nvme_global_info.tx_sess_dgstq_base,
                    g_nvme_global_info.tx_resourcecb_addr,
                    g_nvme_global_info.rxsessprodcb_base,
                    g_nvme_global_info.rx_sess_xtsq_base,
                    g_nvme_global_info.rx_sess_dgstq_base,
                    g_nvme_global_info.rx_resourcecb_addr,
                    g_nvme_global_info.sess_bitmap_addr);


    //2nd HBM alloc
    total_size = sizeof(nvme_cmd_context_t) * max_cmd_context + 
                 sizeof(nvme_cmd_context_ring_entry_t) * max_cmd_context +
                 sizeof(nvme_aol_t) * tx_max_aol + 
                 sizeof(nvme_aol_ring_entry_t) * tx_max_aol +
                 sizeof(nvme_aol_t) * rx_max_aol + 
                 sizeof(nvme_aol_ring_entry_t) * rx_max_aol;

    base_addr = nvme_manager()->HbmAlloc(total_size);

    iter_addr = base_addr;

    //cmd_context_page
    g_nvme_global_info.cmd_context_page_base = iter_addr;
    iter_addr += sizeof(nvme_cmd_context_t) * max_cmd_context;

    //cmd_context_ring
    g_nvme_global_info.cmd_context_ring_base = iter_addr;
    iter_addr += sizeof(nvme_cmd_context_ring_entry_t) * max_cmd_context;

    //tx_aol_page
    g_nvme_global_info.tx_aol_page_base = iter_addr;
    iter_addr += sizeof(nvme_aol_t) * tx_max_aol;

    //tx_aol_ring
    g_nvme_global_info.tx_aol_ring_base = iter_addr;
    iter_addr += sizeof(nvme_aol_ring_entry_t) * tx_max_aol;

    //rx_aol_page
    g_nvme_global_info.rx_aol_page_base = iter_addr;
    iter_addr += sizeof(nvme_aol_t) * rx_max_aol;

    //rx_aol_ring
    g_nvme_global_info.rx_aol_ring_base = iter_addr;
    iter_addr += sizeof(nvme_aol_ring_entry_t) * rx_max_aol;


    g_nvme_ns_info = (nvme_ns_info_t *)malloc(sizeof(nvme_ns_info_t) * max_ns);
    SDK_ASSERT(g_nvme_ns_info != NULL);

    HAL_TRACE_DEBUG("{}: cmd_context_page_base: {:#x}, "
                    "g_nvme_ns_info: {:#x}, " 
                    "cmd_context_ring_base: {:#x}, total_size: {}, "
                    "tx_aol_page_base: {:#x}, tx_aol_ring_base: {:#x}, "
                    "rx_aol_page_base: {:#x}, rx_aol_ring_base: {:#x}",
                    __FUNCTION__, g_nvme_global_info.cmd_context_page_base,
                    (uint64_t)g_nvme_ns_info,
                    g_nvme_global_info.cmd_context_ring_base, total_size,
                    g_nvme_global_info.tx_aol_page_base, 
                    g_nvme_global_info.tx_aol_ring_base,
                    g_nvme_global_info.rx_aol_page_base, 
                    g_nvme_global_info.rx_aol_ring_base);

    //Fill the ring with cmd context page addresses
    for (index = 0; index < max_cmd_context; index++) {
        nvme_cmd_context_ring_entry_write(index, index);
    }

    rsp->set_cmd_context_page_base(g_nvme_global_info.cmd_context_page_base);
    rsp->set_cmd_context_ring_base(g_nvme_global_info.cmd_context_ring_base);

    // Get TX_NMDPR_RING_BASE
    ret = wring_get_meta(types::WRING_TYPE_NMDPR_BIG_TX,
                         0,
                         &wring);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive NMDPR ring base");
        return HAL_RET_ERR;
    } else {
        g_nvme_global_info.tx_nmdpr_ring_base = wring.phys_base_addr;
        g_nvme_global_info.tx_nmdpr_ring_size = wring.num_entries;
        HAL_TRACE_DEBUG("tx_nmdpr_ring_base: {:#x}, size: {}", 
                        g_nvme_global_info.tx_nmdpr_ring_base, 
                        g_nvme_global_info.tx_nmdpr_ring_size);
    }

    // Get RX_NMDPR_RING_BASE
    ret = wring_get_meta(types::WRING_TYPE_NMDPR_BIG_RX,
                         0,
                         &wring);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive NMDPR ring base");
        return HAL_RET_ERR;
    } else {
        g_nvme_global_info.rx_nmdpr_ring_base = wring.phys_base_addr;
        g_nvme_global_info.rx_nmdpr_ring_size = wring.num_entries;
        HAL_TRACE_DEBUG("rx_nmdpr_ring_base: {:#x}, size: {}", 
                        g_nvme_global_info.rx_nmdpr_ring_base,
                        g_nvme_global_info.rx_nmdpr_ring_size);
    }


    //Setup tx_resourcecb
    nvme_resourcecb_t tx_resourcecb;

    memset(&tx_resourcecb, 0, sizeof(nvme_resourcecb_t));
    tx_resourcecb.cmdid_ring_log_sz = log2(max_cmd_context);
    tx_resourcecb.cmdid_ring_ci = 0;
    tx_resourcecb.cmdid_ring_pi = 0;
    tx_resourcecb.cmdid_ring_proxy_ci = 0;

    tx_resourcecb.aol_ring_log_sz = log2(tx_max_aol);
    tx_resourcecb.aol_ring_ci = 0;
    tx_resourcecb.aol_ring_pi = 0;
    tx_resourcecb.aol_ring_proxy_ci = 0;

    tx_resourcecb.page_ring_log_sz = log2(g_nvme_global_info.tx_nmdpr_ring_size);
    tx_resourcecb.page_ring_ci = 0;
    tx_resourcecb.page_ring_pi = 0;
    tx_resourcecb.page_ring_proxy_ci = 0;

    nvme_hbm_write(g_nvme_global_info.tx_resourcecb_addr, (void *)&tx_resourcecb, sizeof(nvme_resourcecb_t));

    //Setup rx_resourcecb
    nvme_resourcecb_t rx_resourcecb;

    memset(&rx_resourcecb, 0, sizeof(nvme_resourcecb_t));
    rx_resourcecb.cmdid_ring_log_sz = log2(max_cmd_context);
    rx_resourcecb.cmdid_ring_ci = 0;
    rx_resourcecb.cmdid_ring_pi = 0;
    rx_resourcecb.cmdid_ring_proxy_ci = 0;

    rx_resourcecb.aol_ring_log_sz = log2(rx_max_aol);
    rx_resourcecb.aol_ring_ci = 0;
    rx_resourcecb.aol_ring_pi = 0;
    rx_resourcecb.aol_ring_proxy_ci = 0;

    rx_resourcecb.page_ring_log_sz = log2(g_nvme_global_info.rx_nmdpr_ring_size);
    rx_resourcecb.page_ring_ci = 0;
    rx_resourcecb.page_ring_pi = 0;
    rx_resourcecb.page_ring_proxy_ci = 0;

    nvme_hbm_write(g_nvme_global_info.rx_resourcecb_addr, (void *)&rx_resourcecb, sizeof(nvme_resourcecb_t));

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
nvme_lif_init (intf::LifSpec& spec, uint32_t lif)
{
    uint32_t            max_ns, max_sess;
    uint32_t            max_cqs, max_sqs;
    uint64_t            cqcb_base_addr; //address in HBM memory
    uint64_t            sqcb_base_addr; //address in HBM memory
    nvme_lif_info_t     *nvme_lif_info_p;

    HAL_TRACE_DEBUG("({}, {}): cur_ns: {}, max_ns: {}, cur_sess: {}, max_sess: {}\n",
                    __FUNCTION__, __LINE__,
                    g_nvme_global_info.cur_ns, g_nvme_global_info.max_ns,
                    g_nvme_global_info.cur_sess, g_nvme_global_info.max_sess);

    HAL_TRACE_DEBUG("lif: {} spec_lif: {}", lif, spec.key_or_handle().lif_id());

    //SDK_ASSERT(lif == spec.key_or_handle().lif_id());
    SDK_ASSERT(lif < MAX_LIFS);

    max_ns = spec.nvme_max_ns();
    max_sess = spec.nvme_max_sess();

    HAL_TRACE_DEBUG("({},{}): LIF: {}, max_ns: {}, max_sess: {}",
                    __FUNCTION__, __LINE__, lif, max_ns, max_sess);

    SDK_ASSERT((g_nvme_global_info.cur_ns + max_ns) < g_nvme_global_info.max_ns);
    SDK_ASSERT((g_nvme_global_info.cur_sess + max_sess) < g_nvme_global_info.max_sess);

    nvme_lif_info_p = &g_nvme_lif_info[lif];

    memset(nvme_lif_info_p, 0, sizeof(nvme_lif_info_t));

    // LIFQState *qstate = lif_manager()->GetLIFQState(lif);
    lif_qstate_t *qstate = lif_manager()->get_lif_qstate(lif);
    if (qstate == nullptr)
        return HAL_RET_ERR;

    max_cqs  = qstate->type[NVME_QTYPE_CQ].num_queues;
    max_sqs  = qstate->type[NVME_QTYPE_SQ].num_queues;

    HAL_TRACE_DEBUG("({},{}): LIF {}, max_CQ: {}, max_SQ: {}",
                    __FUNCTION__, __LINE__, lif, max_cqs, max_sqs);

    nvme_lif_info_p->nscb_base_addr = g_nvme_global_info.nscb_base_addr + 
                                          g_nvme_global_info.cur_ns * sizeof(nvme_nscb_t);
    nvme_lif_info_p->ns_start = g_nvme_global_info.cur_ns;
    g_nvme_global_info.cur_ns += max_ns;
    
    nvme_lif_info_p->sess_start = g_nvme_global_info.cur_sess;
    g_nvme_global_info.cur_sess += max_sess;
    
    nvme_lif_info_p->sess_bitmap_addr = g_nvme_global_info.sess_bitmap_addr + 
                                            BITS_TO_BYTES(g_nvme_global_info.cur_sess);

    nvme_lif_info_p->max_ns = max_ns;
    nvme_lif_info_p->max_sess = max_sess;
    nvme_lif_info_p->cur_sess = 0;

    cqcb_base_addr = lif_manager()->get_lif_qstate_base_addr(lif, NVME_QTYPE_CQ);
    HAL_TRACE_DEBUG("({},{}): Lif {} cqcb_base_addr: {:#x}, max_cqs: {} log_max_cq_entries: {}",
           __FUNCTION__, __LINE__, lif, cqcb_base_addr,
           max_cqs, log2(roundup_to_pow_2(max_cqs)));
    nvme_lif_info_p->cqcb_base_addr = cqcb_base_addr;
    nvme_lif_info_p->max_cq = max_cqs;

    sqcb_base_addr = lif_manager()->get_lif_qstate_base_addr(lif, NVME_QTYPE_SQ);
    HAL_TRACE_DEBUG("({},{}): Lif {} sqcb_base_addr: {:#x}, max_sqs: {} log_max_sq_entries: {}",
           __FUNCTION__, __LINE__, lif, sqcb_base_addr,
           max_sqs, log2(roundup_to_pow_2(max_sqs)));
    nvme_lif_info_p->sqcb_base_addr = sqcb_base_addr;
    nvme_lif_info_p->max_sq = max_sqs;

    SDK_ASSERT((spec.nvme_host_page_size() & (spec.nvme_host_page_size() - 1)) == 0);
    nvme_lif_info_p->log_host_page_size = log2(spec.nvme_host_page_size());

    HAL_TRACE_DEBUG("({},{}): Lif: {}: max_sq: {}, sqcb_base_addr: {:#x}, "
                    "max_cq: {}, cqcb_base_addr: {:#x}, "
                    "max_ns: {}, nscb_base_addr: {:#x}, ns_start: {}, "
                    "max_sess: {}, sess_start: {} "
                    "sess_bitmap_addr: {}, log_host_page_size: {}",
                    __FUNCTION__, __LINE__, lif,
                    nvme_lif_info_p->max_sq, 
                    nvme_lif_info_p->sqcb_base_addr,
                    nvme_lif_info_p->max_cq, 
                    nvme_lif_info_p->cqcb_base_addr,
                    nvme_lif_info_p->max_ns, 
                    nvme_lif_info_p->nscb_base_addr,
                    nvme_lif_info_p->ns_start,
                    nvme_lif_info_p->max_sess, 
                    nvme_lif_info_p->sess_start, 
                    nvme_lif_info_p->sess_bitmap_addr,
                    nvme_lif_info_p->log_host_page_size);

    HAL_TRACE_DEBUG("({},{}): Lif: {}: LIF Init successful\n", __FUNCTION__, __LINE__, lif);

    return HAL_RET_OK;
}

/*
 * Utility functions to read/write PT entries.
 * Ideally, HAL should never program PT entries.
 */
void
nvme_ns_entry_write (uint16_t lif, uint32_t index, uint64_t *ns_ptr)
{
    uint64_t            ns_table_base_addr;

    SDK_ASSERT(lif < MAX_LIFS);

    ns_table_base_addr = g_nvme_lif_info[lif].nscb_base_addr;

    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.addr = (uint64_t)(ns_table_base_addr + (index * sizeof(nvme_nscb_t)));
    args.buf = (uint8_t*)ns_ptr;
    args.size = sizeof(nvme_nscb_t);
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);
}

void
nvme_ns_entry_read (uint16_t lif, uint32_t index, uint64_t *ns_ptr)
{
    uint64_t            ns_table_base_addr;

    SDK_ASSERT(lif < MAX_LIFS);

    ns_table_base_addr = g_nvme_lif_info[lif].nscb_base_addr;

    pd::pd_capri_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.addr = (uint64_t)(ns_table_base_addr + (index * sizeof(nvme_nscb_t)));
    args.buf = (uint8_t*)ns_ptr;
    args.size = sizeof(nvme_nscb_t);
    pd_func_args.pd_capri_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_READ, &pd_func_args);
}

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

static u_int64_t
intr_assert_addr(const int intr)
{
    return INTR_ASSERT_BASE + (intr * INTR_ASSERT_STRIDE);
}


hal_ret_t
nvme_sq_create (NvmeSqSpec& spec, NvmeSqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint32_t     num_sq_wqes, sqwqe_size;
    nvme_sqcb_t       sqcb;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: NVME SQ Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: sq_num: {} sq_wqe_size: {} num_sq_wqes: {} "
                    "base_addr: {} cq_num: {}",
                    __FUNCTION__, spec.sq_num(),
                    spec.sq_wqe_size(), spec.num_sq_wqes(), 
                    spec.base_addr(), spec.cq_num());

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.sq_num() < g_nvme_lif_info[lif].max_sq);
    SDK_ASSERT(spec.cq_num() < g_nvme_lif_info[lif].max_cq);

    sqwqe_size = roundup_to_pow_2(spec.sq_wqe_size());
    num_sq_wqes = roundup_to_pow_2(spec.num_sq_wqes());

    HAL_TRACE_DEBUG("sqwqe_size: {} num_sqwqes: {}", sqwqe_size, num_sq_wqes);

    memset(&sqcb, 0, sizeof(nvme_sqcb_t));
    sqcb.ring_header.total_rings = MAX_SQ_RINGS;
    sqcb.ring_header.host_rings = MAX_SQ_HOST_RINGS;

    sqcb.base_addr = spec.base_addr();

    sqcb.log_wqe_size = log2(sqwqe_size);
    sqcb.log_num_wqes = log2(num_sq_wqes);
    sqcb.log_host_page_size = g_nvme_lif_info[lif].log_host_page_size;
    sqcb.cq_id = spec.cq_num();

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial SQCB State, baseaddr: {:#x} sqcb_size: {}",
                    __FUNCTION__, lif, sqcb.base_addr, sizeof(nvme_sqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&sqcb, sizeof(nvme_sqcb_t));
    lif_manager()->write_qstate(lif, NVME_QTYPE_SQ, spec.sq_num(), (uint8_t *)&sqcb, sizeof(nvme_sqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__,
                    lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_SQ, spec.sq_num()));

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
nvme_cq_create (NvmeCqSpec& spec, NvmeCqResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    uint32_t     num_cq_wqes, cqwqe_size;
    nvme_cqcb_t       cqcb;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: NVME CQ Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} "
                    "base_addr: {} int_num: {}",
                    __FUNCTION__, spec.cq_num(),
                    spec.cq_wqe_size(), spec.num_cq_wqes(), 
                    spec.base_addr(), spec.int_num());

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.cq_num() < g_nvme_lif_info[lif].max_cq);

    cqwqe_size = roundup_to_pow_2(spec.cq_wqe_size());
    num_cq_wqes = roundup_to_pow_2(spec.num_cq_wqes());

    HAL_TRACE_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);

    memset(&cqcb, 0, sizeof(nvme_cqcb_t));
    cqcb.ring_header.total_rings = MAX_CQ_RINGS;
    cqcb.ring_header.host_rings = MAX_CQ_HOST_RINGS;

    cqcb.base_addr = spec.base_addr();

    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.color = 0;

    cqcb.int_assert_addr = intr_assert_addr(spec.int_num());
    rsp->set_cq_intr_tbl_addr(cqcb.int_assert_addr);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial CQCB State, baseaddr: {:#x} cqcb_size: {}",
                    __FUNCTION__, lif, cqcb.base_addr, sizeof(nvme_cqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&cqcb, sizeof(nvme_cqcb_t));
    lif_manager()->write_qstate(lif, NVME_QTYPE_CQ, spec.cq_num(), (uint8_t *)&cqcb, sizeof(nvme_cqcb_t));
    HAL_TRACE_DEBUG("{}: QstateAddr = {:#x}\n", __FUNCTION__,
                    lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_CQ, spec.cq_num()));

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}


hal_ret_t
nvme_ns_create (NvmeNsSpec& spec, NvmeNsResponse *rsp)
{
    uint32_t     lif = spec.hw_lif_id();
    nvme_nscb_t  nscb;
    uint64_t     nscb_addr;
    uint32_t     g_nsid;
    nvme_ns_info_t *ns_info_p;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: NVME NS Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: nsid: {} backend_nsid: {} max_sessions: {} "
                    "size: {} lba_size: {} key_index: {} sec_key_index: {}",
                    __FUNCTION__, spec.nsid(),
                    spec.backend_nsid(), spec.max_sess(), 
                    spec.size(), spec.lba_size(),
                    spec.key_index(), spec.sec_key_index());

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.nsid() != 0);
    SDK_ASSERT(spec.nsid() <= g_nvme_lif_info[lif].max_ns);
    SDK_ASSERT((spec.lba_size() & (spec.lba_size() - 1)) == 0); //power of 2 check
    SDK_ASSERT(g_nvme_lif_info[lif].cur_sess + spec.max_sess() <= g_nvme_lif_info[lif].max_sess);

    //1 based
    nscb_addr = g_nvme_lif_info[lif].nscb_base_addr + (spec.nsid() - 1) * sizeof(nvme_nscb_t);

    memset(&nscb, 0, sizeof(nvme_nscb_t));
    nscb.backend_ns_id = spec.backend_nsid();
    nscb.ns_size = spec.size(); //size in LBAs
    nscb.log_lba_size = log2(spec.lba_size());
    nscb.sess_prodcb_table_addr = g_nvme_global_info.txsessprodcb_base + 
        (g_nvme_lif_info[lif].sess_start + g_nvme_lif_info[lif].cur_sess) * sizeof(nvme_txsessprodcb_t);

    //update global ns info
    g_nsid = g_nvme_lif_info[lif].ns_start + spec.nsid() - 1;
    SDK_ASSERT(g_nsid <= g_nvme_global_info.max_ns);

    ns_info_p = &g_nvme_ns_info[g_nsid];
    ns_info_p->sess_start = g_nvme_lif_info[lif].sess_start + g_nvme_lif_info[lif].cur_sess;
    ns_info_p->max_sess = spec.max_sess();
    ns_info_p->cur_sess = 0;
    ns_info_p->key_index = spec.key_index();
    ns_info_p->sec_key_index = spec.sec_key_index();

    g_nvme_lif_info[lif].cur_sess += spec.max_sess();

    HAL_TRACE_DEBUG("{}: ns->sess_start: {}, lif->curr_sess: {},  key_index: {} sec_key_index: {}",
                    __FUNCTION__, ns_info_p->sess_start, 
                    g_nvme_lif_info[lif].cur_sess,
                    ns_info_p->key_index, ns_info_p->sec_key_index);

    rsp->set_nscb_addr(nscb_addr);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial NSCB State, addr: {:#x}, size: {}",
                    __FUNCTION__, lif, nscb_addr, sizeof(nvme_nscb_t));

    // Convert data before writting to HBM
    memrev((uint8_t*)&nscb, sizeof(nvme_nscb_t));

    nvme_hbm_write(nscb_addr, (void *)&nscb, sizeof(nvme_nscb_t));

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
nvme_sess_create (NvmeSessSpec& spec, NvmeSessResponse *rsp)
{
    uint32_t      lif = spec.hw_lif_id();
    nvme_txsessprodcb_t txsessprodcb;
    uint64_t      txsessprodcb_addr;
    uint64_t      tx_sess_xtsq_base;
    uint64_t      tx_sess_dgstq_base;
    nvme_rxsessprodcb_t rxsessprodcb;
    uint64_t      rxsessprodcb_addr;
    uint64_t      rx_sess_xtsq_base;
    uint64_t      rx_sess_dgstq_base;
    uint32_t      sess_id;
    uint32_t      sesq_qid;
    uint32_t      serq_qid;
    uint64_t      sesq_base;
    uint64_t      serq_base;
    uint32_t      sesq_size;
    uint32_t      serq_size;
    hal_ret_t     ret;
    nvme_nscb_t   *nscb_p;
    uint32_t      g_nsid;
    nvme_ns_info_t *ns_info_p;
    wring_t       wring;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: NVME Sess Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: nsid: {}",
                    __FUNCTION__, spec.nsid())

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.nsid() != 0);
    SDK_ASSERT(spec.nsid() <= g_nvme_lif_info[lif].max_ns);

    g_nsid = g_nvme_lif_info[lif].ns_start + spec.nsid() - 1;
    SDK_ASSERT(g_nsid <= g_nvme_global_info.max_ns);

    ns_info_p = &g_nvme_ns_info[g_nsid];

    SDK_ASSERT(ns_info_p->cur_sess < (ns_info_p->max_sess - 1));

    //Get tcp qid associated with the flow
    proxy_flow_info_t*  pfi = NULL;
    flow_key_t          flow_key = {0};
    vrf_id_t            tid = 0;

    nscb_p = (nvme_nscb_t *)(g_nvme_lif_info[lif].nscb_base_addr + (spec.nsid() - 1) * sizeof(nvme_nscb_t));

    tid = spec.vrf_key_handle().vrf_id();
    extract_flow_key_from_spec(tid, &flow_key, spec.flow_key());

    HAL_TRACE_DEBUG("({}, {}): vrf_id: {}, tid: {}, flow_key: {}",
                    __FUNCTION__, __LINE__, 
                    spec.vrf_key_handle().vrf_id(),
                    tid, flow_key);

    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, &flow_key);
    if(!pfi) {
        HAL_TRACE_ERR("flow info not found for the flow {}", flow_key);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PROXY_NOT_FOUND;
    }

    SDK_ASSERT(pfi != NULL);
    SDK_ASSERT(pfi->proxy != NULL);
    SDK_ASSERT(pfi->proxy->type == types::PROXY_TYPE_TCP);

    serq_qid = sesq_qid = pfi->qid2;

    HAL_TRACE_DEBUG("TCP Flow LIF: {}, QType: {}, QID1: {}, QID2: {}",
                    pfi->proxy->meta->lif_info[0].lif_id,
                    pfi->proxy->meta->lif_info[0].qtype_info[0].qtype_val,
                    pfi->qid1, pfi->qid2);

    // Get Sesq address
    ret = wring_get_meta(types::WRING_TYPE_SESQ,
                         sesq_qid,
                         &wring);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive serq base for transport TCPQ: {}",
                      sesq_qid);
        return HAL_RET_ERR;
    } else {
        sesq_base = wring.phys_base_addr;
        sesq_size = wring.num_entries;
        HAL_TRACE_DEBUG("Sesq id: {:#x} Sesq base: {:#x}, size: {}", 
                        sesq_qid, sesq_base, sesq_size);
    }
    rsp->set_tx_sesq_base(sesq_base);
    rsp->set_tx_sesq_num_entries(sesq_size);

    // Get Serq address
    ret = wring_get_meta(types::WRING_TYPE_SERQ,
                         serq_qid,
                         &wring);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive serq base for transport TCPQ: {}",
                      serq_qid);
        return HAL_RET_ERR;
    } else {
        serq_base = wring.phys_base_addr;
        serq_size = wring.num_entries;
        HAL_TRACE_DEBUG("Serq id: {:#x} Serq base: {:#x}, size: {}", 
                        serq_qid, serq_base, serq_size);
    }
    rsp->set_rx_serq_base(serq_base);
    rsp->set_rx_serq_num_entries(serq_size);

    //global session id
    sess_id = ns_info_p->sess_start + ns_info_p->cur_sess++;
    SDK_ASSERT(sess_id < g_nvme_global_info.max_sess);

    //LIF local session id
    rsp->set_sess_id(sess_id - g_nvme_lif_info[lif].sess_start);

    txsessprodcb_addr = g_nvme_global_info.txsessprodcb_base + sess_id * sizeof(nvme_txsessprodcb_t);
    memset(&txsessprodcb, 0, sizeof(nvme_txsessprodcb_t));
    rsp->set_txsessprodcb_addr(txsessprodcb_addr);

    // Get Tx Sess XTSQ base address
    tx_sess_xtsq_base = g_nvme_global_info.tx_sess_xtsq_base + sess_id * NVME_TX_SESS_XTSQ_SIZE;
    txsessprodcb.xts_q_base_addr = tx_sess_xtsq_base;
    txsessprodcb.xts_q_pi = 0;
    txsessprodcb.xts_q_ci = 0;
    txsessprodcb.log_num_xts_q_entries = log2(NVME_TX_SESS_XTSQ_ENTRIES);
    SDK_ASSERT((NVME_TX_SESS_XTSQ_ENTRIES & (NVME_TX_SESS_XTSQ_ENTRIES - 1)) == 0);
    rsp->set_tx_xtsq_base(tx_sess_xtsq_base);
    rsp->set_tx_xtsq_num_entries(NVME_TX_SESS_XTSQ_ENTRIES);

    // Get Tx Sess DGSTQ base address
    tx_sess_dgstq_base = g_nvme_global_info.tx_sess_dgstq_base + sess_id * NVME_TX_SESS_DGSTQ_SIZE;
    txsessprodcb.dgst_q_base_addr = tx_sess_dgstq_base;
    txsessprodcb.dgst_q_pi = 0;
    txsessprodcb.dgst_q_ci = 0;
    txsessprodcb.log_num_dgst_q_entries = log2(NVME_TX_SESS_DGSTQ_ENTRIES);
    txsessprodcb.tcp_q_base_addr = sesq_base;
    txsessprodcb.tcp_q_pi = 0;
    txsessprodcb.tcp_q_ci = 0;
    txsessprodcb.log_num_tcp_q_entries = log2(sesq_size);
    SDK_ASSERT((NVME_TX_SESS_DGSTQ_ENTRIES & (NVME_TX_SESS_DGSTQ_ENTRIES - 1)) == 0);
    rsp->set_tx_dgstq_base(tx_sess_dgstq_base);
    rsp->set_tx_dgstq_num_entries(NVME_TX_SESS_DGSTQ_ENTRIES);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial txsessprodcb State, addr: {:#x}, size: {}",
                    __FUNCTION__, lif, txsessprodcb_addr, sizeof(nvme_txsessprodcb_t));

    // Convert data before writting to HBM
    memrev((uint8_t*)&txsessprodcb, sizeof(nvme_txsessprodcb_t));

    nvme_hbm_write(txsessprodcb_addr, (void *)&txsessprodcb, sizeof(nvme_txsessprodcb_t));


    rxsessprodcb_addr = g_nvme_global_info.rxsessprodcb_base + sess_id * sizeof(nvme_rxsessprodcb_t);
    memset(&rxsessprodcb, 0, sizeof(nvme_rxsessprodcb_t));
    rsp->set_rxsessprodcb_addr(rxsessprodcb_addr);

    // Get Rx Sess XTSQ base address
    rx_sess_xtsq_base = g_nvme_global_info.rx_sess_xtsq_base + sess_id * NVME_RX_SESS_XTSQ_SIZE;
    rxsessprodcb.xts_q_base_addr = rx_sess_xtsq_base;
    rxsessprodcb.xts_q_pi = 0;
    rxsessprodcb.xts_q_ci = 0;
    rxsessprodcb.log_num_xts_q_entries = log2(NVME_RX_SESS_XTSQ_ENTRIES);
    rxsessprodcb.tcp_q_base_addr = serq_base;
    rxsessprodcb.tcp_q_pi = 0;
    rxsessprodcb.tcp_q_ci = 0;
    rxsessprodcb.log_num_tcp_q_entries = log2(serq_size);
    SDK_ASSERT((NVME_RX_SESS_XTSQ_ENTRIES & (NVME_RX_SESS_XTSQ_ENTRIES - 1)) == 0);
    rsp->set_rx_xtsq_base(rx_sess_xtsq_base);
    rsp->set_rx_xtsq_num_entries(NVME_RX_SESS_XTSQ_ENTRIES);

    // Get Rx Sess DGSTQ base address
    rx_sess_dgstq_base = g_nvme_global_info.rx_sess_dgstq_base + sess_id * NVME_RX_SESS_DGSTQ_SIZE;
    rxsessprodcb.dgst_q_base_addr = rx_sess_dgstq_base;
    rxsessprodcb.dgst_q_pi = 0;
    rxsessprodcb.dgst_q_ci = 0;
    rxsessprodcb.log_num_dgst_q_entries = log2(NVME_RX_SESS_DGSTQ_ENTRIES);
    SDK_ASSERT((NVME_RX_SESS_DGSTQ_ENTRIES & (NVME_RX_SESS_DGSTQ_ENTRIES - 1)) == 0);
    rsp->set_rx_dgstq_base(rx_sess_dgstq_base);
    rsp->set_rx_dgstq_num_entries(NVME_RX_SESS_DGSTQ_ENTRIES);

    // write to hardware
    HAL_TRACE_DEBUG("{}: LIF: {}: Writing initial rxsessprodcb State, addr: {:#x}, size: {}",
                    __FUNCTION__, lif, rxsessprodcb_addr, sizeof(nvme_rxsessprodcb_t));

    // Convert data before writting to HBM
    memrev((uint8_t*)&rxsessprodcb, sizeof(nvme_rxsessprodcb_t));
    nvme_hbm_write(rxsessprodcb_addr, (void *)&rxsessprodcb, sizeof(nvme_rxsessprodcb_t));

    //Setup session xts tx
    nvme_sessxtstxcb_t sessxtstxcb;
    uint64_t sessxtstxcb_addr;
    sessxtstxcb_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_TX_SESS_XTSQ, sess_id);
    memset(&sessxtstxcb, 0, sizeof(nvme_sessxtstxcb_t));

    sessxtstxcb.base_addr = tx_sess_xtsq_base;
    sessxtstxcb.log_num_entries = txsessprodcb.log_num_xts_q_entries;
    sessxtstxcb.log_lba_size = nscb_p->log_lba_size;
    sessxtstxcb.key_index = ns_info_p->key_index;
    sessxtstxcb.sec_key_index = ns_info_p->sec_key_index;

    // Convert data before writting to HBM
    memrev((uint8_t*)&sessxtstxcb, sizeof(nvme_sessxtstxcb_t));
    nvme_hbm_write(sessxtstxcb_addr, (void *)&sessxtstxcb, sizeof(nvme_sessxtstxcb_t));


    //Setup session dgst tx
    nvme_sessdgsttxcb_t sessdgsttxcb;
    uint64_t sessdgsttxcb_addr;
    sessdgsttxcb_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_TX_SESS_DGSTQ, sess_id);
    memset(&sessdgsttxcb, 0, sizeof(nvme_sessdgsttxcb_t));

    sessdgsttxcb.base_addr = tx_sess_dgstq_base;
    sessdgsttxcb.log_num_entries = txsessprodcb.log_num_dgst_q_entries;

    // Convert data before writting to HBM
    memrev((uint8_t*)&sessdgsttxcb, sizeof(nvme_sessdgsttxcb_t));
    nvme_hbm_write(sessdgsttxcb_addr, (void *)&sessdgsttxcb, sizeof(nvme_sessdgsttxcb_t));


    //Setup session xts rx
    nvme_sessxtsrxcb_t sessxtsrxcb;
    uint64_t sessxtsrxcb_addr;
    sessxtsrxcb_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_RX_SESS_XTSQ, sess_id);
    memset(&sessxtsrxcb, 0, sizeof(nvme_sessxtsrxcb_t));

    sessxtsrxcb.base_addr = rx_sess_xtsq_base;
    sessxtsrxcb.log_num_entries = rxsessprodcb.log_num_xts_q_entries;
    sessxtsrxcb.log_lba_size = nscb_p->log_lba_size;
    sessxtsrxcb.key_index = ns_info_p->key_index;
    sessxtsrxcb.sec_key_index = ns_info_p->sec_key_index;

    // Convert data before writting to HBM
    memrev((uint8_t*)&sessxtsrxcb, sizeof(nvme_sessxtsrxcb_t));
    nvme_hbm_write(sessxtsrxcb_addr, (void *)&sessxtsrxcb, sizeof(nvme_sessxtsrxcb_t));


    //Setup session dgst rx
    nvme_sessdgstrxcb_t sessdgstrxcb;
    uint64_t sessdgstrxcb_addr;
    sessdgstrxcb_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_RX_SESS_DGSTQ, sess_id);
    memset(&sessdgstrxcb, 0, sizeof(nvme_sessdgstrxcb_t));

    sessdgstrxcb.base_addr = rx_sess_dgstq_base;
    sessdgstrxcb.log_num_entries = rxsessprodcb.log_num_dgst_q_entries;

    // Convert data before writting to HBM
    memrev((uint8_t*)&sessdgstrxcb, sizeof(nvme_sessdgstrxcb_t));
    nvme_hbm_write(sessdgstrxcb_addr, (void *)&sessdgstrxcb, sizeof(nvme_sessdgstrxcb_t));


    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}


}    // namespace hal
