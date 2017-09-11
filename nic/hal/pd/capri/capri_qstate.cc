// Functions related to Capri LIF 2 Qstate programming.
#include <lif_manager.hpp>
#include <cap_top_csr.h>
#include <capri_loader.h>
#include <lib_model_client.h>

namespace {

#ifndef HAL_GTEST
template <typename T>
void set_qstate_entry(hal::LIFQState *qstate, T *entry) {
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
  entry->write();
}

int clear_qstate_mem(uint64_t base_addr, uint32_t size) {
  // qstate is a multiple for 4K So it is safe to assume
  // 256 byte boundary.
  static uint8_t zeros[256] = {0};
  for (uint32_t i = 0; i < (size / sizeof(zeros)); i++) {
    if (!write_mem(base_addr + (i * sizeof(zeros)), zeros, sizeof(zeros))) {
      return -EIO;
    }
  }
  return 0;
}

#endif

}  // Anonymus namespace

void push_qstate_to_capri(hal::LIFQState *qstate) {
#ifndef HAL_GTEST
  cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

  clear_qstate_mem(qstate->hbm_address, qstate->allocation_size);
  auto *wa_entry = &cap0.db.wa.dhs_lif_qstate_map.entry[qstate->lif_id];
  set_qstate_entry(qstate, wa_entry);
  auto *psp_entry = &cap0.pt.pt.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  set_qstate_entry(qstate, psp_entry);
  auto *pr_entry = &cap0.pr.pr.psp.dhs_lif_qstate_map.entry[qstate->lif_id];
  set_qstate_entry(qstate, pr_entry);
#endif
}

int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size) {
#ifndef HAL_GTEST
  if (!read_mem(q_addr, buf, q_size))
    return -EIO;
#endif
  return 0;
}

int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size) {
#ifndef HAL_GTEST
  if (!write_mem(q_addr, (uint8_t *)buf, q_size))
    return -EIO;
#endif
  return 0;
}

int32_t get_pc_offset(const char *handle, const char *prog_name,
                      const char *label, uint8_t *offset) {
#ifndef HAL_GTEST
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
#endif
  return 0;
}
