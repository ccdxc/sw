#include "dol/test/storage/ssd_core.hpp"
#include <assert.h>
#include <unistd.h>

#define PRP_SIZE(x) (4096ul - ((x) & 0xfff))
#define PRP_VALID(x) (((x) & 7ul) == 0)
#define IS_PAGE_ALIGNED(x)  ((((uint64_t)(x)) & 0xFFF) == 0)

namespace storage_test {

void NvmeSsdCore::PollForIO() {
  while (!terminate_poll_) {
    if (!HandleIO())
      usleep(1000);
  }
}

void NvmeSsdCore::GetWorkingParams(SsdWorkingParams *params) {
  params->byte_capacity = kCapacity;
  params->namespace_id = 1;
  params->blocksize = 4096;
  params->num_blocks = kCapacity/4096;
  params->subq_nentries = kNumSubqEntries;
  params->compq_nentries = kNumCompqEntries;
  params->subq_va = subq_;
  params->compq_va = compq_;
  params->subq_pa = DMAMemV2P(subq_);
  params->compq_pa = DMAMemV2P(compq_);
  params->subq_pi_va = &ctrl_->subq_pi;
  params->subq_pi_pa = DMAMemV2P(&ctrl_->subq_pi);
  params->compq_ci_va = &ctrl_->compq_ci;
  params->compq_ci_pa = DMAMemV2P(&ctrl_->compq_ci);
}

void NvmeSsdCore::Ctor() {
  data_.reset(new uint8_t[kCapacity]);
  subq_ = (NvmeCmd *)DMAMemAlloc(kNumSubqEntries * sizeof(NvmeCmd));
  assert(subq_ != nullptr);
  bzero(subq_, kNumSubqEntries * sizeof(NvmeCmd));
  compq_ = (NvmeStatus *)DMAMemAlloc(kNumCompqEntries * sizeof(NvmeStatus));
  assert(compq_ != nullptr);
  bzero(compq_, kNumCompqEntries * sizeof(NvmeStatus));
  phase_ = 1;
  ctrl_ = (ctrl_data *)DMAMemAlloc(sizeof(*ctrl_));
  assert(ctrl_ != nullptr);
  bzero(ctrl_, sizeof(*ctrl_));
  terminate_poll_ = false;
  tid_ = std::thread(&NvmeSsdCore::PollForIO, this);
}

void NvmeSsdCore::Dtor() {
  terminate_poll_ = true;
  tid_.join();
  DMAMemFree(subq_);
  DMAMemFree(compq_);
  DMAMemFree(ctrl_);
}
 
uint64_t NvmeSsdCore::MoveData(NvmeCmd *cmd, uint64_t prp, uint64_t offset,
                           uint64_t size_left) {
  uint64_t copy_size = std::min(PRP_SIZE(prp), size_left);
  if (cmd->dw0.opc == NVME_READ_CMD_OPCODE) {
    bcopy(&data_[offset], DMAMemP2V(prp), copy_size);
  } else {
    bcopy(DMAMemP2V(prp), &data_[offset], copy_size);
  }
  return copy_size;
}

bool NvmeSsdCore::ExecuteRW(NvmeCmd *cmd) {
  if ((cmd->dw0.opc != NVME_READ_CMD_OPCODE) &&
      (cmd->dw0.opc != NVME_WRITE_CMD_OPCODE)) {
    fprintf(stderr, "Invalid nvme cmd = 0x%x\n", cmd->dw0.opc);
    return false;
  }
  if (cmd->dw0.psdt != 0) {
    fprintf(stderr, "Only PRPs expected\n");
    return false;
  }
  if (cmd->nsid != 1) {
    fprintf(stderr, "Invalid namespace identifier 0x%x\n", cmd->nsid);
    return false;
  }
  uint64_t size = cmd->dw12.nlb;
  size += 1;
  size *= 4096;
  uint64_t offset = cmd->slba * 4096;
  if (size > kMaxIOSize) {
    fprintf(stderr, "I/O size exceeding max limit, slba=0x%lx nblas=0x%x\n",
            cmd->slba, (unsigned)cmd->dw12.nlb);
    return false;
  }
  if ((offset + size) > kCapacity) {
    fprintf(stderr, "I/O exceeding device capacity, slba=0x%lx nblas=0x%x\n",
            cmd->slba, (unsigned)cmd->dw12.nlb);
    return false;
  }
  if (!PRP_VALID(cmd->prp.prp1)) {
    fprintf(stderr, "Invalid prp1\b");
    return false;
  }
  uint64_t size_done = MoveData(cmd, cmd->prp.prp1, offset, size);
  if (size_done >= size)
    return true;
  if (!PRP_VALID(cmd->prp.prp2)) {
    fprintf(stderr, "Invalid prp2\b");
    return false;
  }
  if (PRP_SIZE(cmd->prp.prp2) >= (size - size_done)) {
    MoveData(cmd, cmd->prp.prp2, offset + size_done, size - size_done);
    return true;
  }
  // Pull PRP List.
  uint64_t *prp_list = (uint64_t *)DMAMemP2V(cmd->prp.prp2);
  int ndx = 0;
  while (size_done < size) {
    if (ndx == 8) {
      fprintf(stderr, "Not expecting more than 8 entries in the prp list\n");
      return false;
    }
    if ((ndx != 0) && (IS_PAGE_ALIGNED(prp_list))) {
      fprintf(stderr, "prp list crossing page boundary\n");
      return false;
    }
    if (!PRP_VALID(prp_list[ndx])) {
      fprintf(stderr, "Entry %d in the prp_list(0x%lx) is invalid\n",
              ndx, prp_list[ndx]);
      return false;
    }
    size_done += MoveData(cmd, prp_list[ndx], offset + size_done, size - size_done);
    ndx++;
  }
  return true;
}

bool NvmeSsdCore::HandleIO() {
  NvmeCmd cmd;

  {
    std::lock_guard<std::mutex> l(req_lock_);
    if (ctrl_->subq_pi == ctrl_->subq_ci)
      return false;
    bcopy(&subq_[ctrl_->subq_ci], &cmd, sizeof(cmd));
    ctrl_->subq_ci++;
    if (ctrl_->subq_ci == kNumSubqEntries)
      ctrl_->subq_ci = 0;
  }
  if (!NvmeSsdCore::ExecuteRW(&cmd)) {
    SendResp(&cmd, 2);
    return true;
  }
  SendResp(&cmd, 0);
  return true;
}

void NvmeSsdCore::SendResp(NvmeCmd *cmd, uint32_t code) {
  NvmeStatus st;

  bzero(&st, sizeof(st));
  st.dw3.status = code;
  st.dw3.cid = cmd->dw0.cid;

  bool err = false;
  {
    std::lock_guard<std::mutex> l(comp_lock_);
    uint16_t n = ctrl_->compq_pi;
    uint8_t ph = phase_;
    ctrl_->compq_pi++;
    if (ctrl_->compq_pi == kNumCompqEntries) {
      ctrl_->compq_pi = 0;
      phase_ ^= 1;
    }
    if (ctrl_->compq_pi == ctrl_->compq_ci) {
      err = true;
    } else {
      st.dw3.phase = ph & 1;
      st.dw2.sq_head = ctrl_->subq_ci;
      bcopy(&st, &compq_[n], sizeof(st));
    }
  }
  if (err) {
    fprintf(stderr, "Internal error: response queue full\n");
  } else {
    RaiseInterrupt();
  }
}

}  // namespace storage_test
