// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

// Functions related to Capri LIF 2 Qstate programming.

#include "platform/capri/capri_state.hpp"
#include "platform/capri/capri_qstate.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "asic/rw/asicrw.hpp"

using namespace sdk::platform::utils;

namespace sdk {
namespace platform {
namespace capri {

template <typename T>
void
clear_qstate_entry (T *entry)
{
    entry->vld(1);
    entry->write();
}

template <typename T>
void
reset_qstate_entry (T *entry)
{
    entry->vld(0);
    entry->write();
}

void
capri_clear_qstate_map (uint32_t lif_id)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();

    auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[lif_id];
    clear_qstate_entry(wa_entry);
    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[lif_id];
    clear_qstate_entry(psp_entry);
    auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[lif_id];
    clear_qstate_entry(pr_entry);
}

void
capri_reset_qstate_map (uint32_t lif_id)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();

    auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[lif_id];
    reset_qstate_entry(wa_entry);
    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[lif_id];
    reset_qstate_entry(psp_entry);
    auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[lif_id];
    reset_qstate_entry(pr_entry);
}

#define CAPRI_SET_QSTATE_MAP_ENTRY(QID)                                 \
    entry->length ## QID(qstate->type[QID].qtype_info.entries);         \
    entry->size ## QID(qstate->type[QID].qtype_info.size);

template <typename T>
void
capri_set_qstate_map (lif_qstate_t *qstate, T *entry, uint8_t enable)
{
    entry->qstate_base(qstate->hbm_address >> 12);
    CAPRI_SET_QSTATE_MAP_ENTRY(0);
    CAPRI_SET_QSTATE_MAP_ENTRY(1);
    CAPRI_SET_QSTATE_MAP_ENTRY(2);
    CAPRI_SET_QSTATE_MAP_ENTRY(3);
    CAPRI_SET_QSTATE_MAP_ENTRY(4);
    CAPRI_SET_QSTATE_MAP_ENTRY(5);
    CAPRI_SET_QSTATE_MAP_ENTRY(6);
    CAPRI_SET_QSTATE_MAP_ENTRY(7);
    entry->vld(enable);
    entry->sched_hint_en(1);
    entry->sched_hint_cos(qstate->hint_cos);
    entry->write();
}

#define CAPRI_READ_ASSERT_MAP_ENTRY(QID) \
    SDK_ASSERT(qstate->type[QID].qtype_info.entries == \
               rqstate.type[QID].qtype_info.entries); \
    SDK_ASSERT(qstate->type[QID].qtype_info.size == \
               rqstate.type[QID].qtype_info.size);

void
capri_program_qstate_map (lif_qstate_t *qstate, uint8_t enable)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();

    auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
    capri_set_qstate_map(qstate, wa_entry, enable);
    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    capri_set_qstate_map(qstate, psp_entry, enable);
    auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    capri_set_qstate_map(qstate, pr_entry, enable);
}

#define QSTATE_INFO(QID) \
    qstate->type[QID].qtype_info.entries, qstate->type[QID].qtype_info.size

#define CAPRI_GET_QSTATE_MAP_ENTRY(QID)                                     \
    qstate->type[QID].qtype_info.entries = (uint8_t)entry->length ## QID(); \
    qstate->type[QID].qtype_info.size = (uint8_t)entry->size ## QID();
template <typename T>
void
capri_get_qstate_map (lif_qstate_t *qstate, T *entry)
{
    entry->read();
    qstate->enable = (uint8_t)entry->vld();
    qstate->hbm_address = (uint64_t) (entry->qstate_base() << 12);
    qstate->hint_cos = (uint8_t) entry->sched_hint_cos();
    CAPRI_GET_QSTATE_MAP_ENTRY(0);
    CAPRI_GET_QSTATE_MAP_ENTRY(1);
    CAPRI_GET_QSTATE_MAP_ENTRY(2);
    CAPRI_GET_QSTATE_MAP_ENTRY(3);
    CAPRI_GET_QSTATE_MAP_ENTRY(4);
    CAPRI_GET_QSTATE_MAP_ENTRY(5);
    CAPRI_GET_QSTATE_MAP_ENTRY(6);
    CAPRI_GET_QSTATE_MAP_ENTRY(7);

    SDK_TRACE_DEBUG("Lif: %lu, Qstate map: hbm_addr: %llu, hint_cos: %lu, "
                    "e0: %u,%u, e1: %u,%u, e2: %u,%u, e3: %u,%u, "
                    "e4: %u,%u, e5: %u,%u, e6: %u,%u, e7: %u,%u",
                    qstate->lif_id, qstate->hbm_address, qstate->hint_cos,
                    QSTATE_INFO(0), QSTATE_INFO(1), QSTATE_INFO(2),
                    QSTATE_INFO(3), QSTATE_INFO(4), QSTATE_INFO(5),
                    QSTATE_INFO(6), QSTATE_INFO(7));
}

void
capri_read_qstate_map (lif_qstate_t *qstate)
{
    uint32_t lif_id = qstate->lif_id;
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();

    // Since content is going to be same in ASIC across all 3 blocks -
    // reading from one is enough ??
    auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
    capri_get_qstate_map(qstate, wa_entry);

    memset(qstate, 0, sizeof(lif_qstate_t));
    qstate->lif_id = lif_id;
    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    capri_get_qstate_map(qstate, psp_entry);

    memset(qstate, 0, sizeof(lif_qstate_t));
    qstate->lif_id = lif_id;
    auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    capri_get_qstate_map(qstate, pr_entry);
}

void
capri_reprogram_qstate_map (uint32_t lif_id, uint8_t enable)
{
    lif_qstate_t qstate;
    memset(&qstate, 0, sizeof(lif_qstate_t));
    qstate.lif_id = lif_id;
    capri_read_qstate_map(&qstate);
    capri_program_qstate_map(&qstate, enable);
}

sdk_ret_t
capri_read_qstate (uint64_t q_addr, uint8_t *buf, uint32_t q_size)
{
    return sdk::asic::asic_mem_read(q_addr, buf, q_size);
}

sdk_ret_t
capri_write_qstate (uint64_t q_addr, const uint8_t *buf, uint32_t q_size)
{
    return sdk::asic::asic_mem_write(q_addr, (uint8_t *)buf, q_size);
}

sdk_ret_t
capri_clear_qstate_mem (uint64_t base_addr, uint32_t size)
{
    sdk_ret_t ret = SDK_RET_OK;
    // qstate is a multiple for 4K So it is safe to assume
    // 256 byte boundary.
    static uint8_t zeros[256] = {0};
    for (uint32_t i = 0; i < (size / sizeof(zeros)); i++) {
        ret = sdk::asic::asic_mem_write(base_addr + (i * sizeof(zeros)),
                                        zeros, sizeof(zeros));
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    return ret;
}

sdk_ret_t
capri_clear_qstate (lif_qstate_t *qstate)
{
    return capri_clear_qstate_mem(qstate->hbm_address, qstate->allocation_size);
}

// Deprecated: Cleanup
template <typename T>
void
set_qstate_entry (LIFQState *qstate, T *entry, int cos)
{
    entry->qstate_base(qstate->hbm_address >> 12);
    entry->length0(qstate->params_in.type[0].entries);
    entry->size0(qstate->params_in.type[0].size);
    entry->length1(qstate->params_in.type[1].entries);
    entry->size1(qstate->params_in.type[1].size);
    entry->length2(qstate->params_in.type[2].entries);
    entry->size2(qstate->params_in.type[2].size);
    entry->length3(qstate->params_in.type[3].entries);
    entry->size3(qstate->params_in.type[3].size);
    entry->length4(qstate->params_in.type[4].entries);
    entry->size4(qstate->params_in.type[4].size);
    entry->length5(qstate->params_in.type[5].entries);
    entry->size5(qstate->params_in.type[5].size);
    entry->length6(qstate->params_in.type[6].entries);
    entry->size6(qstate->params_in.type[6].size);
    entry->length7(qstate->params_in.type[7].entries);
    entry->size7(qstate->params_in.type[7].size);
    entry->vld(1);
    entry->sched_hint_en(1);
    entry->sched_hint_cos(cos);
    entry->write();
}

// In case we just need hbm base and nothing else, we can use this
// Below 2 APIs are for retrieving lif base addr and other params upon upgrade
// we want to recover state by reading ASIC
template <typename T>
void
get_qstate_entry_base_address (T *entry, uint64_t *lif_base_addr)
{
    entry->read();
    if (entry->vld() == 1) {
        *lif_base_addr = (uint64_t) (entry->qstate_base() << 12);
    } else {
        *lif_base_addr = 0;
    }
}

// Deprecated: Cleanup
template <typename T>
void
get_qstate_lif_params (LIFQState *qstate, T *entry, uint32_t *is_valid)
{

    entry->read();
    *is_valid = (uint32_t)entry->vld();
    qstate->hbm_address = (uint64_t) (entry->qstate_base() << 12);
    qstate->params_in.type[0].entries = (uint8_t) entry->length0();
    qstate->params_in.type[0].size = (uint8_t) entry->size0();
    qstate->params_in.type[1].entries = (uint8_t) entry->length1();
    qstate->params_in.type[1].size = (uint8_t) entry->size1();
    qstate->params_in.type[2].entries = (uint8_t) entry->length2();
    qstate->params_in.type[2].size = (uint8_t) entry->size2();
    qstate->params_in.type[3].entries = (uint8_t) entry->length3();
    qstate->params_in.type[3].size = (uint8_t) entry->size3();
    qstate->params_in.type[4].entries = (uint8_t) entry->length4();
    qstate->params_in.type[4].size = (uint8_t) entry->size4();
    qstate->params_in.type[5].entries = (uint8_t) entry->length5();
    qstate->params_in.type[5].size = (uint8_t) entry->size5();
    qstate->params_in.type[6].entries = (uint8_t) entry->length6();
    qstate->params_in.type[6].size = (uint8_t) entry->size6();
    qstate->params_in.type[7].entries = (uint8_t) entry->length7();
    qstate->params_in.type[7].size = (uint8_t) entry->size7();
}

// Deprecated: Cleanup
void
push_qstate_to_capri (LIFQState *qstate, int cos)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();

    if (!qstate->params_in.dont_zero_memory) {
        capri_clear_qstate_mem(qstate->hbm_address, qstate->allocation_size);
    }
    auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
    set_qstate_entry(qstate, wa_entry, cos);
    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    set_qstate_entry(qstate, psp_entry, cos);
    auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    set_qstate_entry(qstate, pr_entry, cos);
}

// Deprecated: Cleanup
void
clear_qstate (LIFQState *qstate)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();

    auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
    clear_qstate_entry(wa_entry);
    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    clear_qstate_entry(psp_entry);
    auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    clear_qstate_entry(pr_entry);
}

// Deprecated: Cleanup
void
read_lif_params_from_capri (LIFQState *qstate)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();
    uint32_t is_valid = 0;

    auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
    // Since content is going to be same in ASIC across all 3 blocks - reading from one is enough ??
    get_qstate_lif_params(qstate, psp_entry, &is_valid);
}

int32_t
read_qstate (uint64_t q_addr, uint8_t *buf, uint32_t q_size)
{
    sdk_ret_t rv = sdk::asic::asic_mem_read(q_addr, buf, q_size);
    if (rv != SDK_RET_OK) {
        return -EIO;
    }
    return 0;
}

int32_t
write_qstate (uint64_t q_addr, const uint8_t *buf, uint32_t q_size)
{
    sdk_ret_t rc = sdk::asic::asic_mem_write(q_addr, (uint8_t *)buf, q_size);
    if (rc != SDK_RET_OK) {
        return -EIO;
    }
    return 0;
}

int32_t
get_pc_offset (program_info *pinfo, const char *prog_name,
               const char *label, uint8_t *offset)
{
    mem_addr_t off;

    off = pinfo->symbol_address((char *)prog_name, (char *)label);
    if (off == SDK_INVALID_HBM_ADDRESS)
        return -ENOENT;
    // 64 byte alignment check
    if ((off & 0x3F) != 0) {
        return -EIO;
    }
    // offset can be max 14 bits
    if (off > 0x3FC0) {
        return -EIO;
    }
    *offset = (uint8_t) (off >> 6);
    return 0;
}

} // namespace capri
} // namespace platform
} // namespace sdk
