#include <lif_manager.hpp>
#include <pd/capri/capri_hbm.hpp>

void push_qstate_to_capri(hal::LIFQState *qstate);
int32_t read_qstate(uint32_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint32_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(char *prog_name, char *label, uint8_t *offset);

const static char *kHBMLabel = "lif2qstate_map";
const static uint32_t kHBMSizeKB = 16384;
const static uint32_t kAllocUnit = 4096;

namespace hal {

LIFManager::LIFManager() {
  uint32_t hbm_addr = get_start_offset(kHBMLabel);
  assert(hbm_addr > 0);
  assert(get_size_kb(kHBMLabel) == kHBMSizeKB);
  uint32_t num_units = (kHBMSizeKB * 1024) / kAllocUnit;
  if (hbm_addr & 0xFFF) {
    // Not 4K aligned.
    hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFU;
    num_units--;
  }
  hbm_base_ = hbm_addr;
  hbm_allocator_.reset(new BMAllocator(num_units));
}

int32_t LIFManager::InitLIFQStateImpl(LIFQState *qstate) {
  uint32_t alloc_units;

  alloc_units = (qstate->allocation_size + kAllocUnit - 1) & ~kAllocUnit;
  alloc_units /= kAllocUnit;
  int alloc_offset = hbm_allocator_->Alloc(alloc_units);
  if (alloc_offset < 0)
    return -ENOMEM;
  allocation_sizes_[alloc_offset] = alloc_units;
  alloc_offset *= kAllocUnit;
  qstate->hbm_address = hbm_base_ + alloc_offset;

  push_qstate_to_capri(qstate);

  return 0;
}

int32_t LIFManager::ReadQStateImpl(
    uint32_t q_addr, uint8_t *buf, uint32_t q_size) {
  return read_qstate(q_addr, buf, q_size);
}

int32_t LIFManager::WriteQStateImpl(
    uint32_t q_addr, uint8_t *buf, uint32_t q_size) {
  return write_qstate(q_addr, buf, q_size);
}

int32_t LIFManager::GetPCOffset(
    char *prog_name, char *label, uint8_t *ret_offset) {
  return get_pc_offset(prog_name, label, ret_offset);
}

}  // namespace hal
