#include <stdio.h>
#include <strings.h>

#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/host_mem/c_if.h"
#include "dol/test/storage/model_client/lib_model_client.h"

const static uint32_t	kNvmeNumQs		 = 1;
const static uint32_t	kNvmeNumQsVal		 = (1 << kNvmeNumQs);
const static uint32_t	kNvmeCmdSize		 = 6;
const static uint32_t	kNvmeCmdSizeVal		 = (1 << kNvmeCmdSize);
const static uint32_t	kPvmCmdSize		 = 6;
const static uint32_t	kPvmCmdSizeVal		 = (1 << kPvmCmdSize);
const static uint32_t	kNvmeNumEntries		 = 6;
const static uint32_t	kNvmeNumEntriesVal	 = (1 << kNvmeNumEntries);
const static uint32_t	kPvmNumEntries		 = 6;
const static uint32_t	kPvmNumEntriesVal	 = (1 << kPvmNumEntries);
const static char	*kNvmeSqHandler		 = "storage_tx_nvme_sq_handler.bin";
const static char	*kPvmCqHandler		 = "storage_tx_pvm_cq_handler.bin";
const static uint32_t	kDefaultTotalRings	 = 1;
const static uint32_t	kDefaultHostRings	 = 1;
const static uint32_t	kDefaultNoHostRings	 = 0;
const static uint32_t	kNvmeBeTotalRings	 = 3;
const static uint32_t	kNvmeBeHostRings	 = 0;

const static uint32_t	kDbAddrHost		 = 0x400000;
const static uint32_t	kDbAddrUpdate		 = 0xB;
const static uint32_t	kDbQidShift		 = 24;
const static uint32_t	kDbRingShift		 = 16;
const static uint32_t	kDbUpdateShift		 = 17;
const static uint32_t	kDbLifShift		 = 6;
const static uint32_t	kDbTypeShift		 = 3;

namespace tests {

uint64_t nvme_lif, pvm_lif;
uint64_t nvme_paddr[kNvmeNumQsVal], pvm_paddr;
uint8_t *nvme_queue[kNvmeNumQsVal], *pvm_queue;

int test_setup() {
  // Initialize hal interface
  hal_if::init_hal_if();
  printf("HAL client initialized\n");

  // Initialize host memory
  if (init_host_mem() < 0) {
    printf("Host mem init failed (is model running?)\n");
    return -1;
  }
  printf("Host mem initialized\n");

  // Initialize model client
  if (lib_model_connect() < 0) {
    printf("Failed to connect with model (is model running?)\n");
    return -1;
  }
  printf("Model client initialized\n");

  // Create NVME and PVM LIFs
  hal_if::lif_params_t nvme_lif_params, pvm_lif_params;
  bzero(&nvme_lif_params, sizeof(nvme_lif_params));
  bzero(&pvm_lif_params, sizeof(pvm_lif_params));
  
  nvme_lif_params.type[0].valid = true; 
  nvme_lif_params.type[0].queue_size = kNvmeNumEntries; 
  nvme_lif_params.type[0].num_queues = kNvmeNumQs;  // you get 2^NumQs 
  if (hal_if::create_lif(&nvme_lif_params, &nvme_lif) < 0) {
    printf("can't create nvme lif \n");
    return -1;
  }

  pvm_lif_params.type[0].valid = true; 
  pvm_lif_params.type[0].queue_size = kPvmNumEntries; 
  pvm_lif_params.type[0].num_queues = 0;  // 0 => 2^0 = 1 queue
  if (hal_if::create_lif(&pvm_lif_params, &pvm_lif) < 0) {
    printf("can't create pvm lif \n");
    return -1;
  }
  printf("NVME and PVM LIFs created\n");

  // Allocate host memory for NVME and PVM queues and get the Physical Addresses
  for (int i = 0; i < (int) kNvmeNumQsVal; i++) {
    nvme_queue[i] = (uint8_t *)alloc_host_mem(kNvmeNumEntriesVal * kNvmeCmdSizeVal);
    if (nvme_queue[i] == nullptr) {
      printf("Unable to allocate host memory for nvme_queue %d\n", i);
      return -1;
    }
    nvme_paddr[i] = host_mem_v2p(nvme_queue[i]);
  }
  pvm_queue = (uint8_t *)alloc_host_mem(kPvmNumEntriesVal * kPvmCmdSizeVal);
  if (pvm_queue == nullptr) {
    printf("Unable to allocate host memory for pvm_queue \n");
    return -1;
  }
  pvm_paddr = host_mem_v2p(pvm_queue);
  printf("Host memory allocated\n");

  // Setup the NVME and PVM queue states in Capri
  for (int i = 0; i < (int) kNvmeNumQsVal; i++) {
    if (qstate_if::setup_q_state(nvme_lif, 0, i, (char *) kNvmeSqHandler, 
                                 kDefaultTotalRings, kDefaultHostRings, 
                                 kNvmeNumEntries, nvme_paddr[i], kNvmeCmdSize,
                                 pvm_lif, 0, 0, 0, 0, 0, 0, 0) < 0) {
      printf("Failed to setup NVME queue %d state \n", i);
      return -1;
    }
    printf("Setup NVME queue %d state \n", i);
  }

  if (qstate_if::setup_q_state(pvm_lif, 0, 0, (char *) kPvmCqHandler,
                               kDefaultTotalRings, kDefaultHostRings,
                               kPvmNumEntries, pvm_paddr, kPvmCmdSize, 
                               nvme_lif, 0, 0, 0, 0, 0, 0, 0) < 0) {
    printf("Failed to setup PVM queue state \n");
    return -1;
  }
  printf("Setup PVM queue state \n");

  return 0;
}


void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                        uint8_t ring, uint16_t index) {

  uint64_t db_data = (qid << kDbQidShift) | (ring << kDbRingShift) | index;
  uint64_t db_addr = kDbAddrHost |  (kDbAddrUpdate << kDbUpdateShift) | 
                     (lif << kDbLifShift) | (qtype << kDbLifShift);

  step_doorbell(db_addr, db_data);
}

int send_cmd_and_check(uint8_t *send_cmd, uint8_t *recv_cmd, uint32_t cmd_size,
                       uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring,
                       uint16_t index) {
  int rc;

  rc = memcmp(send_cmd, recv_cmd, cmd_size);
  printf("PRE doorbell cmd comparison %d \n", rc);

  test_ring_doorbell(lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, cmd_size);
  printf("POST doorbell cmd comparison %d \n", rc);

  return rc;
}

int test_run_nvme_pvm_admin_cmd() {
  // Get NVME queue 0, entry 0 to post admin command
  uint8_t *nvme_cmd = nvme_queue[0];

  // Get PVM queue 0, entry 0 to check admin command
  uint8_t *pvm_cmd = pvm_queue;

  int rc;

  bzero(nvme_cmd, kNvmeCmdSizeVal);
  bzero(pvm_cmd, kPvmCmdSizeVal);

  struct NvmeCmd *admin_cmd = (struct NvmeCmd *) nvme_cmd;
  admin_cmd->dw0.opc = NVME_ADMIN_CMD_CREATE_SQ;
  // These values are not interpretted in DOL testing
  admin_cmd->dw10_11.qid = 1;
  admin_cmd->dw10_11.qsize = 64;

  // Send the NVME admin command and check on PVM side
  rc = send_cmd_and_check(nvme_cmd, pvm_cmd, kNvmeCmdSizeVal, 
                          nvme_lif, 0, 0, 0, 1);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}


int test_run_nvme_pvm_read_cmd() {
  // Get NVME queue 1, entry 0 to post read command
  uint8_t *nvme_cmd = nvme_queue[1];

  // Get PVM queue 0, entry 1 to check read command
  // (because previous test case would have sent it to index 0)
  uint8_t *pvm_cmd = pvm_queue + kPvmCmdSizeVal;

  int rc;

  bzero(nvme_cmd, kNvmeCmdSizeVal);
  bzero(pvm_cmd, kPvmCmdSizeVal);

  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;

  // Send the NVME admin command and check on PVM side
  rc = send_cmd_and_check(nvme_cmd, pvm_cmd, kNvmeCmdSizeVal, 
                          nvme_lif, 0, 1, 0, 1);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

}  // namespace tests
