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

namespace hal {

const static uint32_t kAllocUnit = 4096;

typedef struct nvme_global_info_s {
    uint32_t max_ns;
    uint32_t curr_ns;
    uint64_t nscb_base_addr;
    uint32_t max_sess;
    uint32_t curr_sess;
    uint64_t sessprodcb_base_addr;
    uint64_t sess_bitmap_addr;
    uint32_t max_cmd_context;
    uint64_t cmd_context_page_base;
    uint64_t cmd_context_ring_base;

    //tuning params
    uint16_t sess_q_depth;
} nvme_global_info_t;

typedef struct nvme_lif_info_s {
    uint32_t max_ns;
    uint64_t nscb_base_addr;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint64_t sess_bitmap_addr;
    uint64_t sessprodcb_base_addr;
    uint32_t max_cq;
    uint64_t cqcb_base_addr;
    uint32_t max_sq;
    uint64_t sqcb_base_addr;
    uint32_t log_host_page_size;
} nvme_lif_info_t;

static nvme_global_info_t g_nvme_global_info;
static nvme_lif_info_t g_nvme_lif_info[MAX_LIFS];

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
    HAL_TRACE_DEBUG("{}: size: {} alloc_offset: {} hbm_addr: {}\n",
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
    uint32_t            total_size;
    uint64_t            base_addr;
    uint32_t            index;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
 
    HAL_TRACE_DEBUG("({}, {}): NVME Enable Request",
                    __FUNCTION__, __LINE__);

    max_ns  = spec.max_ns();
    max_sess  = spec.max_sess();
    max_cmd_context  = spec.max_cmd_context();

    SDK_ASSERT(max_cmd_context <= (1 << 16));

    HAL_TRACE_DEBUG("{}: max_ns: {}, max_sess: {}, max_cmd_context: {}\n",
                   __FUNCTION__, max_ns, max_sess, max_cmd_context);

    total_size = sizeof(nvme_nscb_t) * max_ns +
                 sizeof(nvme_sessprodcb_t) * max_sess +
                 max_sess / 8 /* bitmap */;

    base_addr = nvme_manager()->HbmAlloc(total_size);

    memset(&g_nvme_global_info, 0, sizeof(g_nvme_global_info));

    g_nvme_global_info.max_ns = max_ns;
    g_nvme_global_info.max_sess = max_sess;
    g_nvme_global_info.max_cmd_context = max_cmd_context;
    g_nvme_global_info.nscb_base_addr = base_addr;
    g_nvme_global_info.sessprodcb_base_addr = 
        base_addr + sizeof(nvme_nscb_t) * max_ns;
    g_nvme_global_info.sess_bitmap_addr = 
        g_nvme_global_info.sessprodcb_base_addr +
        sizeof(nvme_sessprodcb_t) * max_sess;

    HAL_TRACE_DEBUG("{}: base_addr: {:#x}, nscb_base_addr: {:#x} "
                    "sessprodcb_base_addr: {:#x}, sess_bitmap_addr: {:#x}\n",
                   __FUNCTION__, base_addr, g_nvme_global_info.nscb_base_addr, 
                    g_nvme_global_info.sessprodcb_base_addr,
                    g_nvme_global_info.sess_bitmap_addr);


    total_size = sizeof(nvme_cmd_context_t) * max_cmd_context + 
                 sizeof(nvme_cmd_context_ring_entry_t) * max_cmd_context;

    base_addr = nvme_manager()->HbmAlloc(total_size);

    g_nvme_global_info.cmd_context_page_base = base_addr;
    g_nvme_global_info.cmd_context_ring_base = base_addr + 
                                                   sizeof(nvme_cmd_context_t) * max_cmd_context;

    HAL_TRACE_DEBUG("{}: cmd_context_page_base: {:#x}, "
                    "cmd_context_ring_base: {:#x}, total_size: {}\n",
                    __FUNCTION__, g_nvme_global_info.cmd_context_page_base,
                    g_nvme_global_info.cmd_context_ring_base, total_size);

    //Fill the ring with cmd context page addresses
    for (index = 0; index < max_cmd_context; index++) {
        nvme_cmd_context_ring_entry_write(index, index);
    }

    rsp->set_cmd_context_page_base(g_nvme_global_info.cmd_context_page_base);
    rsp->set_cmd_context_ring_base(g_nvme_global_info.cmd_context_ring_base);

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

    HAL_TRACE_DEBUG("({}, {}): curr_ns: {}, max_ns: {}, curr_sess: {}, max_sess: {}\n",
                    __FUNCTION__, __LINE__,
                    g_nvme_global_info.curr_ns, g_nvme_global_info.max_ns,
                    g_nvme_global_info.curr_sess, g_nvme_global_info.max_sess);

    HAL_TRACE_DEBUG("lif: {} spec_lif: {}", lif, spec.key_or_handle().lif_id());

    //SDK_ASSERT(lif == spec.key_or_handle().lif_id());
    SDK_ASSERT(lif < MAX_LIFS);

    max_ns = spec.nvme_max_ns();
    max_sess = spec.nvme_max_sess();

    HAL_TRACE_DEBUG("({},{}): LIF: {}, max_ns: {}, max_sess: {}",
                    __FUNCTION__, __LINE__, lif, max_ns, max_sess);

    SDK_ASSERT((g_nvme_global_info.curr_ns + max_ns) < g_nvme_global_info.max_ns);
    SDK_ASSERT((g_nvme_global_info.curr_sess + max_sess) < g_nvme_global_info.max_sess);

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
                                          g_nvme_global_info.curr_ns * sizeof(nvme_nscb_t);
    g_nvme_global_info.curr_ns += max_ns;
    
    nvme_lif_info_p->sessprodcb_base_addr = g_nvme_global_info.sessprodcb_base_addr + 
                                                g_nvme_global_info.curr_sess * sizeof(nvme_sessprodcb_t);
    g_nvme_global_info.curr_sess += max_sess;
    
    nvme_lif_info_p->sess_bitmap_addr = g_nvme_global_info.sess_bitmap_addr + 
                                            BITS_TO_BYTES(g_nvme_global_info.curr_sess);

    nvme_lif_info_p->max_ns = max_ns;
    nvme_lif_info_p->max_sess = max_sess;

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
                    "max_ns: {}, nscb_base_addr: {:#x}, "
                    "max_sess: {}, sessprodcb_base_addr: {:#x}, "
                    "sess_bitmap_addr: {}, log_host_page_size: {}",
                    __FUNCTION__, __LINE__, lif,
                    nvme_lif_info_p->max_sq, 
                    nvme_lif_info_p->sqcb_base_addr,
                    nvme_lif_info_p->max_cq, 
                    nvme_lif_info_p->cqcb_base_addr,
                    nvme_lif_info_p->max_ns, 
                    nvme_lif_info_p->nscb_base_addr,
                    nvme_lif_info_p->max_sess, 
                    nvme_lif_info_p->sessprodcb_base_addr,
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

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: NVME NS Create for lif {}", __FUNCTION__, lif);


    HAL_TRACE_DEBUG("{}: Inputs: nsid: {} backend_nsid: {} max_sessions: {} "
                    "size: {} lba_size: {}",
                    __FUNCTION__, spec.nsid(),
                    spec.backend_nsid(), spec.max_sess(), 
                    spec.size(), spec.lba_size());

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.nsid() != 0);
    SDK_ASSERT(spec.nsid() <= g_nvme_lif_info[lif].max_ns);
    SDK_ASSERT((spec.lba_size() & (spec.lba_size() - 1)) == 0); //power of 2 check
    SDK_ASSERT(g_nvme_lif_info[lif].cur_sess + spec.max_sess() < g_nvme_lif_info[lif].max_sess);

    //1 based
    nscb_addr = g_nvme_lif_info[lif].nscb_base_addr + (spec.nsid() - 1) * sizeof(nvme_nscb_t);

    memset(&nscb, 0, sizeof(nvme_nscb_t));
    nscb.nsid = spec.nsid();
    nscb.backend_nsid = spec.backend_nsid();
    nscb.size = spec.size(); //size in LBAs
    nscb.log_lba_size = log2(spec.lba_size());
    nscb.sessprodcb_base_addr = 
        g_nvme_lif_info[lif].sessprodcb_base_addr + g_nvme_lif_info[lif].cur_sess * sizeof(nvme_sessprodcb_t);

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

}    // namespace hal
