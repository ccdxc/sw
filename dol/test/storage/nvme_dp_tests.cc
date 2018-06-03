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
#include "dol/test/storage/rdma.hpp"
#include "nic/model_sim/include/lib_model_client.h"


DECLARE_uint64(nvme_scale_iters);
DECLARE_uint64(long_poll_interval);


extern size_t tcid;

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
  read_buf = new dp_mem_t(1, size, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
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
  write_buf = new dp_mem_t(1, size, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  memset(write_buf->read(), byte_val, size);
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

int check_nvme_dp_va_status(uint64_t nvme_status, uint64_t nvme_cmd) {
  // Process the status

  struct NvmeStatus *status = (struct NvmeStatus *)nvme_status;
  struct NvmeCmd *cmd = (struct NvmeCmd *)nvme_cmd;

  if (status->dw3.cid != cmd->dw0.cid ||
      NVME_STATUS_GET_STATUS(*status)) {
    /*printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n",
           status->dw3.cid, status->dw3.status_phase,
           cmd->dw0.cid);*/
    return -1;
  }
  /*
  printf("nvme status: cid %x, sq head %x, status_phase %x \n",
         status->dw3.cid, status->dw2.sq_head,
         status->dw3.status_phase);
  */
  return 0;
}
int test_run_nvme_dp_write_cmd() {
  int rc;
  uint16_t cmd_index, status_index;
  dp_mem_t *nvme_cmd, *nvme_status, *write_buf;

  // Use non-zero queue (0 is Admin Q)
  uint16_t nvme_sq = queues::get_host_nvme_sq(3);
  uint16_t nvme_cq = queues::get_host_nvme_cq(3);

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

int test_run_nvme_dp_read_cmd() {
  int rc;
  uint16_t cmd_index, status_index;
  dp_mem_t *nvme_cmd, *nvme_status, *read_buf;

  // Use non-zero queue (0 is Admin Q)
  uint16_t nvme_sq = queues::get_host_nvme_sq(1);
  uint16_t nvme_cq = queues::get_host_nvme_cq(1);

  if (consume_nvme_sq_cq_entries(nvme_sq, nvme_cq, &nvme_cmd, &nvme_status, 
                                 &cmd_index, &status_index) < 0) {
    return -1;
  }

  // Form the read command
  if ((read_buf = form_nvme_dp_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(), 
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

int test_run_nvme_dp_e2e_test() {
  int rc;
  uint16_t cmd_index, status_index;
  dp_mem_t *nvme_cmd, *nvme_status, *write_buf, *read_buf;
  uint64_t slba;

  // Use non-zero queue (0 is Admin Q)
  uint16_t nvme_sq = queues::get_host_nvme_sq(1);
  uint16_t nvme_cq = queues::get_host_nvme_cq(1);

  // Init the SLBA once and use it for Read & Write commands
  slba = get_next_slba();

  // Consume for write
  if (consume_nvme_sq_cq_entries(nvme_sq, nvme_cq, &nvme_cmd, &nvme_status, 
                                 &cmd_index, &status_index) < 0) {
    return -1;
  }

  // Form the write command
  if ((write_buf = form_nvme_dp_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(), 
                                                   slba, kDefaultNlb)) == NULL) {
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

  // Write status successful ?
  if (rc < 0) {
    printf("Write status failed \n");
    return rc;
  }

  // Consume again for read
  if (consume_nvme_sq_cq_entries(nvme_sq, nvme_cq, &nvme_cmd, &nvme_status, 
                                 &cmd_index, &status_index) < 0) {
    return -1;
  }

  // Form the read command
  if ((read_buf = form_nvme_dp_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(), 
                                                 slba, kDefaultNlb)) == NULL) {
    return -1;
  }


  // Send the NVME admin command
  test_ring_nvme_doorbell(queues::get_nvme_lif(), SQ_TYPE, nvme_sq, 0, cmd_index);

  // Poll for status
  auto func2 = [nvme_status, nvme_cmd] () {
    return check_nvme_dp_status(nvme_status, nvme_cmd);
  };
  rc = poll(func2);

  // Read status successful ?
  if (rc < 0) {
    printf("Read status failed \n");
    return rc;
  }

  // Compare the read and write buffers
  if (memcmp(read_buf->read(), write_buf->read(), kDefaultBufSize) != 0) {
    utils::dump(read_buf->read(), 16);
    utils::dump(write_buf->read(), 16);
    printf("Buffer comparison failed \n");
    rc = -1;
  } else {
    printf("Buffer comparison succeded \n");
    rc = 0;
  }

  return rc;
}

static int perf_marker = 1;
int nvme_dp_write_perf(int num_iter) {
  int i, rc;
  uint16_t cmd_index[num_iter], status_index[num_iter];
  dp_mem_t *nvme_cmd, *nvme_status, *write_buf;
  uint64_t nvme_cmd_va[num_iter], nvme_status_va[num_iter];

  // Use non-zero queue (0 is Admin Q)
  uint16_t nvme_sq = queues::get_host_nvme_sq(3);
  uint16_t nvme_cq = queues::get_host_nvme_cq(3);

  // Marker for requests - start
  testcase_begin(tcid, perf_marker);

  // Iterate
  for (i = 0; i < num_iter; i++) {
    // Consume the entries
    if (consume_nvme_sq_cq_entries(nvme_sq, nvme_cq, &nvme_cmd, &nvme_status, 
                                   &cmd_index[i], &status_index[i]) < 0) {
      return -1;
    }

    // Save the PAs
    nvme_cmd_va[i] = nvme_cmd->va();
    nvme_status_va[i] = nvme_status->va();

    // Form the write command
    if ((write_buf = form_nvme_dp_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(), 
                                                     get_next_slba(), kDefaultNlb)) == NULL) {
      return -1;
    }
  }

  // Marker for requests - end
  testcase_end(tcid, perf_marker++);

  // Send the NVME admin command
  test_ring_nvme_doorbell(queues::get_nvme_lif(), SQ_TYPE, nvme_sq, 0, cmd_index[i-1]);

  // Marker for responses - start
  testcase_begin(tcid, perf_marker);

  // Poll and check all previous entries status. Match against all other entries as
  // there is no guarantee of ordering.
  auto func1 = [&nvme_status_va, &nvme_cmd_va, i] () {
    int j, k;
    for (j = 0; j < i; j++) {
      for (k = 0; k < i; k++) {
        if (check_nvme_dp_va_status(nvme_status_va[j], nvme_cmd_va[k]) >= 0) break;
      }
      // Any status check failed => return failure
      if (k == i) return -1;
    }
    return 0;
  };
  // Run for 6 hours as this can take a long time
  Poller poll((6 * 3600), false);
  rc = poll(func1);
  if (rc < 0) {
    printf("Failed all entries status check \n");
  } else {
    printf("Passed all entries status check \n");
  }

  // Marker for responses - end
  testcase_end(tcid, perf_marker++);

  return rc;
}

int test_run_nvme_dp_write_perf() {
  if (nvme_dp_write_perf(4) < 0) {
    printf("Failed to prime the perf test \n");
    return -1;
  }
  printf("Successfully primed the perf test \n");

  sleep(600); //Wait for acks to drain

  printf("Starting scaling of perf test (after draining of acks) \n");
  if (nvme_dp_write_perf(27) < 0) {
    printf("Failed to scale the perf test \n");
    return -1;
  }
  printf("Successfully scaled the perf test \n");
  return 0;
}

int nvme_dp_read_perf(int num_iter) {
  int i, rc;
  uint16_t cmd_index[num_iter], status_index[num_iter];
  dp_mem_t *nvme_cmd, *nvme_status, *read_buf;
  uint64_t nvme_cmd_va[num_iter], nvme_status_va[num_iter];

  // Use non-zero queue (0 is Admin Q)
  uint16_t nvme_sq = queues::get_host_nvme_sq(3);
  uint16_t nvme_cq = queues::get_host_nvme_cq(3);

  // Marker for requests - start
  testcase_begin(tcid, perf_marker);

  // Iterate
  for (i = 0; i < num_iter; i++) {
    // Consume the entries
    if (consume_nvme_sq_cq_entries(nvme_sq, nvme_cq, &nvme_cmd, &nvme_status, 
                                   &cmd_index[i], &status_index[i]) < 0) {
      return -1;
    }

    // Save the PAs
    nvme_cmd_va[i] = nvme_cmd->va();
    nvme_status_va[i] = nvme_status->va();

    // Form the read command
    if ((read_buf = form_nvme_dp_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(), 
                                                     get_next_slba(), kDefaultNlb)) == NULL) {
      return -1;
    }
  }

  // Marker for requests - end
  testcase_end(tcid, perf_marker++);

  // Send the NVME admin command
  test_ring_nvme_doorbell(queues::get_nvme_lif(), SQ_TYPE, nvme_sq, 0, cmd_index[i-1]);

  // Marker for responses - start
  testcase_begin(tcid, perf_marker);

  // Poll and check all previous entries status. Match against all other entries as
  // there is no guarantee of ordering.
  auto func1 = [&nvme_status_va, &nvme_cmd_va, i] () {
    int j, k;
    for (j = 0; j < i; j++) {
      for (k = 0; k < i; k++) {
        if (check_nvme_dp_va_status(nvme_status_va[j], nvme_cmd_va[k]) >= 0) break;
      }
      // Any status check failed => return failure
      if (k == i) return -1;
    }
    return 0;
  };
  // Run for 6 hours as this can take a long time
  Poller poll((6 * 3600), false);
  rc = poll(func1);
  if (rc < 0) {
    printf("Failed all entries status check \n");
  } else {
    printf("Passed all entries status check \n");
  }

  // Marker for responses - end
  testcase_end(tcid, perf_marker++);

  return rc;
}

int test_run_nvme_dp_read_perf() {
  if (nvme_dp_read_perf(4) < 0) {
    printf("Failed to prime the perf test \n");
    return -1;
  }
  printf("Successfully primed the perf test \n");

  sleep(600); //Wait for acks to drain

  printf("Starting scaling of perf test (after draining of acks) \n");
  if (nvme_dp_read_perf(27) < 0) {
    printf("Failed to scale the perf test \n");
    return -1;
  }
  printf("Successfully scaled the perf test \n");
  return 0;
}
int test_run_nvme_dp_write_scale() {
  printf("Starting NVME Datapath WRITE scale test \n");
  for (int i = 0; i < (int) FLAGS_nvme_scale_iters; i++) {
    if (test_run_nvme_dp_write_cmd() < 0) {
      printf("Iteration %d FAILED \n", i);
      return -1;
    } else {
      printf("Iteration %d SUCCEEDED \n", i);
    }
  }
  return 0;
}

int test_run_nvme_dp_read_scale() {
  printf("Starting NVME Datapath READ scale test \n");
  for (int i = 0; i < (int) FLAGS_nvme_scale_iters; i++) {
    if (test_run_nvme_dp_read_cmd() < 0) {
      printf("Iteration %d FAILED \n", i);
      return -1;
    } else {
      printf("Iteration %d SUCCEEDED \n", i);
    }
  }
  return 0;
}

int test_run_nvme_dp_e2e_scale() {
  printf("Starting NVME Datapath E2E scale test \n");
  for (int i = 0; i < (int) FLAGS_nvme_scale_iters; i++) {
    if (test_run_nvme_dp_e2e_test() < 0) {
      printf("Iteration %d FAILED \n", i);
      return -1;
    } else {
      printf("Iteration %d SUCCEEDED \n", i);
    }
  }
  return 0;
}

}  // namespace tests
