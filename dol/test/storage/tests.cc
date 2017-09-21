#include <stdio.h>
#include <strings.h>

#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/r2n.hpp"
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
    printf("Failed to setup lif and queues \n");
    return -1;
  }
  printf("Setup lif and queues \n");

  return 0;
}


void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                        uint8_t ring, uint16_t index) {

  uint64_t db_data = (qid << kDbQidShift) | (ring << kDbRingShift) | index;
  uint64_t db_addr = kDbAddrHost |  (kDbAddrUpdate << kDbUpdateShift) | 
                     (lif << kDbLifShift) | (qtype << kDbTypeShift);

  step_doorbell(db_addr, db_data);
}

int send_and_check(uint8_t *send_cmd, uint8_t *recv_cmd, uint32_t size,
                   uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring,
                   uint16_t index) {
  int rc;

  printf("Sending data size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, size);
  printf("PRE doorbell cmd comparison %d \n", rc);

  test_ring_doorbell(lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, size);
  printf("POST doorbell cmd comparison %d \n", rc);

  return rc;
}

int consume_nvme_pvm_sq_entries(uint16_t nvme_q, uint16_t pvm_q, 
                                uint8_t **nvme_cmd, uint8_t **pvm_cmd, 
                                uint16_t *nvme_index, uint16_t *pvm_index) {

  if (!nvme_cmd || !pvm_cmd || !nvme_index || !pvm_index) {
    return -1;
  }
  *nvme_cmd = (uint8_t *) queues::nvme_sq_consume_entry(nvme_q, nvme_index);
  *pvm_cmd = (uint8_t *) queues::pvm_sq_consume_entry(pvm_q, pvm_index);

  if (*nvme_cmd == nullptr || *pvm_cmd == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }

  bzero(*nvme_cmd, sizeof(struct NvmeCmd));
  bzero(*pvm_cmd, sizeof(struct NvmeCmd)); // TODO: This should be PvmCmd
  return 0;
}

int consume_nvme_pvm_cq_entries(uint16_t nvme_q, uint16_t pvm_q, 
                                uint8_t **nvme_status, uint8_t **pvm_status, 
                                uint16_t *nvme_index, uint16_t *pvm_index) {

  if (!nvme_status || !pvm_status || !nvme_index || !pvm_index) {
    return -1;
  }
  *nvme_status = (uint8_t *) queues::nvme_cq_consume_entry(nvme_q, nvme_index);
  *pvm_status = (uint8_t *) queues::pvm_cq_consume_entry(pvm_q, pvm_index);

  if (*nvme_status == nullptr || *pvm_status == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }

  bzero(*nvme_status, sizeof(struct NvmeStatus));
  bzero(*pvm_status, sizeof(struct PvmStatus)); 
  return 0;
}

int test_run_nvme_pvm_admin_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_cmd, *pvm_cmd;
  // Consume NVME queue 0 entry to post admin command
  // Consume PVM queue 0 to check admin command
  uint16_t nvme_q = 0, pvm_q = 0;

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct NvmeCmd *admin_cmd = (struct NvmeCmd *) nvme_cmd;
  admin_cmd->dw0.opc = NVME_ADMIN_CMD_CREATE_SQ;
  // These values are not interpretted in DOL testing
  admin_cmd->dw10_11.qid = 1;
  admin_cmd->dw10_11.qsize = 64;

  // Send the NVME admin command and check on PVM side
  rc = send_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                      queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}


int test_run_nvme_pvm_read_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_cmd, *pvm_cmd;
  // Consume NVME queue 1 entry to post read command
  // Consume PVM queue 1 to check read command
  uint16_t nvme_q = 1, pvm_q = 1;

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  void *data = (uint8_t *)alloc_host_mem(4*1024);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  read_cmd->prp.prp1 = (uint64_t) data;
  read_cmd->slba = 0x5;
  read_cmd->dw12.nlb = 0x1;

  // Send the NVME admin command and check on PVM side
  rc = send_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                      queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_write_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_cmd, *pvm_cmd;
  // Consume NVME queue 1 entry to post write command
  // Consume PVM queue 1 to check write command
  uint16_t nvme_q = 1, pvm_q = 1;

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  void *data = (uint8_t *)alloc_host_mem(4*1024);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  read_cmd->prp.prp1 = (uint64_t) data;
  read_cmd->slba = 0x6;
  read_cmd->dw12.nlb = 0x1;

  // Send the NVME admin command and check on PVM side
  rc = send_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                      queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_hashing1() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_cmd, *pvm_cmd;
  // Consume NVME queue 2 entry to post write command
  // Consume PVM queue 0 (by hashing) to check write command
  uint16_t nvme_q = 2, pvm_q = 0;

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  void *data = (uint8_t *)alloc_host_mem(4*1024);
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd;
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  write_cmd->prp.prp1 = (uint64_t) data;
  write_cmd->slba = 0x7;
  write_cmd->dw12.nlb = 0x1;

  // Send the NVME admin command and check on PVM side
  rc = send_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                      queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_hashing2() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_cmd, *pvm_cmd;
  // Consume NVME queue 3 entry to post read command
  // Consume PVM queue 1 (by hashing) to check read command
  uint16_t nvme_q = 3, pvm_q = 1;

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  void *data = (uint8_t *)alloc_host_mem(4*1024);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  read_cmd->prp.prp1 = (uint64_t) data;
  read_cmd->slba = 0x8;
  read_cmd->dw12.nlb = 0x1;

  // Send the NVME admin command and check on PVM side
  rc = send_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
                      queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int pvm_status_trailer_update(struct PvmStatus *status, uint16_t lif, 
                              uint8_t qtype, uint32_t qid) {
  uint64_t dst_qaddr;

  qstate_if::write_bit_fields(status->pvm_trailer, 0, 11, lif);
  qstate_if::write_bit_fields(status->pvm_trailer, 11, 3, qtype);
  qstate_if::write_bit_fields(status->pvm_trailer, 14, 24, qid);
  if (hal_if::get_lif_qstate_addr(lif, qtype, qid, &dst_qaddr) < 0) {
    printf("Can't get the LIF->qstate addr resolved for PVM status\n");
    return -1;
  }
  qstate_if::write_bit_fields(status->pvm_trailer, 38, 34, dst_qaddr);
  printf("PVM status: base addr %lx \n", dst_qaddr);
  return 0;
}

int test_run_pvm_nvme_admin_status() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_status, *pvm_status;
  // Consume PVM queue 0 entry to post admin command status
  // Consume NVME queue 0 to check admin command status
  uint16_t nvme_q = 0, pvm_q = 0;

  if (consume_nvme_pvm_cq_entries(nvme_q, pvm_q, &nvme_status, &pvm_status, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct PvmStatus *admin_status = (struct PvmStatus *) pvm_status;
  admin_status->nvme_status.dw3.cid = 0xA0;
  admin_status->nvme_status.dw3.status = 0;
  if (pvm_status_trailer_update(admin_status, queues::get_nvme_lif(), 
                                CQ_TYPE, nvme_q) < 0) {
    return -1;
  }

  // Send the PVM admin command status and check on NVME side
  rc = send_and_check(pvm_status, nvme_status, sizeof(struct NvmeStatus), 
                      queues::get_pvm_lif(), CQ_TYPE, pvm_q, 0, pvm_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_pvm_nvme_read_status() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_status, *pvm_status;
  // Consume PVM queue 1 entry to post read command status
  // Consume NVME queue 1 to check read command status
  uint16_t nvme_q = 1, pvm_q = 1;

  if (consume_nvme_pvm_cq_entries(nvme_q, pvm_q, &nvme_status, &pvm_status, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct PvmStatus *read_status = (struct PvmStatus *) pvm_status;
  read_status->nvme_status.dw3.cid = 0x20;
  read_status->nvme_status.dw3.status = 0;
  if (pvm_status_trailer_update(read_status, queues::get_nvme_lif(), 
                                CQ_TYPE, nvme_q) < 0) {
    return -1;
  }

  // Send the PVM admin command status and check on NVME side
  rc = send_and_check(pvm_status, nvme_status, sizeof(struct NvmeStatus), 
                      queues::get_pvm_lif(), CQ_TYPE, pvm_q, 0, pvm_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_pvm_nvme_write_status() {
  int rc;
  uint16_t nvme_index, pvm_index;
  uint8_t *nvme_status, *pvm_status;
  // Consume PVM queue 0 entry to post write command status
  // Consume NVME queue 2 to check write command status
  uint16_t nvme_q = 2, pvm_q = 0;

  if (consume_nvme_pvm_cq_entries(nvme_q, pvm_q, &nvme_status, &pvm_status, 
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct PvmStatus *write_status = (struct PvmStatus *) pvm_status;
  write_status->nvme_status.dw3.cid = 0x30;
  write_status->nvme_status.dw3.status = 0;
  if (pvm_status_trailer_update(write_status, queues::get_nvme_lif(), 
                                CQ_TYPE, nvme_q) < 0) {
    return -1;
  }

  // Send the PVM admin command status and check on NVME side
  rc = send_and_check(pvm_status, nvme_status, sizeof(struct NvmeStatus), 
                      queues::get_pvm_lif(), CQ_TYPE, pvm_q, 0, pvm_index);


  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_read_cmd() {
  int rc;
  void *r2n_buf = r2n::r2n_buf_alloc();
  if (!r2n_buf) {
    printf("can't alloc r2n buf\n");
    return -1;
  }
  uint16_t r2n_index;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_q = 2;

  r2n_wqe_buf = (uint8_t *) queues::pvm_sq_consume_entry(r2n_q, &r2n_index);
  if (r2n_wqe_buf == nullptr) {
    printf("can't consume r2n wqe entry \n");
    return -1;
  }

  r2n::r2n_wqe_init(r2n_wqe_buf, r2n_buf);
  r2n::r2n_nvme_be_cmd_init(r2n_buf, r2n_q, 0, 0, 0, 1);

  uint16_t ssd_index;
  uint16_t ssd_q = 19;
  uint8_t *nvme_cmd = r2n::r2n_nvme_cmd_ptr(r2n_buf);
  uint8_t *ssd_cmd = (uint8_t *) queues::pvm_sq_consume_entry(ssd_q, &ssd_index);
  if (nvme_cmd == nullptr || ssd_cmd == nullptr) {
    printf("can't consume r2n wqe entry \n");
    return -1;
  }
  bzero(nvme_cmd, sizeof(struct NvmeCmd));
  bzero(ssd_cmd, sizeof(struct NvmeCmd)); 

  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  //read_cmd->prp.prp1 = (uint64_t) data;
  read_cmd->slba = 0x9;
  read_cmd->dw12.nlb = 0x1;

  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd), 
                      queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_write_cmd() {
  int rc;
  void *r2n_buf = r2n::r2n_buf_alloc();
  if (!r2n_buf) {
    printf("can't alloc r2n buf\n");
    return -1;
  }
  uint16_t r2n_index;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_q = 2;

  r2n_wqe_buf = (uint8_t *) queues::pvm_sq_consume_entry(r2n_q, &r2n_index);
  if (r2n_wqe_buf == nullptr) {
    printf("can't consume r2n wqe entry \n");
    return -1;
  }

  r2n::r2n_wqe_init(r2n_wqe_buf, r2n_buf);
  r2n::r2n_nvme_be_cmd_init(r2n_buf, r2n_q, 0, 0, 0, 1);

  uint16_t ssd_index;
  uint16_t ssd_q = 19;
  uint8_t *nvme_cmd = r2n::r2n_nvme_cmd_ptr(r2n_buf);
  uint8_t *ssd_cmd = (uint8_t *) queues::pvm_sq_consume_entry(ssd_q, &ssd_index);
  if (nvme_cmd == nullptr || ssd_cmd == nullptr) {
    printf("can't consume r2n wqe entry \n");
    return -1;
  }
  bzero(nvme_cmd, sizeof(struct NvmeCmd));
  bzero(ssd_cmd, sizeof(struct NvmeCmd)); 

  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd;
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  //write_cmd->prp.prp1 = (uint64_t) data;
  write_cmd->slba = 0x8;
  write_cmd->dw12.nlb = 0x1;

  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd), 
                      queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

}  // namespace tests
