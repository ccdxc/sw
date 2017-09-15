#include <stdio.h>
#include <strings.h>

#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/host_mem/c_if.h"
#include "dol/test/storage/model_client/lib_model_client.h"

const static uint32_t	kDbAddrHost		 = 0x400000;
const static uint32_t	kDbAddrUpdate		 = 0xB;
const static uint32_t	kDbQidShift		 = 24;
const static uint32_t	kDbRingShift		 = 16;
const static uint32_t	kDbUpdateShift		 = 17;
const static uint32_t	kDbLifShift		 = 6;
const static uint32_t	kDbTypeShift		 = 3;

namespace tests {

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

 
  // Initialize queues
  if (queues::queues_setup() < 0) {
    printf("Failed to setup lif and quuees \n");
    return -1;
  }
  printf("Setup lif and quuees \n");

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

  printf("Sending command size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", cmd_size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, cmd_size);
  printf("PRE doorbell cmd comparison %d \n", rc);

  test_ring_doorbell(lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, cmd_size);
  printf("POST doorbell cmd comparison %d \n", rc);

  return rc;
}

int test_run_nvme_pvm_admin_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;

  // Consume NVME queue 0 entry to post admin command
  uint8_t *nvme_cmd = (uint8_t *) queues::nvme_sq_consume_entry(0, &nvme_index);

  // Consume PVM queue 0 to check admin command
  uint8_t *pvm_cmd = (uint8_t *) queues::pvm_sq_consume_entry(0, &pvm_index);

  if (nvme_cmd == nullptr || pvm_cmd == nullptr) {
    printf("can't consume entries \n");
  }

  bzero(nvme_cmd, sizeof(struct NvmeCmd));
  bzero(pvm_cmd, sizeof(struct NvmeCmd)); // TODO: This should be PvmCmd

  struct NvmeCmd *admin_cmd = (struct NvmeCmd *) nvme_cmd;
  admin_cmd->dw0.opc = NVME_ADMIN_CMD_CREATE_SQ;
  // These values are not interpretted in DOL testing
  admin_cmd->dw10_11.qid = 1;
  admin_cmd->dw10_11.qsize = 64;

  // Send the NVME admin command and check on PVM side
  rc = send_cmd_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                          queues::get_nvme_lif(), SQ_TYPE, 0, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}


int test_run_nvme_pvm_read_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;

  // Consume NVME queue 1 entry to post read command
  uint8_t *nvme_cmd = (uint8_t *) queues::nvme_sq_consume_entry(1, &nvme_index);

  // Consume PVM queue 1 to check read command
  uint8_t *pvm_cmd = (uint8_t *) queues::pvm_sq_consume_entry(1, &pvm_index);

  if (nvme_cmd == nullptr || pvm_cmd == nullptr) {
    printf("can't consume entries \n");
  }

  bzero(nvme_cmd, sizeof(struct NvmeCmd));
  bzero(pvm_cmd, sizeof(struct NvmeCmd)); // TODO: This should be PvmCmd

  void *data = (uint8_t *)alloc_host_mem(4*1024);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  read_cmd->prp.prp1 = (uint64_t) data;
  read_cmd->slba = 0x6;
  read_cmd->dw12.nlb = 0x1;

  // Send the NVME admin command and check on PVM side
  rc = send_cmd_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                          queues::get_nvme_lif(), SQ_TYPE, 1, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_write_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;

  // Consume NVME queue 1 entry to post write command
  uint8_t *nvme_cmd = (uint8_t *) queues::nvme_sq_consume_entry(1, &nvme_index);

  // Consume PVM queue 1 to check write command
  uint8_t *pvm_cmd = (uint8_t *) queues::pvm_sq_consume_entry(1, &pvm_index);

  if (nvme_cmd == nullptr || pvm_cmd == nullptr) {
    printf("can't consume entries \n");
  }

  bzero(nvme_cmd, sizeof(struct NvmeCmd));
  bzero(pvm_cmd, sizeof(struct NvmeCmd)); // TODO: This should be PvmCmd

  void *data = (uint8_t *)alloc_host_mem(4*1024);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  read_cmd->prp.prp1 = (uint64_t) data;
  read_cmd->slba = 0x6;
  read_cmd->dw12.nlb = 0x1;

  // Send the NVME admin command and check on PVM side
  rc = send_cmd_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                          queues::get_nvme_lif(), SQ_TYPE, 1, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

}  // namespace tests
