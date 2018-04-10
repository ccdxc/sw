#ifndef DOL_TEST_STORAGE_SSD_CORE_HPP
#define DOL_TEST_STORAGE_SSD_CORE_HPP

#include <memory>
#include <mutex>
#include <stdint.h>
#include <strings.h>
#include <thread>
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/dp_mem.hpp"

using namespace dp_mem;

namespace storage_test {

// Working parameters of an SSD.
struct SsdWorkingParams {
  // Basic parameters
  uint64_t byte_capacity;
  uint32_t namespace_id;
  uint32_t blocksize;
  uint64_t num_blocks;

  // Queue parameters
  uint64_t subq_nentries;
  uint64_t compq_nentries;
  dp_mem_t *subq;
  dp_mem_t *compq;

  // Producer/Consumer indices
  uint32_t *subq_pi_va;
  uint64_t subq_pi_pa;
  uint32_t *compq_ci_va;
  uint64_t compq_ci_pa;
};

// SSD simulation for DOL. Points to note:
//   - This SSD comes with no admin queue and one I/O queue
//     pair pre-initialized.
//   - Currently the SSD has a fixed capacity of 1MB which is
//     non-persistent.
//   - It only supports Read/Write commands with PRPs. 
//   - The max payload supported is 64KB per command (9 PRPs).
//   - The SSDs operates using libhostmem.so
class NvmeSsdCore {
 public:
  static constexpr uint64_t kCapacity = 1024 * 1024;
  static constexpr uint64_t kNumSubqEntries = 64;
  static constexpr uint64_t kNumCompqEntries = 64;
  static constexpr uint64_t kMaxIOSize = 64 * 1024;

  // Pure virtual functions.
  virtual void *DMAMemAlloc(uint32_t size) = 0;
  virtual void DMAMemFree(void *ptr) = 0;
  virtual uint64_t DMAMemV2P(void *ptr) = 0;
  virtual void *DMAMemP2V(uint64_t addr) = 0;
  virtual void DMAMemCopyP2V(uint64_t pa, void *va, uint32_t size) = 0;
  virtual void DMAMemCopyV2P(void *va, uint64_t pa, uint32_t size) = 0;
  virtual void RaiseInterrupt(uint16_t index) = 0;

  // The actual contructor and destructor.
  // Since they call pure virtual functions, they need to be
  // called from the implementation's ctor/dtor.
  void Ctor();
  void Dtor();

  void GetWorkingParams(SsdWorkingParams *params);
  void PollForIO();

 private:

  uint64_t MoveData(NvmeCmd *cmd, uint64_t prp, uint64_t offset,
                    uint64_t size_left);
  bool ExecuteRW(NvmeCmd *cmd);
  bool HandleIO();
  void SendResp(NvmeCmd *cmd, uint32_t code);

  std::thread tid_;
  bool terminate_poll_;
  std::unique_ptr<uint8_t[]> data_;
  uint8_t phase_;
  std::mutex req_lock_, comp_lock_;
  uint64_t intr_addr_, intr_data_;
  bool intr_enabled_ = false;
  dp_mem_t *subq_;
  dp_mem_t *compq_;
  // Pads are to lineup host_mem access at 64 bytes for RTL simulation checks
  struct ctrl_data {
    uint32_t subq_pi;
    uint8_t pad0[60];
    uint32_t subq_ci;
    uint8_t pad1[60];
    uint32_t compq_pi;
    uint8_t pad2[60];
    uint32_t compq_ci;
    uint8_t pad3[60];
  } *ctrl_;
}; 

}  // namespace storage_test

#endif  // DOL_TEST_STORAGE_SSD_CORE_HPP
