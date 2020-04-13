//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
// Hardware FIFO
//-----------------------------------------------------------------------------

#include "nic/sdk/platform/elba/elba_fifo.hpp"
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "third-party/asic/elba/model/elb_soc/elb_hff_csr.h"

namespace sdk      {
namespace platform {
namespace elba     {

#define ELBA_MAX_HW_FIFO_PROFILES 16
#define ELBA_NUM_OCC_BITS          2

typedef struct elba_profile_s {
    bool        initialized;
    uint32_t    profile_number;
} elba_profile_t;

static elba_profile_t elba_profile[ELBA_MAX_HW_FIFO_PROFILES];
static int elba_profile_idx = 1; // let's start with idx 1 (for DOL)

static sdk_ret_t
elba_init_hw_profile (hw_fifo_prof_t *prof)
{
    elb_top_csr_t &   elb0     = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_hff_csr_t     *hff_csr = &elb0.ms.soc.hff;
    elb_hff_csr_cfg_hff_resource_profile_0_t *hff_profile =
        &hff_csr->cfg_hff_resource_profile_0;

    SDK_ASSERT(elba_profile_idx < ELBA_MAX_HW_FIFO_PROFILES);

    hff_profile += elba_profile_idx;
    hff_profile->read();
    hff_profile->occ_bit_num(ELBA_NUM_OCC_BITS);
    uint32_t watermark_bitmap = 1 << prof->lg2occ1 | 1 << prof->lg2occ2 |
                                1 << prof->lg2occ3;
    hff_profile->watermark_bit_map(watermark_bitmap);
    hff_profile->write();
    SDK_TRACE_DEBUG("init hw fifo profile %d occ %d watermark 0x%x",
                     elba_profile_idx, ELBA_NUM_OCC_BITS, watermark_bitmap);

    elba_profile[elba_profile_idx].initialized = true;
    elba_profile[elba_profile_idx].profile_number = elba_profile_idx;

    prof->opaque = (uint64_t)&elba_profile[elba_profile_idx];
    elba_profile_idx++;

    return SDK_RET_OK;
}

sdk_ret_t
elba_init_hw_fifo (int fifo_num, uint64_t addr, int n, hw_fifo_prof_t *prof)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_hff_csr_t *hff_csr = &elb0.ms.soc.hff;
    elb_hff_csr_cfg_hff_resource_0_t *hff_resource =
        &hff_csr->cfg_hff_resource_0;
    elb_hff_csr_dhs_fifo_ptr_t *dhs_fifo_ptr = &hff_csr->dhs_fifo_ptr;

    hff_resource  += fifo_num;

    SDK_TRACE_DEBUG("init hw fifo %d addr 0x%llx size %d", fifo_num, addr, n);
    if (prof && !prof->opaque) {
        elba_init_hw_profile(prof);
    }
    hff_resource->read();
    // addr is 64B aligned
    SDK_ASSERT((addr & 0x3f) == 0);
    hff_resource->base_addr(addr >> 6);
    hff_resource->data_size(n);
    hff_resource->enable(1);
    if (prof) {
        elba_profile_t *elba_profile = (elba_profile_t *)prof->opaque;
        hff_resource->response_profile(elba_profile->profile_number);
        SDK_TRACE_DEBUG("init hw fifo %d profile number %d",
                fifo_num, elba_profile->profile_number);
    }
    hff_resource->write();
    dhs_fifo_ptr->entry[fifo_num].read();
    dhs_fifo_ptr->entry[fifo_num].full(1);
    dhs_fifo_ptr->entry[fifo_num].empty(0);
    dhs_fifo_ptr->entry[fifo_num].write();

    return SDK_RET_OK;
}

sdk_ret_t
elba_get_hw_fifo_info (int fifo_num, hw_fifo_stats_t *stats)
{
    elb_top_csr_t & elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_hff_csr_t *hff_csr = &elb0.ms.soc.hff;
    elb_hff_csr_dhs_fifo_ptr_t *dhs_fifo_ptr = &hff_csr->dhs_fifo_ptr;

    dhs_fifo_ptr->entry[fifo_num].read();
    stats->head = (uint32_t)dhs_fifo_ptr->entry[fifo_num].head_ptr();
    stats->tail = (uint32_t)dhs_fifo_ptr->entry[fifo_num].tail_ptr();
    stats->empty = dhs_fifo_ptr->entry[fifo_num].empty() == 1;
    stats->full = dhs_fifo_ptr->entry[fifo_num].full() == 1;

    SDK_TRACE_DEBUG("get hw fifo %d head %d tail %d empty %d full %d",
            fifo_num, stats->head, stats->tail, stats->empty, stats->full);

    return SDK_RET_OK;
}

sdk_ret_t
elba_set_hw_fifo_info (int fifo_num, hw_fifo_stats_t *stats)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_hff_csr_t*hff_csr = &elb0.ms.soc.hff;
    elb_hff_csr_dhs_fifo_ptr_t *dhs_fifo_ptr = &hff_csr->dhs_fifo_ptr;

    dhs_fifo_ptr->entry[fifo_num].read();
    dhs_fifo_ptr->entry[fifo_num].head_ptr(stats->head);
    dhs_fifo_ptr->entry[fifo_num].tail_ptr(stats->tail);
    dhs_fifo_ptr->entry[fifo_num].empty(stats->empty);
    dhs_fifo_ptr->entry[fifo_num].full(stats->full);
    dhs_fifo_ptr->entry[fifo_num].write();
    SDK_TRACE_DEBUG("set hw fifo %d head %d tail %d empty %d full %d",
            fifo_num, stats->head, stats->tail, stats->empty, stats->full);

    return SDK_RET_OK;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
