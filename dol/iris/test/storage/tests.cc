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

#include "dol/iris/test/storage/hal_if.hpp"
#include "dol/iris/test/storage/utils.hpp"
#include "dol/iris/test/storage/qstate_if.hpp"
#include "dol/iris/test/storage/nvme.hpp"
#include "dol/iris/test/storage/r2n.hpp"
#ifdef __x86_64__
#include "dol/iris/test/storage/rdma.hpp"
#include "dol/iris/test/storage/queues.hpp"
#endif
#include "dol/iris/test/storage/xts.hpp"
#include "dol/iris/test/storage/tests.hpp"
#include "pal_compat.hpp"


const static uint32_t  kR2nWqeSize           = 64;
const static uint32_t  kR2nStatusSize        = 64;
const static uint32_t  kR2nStatusNvmeOffset  = 16;

const static uint32_t  kHbmSsdBitmapSize     = (16 * 4096);
const static uint32_t  kHbmRWBufSize         = 4096;

const static uint32_t  kSeqDbDataSize        = 4;
const static uint32_t  kSeqDbDataMagic       = 0xAAAAAAAA;

bool rdma_ring_db = true;

namespace tests {

dp_mem_t *read_buf;
dp_mem_t *write_buf;
dp_mem_t *read_buf2;

dp_mem_t *read_hbm_buf;
dp_mem_t *write_hbm_buf;
dp_mem_t *read_hbm_buf2;
dp_mem_t *write_hbm_buf2;
dp_mem_t *time_adv_buf;

dp_mem_t *seq_db_data;

static uint16_t global_cid = 0x1;
static uint64_t global_slba = 0x0000;
static uint64_t global_byte = 0xA0;

/*
 * This function provides non-blocking poll on an HBM location in use by the test to
 * ensure that simulation time advances.
 */
static void
verification_time_advance(void)
{
#ifdef __x86_64__
    if (time_adv_buf) {
        assert(time_adv_buf->is_mem_type_hbm());
        time_adv_buf->read_thru();
    }
#endif
}

int Poller::operator()(std::function<int(void)> poll_func) {
  std::time_t start = std::time(nullptr);
  std::time_t end;
  int rv;
  do {
    rv = poll_func();
    if(0 == rv)
      return rv;
    verification_time_advance();
    if(fast_poll) {
      usleep(10000); //Sleep 10msec
    } else {
      // For performance mode
      sleep(30);
    }
    end = std::time(nullptr);
  } while(end - start < timeout);
  printf("Polling timeout %d exceeded - Giving up! \n", timeout);
  return -1;
}

int test_setup() {
  // Allocate a scratch buffer for use by verification_time_advance().
  // This must be done early for poller operations to work.
  time_adv_buf = new dp_mem_t(1, sizeof(uint32_t));

  if (run_pdma_tests) {
      queues::seq_queue_pdma_num_set(FLAGS_num_pdma_queues);
  }

  if (run_acc_scale_tests_map) {
      queues::seq_queue_acc_sub_num_set(FLAGS_acc_scale_submissions,
                                        FLAGS_acc_scale_chain_replica,
                                        ACC_SCALE_TEST_MAX +
                                        ACC_SCALE_TEST_NUM_DUAL_SQ_TESTS,
                                        ACC_SCALE_TEST_NUM_TRUE_CHAINING_TESTS);
  }

  // Initialize resources
  if (queues::resources_init() < 0) {
    printf("Failed to initialize resources \n");
    return -1;
  }
  printf("Initialized resources \n");

  // Setup LIFs
  if (queues::lifs_setup() < 0) {
    printf("Failed to setup LIFs\n");
    return -1;
  }
  printf("Setup LIFs \n");

  // Setup NVME PVM U-turn queues
  if (queues::nvme_pvm_queues_setup() < 0) {
    printf("Failed to setup NVME PVM U-turn queues \n");
    return -1;
  }
  printf("Setup NVME PVM U-turn queues  \n");

  // Setup PVM queues
  if (queues::pvm_queues_setup() < 0) {
    printf("Failed to setup PVM queues \n");
    return -1;
  }
  printf("Setup PVM queues  \n");

  // Setup Sequencer queues
  if (queues::seq_queues_setup() < 0) {
    printf("Failed to setup Sequencer queues \n");
    return -1;
  }
  printf("Setup Sequeuncer queues \n");
  return 0;
}

int alloc_buffers() {

  // Allocate the read and write buffer
  // TODO: Have a fancy allocator with various pages
  read_buf = new dp_mem_t(1, kDefaultBufSize, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  read_buf2 = new dp_mem_t(1, kDefaultBufSize, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  write_buf = new dp_mem_t(1, kDefaultBufSize, DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
  printf("read_buf address %lx write_buf address %lx\n", read_buf->va(), write_buf->va());

  // Note: Read/Write buffer allocations have to be page aligned for use in PRP
  // Allocate the read buffer in HBM for the sequencer
  read_hbm_buf = new dp_mem_t(1, kHbmRWBufSize, DP_MEM_ALIGN_PAGE);

  // Allocate the write buffer in HBM for the sequencer
  write_hbm_buf = new dp_mem_t(1, kHbmRWBufSize, DP_MEM_ALIGN_PAGE);

  printf("HBM read_buf address %lx write_buf address %lx\n",
         read_hbm_buf->pa(), write_hbm_buf->pa());

  // Allocate the read buffer2 in HBM for the sequencer
  read_hbm_buf2 = new dp_mem_t(1, kHbmRWBufSize, DP_MEM_ALIGN_PAGE);

  // Allocate the write buffer2 in HBM for the sequencer
  write_hbm_buf2 = new dp_mem_t(1, kHbmRWBufSize, DP_MEM_ALIGN_PAGE);

  printf("HBM read_buf2 address %lx write_buf2 address %lx\n",
         read_hbm_buf2->pa(), write_hbm_buf2->pa());

  // Allocate sequencer doorbell data that will be updated by sequencer and read by PVM
  if ((seq_db_data = new dp_mem_t(1, kSeqDbDataSize, DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM)) == nullptr) return -1;

  return 0;

}

/*
 * For certain types of resource (such as AOLs, SGLs, etc.), the default
 * placement is host memory, similar to what Driver would do. However,
 * when model is run together with RTL, the model would exhibit PCIe coherency
 * issues. What is known so far is:
 *
 * 1) When model runs by itself, there are no coherency issues
 * 2) When model runs with RTL, RTL works (i.e., RTL sees and acts on correct data)
 *    but model doesn't.
 *
 * The workaround is to place the affected resources in HBM.
 */
dp_mem_type_t
test_mem_type_workaround(dp_mem_type_t preference)
{
    /*
     * FLAGS_rtl means model runs with RTL.
     */
    if (preference == DP_MEM_TYPE_HOST_MEM) {
        if (FLAGS_rtl) {
            return DP_MEM_TYPE_HBM;
        }
    }

    return preference;
}

/*
 * Collection point for doing eos_ignore_addr() on generic system resources
 * over which we have no control. For example, the P4 flow_stats area.
 */
void
test_generic_eos_ignore(void)
{
    uint64_t    hbm_addr;
    uint32_t    hbm_size;

    /*
     * Starting around the end of July, 2018, RTL suddenly complained about
     * an EOS mismatch in the P4 flow_stats area.
     */
    if (hal_if::alloc_hbm_address("flow_stats", &hbm_addr, &hbm_size) == 0) {
        hbm_size *= 1024;
        printf("eos_ignore_addr flow_stats region 0x%lx len %u\n", hbm_addr, hbm_size);
        EOS_IGNORE_ADDR(hbm_addr, hbm_size);
    }
}

void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                        uint8_t ring, uint16_t index,
                        bool suppress_log) {
  uint64_t db_data;
  uint64_t db_addr;

  queues::get_host_doorbell(lif, qtype, qid, ring, index, &db_addr, &db_data);

  if (!suppress_log) {
      printf("Ring Doorbell: Addr %lx Data %lx \n", db_addr, db_data);
  }
  WRITE_DB64(db_addr, db_data);
}

void test_ring_nvme_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                             uint8_t ring, uint16_t index) {
  uint64_t db_data;
  uint64_t db_addr;

  queues::get_nvme_doorbell(lif, qtype, qid, ring, index, &db_addr, &db_data);

  printf("Ring Doorbell: Addr %lx Data %lx \n", db_addr, db_data);
  WRITE_DB64(db_addr, db_data);
}

int send_and_check(dp_mem_t *send_cmd, dp_mem_t *recv_cmd, uint32_t size,
                   uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring,
                   uint16_t index) {
  int rc;

  printf("Sending data size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd->read(), recv_cmd->read(), size);
  printf("PRE doorbell cmd comparison %d \n", rc);

  test_ring_doorbell(lif, qtype, qid, ring, index);

  // Poll for status
  auto func1 = [send_cmd, recv_cmd, size] () {
    return memcmp(send_cmd->read_thru(), recv_cmd->read_thru(), size);
  };
  Poller poll;
  rc = poll(func1);
  printf("POST doorbell cmd comparison %d \n", rc);

  return rc;
}

int send_nvme_and_check(dp_mem_t *send_cmd, dp_mem_t *recv_cmd, uint32_t size,
                        uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring,
                        uint16_t index) {
  int rc;

  printf("Sending data size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd->read(), recv_cmd->read(), size);
  printf("PRE NVME doorbell cmd comparison %d \n", rc);

  test_ring_nvme_doorbell(lif, qtype, qid, ring, index);

  // Poll for status
  auto func1 = [send_cmd, recv_cmd, size] () {
    return memcmp(send_cmd->read_thru(), recv_cmd->read_thru(), size);
  };
  Poller poll;
  rc = poll(func1);
  printf("POST NVME doorbell cmd comparison %d \n", rc);

  return rc;
}

int send_and_check_ignore_cid(dp_mem_t *send_cmd, dp_mem_t *recv_cmd,
                              uint32_t size, uint16_t lif, uint8_t qtype,
                              uint32_t qid, uint8_t ring, uint16_t index) {
  int rc;

  printf("Sending data size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd->read(), recv_cmd->read(), size);
  printf("PRE doorbell cmd comparison %d \n", rc);

  test_ring_doorbell(lif, qtype, qid, ring, index);

  // Poll for status
  auto func1 = [send_cmd, recv_cmd, size] () {
    struct NvmeCmd *send_nvme_cmd = (struct NvmeCmd *) send_cmd->read_thru();
    struct NvmeCmd *recv_nvme_cmd = (struct NvmeCmd *) recv_cmd->read_thru();
    recv_nvme_cmd->dw0.cid = send_nvme_cmd->dw0.cid;
    return memcmp(send_nvme_cmd, recv_nvme_cmd, size);
  };
  Poller poll;
  rc = poll(func1);
  printf("POST doorbell cmd comparison (ignoring cid) %d \n", rc);

  return rc;
}

int check_ignore_cid(dp_mem_t *send_cmd, dp_mem_t *recv_cmd, uint32_t size) {
  int rc;

  // Poll for status
  auto func1 = [send_cmd, recv_cmd, size] () {
    struct NvmeCmd *send_nvme_cmd = (struct NvmeCmd *) send_cmd->read_thru();
    struct NvmeCmd *recv_nvme_cmd = (struct NvmeCmd *) recv_cmd->read_thru();
    recv_nvme_cmd->dw0.cid = send_nvme_cmd->dw0.cid;
    return memcmp(send_nvme_cmd, recv_nvme_cmd, size);
  };
  Poller poll;
  rc = poll(func1);
  printf("Colletive cmd comparison (ignoring cid) %d \n", rc);

  return rc;
}

int consume_nvme_pvm_sq_entries(uint16_t nvme_q, uint16_t pvm_q,
                                dp_mem_t **nvme_cmd, dp_mem_t **pvm_cmd,
                                uint16_t *nvme_index, uint16_t *pvm_index) {

  if (!nvme_cmd || !pvm_cmd || !nvme_index || !pvm_index) {
    return -1;
  }
  *nvme_cmd = queues::nvme_sq_consume_entry(nvme_q, nvme_index);
  *pvm_cmd = queues::pvm_sq_consume_entry(pvm_q, pvm_index);

  if (*nvme_cmd == nullptr || *pvm_cmd == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }

  (*nvme_cmd)->clear_thru();
  (*pvm_cmd)->clear_thru();
  return 0;
}

int consume_nvme_pvm_cq_entries(uint16_t nvme_q, uint16_t pvm_q,
                                dp_mem_t **nvme_status, dp_mem_t **pvm_status,
                                uint16_t *nvme_index, uint16_t *pvm_index) {

  if (!nvme_status || !pvm_status || !nvme_index || !pvm_index) {
    return -1;
  }
  *nvme_status = queues::nvme_cq_consume_entry(nvme_q, nvme_index);
  *pvm_status = queues::pvm_cq_consume_entry(pvm_q, pvm_index);

  if (*nvme_status == nullptr || *pvm_status == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }

  (*nvme_status)->clear_thru();
  (*pvm_status)->clear_thru();
  return 0;
}

int consume_r2n_entry(uint16_t r2n_q, uint16_t ssd_handle, uint8_t io_priority,
                      uint8_t is_read, dp_mem_t **r2n_buf,  dp_mem_t **r2n_wqe_buf,
                      dp_mem_t **nvme_cmd, uint16_t *r2n_index) {

  if (!r2n_buf || !r2n_wqe_buf || !nvme_cmd || !r2n_index) {
    return -1;
  }
  *r2n_buf = r2n::r2n_buf_alloc();
  if (*r2n_buf == nullptr) {
    printf("can't alloc r2n buf\n");
    return -1;
  }

  *r2n_wqe_buf = queues::pvm_sq_consume_entry(r2n_q, r2n_index);
  if (*r2n_wqe_buf == nullptr) {
    printf("can't consume r2n wqe entry \n");
    return -1;
  }

  r2n::r2n_wqe_init(*r2n_wqe_buf, *r2n_buf, R2N_OPCODE_PROCESS_WQE);
  r2n::r2n_nvme_be_cmd_init(*r2n_buf, r2n_q, ssd_handle, io_priority, is_read, 1);

  *nvme_cmd = r2n::r2n_nvme_cmd_ptr(*r2n_buf);
  if (*nvme_cmd == nullptr) {
    printf("can't consume nvme command \n");
    return -1;
  }
  (*nvme_cmd)->clear_thru();
  return 0;
}

int form_r2n_seq_wqe(uint16_t ssd_handle, uint8_t io_priority, uint8_t is_read,
                     uint16_t r2n_q, dp_mem_t **r2n_buf,  dp_mem_t **r2n_wqe_buf,
                     dp_mem_t **nvme_cmd) {

  if (!r2n_wqe_buf || !nvme_cmd) {
    return -1;
  }
  *r2n_buf = r2n::r2n_buf_alloc();
  if (*r2n_buf == nullptr) {
    printf("can't alloc r2n buf\n");
    return -1;
  }

  *r2n_wqe_buf = new dp_mem_t(1, kR2nWqeSize);
  if (*r2n_wqe_buf == nullptr) {
    printf("can't alloc r2n wqe entry \n");
    return -1;
  }

  r2n::r2n_wqe_init(*r2n_wqe_buf, *r2n_buf, R2N_OPCODE_PROCESS_WQE);
  r2n::r2n_nvme_be_cmd_init(*r2n_buf, r2n_q, ssd_handle, io_priority, is_read, 1);

  *nvme_cmd = r2n::r2n_nvme_cmd_ptr(*r2n_buf);
  if (*nvme_cmd == nullptr) {
    printf("can't alloc nvme command \n");
    return -1;
  }
  (*nvme_cmd)->clear_thru();
  return 0;
}

int consume_ssd_entry(uint16_t ssd_q, dp_mem_t **ssd_cmd, uint16_t *ssd_index) {

  if (!ssd_cmd || !ssd_index) {
    return -1;
  }
  *ssd_cmd = queues::pvm_sq_consume_entry(ssd_q, ssd_index);
  if (*ssd_cmd == nullptr) {
    printf("can't consume ssd command \n");
    return -1;
  }
  (*ssd_cmd)->clear_thru();
  return 0;
}

uint8_t get_next_byte() {
  global_byte++;
  return global_byte;
}

uint16_t get_next_cid() {
  global_cid++;
  return global_cid;
}

void reset_slba() {
  global_slba = 0;
}

uint64_t get_next_slba() {
  global_slba += 1;
  return global_slba;
}

void reset_seq_db_data() {
  seq_db_data->clear_thru();
}

int form_read_cmd_with_buf(dp_mem_t *nvme_cmd, uint32_t size, uint16_t cid,
                           uint64_t slba, uint16_t nlb)
{
  read_buf->clear_thru();
  nvme_cmd->clear();
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd->read();
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  // TODO: PRP list based on size
  read_cmd->prp.prp1 = read_buf->pa();
  read_cmd->dw0.cid = cid;
  read_cmd->nsid = kDefaultNsid;
  read_cmd->slba = slba;
  read_cmd->dw12.nlb = nlb;
  nvme_cmd->write_thru();

  return 0;
}

int form_write_cmd_with_buf(dp_mem_t *nvme_cmd, uint32_t size, uint16_t cid,
                            uint64_t slba, uint16_t nlb)
{
  uint8_t byte_val = get_next_byte();
  write_buf->fill_thru(byte_val);
  nvme_cmd->clear();
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd->read();
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  // TODO: PRP list based on size
  write_cmd->prp.prp1 = write_buf->pa();
  write_cmd->dw0.cid = cid;
  write_cmd->nsid = kDefaultNsid;
  write_cmd->slba = slba;
  write_cmd->dw12.nlb = nlb;
  nvme_cmd->write_thru();

  return 0;
}

int form_read_cmd_with_hbm_buf(dp_mem_t *nvme_cmd, uint32_t size, uint16_t cid,
                               uint64_t slba, uint16_t nlb)
{
  // Init the read buf as this will be the final destination despite staging
  // in HBM
  read_buf->clear_thru();
  nvme_cmd->clear();
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd->read();
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  // Use the HBM buffer setup by the sequencer
  read_cmd->prp.prp1 = read_hbm_buf->pa();
  read_cmd->dw0.cid = cid;
  read_cmd->nsid = kDefaultNsid;
  read_cmd->slba = slba;
  read_cmd->dw12.nlb = nlb;
  nvme_cmd->write_thru();

  return 0;
}

int form_write_cmd_with_hbm_buf(dp_mem_t *nvme_cmd, uint32_t size, uint16_t cid,
                                uint64_t slba, uint16_t nlb)
{
  // Init the write buf as this will be the source despite staging
  // in HBM
  uint8_t byte_val = get_next_byte();
  write_buf->fill_thru(byte_val);
  nvme_cmd->clear();
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd->read();
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  // Use the HBM buffer setup by the sequencer
  write_cmd->prp.prp1 = write_hbm_buf->pa();
  write_cmd->dw0.cid = cid;
  write_cmd->nsid = kDefaultNsid;
  write_cmd->slba = slba;
  write_cmd->dw12.nlb = nlb;
  nvme_cmd->write_thru();

  return 0;
}

int form_read_cmd_no_buf(dp_mem_t *nvme_cmd, uint64_t slba)
{
  nvme_cmd->clear();
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd->read();
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  read_cmd->dw0.cid = get_next_cid();
  read_cmd->nsid = kDefaultNsid;
  read_cmd->slba = slba;
  read_cmd->dw12.nlb = kDefaultNlb;
  nvme_cmd->write_thru();

  return 0;
}

int form_write_cmd_no_buf(dp_mem_t *nvme_cmd, uint64_t slba)
{
  nvme_cmd->clear();
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd->read();
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  write_cmd->dw0.cid = get_next_cid();
  write_cmd->nsid = kDefaultNsid;
  write_cmd->slba = slba;
  write_cmd->dw12.nlb = kDefaultNlb;
  nvme_cmd->write_thru();

  return 0;
}

int check_nvme_status(dp_mem_t *nvme_status, dp_mem_t *nvme_cmd) {
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

int test_run_nvme_pvm_admin_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;
  dp_mem_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 0 entry to post admin command
  // Consume PVM queue 0 to check admin command
  uint16_t nvme_q = queues::get_host_nvme_sq(0);
  uint16_t pvm_q = queues::get_pvm_nvme_sq(0);

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct NvmeCmd *admin_cmd = (struct NvmeCmd *) nvme_cmd->read();
  admin_cmd->dw0.opc = NVME_ADMIN_CMD_CREATE_SQ;
  admin_cmd->dw0.cid = get_next_cid();
  // These values are not interpretted in DOL testing
  admin_cmd->dw10_11.qid = 1;
  admin_cmd->dw10_11.qsize = 64;
  nvme_cmd->write_thru();

  printf("pvm_cmd %p \n", pvm_cmd->read());

  // Send the NVME admin command and check on PVM side
  rc = send_nvme_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd),
                           queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_read_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;
  dp_mem_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 1 entry to post read command
  // Consume PVM queue 1 to check read command
  uint16_t nvme_q = queues::get_host_nvme_sq(1);
  uint16_t pvm_q = queues::get_pvm_nvme_sq(1);

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  // Form the read command
  if (form_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                             get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }

  // Send the NVME admin command and check on PVM side
  rc = send_nvme_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd),
                           queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_write_cmd() {
  int rc;
  uint16_t nvme_index, pvm_index;
  dp_mem_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 1 entry to post write command
  // Consume PVM queue 1 to check write command
  uint16_t nvme_q = queues::get_host_nvme_sq(1);
  uint16_t pvm_q = queues::get_pvm_nvme_sq(1);

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  // Form the write command
  if (form_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                              get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }


  // Send the NVME admin command and check on PVM side
  rc = send_nvme_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd),
                           queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_hashing1() {
  int rc;
  uint16_t nvme_index, pvm_index;
  dp_mem_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 2 entry to post write command
  // Consume PVM queue 0 (by hashing) to check write command
  uint16_t nvme_q = queues::get_host_nvme_sq(2);
  uint16_t pvm_q = queues::get_pvm_nvme_sq(0);

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  // Form the write command
  if (form_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                              get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }

  // Send the NVME admin command and check on PVM side
  rc = send_nvme_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd),
                           queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_pvm_hashing2() {
  int rc;
  uint16_t nvme_index, pvm_index;
  dp_mem_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 3 entry to post read command
  // Consume PVM queue 1 (by hashing) to check read command
  uint16_t nvme_q = queues::get_host_nvme_sq(3);
  uint16_t pvm_q = queues::get_pvm_nvme_sq(1);

  if (consume_nvme_pvm_sq_entries(nvme_q, pvm_q, &nvme_cmd, &pvm_cmd,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  // Form the read command
  if (form_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                             get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }

  // Send the NVME admin command and check on PVM side
  rc = send_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd),
                      queues::get_nvme_lif(), SQ_TYPE, nvme_q, 0, nvme_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int pvm_status_trailer_update(dp_mem_t *pvm_status, uint16_t lif,
                              uint8_t qtype, uint32_t qid) {
  uint64_t dst_qaddr;
  dp_mem_t *pvm_trailer = pvm_status->fragment_find(offsetof(struct PvmStatus, pvm_trailer),
                                                    sizeof(struct PvmStatus) -
                                                    offsetof(struct PvmStatus, pvm_trailer));

  pvm_trailer->write_bit_fields(0, 11, lif);
  pvm_trailer->write_bit_fields(11, 3, qtype);
  pvm_trailer->write_bit_fields(14, 24, qid);
  if (hal_if::get_lif_qstate_addr(lif, qtype, qid, &dst_qaddr) < 0) {
    printf("Can't get the LIF->qstate addr resolved for PVM status\n");
    return -1;
  }
  pvm_trailer->write_bit_fields(38, 34, dst_qaddr);
  printf("PVM status: base addr %lx \n", dst_qaddr);

  pvm_status->write_thru();
  return 0;
}

int test_run_pvm_nvme_admin_status() {
  int rc;
  uint16_t nvme_index, pvm_index;
  dp_mem_t *nvme_status, *pvm_status;

  // Consume PVM queue 0 entry to post admin command status
  // Consume NVME queue 0 to check admin command status
  uint16_t nvme_q = queues::get_host_nvme_cq(0);
  uint16_t pvm_q = queues::get_pvm_nvme_cq(0);

  if (consume_nvme_pvm_cq_entries(nvme_q, pvm_q, &nvme_status, &pvm_status,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct PvmStatus *admin_status = (struct PvmStatus *) pvm_status->read();
  admin_status->nvme_status.dw3.cid = get_next_cid();
  admin_status->nvme_status.dw3.status_phase = 0;
  if (pvm_status_trailer_update(pvm_status, queues::get_nvme_lif(),
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
  dp_mem_t *nvme_status, *pvm_status;

  // Consume PVM queue 1 entry to post read command status
  // Consume NVME queue 1 to check read command status
  uint16_t nvme_q = queues::get_host_nvme_cq(1);
  uint16_t pvm_q = queues::get_pvm_nvme_cq(1);

  if (consume_nvme_pvm_cq_entries(nvme_q, pvm_q, &nvme_status, &pvm_status,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct PvmStatus *read_status = (struct PvmStatus *) pvm_status->read();
  read_status->nvme_status.dw3.cid = get_next_cid();
  read_status->nvme_status.dw3.status_phase = 0;
  if (pvm_status_trailer_update(pvm_status, queues::get_nvme_lif(),
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
  dp_mem_t *nvme_status, *pvm_status;

  // Consume PVM queue 0 entry to post write command status
  // Consume NVME queue 2 to check write command status
  uint16_t nvme_q = queues::get_host_nvme_cq(2);
  uint16_t pvm_q = queues::get_pvm_nvme_cq(0);

  if (consume_nvme_pvm_cq_entries(nvme_q, pvm_q, &nvme_status, &pvm_status,
                                  &nvme_index, &pvm_index) < 0) {
    return -1;
  }

  struct PvmStatus *write_status = (struct PvmStatus *) pvm_status->read();
  write_status->nvme_status.dw3.cid = get_next_cid();
  write_status->nvme_status.dw3.status_phase = 0;
  if (pvm_status_trailer_update(pvm_status, queues::get_nvme_lif(),
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
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t ssd_handle = 0; uint8_t io_priority = 0; uint8_t is_read = 1;
  dp_mem_t *nvme_cmd, *ssd_cmd;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t ssd_q = queues::get_pvm_ssd_sq(ssd_handle);

  if ((consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, &nvme_cmd, &r2n_index) < 0) ||
      (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0)) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Form the read command
  if (form_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                             get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }


  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check_ignore_cid(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd),
                                 queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_write_cmd() {
  int rc;
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t ssd_handle = 0; uint8_t io_priority = 0; uint8_t is_read = 0;
  dp_mem_t *nvme_cmd, *ssd_cmd;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t ssd_q = queues::get_pvm_ssd_sq(ssd_handle);

  if ((consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, &nvme_cmd, &r2n_index) < 0) ||
      (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0)) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }


  // Form the write command
  if (form_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                              get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }

  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check_ignore_cid(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd),
                                 queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_ssd_pri1() {
  int rc;
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t ssd_handle = 1; uint8_t io_priority = 0; uint8_t is_read = 1;
  dp_mem_t *nvme_cmd, *ssd_cmd;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t ssd_q = queues::get_pvm_ssd_sq(ssd_handle);

  if ((consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, &nvme_cmd, &r2n_index) < 0) ||
      (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0)) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Form the read command
  if (form_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                             get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }


  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check_ignore_cid(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd),
                                 queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_ssd_pri2() {
  int rc;
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t ssd_handle = 15; uint8_t io_priority = 0; uint8_t is_read = 0;
  dp_mem_t *nvme_cmd, *ssd_cmd;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t ssd_q = queues::get_pvm_ssd_sq(ssd_handle);

  if ((consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, &nvme_cmd, &r2n_index) < 0) ||
      (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0)) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Form the write command
  if (form_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                              get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }

  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check_ignore_cid(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd),
                                 queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_ssd_pri3() {
  int rc;
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t ssd_handle = 8; uint8_t io_priority = 1; uint8_t is_read = 1;
  dp_mem_t *nvme_cmd, *ssd_cmd;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t ssd_q = queues::get_pvm_ssd_sq(ssd_handle);

  if ((consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, &nvme_cmd, &r2n_index) < 0) ||
      (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0)) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Form the read command
  if (form_read_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                             get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }


  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check_ignore_cid(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd),
                                 queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_r2n_ssd_pri4() {
  int rc;
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t ssd_handle = 0; uint8_t io_priority = 2; uint8_t is_read = 0;
  dp_mem_t *nvme_cmd, *ssd_cmd;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t ssd_q = queues::get_pvm_ssd_sq(ssd_handle);

  if ((consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, &nvme_cmd, &r2n_index) < 0) ||
      (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0)) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Form the write command
  if (form_write_cmd_with_buf(nvme_cmd, kDefaultBufSize, get_next_cid(),
                              get_next_slba(), kDefaultNlb) < 0) {
    return -1;
  }

  // Send the NVME write command to local target and check on SSD side
  rc = send_and_check_ignore_cid(nvme_cmd, ssd_cmd, sizeof(struct NvmeCmd),
                                 queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int send_r2n_pri_cmd(uint16_t ssd_handle, uint8_t io_priority,
                     uint8_t is_read, dp_mem_t **nvme_cmd) {
  dp_mem_t *r2n_buf;
  dp_mem_t *r2n_wqe_buf;
  uint16_t r2n_index;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);

  if (!nvme_cmd) return -1;

  if (consume_r2n_entry(r2n_q, ssd_handle, io_priority, is_read, &r2n_buf,
                         &r2n_wqe_buf, nvme_cmd, &r2n_index) < 0) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Form the write command
  if (is_read) {
    if (form_read_cmd_with_buf(*nvme_cmd, kDefaultBufSize, get_next_cid(),
                               get_next_slba(), kDefaultNlb) < 0) {
      return -1;
    }
  } else {
    if (form_write_cmd_with_buf(*nvme_cmd, kDefaultBufSize, get_next_cid(),
                                get_next_slba(), kDefaultNlb) < 0) {
      return -1;
    }
  }

  // Send the NVME write command to local target and check on SSD side
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);
  return 0;
}

int run_nvme_be_wrr(uint16_t ssd_q, uint16_t nvme_be_q,
                    uint16_t ssd_handle, uint8_t io_priority, uint8_t is_read,
                    uint8_t hi_weight, uint8_t med_weight, uint8_t lo_weight,
                    uint8_t hi_running, uint8_t med_running, uint8_t lo_running,
                    uint8_t num_running, uint8_t max_cmds) {
  int rc;
  uint16_t ssd_index;
  dp_mem_t *nvme_cmd[3], *ssd_cmd;

  // Expect high priority command out with this distribution
  if (qstate_if::update_pri_q_state(queues::get_pvm_lif(), SQ_TYPE, nvme_be_q,
                                    hi_weight, med_weight, lo_weight,
                                    hi_running, med_running, lo_running,
                                    num_running, max_cmds) < 0) {
    printf("can't update priority queue state \n");
    return -1;
  }

  // Consume entry in SSD queue
  if (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0) {
    printf("can't init and consume r2n/ssd entries \n");
    return -1;
  }

  // Iterate through priority commands and send the high priority command
  for (uint16_t i = 0; i < 3; i++) {
    if (send_r2n_pri_cmd(ssd_handle, i, is_read, &nvme_cmd[i]) < 0) {
      printf("cant send pri cmd %d \n", i);
      return -1;
    }
  }

  // Check if high priority command has been dequeued
  // NOTE: Don't overwrite rc from hereon
  rc = check_ignore_cid(nvme_cmd[io_priority], ssd_cmd, sizeof(struct NvmeCmd));

  // Clear state with this distribution to dequeue all commands
  if (qstate_if::update_pri_q_state(queues::get_pvm_lif(), SQ_TYPE, nvme_be_q,
                                    hi_weight, med_weight, lo_weight,
                                    0, 0, 0, // hi_running, med_running, lo_running
                                    0, max_cmds) < 0) { // num_running set to 0
    printf("can't clear priority queue state \n");
    return -1;
  }

  // Consume 3 more entries in SSD queue
  for (uint16_t i = 0; i < 3; i++) {
    if (consume_ssd_entry(ssd_q, &ssd_cmd, &ssd_index) < 0) {
      printf("can't consume additional ssd entry %u \n", i);
      return -1;
    }
  }

  // Send the high priority command once again which will trigger dequeue of all
  // 3 queues and hence the 3 SSD queue entries consumed would be all written to
  if (send_r2n_pri_cmd(ssd_handle, io_priority, is_read,
                       &nvme_cmd[io_priority]) < 0) {
      printf("cant send pri cmd %d \n", io_priority);
      return -1;
  }

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_be_wrr1() {

  uint16_t ssd_handle = 0;
  return run_nvme_be_wrr(queues::get_pvm_ssd_sq(ssd_handle),     // ssd_q
                         queues::get_pvm_nvme_be_sq(ssd_handle), // nvme_be_q
                         ssd_handle, 0, 1,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         1, 4, 2,   // hi_running, med_running, lo_running
                         7, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr2() {

  uint16_t ssd_handle = 1;
  return run_nvme_be_wrr(queues::get_pvm_ssd_sq(ssd_handle),     // ssd_q
                         queues::get_pvm_nvme_be_sq(ssd_handle), // nvme_be_q
                         ssd_handle, 1, 1,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 0, 2,   // hi_running, med_running, lo_running
                         8, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr3() {

  uint16_t ssd_handle = 10;
  return run_nvme_be_wrr(queues::get_pvm_ssd_sq(ssd_handle),     // ssd_q
                         queues::get_pvm_nvme_be_sq(ssd_handle), // nvme_be_q
                         ssd_handle, 2, 1,  // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 4, 1,   // hi_running, med_running, lo_running
                         11, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr4() {

  uint16_t ssd_handle = 3;
  return run_nvme_be_wrr(queues::get_pvm_ssd_sq(ssd_handle),     // ssd_q
                         queues::get_pvm_nvme_be_sq(ssd_handle), // nvme_be_q
                         ssd_handle, 1, 0,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 3, 2,   // hi_running, med_running, lo_running
                         11, 63);   // num_running, max_cmds
}

int test_run_nvme_be_wrr5() {

  uint16_t ssd_handle = 8;
  return run_nvme_be_wrr(queues::get_pvm_ssd_sq(ssd_handle),     // ssd_q
                         queues::get_pvm_nvme_be_sq(ssd_handle), // nvme_be_q
                         ssd_handle, 0, 0,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         2, 4, 2,   // hi_running, med_running, lo_running
                         8, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr6() {

  uint16_t ssd_handle = 12;
  return run_nvme_be_wrr(queues::get_pvm_ssd_sq(ssd_handle),     // ssd_q
                         queues::get_pvm_nvme_be_sq(ssd_handle), // nvme_be_q
                         ssd_handle, 2, 0,  // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 4, 0,   // hi_running, med_running, lo_running
                         10, 63);   // num_running, max_cmds
}

int test_run_nvme_e2e_io(uint16_t io_priority, uint16_t is_read) {
  int rc;
  dp_mem_t *cmd_buf;
  uint16_t ssd_handle = 2;
  uint16_t pvm_index;
  dp_mem_t *status_buf;
  uint16_t pvm_q = queues::get_pvm_r2n_tgt_sq(0);

  // Reset the SLBA for this test
  reset_slba();

  // Consume status entry from PVM CQ
  status_buf = queues::pvm_cq_consume_entry(pvm_q, &pvm_index);
  if (status_buf == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }
  status_buf->clear_thru();

  // Send the R2N Command
  if (send_r2n_pri_cmd(ssd_handle, io_priority, is_read, &cmd_buf) < 0) {
    printf("cant send pri cmd %d \n", io_priority);
    return -1;
  }

#if 0
  // Test code to check the doorbell (now done in P4)
  // Ring the SSD doorbell.
  queues::ring_nvme_e2e_ssd();
#endif

  // Process the status
  dp_mem_t *nvme_status = status_buf->fragment_find(kR2nStatusNvmeOffset,
                                                    sizeof(struct NvmeStatus));

  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  return rc;
}

int test_run_nvme_read_comp1() {
  return test_run_nvme_e2e_io(0, 1); // io_priority, is_read
}

int test_run_nvme_write_comp1() {
  return test_run_nvme_e2e_io(0, 0); // io_priority, is_read
}

int test_run_nvme_read_comp2() {
  return test_run_nvme_e2e_io(1, 1); // io_priority, is_read
}

int test_run_nvme_write_comp2() {
  return test_run_nvme_e2e_io(1, 0); // io_priority, is_read
}

int test_run_nvme_read_comp3() {
  return test_run_nvme_e2e_io(2, 1); // io_priority, is_read
}

int test_run_nvme_write_comp3() {
  return test_run_nvme_e2e_io(2, 0); // io_priority, is_read
}

int test_run_nvme_local_e2e(uint16_t io_priority) {

  int rc;

  // First write data
  rc = test_run_nvme_e2e_io(io_priority, 0); // is_read
  if (rc < 0) {
    printf("e2e test write part failed \n");
    return rc;
  }

  // Then read back the data
  rc = test_run_nvme_e2e_io(io_priority, 1); // is_read
  if (rc < 0) {
    printf("e2e test write part failed \n");
    return rc;
  }

  rc = memcmp(read_buf->read_thru(), write_buf->read_thru(), kDefaultBufSize);
  printf("\nE2E: post memcmp %d \n", rc);

  // rc could be <, ==, > 0. We need to return -1 from this API on error.
  return (rc ? -1 : 0);
}

int test_run_nvme_local_e2e1() {
  return test_run_nvme_local_e2e(0);
}

int test_run_nvme_local_e2e2() {
  return test_run_nvme_local_e2e(1);
}

int test_run_nvme_local_e2e3() {
  return test_run_nvme_local_e2e(2);
}

int form_seq_r2n_wqe_cmd(uint16_t seq_r2n_q, uint16_t ssd_handle, uint8_t io_priority,
                         uint8_t is_read, dp_mem_t **cmd_buf, uint16_t r2n_q,
                         dp_mem_t **r2n_wqe_buf) {
  dp_mem_t *r2n_buf;

  if (!cmd_buf || !r2n_wqe_buf) return -1;

  if (form_r2n_seq_wqe(ssd_handle, io_priority, is_read, r2n_q, &r2n_buf, r2n_wqe_buf,
                       cmd_buf) < 0) {
    printf("can't form seq r2n entries \n");
    return -1;
  }

  // Form the write command
  if (is_read) {
    if (form_read_cmd_with_hbm_buf(*cmd_buf, kDefaultBufSize, get_next_cid(),
                                   get_next_slba(), kDefaultNlb) < 0) {
      return -1;
    }
  } else {
    if (form_write_cmd_with_hbm_buf(*cmd_buf, kDefaultBufSize, get_next_cid(),
                                    get_next_slba(), kDefaultNlb) < 0) {
      return -1;
    }
  }

  return 0;
}

int test_seq_write_r2n(uint16_t seq_pdma_q, uint16_t seq_r2n_q,
                       uint16_t ssd_handle, uint16_t io_priority) {
  uint16_t seq_pdma_index;
  dp_mem_t *seq_pdma_desc;
  uint16_t seq_r2n_index;
  dp_mem_t *seq_r2n_desc;
  uint64_t db_data;
  uint64_t db_addr;
  uint16_t r2n_q = queues::get_pvm_r2n_host_sq(0);
  dp_mem_t *r2n_wqe_buf;
  dp_mem_t *cmd_buf;
  uint16_t pvm_status_q = queues::get_pvm_r2n_cq(0);
  uint16_t pvm_status_index;
  dp_mem_t *status_buf;
  int rc;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  status_buf->clear_thru();

  // Form the r2n wqe
  if (form_seq_r2n_wqe_cmd(seq_r2n_q, ssd_handle, io_priority, 0, /* is_read */
                           &cmd_buf, r2n_q, &r2n_wqe_buf) < 0) {
    printf("Can't form sequencer R2N wqe + cmd \n");
    return -1;
  }

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Sequencer #2: R2N descriptor
  seq_r2n_desc = queues::seq_sq_consume_entry(seq_r2n_q, &seq_r2n_index);
  seq_r2n_desc->clear();

  // Fill the PDMA descriptor
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_r2n_q, 0,
                             seq_r2n_index, &db_addr, &db_data);
  seq_pdma_desc->write_bit_fields(0, 64, db_addr);
  seq_pdma_desc->write_bit_fields(64, 64, bswap_64(db_data));
  seq_pdma_desc->write_bit_fields(128, 64, write_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, write_hbm_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);
  // Enable the next doorbell
  seq_pdma_desc->write_bit_fields(408, 1, 1);
  seq_pdma_desc->write_thru();

  // Fill the R2N descriptor
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, r2n_q, &qaddr) < 0) {
    printf("Can't get R2N qaddr \n");
    return -1;
  }
  seq_r2n_desc->write_bit_fields(0, 64, r2n_wqe_buf->pa());
  seq_r2n_desc->write_bit_fields(64, 32, kR2nWqeSize);
  seq_r2n_desc->write_bit_fields(96, 11, queues::get_pvm_lif());
  seq_r2n_desc->write_bit_fields(107, 3, SQ_TYPE);
  seq_r2n_desc->write_bit_fields(110, 24, r2n_q);
  seq_r2n_desc->write_bit_fields(134, 34, qaddr);
  seq_r2n_desc->write_thru();

  // Kickstart the sequencer
  test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0, seq_pdma_index);

  // Process the status
  dp_mem_t *nvme_status = status_buf->fragment_find(kR2nStatusNvmeOffset,
                                                    sizeof(struct NvmeStatus));

  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  return rc;
}

int test_seq_read_r2n(uint16_t seq_pdma_q, uint16_t ssd_handle,
                      uint16_t io_priority) {
  uint16_t seq_pdma_index;
  dp_mem_t *seq_pdma_desc;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t r2n_index;
  dp_mem_t *r2n_wqe_buf;
  dp_mem_t *r2n_buf;
  dp_mem_t *cmd_buf;
  uint16_t pvm_status_q = queues::get_pvm_r2n_cq(0);
  uint16_t pvm_status_index;
  dp_mem_t *status_buf;
  int rc;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  status_buf->clear_thru();

  // Consume the r2n entry and wqe
  if (consume_r2n_entry(r2n_q, ssd_handle, io_priority, 1, /* is_read */
                        &r2n_buf, &r2n_wqe_buf, &cmd_buf, &r2n_index) < 0) {
    printf("Can't form consume read sequencer R2N buf \n");
    return -1;
  }

  // From the NVME read command in the R2N buffer
  if (form_read_cmd_with_hbm_buf(cmd_buf, kDefaultBufSize, get_next_cid(),
                                 get_next_slba(), kDefaultNlb) < 0) {
      return -1;
  }

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Fill the PDMA descriptor
  seq_pdma_desc->write_bit_fields(128, 64, read_hbm_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, read_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);
  // Form the intadd/data and enable it
  seq_pdma_desc->write_bit_fields(312, 64, seq_db_data->pa());
  seq_pdma_desc->write_bit_fields(376, 32, kSeqDbDataMagic);
  seq_pdma_desc->write_bit_fields(409, 1, 1);
  seq_pdma_desc->write_thru();

  // Update the R2N WQE with the doorbell to the PDMA descriptor
  r2n::r2n_wqe_db_update(r2n_wqe_buf, queues::get_seq_lif(), SQ_TYPE,
                         seq_pdma_q, seq_pdma_index);

  // Kickstart the R2N module with the read command (whose completion will
  // trigger the sequencer)
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // Process the status
  dp_mem_t *nvme_status = status_buf->fragment_find(kR2nStatusNvmeOffset,
                                                    sizeof(struct NvmeStatus));

  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  // Poll for DMA completion of read data only if status is successful
  if (rc >= 0) {
    auto func2 = [] () {
      if  (*((uint32_t *) seq_db_data->read_thru()) != kSeqDbDataMagic) {
        //printf("Sequencer magic incorrect %lx \n", *((uint32_t *) seq_db_data->read_thru()));
        return -1;
      }
      return 0;
    };

    rc = poll(func2);
  }

  return rc;
}

int test_seq_e2e_r2n(uint16_t seq_pdma_q, uint16_t seq_r2n_q,
                     uint16_t ssd_handle, uint16_t io_priority) {
  int rc;

  // Send the write command and check status
  rc = test_seq_write_r2n(seq_pdma_q, seq_r2n_q, ssd_handle,
                          io_priority);
  if (rc < 0) return -1;

  // Memcmp before reading
  rc = memcmp(read_buf->read(), write_buf->read(), kDefaultBufSize);
  printf("\nE2E: pre memcmp %d \n", rc);

  // Send the read command and check status
  rc = test_seq_read_r2n(seq_pdma_q, ssd_handle, io_priority);
  if (rc < 0) return -1;

  // Memcmp after reading and verify its the same
  rc = memcmp(read_buf->read_thru(), write_buf->read_thru(), kDefaultBufSize);
  printf("\nE2E: post memcmp %d \n", rc);

  return rc;
}


int test_run_seq_write1() {
  return test_seq_write_r2n(queues::get_seq_pdma_sq(0), // seq_pdma_q
                            queues::get_seq_r2n_sq(0),  // seq_r2n_q
                            2, 0);   // ssd_handle, io_priority
}

int test_run_seq_write2() {
  return test_seq_write_r2n(queues::get_seq_pdma_sq(1), // seq_pdma_q
                            queues::get_seq_r2n_sq(7),  // seq_r2n_q
                            2, 1);   // ssd_handle, io_priority
}

int test_run_seq_write3() {
  return test_seq_write_r2n(queues::get_seq_pdma_sq(0), // seq_pdma_q
                            queues::get_seq_r2n_sq(6),  // seq_r2n_q
                            2, 2);   // ssd_handle, io_priority
}

int test_run_seq_write4() {
  return test_seq_write_r2n(queues::get_seq_pdma_sq(7), // seq_pdma_q
                            queues::get_seq_r2n_sq(6),  // seq_r2n_q
                            2, 2);   // ssd_handle, io_priority
}

int test_run_seq_read1() {
  return test_seq_read_r2n(queues::get_seq_pdma_sq(0),  // seq_pdma_q
                           2, 0);   // ssd_handle, io_priority
}

int test_run_seq_read2() {
  return test_seq_read_r2n(queues::get_seq_pdma_sq(2),  // seq_pdma_q
                           2, 1);   // ssd_handle, io_priority
}

int test_run_seq_read3() {
  return test_seq_read_r2n(queues::get_seq_pdma_sq(0),  // seq_pdma_q
                           2, 2);   // ssd_handle, io_priority
}

int test_run_seq_read4() {
  return test_seq_read_r2n(queues::get_seq_pdma_sq(6),  // seq_pdma_q
                           2, 1);   // ssd_handle, io_priority
}

int test_run_seq_e2e1() {
  return test_seq_e2e_r2n(queues::get_seq_pdma_sq(0), // seq_pdma_q
                          queues::get_seq_r2n_sq(0),  // seq_r2n_q
                          2, 0);   // ssd_handle, io_priority
}

int test_run_seq_e2e2() {
  return test_seq_e2e_r2n(queues::get_seq_pdma_sq(0), // seq_pdma_q
                          queues::get_seq_r2n_sq(1),  // seq_r2n_q
                          2, 1);   // ssd_handle, io_priority
}

int test_run_seq_e2e3() {
  return test_seq_e2e_r2n(queues::get_seq_pdma_sq(1), // seq_pdma_q
                          queues::get_seq_r2n_sq(1),  // seq_r2n_q
                          2, 2);   // ssd_handle, io_priority
}

int test_run_seq_e2e4() {
  return test_seq_e2e_r2n(queues::get_seq_pdma_sq(5), // seq_pdma_q
                          queues::get_seq_r2n_sq(5),  // seq_r2n_q
                          2, 0);   // ssd_handle, io_priority
}

int test_seq_write_xts_r2n(uint16_t seq_pdma_q, uint16_t seq_r2n_q,
                           uint16_t ssd_handle, uint16_t io_priority, XtsCtx& xts_ctx) {
  uint16_t seq_pdma_index;
  dp_mem_t *seq_pdma_desc;
  uint16_t seq_r2n_index;
  dp_mem_t *seq_r2n_desc;
  uint64_t db_data;
  uint64_t db_addr;
  uint16_t r2n_q = queues::get_pvm_r2n_host_sq(0);
  dp_mem_t *r2n_wqe_buf;
  dp_mem_t *cmd_buf;
  uint16_t pvm_status_q = queues::get_pvm_r2n_cq(0);
  uint16_t pvm_status_index;
  dp_mem_t *status_buf;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  status_buf->clear_thru();

  // Form the r2n wqe
  if (form_seq_r2n_wqe_cmd(seq_r2n_q, ssd_handle, io_priority, 0, /* is_read */
                           &cmd_buf, r2n_q, &r2n_wqe_buf) < 0) {
    printf("Can't form sequencer R2N wqe + cmd \n");
    return -1;
  }

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Sequencer #3: R2N descriptor
  seq_r2n_desc = queues::seq_sq_consume_entry(seq_r2n_q, &seq_r2n_index);
  seq_r2n_desc->clear();

  // Fill the PDMA descriptor
  seq_pdma_desc->write_bit_fields(128, 64, write_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, write_hbm_buf2->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);

  xts_ctx.op = xts::AES_ENCR_ONLY;
  xts_ctx.src_buf = (void*)write_hbm_buf2->va();
  xts_ctx.is_src_hbm_buf = write_hbm_buf2->is_mem_type_hbm();
  xts_ctx.dst_buf = (void*)write_hbm_buf->va();
  xts_ctx.is_dst_hbm_buf = write_hbm_buf->is_mem_type_hbm();
  xts_ctx.num_sectors = kDefaultBufSize/SECTOR_SIZE;
  xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;
  xts_ctx.init(kDefaultBufSize);
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_r2n_q, 0,
                             seq_r2n_index, &xts_ctx.xts_db_addr, &xts_ctx.exp_db_data);
  //xts_ctx.exp_db_data = bswap_64(xts_ctx.exp_db_data);
  printf("r2n_db_addr %lx r2n_db_data %lu\n", xts_ctx.xts_db_addr, xts_ctx.exp_db_data);
  xts_ctx.test_seq_xts();
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, xts_ctx.seq_xts_q, 0,
                             xts_ctx.seq_xts_index_get(), &db_addr, &db_data);
  queues::seq_sq_batch_consume_end(xts_ctx.seq_xts_q);

  seq_pdma_desc->write_bit_fields(0, 64, db_addr);
  seq_pdma_desc->write_bit_fields(64, 64, bswap_64(db_data));
  // Enable the next doorbell
  seq_pdma_desc->write_bit_fields(408, 1, 1);
  seq_pdma_desc->write_thru();

  // Fill the R2N descriptor
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, r2n_q, &qaddr) < 0) {
    printf("Can't get R2N qaddr \n");
    return -1;
  }
  seq_r2n_desc->write_bit_fields(0, 64, r2n_wqe_buf->pa());
  seq_r2n_desc->write_bit_fields(64, 32, kR2nWqeSize);
  seq_r2n_desc->write_bit_fields(96, 11, queues::get_pvm_lif());
  seq_r2n_desc->write_bit_fields(107, 3, SQ_TYPE);
  seq_r2n_desc->write_bit_fields(110, 24, r2n_q);
  seq_r2n_desc->write_bit_fields(134, 34, qaddr);
  seq_r2n_desc->write_thru();

  // Kickstart the sequencer
  test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0, seq_pdma_index);

  // Process the status
  dp_mem_t *nvme_status = status_buf->fragment_find(kR2nStatusNvmeOffset,
                                                    sizeof(struct NvmeStatus));
  auto func = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  int rv = poll(func);
  return rv;
}

int test_seq_read_xts_r2n(uint16_t seq_pdma_q, uint16_t ssd_handle,
                      uint16_t io_priority, XtsCtx& xts_ctx) {
  uint16_t seq_pdma_index;
  dp_mem_t *seq_pdma_desc;
  uint16_t r2n_q = queues::get_pvm_r2n_tgt_sq(0);
  uint16_t r2n_index;
  dp_mem_t *r2n_wqe_buf;
  dp_mem_t *r2n_buf;
  dp_mem_t *cmd_buf;
  uint16_t pvm_status_q = queues::get_pvm_r2n_cq(0);
  uint16_t pvm_status_index;
  dp_mem_t *status_buf;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  status_buf->clear_thru();

  // Consume pdma entry
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Consume the r2n entry and wqe
  if (consume_r2n_entry(r2n_q, ssd_handle, io_priority, 1, /* is_read */
                        &r2n_buf, &r2n_wqe_buf, &cmd_buf, &r2n_index) < 0) {
    printf("Can't form consume read sequencer R2N buf \n");
    return -1;
  }

  // From the NVME read command in the R2N buffer
  if (form_read_cmd_with_hbm_buf(cmd_buf, kDefaultBufSize, get_next_cid(),
                                 get_next_slba(), kDefaultNlb) < 0) {
      return -1;
  }

  // Sequencer #1: XTS
  xts_ctx.op = xts::AES_DECR_ONLY;
  xts_ctx.src_buf = (void*)read_hbm_buf->va();
  xts_ctx.is_src_hbm_buf = read_hbm_buf->is_mem_type_hbm();
  xts_ctx.dst_buf = (void*)read_hbm_buf2->va();
  xts_ctx.is_dst_hbm_buf = read_hbm_buf2->is_mem_type_hbm();
  xts_ctx.num_sectors = kDefaultBufSize/SECTOR_SIZE;
  xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;
  xts_ctx.init(kDefaultBufSize);
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0,
      seq_pdma_index, &xts_ctx.xts_db_addr, &xts_ctx.exp_db_data);
  //xts_ctx.exp_db_data = bswap_64(xts_ctx.exp_db_data);
  xts_ctx.test_seq_xts();
  queues::seq_sq_batch_consume_end(xts_ctx.seq_xts_q);

  // Sequencer #2: PDMA descriptor

  // Fill the PDMA descriptor
  seq_pdma_desc->write_bit_fields(128, 64, read_hbm_buf2->pa());
  seq_pdma_desc->write_bit_fields(192, 64, read_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);
  // Form the interrupt add/data and enable it
  seq_pdma_desc->write_bit_fields(312, 64, seq_db_data->pa());
  seq_pdma_desc->write_bit_fields(376, 32, kSeqDbDataMagic);
  seq_pdma_desc->write_bit_fields(409, 1, 1);
  seq_pdma_desc->write_thru();

  // Update the R2N WQE with the doorbell to the PDMA descriptor
  r2n::r2n_wqe_db_update(r2n_wqe_buf, queues::get_seq_lif(), SQ_TYPE,
                         xts_ctx.seq_xts_q, xts_ctx.seq_xts_index_get());

  // Kickstart the R2N module with the read command (whose completion will
  // trigger the sequencer)
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // Process the status
  dp_mem_t *nvme_status = status_buf->fragment_find(kR2nStatusNvmeOffset,
                                                    sizeof(struct NvmeStatus));

  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  int rv = poll(func1);

  auto func2 = [] () {
    if  (*((uint32_t *) seq_db_data->read_thru()) != kSeqDbDataMagic) {
      //printf("Sequencer magic incorrect %lx \n", *((uint32_t *) seq_db_data->read_thru()));
      return -1;
    }
    return 0;
  };

  if(0 == rv) return poll(func2);
  return rv;
}

int test_seq_e2e_xts_r2n(uint16_t seq_pdma_q, uint16_t seq_r2n_q,
                     uint16_t ssd_handle, uint16_t io_priority) {
  int rc;
  XtsCtx xts_ctx_write, xts_ctx_read;

  read_buf->clear_thru();

  // Send the write command and check status
  rc = test_seq_write_xts_r2n(seq_pdma_q, seq_r2n_q, ssd_handle,
                          io_priority, xts_ctx_write);
  if (rc < 0) return -1;

  // Memcmp before reading
  rc = memcmp(read_buf->read(), write_buf->read(), kDefaultBufSize);
  printf("\nE2E: pre memcmp %d \n", rc);

  // Send the read command and check status
  rc = test_seq_read_xts_r2n(seq_pdma_q, ssd_handle, io_priority, xts_ctx_read);
  if (rc < 0) return -1;

  // Memcmp after reading and verify its the same
  rc = memcmp(read_buf->read_thru(), write_buf->read_thru(), kDefaultBufSize);
  printf("\nE2E: post memcmp %d \n", rc);

  return rc;
}

int test_seq_e2e_xts_r2n1() {
  return test_seq_e2e_xts_r2n(queues::get_seq_pdma_sq(5), // seq_pdma_q
                              queues::get_seq_r2n_sq(5),  // seq_r2n_q
                              2, 0);   // ssd_handle, io_priority
}

int test_seq_write_roce(uint32_t seq_pdma_q, uint32_t seq_roce_q,
                        uint32_t pvm_roce_sq, uint64_t pdma_src_addr,
                        uint64_t pdma_dst_addr, uint32_t pdma_data_size,
                        uint64_t roce_wqe_addr, uint32_t roce_wqe_size) {

  uint16_t seq_pdma_index;
  dp_mem_t *seq_pdma_desc;
  uint16_t seq_roce_index;
  dp_mem_t *seq_roce_desc;
  uint64_t db_data;
  uint64_t db_addr;

  printf("pdma_q %u, roce_q %u, roce_sq %u, pdma_src_addr %lx, "
         "pdma_dst_addr %lx, pdma_data_size %u, "
         "roce_wqe_addr %lx, roce_wqe_size %u \n",
         seq_pdma_q, seq_roce_q, pvm_roce_sq, pdma_src_addr,
         pdma_dst_addr, pdma_data_size, roce_wqe_addr, roce_wqe_size);

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Sequencer #2: R2N descriptor
  seq_roce_desc = queues::seq_sq_consume_entry(seq_roce_q, &seq_roce_index);
  seq_roce_desc->clear();

  // Fill the PDMA descriptor
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_roce_q, 0,
           seq_roce_index, &db_addr, &db_data);
  seq_pdma_desc->write_bit_fields(0, 64, db_addr);
  seq_pdma_desc->write_bit_fields(64, 64, bswap_64(db_data));
  seq_pdma_desc->write_bit_fields(128, 64, pdma_src_addr);
  seq_pdma_desc->write_bit_fields(192, 64, pdma_dst_addr);
  seq_pdma_desc->write_bit_fields(256, 32, pdma_data_size);
  // Enable the next doorbell
  seq_pdma_desc->write_bit_fields(408, 1, 1);
  seq_pdma_desc->write_thru();

  // Fill the Sequencer ROCE descriptor
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, pvm_roce_sq, &qaddr) < 0) {
    printf("Can't get PVM's ROCE SQ qaddr \n");
    return -1;
  }
  seq_roce_desc->write_bit_fields(0, 64, roce_wqe_addr);
  seq_roce_desc->write_bit_fields(64, 32, roce_wqe_size);
  seq_roce_desc->write_bit_fields(96, 11, queues::get_pvm_lif());
  seq_roce_desc->write_bit_fields(107, 3, SQ_TYPE);
  seq_roce_desc->write_bit_fields(110, 24, pvm_roce_sq);
  seq_roce_desc->write_bit_fields(134, 34, qaddr);
  seq_roce_desc->write_bit_fields(168, 8, 1);
  seq_roce_desc->write_thru();

  // Kickstart the sequencer
  if(rdma_ring_db) {
    test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0, seq_pdma_index);
  }

  return 0;
}

int test_seq_roce_op_pdma_prefilled(uint16_t seq_start_q,
                                    uint16_t seq_start_index,
                                    dp_mem_t *seq_roce_desc,
                                    uint32_t pvm_roce_sq,
                                    dp_mem_t *sqwqe)
{
  uint64_t qaddr;

  printf("seq_start_q %u, seq_start_index %u, pvm_roce_sq %u\n",
         seq_start_q, seq_start_index, pvm_roce_sq);

  // Fill the Sequencer ROCE descriptor
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE,
                                 pvm_roce_sq, &qaddr) < 0) {
    printf("Can't get PVM's ROCE SQ qaddr \n");
    return -1;
  }

  // Sequencer: R2N descriptor
  seq_roce_desc->write_bit_fields(0, 64, sqwqe->pa());
  seq_roce_desc->write_bit_fields(64, 32, sqwqe->line_size_get());
  seq_roce_desc->write_bit_fields(96, 11, queues::get_pvm_lif());
  seq_roce_desc->write_bit_fields(107, 3, SQ_TYPE);
  seq_roce_desc->write_bit_fields(110, 24, pvm_roce_sq);
  seq_roce_desc->write_bit_fields(134, 34, qaddr);
  seq_roce_desc->write_bit_fields(168, 8, 1);
  seq_roce_desc->write_thru();

  // Kickstart the sequencer
  test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_start_q,
                     0, seq_start_index);

  return 0;
}

int test_seq_read_roce(uint32_t seq_pdma_q, uint32_t seq_roce_q, uint32_t pvm_roce_sq,
                       uint64_t pdma_src_addr, uint64_t pdma_dst_addr, uint32_t pdma_data_size,
                       uint8_t pdma_dst_lif_override, uint16_t pdma_dst_lif,
                       uint64_t roce_wqe_addr, uint32_t roce_wqe_size) {

  uint16_t seq_pdma_index;
  dp_mem_t *seq_pdma_desc;
  uint16_t seq_roce_index;
  dp_mem_t *seq_roce_desc;

  reset_seq_db_data();

  printf("seq_pdma_q %u, seq_roce_q %u, pvm_roce_sq %u, "
         "pdma_src_addr %lx, pdma_dst_addr %lx, "
         "pdma_data_size %u, pdma_dst_lif_override %u, pdma_dst_lif %u\n",
	     seq_pdma_q, seq_roce_q, pvm_roce_sq, pdma_src_addr,
         pdma_dst_addr, pdma_data_size, pdma_dst_lif_override, pdma_dst_lif);

  // Sequencer #1: R2N descriptor
  seq_roce_desc = queues::seq_sq_consume_entry(seq_roce_q, &seq_roce_index);
  seq_roce_desc->clear();

  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, pvm_roce_sq, &qaddr) < 0) {
    printf("Can't get PVM's ROCE SQ qaddr \n");
    return -1;
  }
  seq_roce_desc->write_bit_fields(0, 64, roce_wqe_addr);
  seq_roce_desc->write_bit_fields(64, 32, roce_wqe_size);
  seq_roce_desc->write_bit_fields(96, 11, queues::get_pvm_lif());
  seq_roce_desc->write_bit_fields(107, 3, SQ_TYPE);
  seq_roce_desc->write_bit_fields(110, 24, pvm_roce_sq);
  seq_roce_desc->write_bit_fields(134, 34, qaddr);
  seq_roce_desc->write_bit_fields(168, 8, 1);
  seq_roce_desc->write_thru();

  // Sequencer #2: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();
  seq_pdma_desc->write_bit_fields(128, 64, pdma_src_addr);
  seq_pdma_desc->write_bit_fields(192, 64, pdma_dst_addr);
  seq_pdma_desc->write_bit_fields(256, 32, pdma_data_size);
  seq_pdma_desc->write_bit_fields(300, 1, pdma_dst_lif_override);
  seq_pdma_desc->write_bit_fields(301, 11, pdma_dst_lif);
  // Form the interrupt add/data and enable it
  seq_pdma_desc->write_bit_fields(312, 64, seq_db_data->pa());
  seq_pdma_desc->write_bit_fields(376, 32, kSeqDbDataMagic);
  seq_pdma_desc->write_bit_fields(409, 1, 1);
  seq_pdma_desc->write_thru();

  // Kickstart the sequencer
  if(rdma_ring_db) {
    test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_roce_q, 0, seq_roce_index);
  }

  return 0;
}

int test_seq_pdma_write(uint16_t seq_pdma_q,
                        dp_mem_t *curr_wr_buf,
                        dp_mem_t *curr_wr_hbm_buf,
                        uint64_t curr_db_data_pa,
                        uint32_t exp_db_data_value) {
  uint16_t  seq_pdma_index;
  dp_mem_t  *seq_pdma_desc;

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Fill the PDMA descriptor
  seq_pdma_desc->write_bit_fields(128, 64, curr_wr_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, curr_wr_hbm_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);

  // Form the interrupt add/data and enable it
  seq_pdma_desc->write_bit_fields(312, 64, curr_db_data_pa);
  seq_pdma_desc->write_bit_fields(376, 32, exp_db_data_value);
  seq_pdma_desc->write_bit_fields(409, 1, 1);
  seq_pdma_desc->write_thru();

  // Kickstart the sequencer
  test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0,
                     seq_pdma_index, true);

  return 0;
}

int test_seq_pdma_read(uint16_t seq_pdma_q,
                       dp_mem_t *curr_rd_buf,
                       dp_mem_t *curr_rd_hbm_buf,
                       uint64_t curr_db_data_pa,
                       uint32_t exp_db_data_value) {
  uint16_t  seq_pdma_index;
  dp_mem_t  *seq_pdma_desc;

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Fill the PDMA descriptor
  seq_pdma_desc->write_bit_fields(128, 64, curr_rd_hbm_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, curr_rd_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);
  // Form the interrupt add/data and enable it
  seq_pdma_desc->write_bit_fields(312, 64, curr_db_data_pa);
  seq_pdma_desc->write_bit_fields(376, 32, exp_db_data_value);
  seq_pdma_desc->write_bit_fields(409, 1, 1);
  seq_pdma_desc->write_thru();

  // Kickstart the sequencer
  test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0,
                     seq_pdma_index, true);

  return 0;
}

int test_run_seq_pdma_multi_xfers() {
    dp_mem_t    *wr_seq_db_data;
    dp_mem_t    *rd_seq_db_data;
    dp_mem_t    *exp_seq_db_data;
    dp_mem_t    *pdma_wr_buf;
    dp_mem_t    *pdma_rd_buf;
    dp_mem_t    *pdma_wr_hbm_buf;
    uint32_t    exp_db_data_value;
    uint32_t    total_seq_db_data_size;
    int         val_pdma_queues = NUM_TO_VAL(FLAGS_num_pdma_queues);
    int         i;
    int         rc;

    /*
     * We allocate db_data as stream of bytes for easy comparison
     */
    assert(val_pdma_queues);
    total_seq_db_data_size = kSeqDbDataSize * val_pdma_queues;
    wr_seq_db_data = new dp_mem_t(1, total_seq_db_data_size);
    rd_seq_db_data = new dp_mem_t(1, total_seq_db_data_size);
    exp_seq_db_data = new dp_mem_t(1, total_seq_db_data_size,
                                  DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

#define SEQ_DB_DATA_ENTRY_PA(pa, i) \
    (pa + (i * kSeqDbDataSize))

    // Write from the same source host buffer, but read into different
    // destination host buffers
    pdma_wr_buf = new dp_mem_t(1, kDefaultBufSize,
                               DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
    pdma_wr_buf->fill_thru(0x55);
    pdma_rd_buf = new dp_mem_t(val_pdma_queues, kDefaultBufSize,
                               DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
    pdma_wr_hbm_buf = new dp_mem_t(val_pdma_queues, kDefaultBufSize,
                                   DP_MEM_ALIGN_PAGE);

    // Set expected completion doorbell data value
    exp_db_data_value = 0xdbdbdbdb;
    exp_seq_db_data->fill_thru(0xdb);

    for (i = 0; i < val_pdma_queues; i++) {

        // Write from the same source host buffer,
        // but to different per-queue destination HBM buffers
        test_seq_pdma_write(queues::get_seq_pdma_sq(i), pdma_wr_buf,
                            pdma_wr_hbm_buf,
                            SEQ_DB_DATA_ENTRY_PA(wr_seq_db_data->pa(), i),
                            exp_db_data_value);

        // Read into different per-queue destination host buffers,
        // from the different per-queue source HBM buffers above
        test_seq_pdma_read(queues::get_seq_pdma_sq(i), pdma_rd_buf,
                           pdma_wr_hbm_buf,
                           SEQ_DB_DATA_ENTRY_PA(rd_seq_db_data->pa(), i),
                           exp_db_data_value);
        pdma_wr_hbm_buf->line_advance();
        pdma_rd_buf->line_advance();
        if (i && ((i % 100) == 0)) {
            printf("%s: submitted %d PDMA write/read transfer pairs\n",
                   __FUNCTION__, i);
        }
    }

    // Poll for DMA completion
    auto dma_db_compl_verify = [wr_seq_db_data, rd_seq_db_data,
                                exp_seq_db_data, total_seq_db_data_size] () {

        if (memcmp(wr_seq_db_data->read_thru(), exp_seq_db_data->read(),
                   total_seq_db_data_size) ||
            memcmp(rd_seq_db_data->read_thru(), exp_seq_db_data->read(),
                   total_seq_db_data_size)) {
            return -1;
        }
        return 0;
    };

    Poller poll(FLAGS_long_poll_interval * FLAGS_num_pdma_queues);
    rc = poll(dma_db_compl_verify);

    printf("%s: completed %d PDMA write/read transfer pairs. rc = %d\n",
           __FUNCTION__, (int)val_pdma_queues, rc);
    if (rc == 0) {
        auto wr_rd_data_verify = [pdma_wr_buf, pdma_rd_buf] (int qid) {

            pdma_rd_buf->line_set(qid);
            if (memcmp(pdma_rd_buf->read_thru(), pdma_wr_buf->read(),
                       kDefaultBufSize)) {
                printf("%s: all transfers completed but read buffer for "
                       "queue %d has incorrect data\n", __FUNCTION__, qid);
                return -1;
            }
            return 0;
        };

        // Verify 1st, middle, and last buffer data
        rc = wr_rd_data_verify(0);
        if (rc == 0) {
            rc = wr_rd_data_verify(val_pdma_queues / 2);
        }
        if (rc == 0) {
            rc = wr_rd_data_verify(val_pdma_queues - 1);
        }
    }

    delete pdma_rd_buf;
    delete pdma_wr_buf;
    delete exp_seq_db_data;
    delete rd_seq_db_data;
    delete wr_seq_db_data;
    delete pdma_wr_hbm_buf;

    return rc;
}

// Keep a rolling write data buffer and slba to pair up with read case for comparison
dp_mem_t *rolling_write_data_buf = NULL;
uint64_t rolling_write_slba = 0;

int test_run_rdma_e2e_write() {
  uint16_t ssd_handle = 2; // the SSD handle
  dp_mem_t *cmd_buf = NULL;
  int rc;

  // Get the SLBA to write to and read from
  rolling_write_slba = get_next_slba();

  StartRoceWriteSeq(ssd_handle, get_next_byte(), &cmd_buf, rolling_write_slba, &rolling_write_data_buf);
  printf("Started sequencer to PDMA + write command send over ROCE \n");

  //printf("Dumping NVME command sent \n");
  //utils::dump(cmd_buf);

  dp_mem_t *rcv_buf = rdma_get_initiator_rcv_buf();
  dp_mem_t *nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));
  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  if (rc < 0)
    printf("Failure in retriving status \n");
  else
    printf("Successfully retrived status \n");

  // Post the Initiator buffers back so that RDMA can reuse them.
  PostInitiatorRcvBuf1();
  IncrInitiatorRcvBufPtr();

  return rc;
}

int test_run_rdma_e2e_read() {
  uint32_t seq_pdma_q = queues::get_seq_pdma_sq(3);
  uint32_t seq_roce_q = queues::get_seq_roce_sq(3);
  uint16_t ssd_handle = 2; // the SSD handle
  dp_mem_t *cmd_buf = NULL;
  dp_mem_t *data_buf = NULL;
  int rc;


  StartRoceReadSeq(seq_pdma_q, seq_roce_q, ssd_handle, &cmd_buf, &data_buf, rolling_write_slba, 0, 0, 0);
  printf("Started read command send over ROCE \n");

  //printf("Dumping NVME command sent \n");
  //utils::dump(cmd_buf);

  // Process the status
  dp_mem_t *rcv_buf = rdma_get_initiator_rcv_buf();
  dp_mem_t *nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));

  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  // Poll for DMA completion of read data only if status is successful
  if (rc == 0) {
    printf("Successfully retrived status \n");
    auto func2 = [] () {
      if  (*((uint32_t *) seq_db_data->read_thru()) != kSeqDbDataMagic) {
        //printf("Sequencer magic incorrect %lx \n", *((uint32_t *) seq_db_data->read_thru()));
        return -1;
      }
      return 0;
    };

    rc = poll(func2);

    // Now compare the contents
    if (rc == 0) {
      printf("Successfully retrived data \n");
      if (!rolling_write_data_buf) {
        printf("No write data buffer for comparison \n");
      } else {
        // Enable this to debug as needed
        //printf("Dumping data buffer which contains NVME read data\n");
        //utils::dump(data_buf->read_thru());
        //printf("Dumping rolling NVME write data buffer\n");
        //utils::dump(rolling_write_data_buf->read_thru());
        if (memcmp(data_buf->read_thru(), rolling_write_data_buf->read_thru(), kDefaultBufSize)) {
          printf("Comparison of RDMA read and write buffer failed \n");
          rc = -1;
        } else {
          printf("Comparison of RDMA read and write buffer successful \n");
          rc = 0;
        }
      }
    } else {
      printf("Failure in retriving data \n");
    }
  } else {
    printf("Failure in retriving status \n");
  }

  // Post the Initiator buffers back so that RDMA can reuse them.
  PostInitiatorRcvBuf1();
  IncrInitiatorRcvBufPtr();

  return rc;
}

int test_run_rdma_lif_override() {
  uint32_t seq_pdma_q = queues::get_seq_pdma_sq(4);
  uint32_t seq_roce_q = queues::get_seq_roce_sq(4);
  uint16_t ssd_handle = 2; // the SSD handle
  dp_mem_t *cmd_buf = NULL;
  dp_mem_t *data_buf = NULL;
  int rc;


  StartRoceReadSeq(seq_pdma_q, seq_roce_q, ssd_handle, &cmd_buf, &data_buf, rolling_write_slba,
                   1, queues::get_nvme_lif(), queues::get_nvme_bdf());
  printf("Started read command send over ROCE \n");

  //printf("Dumping NVME command sent \n");
  //utils::dump(cmd_buf);

  // Process the status
  dp_mem_t *rcv_buf = rdma_get_initiator_rcv_buf();
  dp_mem_t *nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));

  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  // Poll for DMA completion of read data only if status is successful
  if (rc >= 0) {
    printf("Successfully retrived status \n");
    auto func2 = [] () {
      if  (*((uint32_t *) seq_db_data->read_thru()) != kSeqDbDataMagic) {
        //printf("Sequencer magic incorrect %lx \n", *((uint64_t *) uint32_t));
        return -1;
      }
      return 0;
    };

    rc = poll(func2);

    // Now compare the contents
    if (rc >= 0) {
      printf("Successfully retrived data \n");
      if (!rolling_write_data_buf) {
        printf("No write data buffer for comparison \n");
      } else {
        // Enable this to debug as needed
        //printf("Dumping data buffer which contains NVME read data\n");
        //utils::dump(data_buf->read_thru());
        //printf("Dumping rolling NVME write data buffer\n");
        //utils::dump(rolling_write_data_buf->read_thru());
        if (memcmp(data_buf->read_thru(), rolling_write_data_buf->read_thru(), kDefaultBufSize)) {
          printf("Comparison of RDMA read and write buffer failed \n");
          rc = -1;
        } else {
          printf("Comparison of RDMA read and write buffer successful \n");
          rc = 0;
        }
      }
    } else {
      printf("Failure in retriving data \n");
    }
  } else {
    printf("Failure in retriving status \n");
  }

  // Post the Initiator buffers back so that RDMA can reuse them.
  PostInitiatorRcvBuf1();
  IncrInitiatorRcvBufPtr();

  return rc;
}

int test_run_rdma_e2e_xts_write(uint16_t seq_pdma_q,
                                uint16_t seq_xts_q,
                                uint16_t seq_roce_q,
                                uint16_t ssd_handle,
                                uint16_t io_priority,
                                XtsCtx& xts_ctx)
{
  dp_mem_t *seq_pdma_desc;
  dp_mem_t *seq_roce_desc;
  dp_mem_t *cmd_buf;
  uint16_t seq_pdma_index;
  uint16_t seq_roce_index;
  uint64_t db_data;
  uint64_t db_addr;
  uint8_t  byte_val;
  int rc;

  // Get the SLBA to write to and read from
  rolling_write_slba = get_next_slba();

  // Get the HBM buffer for the write data to be PDMA'ed to
  // before sending over RDMA
  dp_mem_t *r2n_hbm_buf = new dp_mem_t(1, rdma_r2n_buf_size(), DP_MEM_ALIGN_PAGE);

  // Initialize and form the write command
  r2n::r2n_nvme_be_cmd_buf_init(r2n_hbm_buf, NULL, 0, ssd_handle,
                                0, 0, 0, &cmd_buf);
  tests::form_write_cmd_no_buf(cmd_buf, rolling_write_slba);

  // initialize write data pattern
  byte_val = get_next_byte();
  write_buf->fill_thru(byte_val);

  // Sequencer: R2N descriptor
  seq_roce_desc = queues::seq_sq_consume_entry(seq_roce_q, &seq_roce_index);
  seq_roce_desc->clear();

  // Sequencer: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // Pre-fill the PDMA descriptor
  assert(rdma_r2n_data_size() <= kDefaultBufSize);
  seq_pdma_desc->write_bit_fields(128, 64, write_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, write_hbm_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, kDefaultBufSize);

  // encrypt data and store in r2n_hbm_buf payload area
  dp_mem_t *r2n_pyld = r2n_hbm_buf->fragment_find(rdma_r2n_data_offset() -
                                                  offsetof(r2n::r2n_buf_t, cmd_buf),
                                                  rdma_r2n_data_size());
  xts_ctx.op = xts::AES_ENCR_ONLY;
  xts_ctx.seq_xts_q = seq_xts_q;
  xts_ctx.src_buf = (void*)write_hbm_buf->va();
  xts_ctx.is_src_hbm_buf = write_hbm_buf->is_mem_type_hbm();
  xts_ctx.dst_buf = (void*)r2n_pyld->va();
  xts_ctx.is_dst_hbm_buf = r2n_pyld->is_mem_type_hbm();
  xts_ctx.num_sectors = rdma_r2n_data_size() / SECTOR_SIZE;
  xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;
  xts_ctx.init(rdma_r2n_data_size());

  // Xfer sequence is: pdma (seq_pdma_desc) -> xts encrypt -> seq roce (seq_roce_desc) ->
  // RDMA roce initiator sq (sqwqe pointing to r2n_hbm_buf).
  //
  // See StartRoceWritePdmaPrefilled() and test_seq_write_roce_pdma_prefilled().

  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_roce_q, 0,
                             seq_roce_index, &xts_ctx.xts_db_addr,
                             &xts_ctx.exp_db_data);
  printf("After XTS, db_addr is %lx db_data %lx\n",
         xts_ctx.xts_db_addr, xts_ctx.exp_db_data);
  xts_ctx.test_seq_xts();
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, xts_ctx.seq_xts_q, 0,
                             xts_ctx.seq_xts_index_get(), &db_addr, &db_data);
  queues::seq_sq_batch_consume_end(xts_ctx.seq_xts_q);

  seq_pdma_desc->write_bit_fields(0, 64, db_addr);
  seq_pdma_desc->write_bit_fields(64, 64, bswap_64(db_data));

  // After PDMA, the next doorbell would be XTS
  seq_pdma_desc->write_bit_fields(408, 1, 1);
  seq_pdma_desc->write_thru();

  StartRoceWritePdmaPrefilled(seq_pdma_q, seq_pdma_index,
                              seq_roce_q, seq_roce_index,
                              seq_roce_desc, r2n_hbm_buf);
  printf("Sequencer to PDMA + XTS + wr_buf byte_val 0x%x sent over ROCE\n",
         byte_val);

  dp_mem_t *rcv_buf = rdma_get_initiator_rcv_buf();
  dp_mem_t *nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));
  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  if (rc < 0)
    printf("Failure in retrieving status \n");
  else
    printf("Successfully retrieved status \n");

  // Save the rolling write data buffer
  rolling_write_data_buf = write_buf;

#if 0
  printf("Dumping raw data buffer\n");
  utils::dump(write_hbm_buf->read_thru(), 128);
  printf("Dumping encrypted data buffer\n");
  utils::dump(r2n_pyld->read_thru(), 128);
  printf("Dumping target_rcv_buf\n");
  utils::dump(rdma_get_target_write_data_buf()->read_thru(), 128);
#endif

  // Post the Initiator buffers back so that RDMA can reuse them.
  PostInitiatorRcvBuf1();
  IncrInitiatorRcvBufPtr();

  return rc;
}

int test_run_rdma_e2e_xts_read(uint16_t seq_pdma_q,
                               uint16_t seq_xts_q,
                               uint16_t seq_roce_q,
                               uint16_t ssd_handle,
                               uint16_t io_priority,
                               XtsCtx& xts_ctx)
{
  dp_mem_t *seq_pdma_desc;
  dp_mem_t *seq_roce_desc;
  dp_mem_t *cmd_buf;
  uint16_t seq_pdma_index;
  uint16_t seq_roce_index;
  int rc;

  reset_seq_db_data();

  // Get userspace R2N buffer for read command and data.
  dp_mem_t *r2n_send_buf = new dp_mem_t(1, rdma_r2n_buf_size(), DP_MEM_ALIGN_PAGE);

  // Initialize and form the read command
  r2n::r2n_nvme_be_cmd_buf_init(r2n_send_buf, NULL, 0, ssd_handle,
                                0, 1, 0, &cmd_buf);
  tests::form_read_cmd_no_buf(cmd_buf, rolling_write_slba);

  // Get the HBM buffer for the write back data for the read command
  dp_mem_t *r2n_write_buf = new dp_mem_t(1, rdma_r2n_buf_size(), DP_MEM_ALIGN_PAGE);

  // Xfer sequence is: RDMA roce sq (write_wqe xfers from target_rcv_buf_va
  // to r2n_write_buf) -> xts decrypt to read_hbm_buf -> PDMA to read_buf
  //
  // See also StartRoceReadPdmaPrefilled()

  // Sequencer: R2N descriptor
  seq_roce_desc = queues::seq_sq_consume_entry(seq_roce_q, &seq_roce_index);
  seq_roce_desc->clear();

  // Sequencer: PDMA descriptor
  seq_pdma_desc = queues::seq_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  seq_pdma_desc->clear();

  // decrypt data from r2n_write_buf payload and store in read_hbm_buf
  read_hbm_buf->clear_thru();
  read_buf->clear_thru();

  xts_ctx.op = xts::AES_DECR_ONLY;
  xts_ctx.seq_xts_q = seq_xts_q;
  xts_ctx.src_buf = (void*)r2n_write_buf->va();
  xts_ctx.is_src_hbm_buf = r2n_write_buf->is_mem_type_hbm();
  xts_ctx.dst_buf = (void*)read_hbm_buf->va();
  xts_ctx.is_dst_hbm_buf = read_hbm_buf->is_mem_type_hbm();
  xts_ctx.num_sectors = rdma_r2n_data_size() / SECTOR_SIZE;
  xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;
  xts_ctx.init(rdma_r2n_data_size());
  queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE, seq_pdma_q, 0,
                             seq_pdma_index, &xts_ctx.xts_db_addr,
                             &xts_ctx.exp_db_data);
  printf("After XTS, db_addr is %lx db_data %lx\n",
         xts_ctx.xts_db_addr, xts_ctx.exp_db_data);
  xts_ctx.test_seq_xts();
  queues::seq_sq_batch_consume_end(xts_ctx.seq_xts_q);

  // Pre-fill the PDMA descriptor
  seq_pdma_desc->write_bit_fields(128, 64, read_hbm_buf->pa());
  seq_pdma_desc->write_bit_fields(192, 64, read_buf->pa());
  seq_pdma_desc->write_bit_fields(256, 32, rdma_r2n_data_size());


  // Form the interrupt add/data and enable it
  seq_pdma_desc->write_bit_fields(312, 64, seq_db_data->pa());
  seq_pdma_desc->write_bit_fields(376, 32, kSeqDbDataMagic);
  seq_pdma_desc->write_bit_fields(409, 1, 1);
  seq_pdma_desc->write_thru();

  StartRoceReadWithNextLifQueue(seq_roce_q, seq_roce_index, seq_roce_desc,
                                r2n_send_buf, r2n_write_buf,
                                rdma_r2n_data_size(), queues::get_seq_lif(),
                                SQ_TYPE, xts_ctx.seq_xts_q);
  printf("Sequencer to ROCE + XTS + PDMA to local bufer sent\n");

  // Process the status
  dp_mem_t *rcv_buf = rdma_get_initiator_rcv_buf();
  dp_mem_t *nvme_status = rcv_buf->fragment_find(kR2nStatusNvmeOffset,
                                                 sizeof(struct NvmeStatus));

  // Poll for status
  auto func1 = [nvme_status, cmd_buf] () {
    return check_nvme_status(nvme_status, cmd_buf);
  };
  Poller poll;
  rc = poll(func1);

  // Poll for DMA completion of read data only if status is successful
  if (rc >= 0) {
    printf("Successfully retrieved status \n");
    auto func2 = [] () {
      if  (*((uint32_t *) seq_db_data->read_thru()) != kSeqDbDataMagic) {
        return -1;
      }
      return 0;
    };

    rc = poll(func2);

    // Now compare the contents
    if (rc >= 0) {
      printf("Successfully retrieved data \n");
      if (!rolling_write_data_buf) {
        printf("No write data buffer for comparison \n");
      } else {
        if (memcmp(read_buf->read_thru(), rolling_write_data_buf->read_thru(),
                   rdma_r2n_data_size())) {
#if 0
          printf("Dumping r2n_write_buf\n");
          utils::dump(r2n_write_buf->read_thru(), 128);
          printf("Dumping decrypted read_hbm_buf\n");
          utils::dump(read_hbm_buf->read_thru(), 128);
          printf("Dumping host data buffer which contains decrypted data\n");
          utils::dump(read_buf->read_thru(), 128);
#endif
          printf("Comparison of RDMA read and write buffer failed \n");
          rc = -1;
        } else {
          printf("Comparison of RDMA read and write buffer successful \n");
          rc = 0;
        }
      }
    } else {
      printf("Failure in retrieving data \n");
    }
  } else {
    printf("Failure in retrieving status \n");
  }

#if 0
  printf("Dumping target_rcv_buf\n");
  utils::dump(rdma_get_target_write_data_buf()->read_thru(), 128);
#endif

  // Post the Initiator buffers back so that RDMA can reuse them.
  PostInitiatorRcvBuf1();
  IncrInitiatorRcvBufPtr();

  return rc;
}

int test_run_rdma_e2e_xts_write1(void)
{
    XtsCtx xts_ctx;

    return test_run_rdma_e2e_xts_write(queues::get_seq_pdma_sq(5),
                                       queues::get_seq_xts_sq(5),
                                       queues::get_seq_roce_sq(5),
                                       2, 0,   // ssd_handle, io_priority
                                       xts_ctx);
}

int test_run_rdma_e2e_xts_read1(void)
{
    XtsCtx xts_ctx;

    return test_run_rdma_e2e_xts_read(queues::get_seq_pdma_sq(6),
                                      queues::get_seq_xts_sq(6),
                                      queues::get_seq_roce_sq(6),
                                      2, 0,   // ssd_handle, io_priority
                                      xts_ctx);
}

// Verify data and dump on any miscompare.
int
test_data_verify_and_dump(uint8_t *expected_data,
                          uint8_t *actual_data,
                          uint32_t len)
{
    int         cmp_result;
    uint32_t    offs;

    cmp_result = memcmp(expected_data, actual_data, len);
    if (cmp_result) {
        for (offs = 0; offs < len; offs++) {
            if (actual_data[offs] != expected_data[offs]) {
                break;
            }
        }
        printf("Data of length %u mismatch at offset %d\n", len, offs);
#if 0
        printf("\nDumping expected data starting at offset %u\n", offs);
        utils::dump(expected_data + offs, len - offs);
        printf("\nDumping actual data starting at offset %u\n", offs);
        utils::dump(actual_data + offs, len - offs);
#endif
        return -1;
    }

    return 0;
}

// Read 64-bit value from CSR
uint64_t
test_csr_64bit_get(uint64_t reg_addr)
{
    uint32_t lo_val, hi_val;

    lo_val = READ_REG32(reg_addr);
    hi_val = READ_REG32(reg_addr + 4);
    return ((uint64_t)hi_val << 32) | lo_val;
}

// Write 64-bit value to CSR
void
test_csr_64bit_set(uint64_t reg_addr, uint64_t val64)
{
    WRITE_REG32(reg_addr, val64 & 0xffffffff);
    WRITE_REG32(reg_addr + 4, val64 >> 32);
}

// Read 32-bit value from CSR
uint32_t
test_csr_32bit_get(uint64_t reg_addr)
{
    uint32_t lo_val;

    lo_val = READ_REG32(reg_addr);
    return lo_val;
}

// Write 32-bit value from CSR
void
test_csr_32bit_set(uint64_t reg_addr, uint32_t val32)
{
    WRITE_REG32(reg_addr, val32);
}

}  // namespace tests
