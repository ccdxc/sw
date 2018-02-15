#include "dol/test/storage/ssd_core.hpp"
#include "dol/test/storage/utils.hpp"
#include <assert.h>
#include <unistd.h>
#include <stddef.h>

#define PRP_SIZE(x) (4096ul - ((x) & 0xfff))
#define PRP_VALID(x) (((x) & 7ul) == 0)
#define IS_PAGE_ALIGNED(x)  ((((uint64_t)(x)) & 0xFFF) == 0)

namespace storage_test {

void NvmeSsdCore::PollForIO() {
  while (!terminate_poll_) {
    HandleIO();
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
  params->subq = subq_;
  params->compq = compq_;
  params->subq_pi = subq_pi_;
  params->subq_ci = subq_ci_;
  params->compq_pi = compq_pi_;
  params->compq_ci = compq_ci_;
}

void NvmeSsdCore::Ctor() {
  data_.reset(new uint8_t[kCapacity]);
  subq_ = new dp_mem_t(kNumSubqEntries, sizeof(NvmeCmd));
  compq_ = new dp_mem_t(kNumCompqEntries, sizeof(NvmeStatus));
  phase_ = 1;
  ctrl_ = new dp_mem_t(1, sizeof(struct ctrl_data));

  subq_pi_ = ctrl_->fragment_find(offsetof(struct ctrl_data, subq_pi),
                                  sizeof(((struct ctrl_data *)0)->subq_pi));
  subq_ci_ = ctrl_->fragment_find(offsetof(struct ctrl_data, subq_ci),
                                  sizeof(((struct ctrl_data *)0)->subq_ci));
  compq_pi_ = ctrl_->fragment_find(offsetof(struct ctrl_data, compq_pi),
                                   sizeof(((struct ctrl_data *)0)->compq_pi));
  compq_ci_ = ctrl_->fragment_find(offsetof(struct ctrl_data, compq_ci),
                                   sizeof(((struct ctrl_data *)0)->compq_ci));
  terminate_poll_ = false;
  tid_ = std::thread(&NvmeSsdCore::PollForIO, this);
}

void NvmeSsdCore::Dtor() {
  terminate_poll_ = true;
  tid_.join();
  delete subq_;
  delete compq_;
  delete ctrl_;
}

uint64_t NvmeSsdCore::MoveData(NvmeCmd *cmd, uint64_t prp, uint64_t offset,
                           uint64_t size_left) {
  uint64_t copy_size = std::min(PRP_SIZE(prp), size_left);
 
  if (cmd->dw0.opc == NVME_READ_CMD_OPCODE) {
    DMAMemCopyV2P(&data_[offset], prp, copy_size);
  } else {
    DMAMemCopyP2V(prp, &data_[offset], copy_size);
  } 
  return copy_size;
}

bool NvmeSsdCore::ExecuteRW(NvmeCmd *cmd) {
  //printf("NVME command received by SSD\n");
  //utils::dump((uint8_t *) cmd);

  if ((cmd->dw0.opc != NVME_READ_CMD_OPCODE) &&
      (cmd->dw0.opc != NVME_WRITE_CMD_OPCODE)) {
    fprintf(stderr, "Invalid nvme cmd = 0x%x\n", cmd->dw0.opc);
    return false;
  }
  if (NVME_CMD_PSDT(*cmd) != 0) {
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
  std::unique_ptr<uint64_t[]> plist(new uint64_t[8]);
  uint64_t *prp_list = plist.get();
  DMAMemCopyP2V(cmd->prp.prp2, prp_list, std::min(PRP_SIZE(cmd->prp.prp2), 64ul));
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
    uint32_t *pi = (uint32_t *)subq_pi_->read_thru();
    uint32_t *ci = (uint32_t *)subq_ci_->read_thru();

    if (*pi == *ci)
      return false;
    subq_->line_set(*ci);
    memcpy(&cmd, subq_->read_thru(), sizeof(cmd));
    *ci = (*ci + 1) % kNumSubqEntries;
    subq_ci_->write_thru();
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
  uint32_t *pi;
  uint32_t *ci;

  bzero(&st, sizeof(st));
  NVME_STATUS_SET_STATUS(st, code);
  st.dw3.cid = cmd->dw0.cid;

  bool err = false;
  {
    std::lock_guard<std::mutex> l(comp_lock_);
    uint8_t ph = phase_;
    pi = (uint32_t *)compq_pi_->read_thru();
    ci = (uint32_t *)compq_ci_->read_thru();
    uint32_t n = *pi;

    (*pi)++;
    if (*pi == kNumCompqEntries) {
      *pi = 0;
      phase_ ^= 1;
    }
    if (*pi == *ci) {
      err = true;
    } else {
      NVME_STATUS_SET_PHASE(st, ph);
      st.dw2.sq_head = *((uint32_t *)subq_ci_->read_thru());
      compq_->line_set(n);
      memcpy(compq_->read(), &st, sizeof(st));
      compq_->write_thru();
      compq_pi_->write_thru();
    }
  }
  if (err) {
    fprintf(stderr, "Internal error: response queue full\n");
  } else {
    RaiseInterrupt(*pi);
  }
}

}  // namespace storage_test
