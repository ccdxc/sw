#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"

void push_qstate_to_capri(hal::LIFQState *qstate);
void clear_qstate(hal::LIFQState *qstate);
int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(const char *handle, const char *prog_name,
                      const char *label, uint8_t *offset);

const static char *kHBMLabel = "lif2qstate_map";
const static uint32_t kHBMSizeKB = 20480;
const static uint32_t kAllocUnit = 4096;

namespace hal {

LIFManager::LIFManager() {
  uint64_t hbm_addr = get_start_offset(kHBMLabel);
  assert(hbm_addr > 0);
  assert(get_size_kb(kHBMLabel) == kHBMSizeKB);
  uint32_t num_units = (kHBMSizeKB * 1024) / kAllocUnit;
  if (hbm_addr & 0xFFF) {
    // Not 4K aligned.
    hbm_addr = (hbm_addr + 0xFFFull) & ~0xFFFull;
    num_units--;
  }
  hbm_base_ = hbm_addr;
  hbm_allocator_.reset(new BMAllocator(num_units));
}

int32_t LIFManager::InitLIFQStateImpl(LIFQState *qstate) {
  uint32_t alloc_units;

  alloc_units = (qstate->allocation_size + kAllocUnit - 1) & ~(kAllocUnit - 1);
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

void LIFManager::DeleteLIFQStateImpl(LIFQState *qstate) {
  clear_qstate(qstate);
  int alloc_offset = qstate->hbm_address - hbm_base_;
  if (allocation_sizes_.find(alloc_offset) != allocation_sizes_.end()) {
    hbm_allocator_->Free(alloc_offset, allocation_sizes_[alloc_offset]);
    allocation_sizes_.erase(alloc_offset);
  }
}

int32_t LIFManager::ReadQStateImpl(
    uint64_t q_addr, uint8_t *buf, uint32_t q_size) {
  return read_qstate(q_addr, buf, q_size);
}

int32_t LIFManager::WriteQStateImpl(
    uint64_t q_addr, const uint8_t *buf, uint32_t q_size) {
  return write_qstate(q_addr, buf, q_size);
}

int32_t LIFManager::GetPCOffset(
    const char *handle, const char *prog_name,
    const char *label, uint8_t *ret_offset) {
  return get_pc_offset(handle, prog_name, label, ret_offset);
}

}  // namespace hal
