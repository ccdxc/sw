
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
#include "dol/test/storage/r2n.hpp"
#include "dol/test/storage/rdma.hpp"
#include "dol/test/storage/xts.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"
#include "dol/test/storage/tests.hpp"

extern dp_mem_t *initiator_rcv_buf_va;
extern bool rdma_ring_db;
extern size_t tcid;

namespace tests {

// Keep a rolling write data buffer and slba to pair up with read case for comparison
extern dp_mem_t *rolling_write_data_buf;
extern uint64_t rolling_write_slba;
static uint32_t num_iter = 8;
static uint64_t iters_comp = 0;
static bool first_run = true;

extern int check_nvme_status(dp_mem_t *nvme_status, dp_mem_t *nvme_cmd);
extern uint64_t get_next_slba();
extern void reset_slba();
extern uint8_t get_next_byte();
const static uint32_t  kR2nStatusNvmeOffset  = 16;

int test_run_perf_rdma_e2e_write() {
  uint16_t ssd_handle = 2; // the SSD handle
  dp_mem_t *cmd_buf[32] = { };
  int rc;

  if(first_run) {
    first_run = false;
  } else {
    sleep(600); //Wait for acks to drain
    num_iter = 32;
  }
  
  int iter = 1;
  testcase_begin(tcid, iter);
  reset_slba();
  for(uint32_t i = 0; i < num_iter; i++) {
    // Get the SLBA to write to and read from
    rolling_write_slba = get_next_slba();

    if(i == num_iter - 1) { 
      rdma_ring_db = true;
      testcase_end(tcid, iter++);
    } else {
      rdma_ring_db = false;
    }
    StartRoceWriteSeq(ssd_handle, get_next_byte(), &cmd_buf[i], rolling_write_slba, &rolling_write_data_buf);
    printf("Started sequencer to PDMA + write command send over ROCE \n");
  }

  initiator_rcv_buf_va->line_set(iters_comp);
  dp_mem_t *rcv_buf = rdma_get_initiator_rcv_buf();
  dp_mem_t *nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));
  // Poll for status
  dp_mem_t* nvme_cmd = cmd_buf[0];
  auto func1 = [nvme_status, nvme_cmd] () {
    return check_nvme_status(nvme_status, nvme_cmd);
  };
  Poller poll;
  rc = poll(func1);
  testcase_begin(tcid, iter);

  if (rc < 0)
    printf("Failure in retriving status \n");
  else
    printf("Successfully retrived status \n");

  fflush(stdout);
  if(0 == rc) {
  nvme_cmd = cmd_buf[num_iter-1];
  initiator_rcv_buf_va->line_set(iters_comp + num_iter - 1);
  rcv_buf = rdma_get_initiator_rcv_buf();
  nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));
  auto func2 = [nvme_status, nvme_cmd] () {
    return check_nvme_status(nvme_status, nvme_cmd);
  };
  Poller poll;
  rc = poll(func2);
  testcase_end(tcid, iter);

  if (rc < 0)
    printf("Failure in retriving status \n");
  else
    printf("Successfully retrived status \n");
  }
  fflush(stdout);

  for(uint32_t i = 0; i < num_iter; i++) {
    nvme_cmd = cmd_buf[i];
    initiator_rcv_buf_va->line_set(iters_comp + i);
    rcv_buf = rdma_get_initiator_rcv_buf();
    nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                   sizeof(struct NvmeStatus));
    //struct NvmeStatus *status = (struct NvmeStatus *)nvme_status->read_thru();
    //struct NvmeCmd *cmd = (struct NvmeCmd *)nvme_cmd->read_thru();
    //printf("exp: %x rcvd: %x\n", status->dw3.cid, cmd->dw0.cid);

  }
  fflush(stdout);
  // Save the rolling write data buffer
  rolling_write_data_buf = rdma_get_target_write_data_buf();

  // Post the Initiator buffers back so that RDMA can reuse them. 
  initiator_rcv_buf_va->line_set(iters_comp);
  for(uint32_t i = 0; i < num_iter; i++) {
    PostInitiatorRcvBuf1();
    IncrInitiatorRcvBufPtr();
  }
  iters_comp += num_iter;

  return rc;
}

}
