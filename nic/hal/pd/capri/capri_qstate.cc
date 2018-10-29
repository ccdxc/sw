// Functions related to Capri LIF 2 Qstate programming.
#include "nic/hal/plugins/cfg/lif/lif_manager.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/include/asic_pd.hpp"
#include "nic/include/asic_pd.hpp"

namespace {

template <typename T>
void set_qstate_entry(hal::LIFQState *qstate, T *entry, int cos) {
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

template <typename T>
void clear_qstate_entry(T *entry) {
  entry->vld(1);
  entry->write();
}

// In case we just need hbm base and nothing else, we can use this
// Below 2 APIs are for retrieving lif base addr and other params upon upgrade
// we want to recover state by reading ASIC
template <typename T>
void get_qstate_entry_base_address(T *entry, uint64_t *lif_base_addr) {
    entry->read();
    if (entry->vld() == 1) {
        *lif_base_addr = (uint64_t) (entry->qstate_base() << 12);
    } else {
        *lif_base_addr = 0;
    }
}

template <typename T>
void get_qstate_lif_params(hal::LIFQState *qstate, T *entry, uint32_t *is_valid) {

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


int clear_qstate_mem(uint64_t base_addr, uint32_t size) {
  // qstate is a multiple for 4K So it is safe to assume
  // 256 byte boundary.
  static uint8_t zeros[256] = {0};
  for (uint32_t i = 0; i < (size / sizeof(zeros)); i++) {
    hal_ret_t rc = hal::pd::asic_mem_write(base_addr + (i * sizeof(zeros)),
                                           zeros, sizeof(zeros));
    if (rc != HAL_RET_OK) {
        return -EIO;
    }
  }
  return 0;
}


}  // Anonymous namespace

void push_qstate_to_capri(hal::LIFQState *qstate, int cos) {
  cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

  if (!qstate->params_in.dont_zero_memory) {
      clear_qstate_mem(qstate->hbm_address, qstate->allocation_size);
  }
  auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
  set_qstate_entry(qstate, wa_entry, cos);
  auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  set_qstate_entry(qstate, psp_entry, cos);
  auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  set_qstate_entry(qstate, pr_entry, cos);
}

void clear_qstate(hal::LIFQState *qstate) {
  cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

  auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
  clear_qstate_entry(wa_entry);
  auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  clear_qstate_entry(psp_entry);
  auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  clear_qstate_entry(pr_entry);
}

void read_lif_params_from_capri(hal::LIFQState *qstate) {
  cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
  uint32_t is_valid = 0;

  auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  // Since content is going to be same in ASIC across all 3 blocks - reading from one is enough ??
  get_qstate_lif_params(qstate, psp_entry, &is_valid);
}


int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size) {
  hal_ret_t rv = hal::pd::asic_mem_read(q_addr, buf, q_size);
  if (rv != HAL_RET_OK) {
    return -EIO;
  }
  return 0;
}

int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size) {
  hal_ret_t rc = hal::pd::asic_mem_write(q_addr, (uint8_t *)buf, q_size);
  if (rc != HAL_RET_OK) {
    return -EIO;
  }
  return 0;
}

int32_t get_pc_offset(const char *handle, const char *prog_name,
                      const char *label, uint8_t *offset) {
  uint64_t off;

  if (capri_program_label_to_offset(handle, (char *)prog_name,
                                    (char *)label, &off) < 0)
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
