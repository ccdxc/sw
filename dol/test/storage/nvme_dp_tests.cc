#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <byteswap.h>
#include <assert.h>
#include <functional>
#include <ctime>
#include <netinet/in.h>

#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/tests.hpp"
#include "nic/model_sim/include/lib_model_client.h"



namespace tests {


int consume_nvme_sq_cq_entries(uint16_t nvme_sq, uint16_t nvme_cq, 
                               dp_mem_t **nvme_cmd, dp_mem_t **nvme_status, 
                               uint16_t *cmd_index, uint16_t *status_index) {

  if (!nvme_cmd || !nvme_status || !cmd_index || !status_index) {
    return -1;
  }
  *nvme_cmd = queues::nvme_sq_consume_entry(nvme_sq, cmd_index);
  *nvme_status = queues::nvme_cq_consume_entry(nvme_cq, status_index);

  if (*nvme_cmd == nullptr || *nvme_status == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }

  (*nvme_cmd)->clear_thru();
  (*nvme_status)->clear_thru();
  return 0;
}

dp_mem_t* form_nvme_dp_read_cmd_with_buf(dp_mem_t *nvme_cmd, uint32_t size, uint16_t cid, 
                                         uint64_t slba, uint16_t nlb)
{
  dp_mem_t *read_buf;
  read_buf = new dp_mem_t(1, kDefaultBufSize, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  read_buf->clear_thru();
  nvme_cmd->clear();
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd->read();
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  // TODO: PRP list based on size
  read_cmd->prp.prp1 = read_buf->pa();
  printf("Read buffer PA %lx \n", read_buf->pa());
  read_cmd->dw0.cid = cid; 
  read_cmd->nsid = kDefaultNsid; 
  read_cmd->slba = slba;
  read_cmd->dw12.nlb = nlb;
  nvme_cmd->write_thru();

  return read_buf;
}

dp_mem_t* form_nvme_dp_write_cmd_with_buf(dp_mem_t *nvme_cmd, uint32_t size, uint16_t cid, 
                                          uint64_t slba, uint16_t nlb)
{
  uint8_t byte_val = get_next_byte();
  dp_mem_t *write_buf;
  write_buf = new dp_mem_t(1, kDefaultBufSize, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  memset(write_buf->read(), byte_val, kDefaultBufSize);
  write_buf->write_thru();
  nvme_cmd->clear();
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd->read();
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  // TODO: PRP list based on size
  write_cmd->prp.prp1 = write_buf->pa();
  printf("Write buffer PA %lx \n", write_buf->pa());
  write_cmd->dw0.cid = cid; 
  write_cmd->nsid = kDefaultNsid; 
  write_cmd->slba = slba;
  write_cmd->dw12.nlb = nlb;
  nvme_cmd->write_thru();

  return write_buf;
}


int check_nvme_dp_status(dp_mem_t *nvme_status, dp_mem_t *nvme_cmd) {
  // Process the status

  struct NvmeStatus *status = (struct NvmeStatus *)nvme_status->read_thru();
  struct NvmeCmd *cmd = (struct NvmeCmd *)nvme_cmd->read_thru();

  if (status->dw3.cid != cmd->dw0.cid ||
      NVME_STATUS_GET_STATUS(*status)) {
    /*printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n",
           status->dw3.cid, status->dw3.status_phase,
           cmd->dw0.cid);*/
    return -1;
  }
  printf("nvme status: cid %x, sq head %x, status_phase %x \n",
         status->dw3.cid, status->dw2.sq_head,
         status->dw3.status_phase);
  return 0;
}

int test_run_nvme_dp_write_cmd() {
  int rc;
  uint16_t cmd_index, status_index;
  dp_mem_t *nvme_cmd, *nvme_status, *write_buf;

  // Use non-zero queue (0 is Admin Q)
  uint16_t nvme_sq = queues::get_host_nvme_sq(1);
  uint16_t nvme_cq = queues::get_host_nvme_cq(1);

  if (consume_nvme_sq_cq_entries(nvme_sq, nvme_cq, &nvme_cmd, &nvme_status, 
                                 &cmd_index, &status_index) < 0) {
    return -1;
  }

  // Form the write command
  if ((write_buf = form_nvme_dp_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(), 
                                                   get_next_slba(), kDefaultNlb)) == NULL) {
    return -1;
  }


  // Send the NVME admin command
  test_ring_nvme_doorbell(queues::get_nvme_lif(), SQ_TYPE, nvme_sq, 0, cmd_index);

  // Poll for status
  auto func1 = [nvme_status, nvme_cmd] () {
    return check_nvme_dp_status(nvme_status, nvme_cmd);
  };
  Poller poll;
  rc = poll(func1);

  return rc;
}

}  // namespace tests
