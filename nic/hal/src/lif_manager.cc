#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/pd/pd_api.hpp"
// #include "nic/hal/pd/capri/capri_hbm.hpp"

#if 0
void push_qstate_to_capri(hal::LIFQState *qstate);
void clear_qstate(hal::LIFQState *qstate);
int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(const char *handle, const char *prog_name,
                      const char *label, uint8_t *offset);
#endif

const static char *kHBMLabel = "lif2qstate_map";
const static uint32_t kHBMSizeKB = 131072;
const static uint32_t kAllocUnit = 4096;

namespace hal {

LIFManager::LIFManager() {
  pd::pd_get_start_offset_args_t off_args = {0};
  pd::pd_get_size_kb_args_t size_args = {0};

  off_args.reg_name = kHBMLabel;
  pd::hal_pd_call(pd::PD_FUNC_ID_GET_START_OFFSET, (void *)&off_args);
  uint64_t hbm_addr = off_args.offset;
  assert(hbm_addr > 0);

  size_args.reg_name = kHBMLabel;
  pd::hal_pd_call(pd::PD_FUNC_ID_GET_REG_SIZE, (void *)&size_args);
  assert(size_args.size == kHBMSizeKB);

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

  pd::pd_push_qstate_to_capri_args_t args = {0};
  args.qstate = qstate;
  pd::hal_pd_call(pd::PD_FUNC_ID_PUSH_QSTATE, (void*)&args);

  return 0;
}

void LIFManager::DeleteLIFQStateImpl(LIFQState *qstate) {
  pd::pd_clear_qstate_args_t args = {0};
  args.qstate = qstate;
  pd::hal_pd_call(pd::PD_FUNC_ID_CLEAR_QSTATE, (void*)&args);
  int alloc_offset = qstate->hbm_address - hbm_base_;
  if (allocation_sizes_.find(alloc_offset) != allocation_sizes_.end()) {
    hbm_allocator_->Free(alloc_offset, allocation_sizes_[alloc_offset]);
    allocation_sizes_.erase(alloc_offset);
  }
}

int32_t LIFManager::ReadQStateImpl(
    uint64_t q_addr, uint8_t *buf, uint32_t q_size) {
    pd::pd_read_qstate_args_t args = {0};
    args.q_addr = q_addr;
    args.buf = buf;
    args.q_size = q_size;
    pd::hal_pd_call(pd::PD_FUNC_ID_READ_QSTATE, (void*)&args);

  return 0;
}

int32_t LIFManager::WriteQStateImpl(
    uint64_t q_addr, const uint8_t *buf, uint32_t q_size) {
  pd::pd_write_qstate_args_t args = {0};
  args.q_addr = q_addr;
  args.buf = buf;
  args.q_size = q_size;
  pd::hal_pd_call(pd::PD_FUNC_ID_WRITE_QSTATE, (void*)&args);

  return 0;
}

int32_t LIFManager::GetPCOffset(
    const char *handle, const char *prog_name,
    const char *label, uint8_t *ret_offset) {
  pd::pd_get_pc_offset_args_t args;
  args.handle = handle;
  args.prog_name = prog_name;
  args.label = label;
  args.offset = ret_offset;
  return pd::hal_pd_call(pd::PD_FUNC_ID_GET_PC_OFFSET, (void*)&args);
}

}  // namespace hal
