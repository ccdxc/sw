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

const static uint32_t  kDefaultBufSize       = 4096;
const static uint32_t  kDefaultNlb           = 0;
const static uint32_t  kDefaultNsid          = 1;
const static uint32_t  kR2nWqeSize           = 64;
const static uint32_t  kR2nStatusSize        = 64;
const static uint32_t  kR2nStatusNvmeOffset  = 16;

const static uint32_t  kHbmSsdBitmapSize     = (16 * 4096);
const static uint32_t  kHbmRWBufSize         = (8 * 1024);

const static uint32_t  kSeqDescSize          = 64;

const static uint32_t  kSeqDbDataSize        = 8;
const static uint64_t  kSeqDbDataMagic       = 0xAAAAAAAAAAAAAAAAULL;

const static uint32_t  kAolSize              = 64;
const static uint32_t  kXtsDescSize          = 128;

namespace xts {
extern std::vector<TestCtx> xts_tests;
}

namespace tests {

void *read_buf;
void *write_buf;
void *read_buf2;

uint64_t read_hbm_buf;
uint64_t write_hbm_buf;
uint64_t read_hbm_buf2;
uint64_t write_hbm_buf2;

void *seq_db_data;

static uint16_t global_cid = 0x1;
static uint64_t global_slba = 0x0000;
static uint64_t global_byte = 0xA0;

class Poller {
public:
  Poller() : timeout(60) { }
  Poller(int timeout) : timeout(timeout) { }
  int operator()(std::function<int(void)> poll_func) {
    std::time_t start = std::time(nullptr);
    std::time_t end;
    int rv;
    do {
      rv = poll_func();
      if(0 == rv)
        return rv;
      usleep(10000); //Sleep 10msec
      end = std::time(nullptr);
    } while(end - start < timeout);
    printf("Polling timeout %d exceeded - Giving up! \n", timeout);
    return -1;
  }
private:
  int timeout; //Default overall timeout
};

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

  // Initialize storage hbm memory
  if (utils::hbm_buf_init() < 0) {
    printf("HBM buf init failed is \n");
    return -1;
  }
  printf("HBM buf initialized\n");

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

  // Allocate the read and write buffer
  // TODO: Have a fancy allocator with various pages
  read_buf = alloc_page_aligned_host_mem(kDefaultBufSize);
  read_buf2 = alloc_page_aligned_host_mem(kDefaultBufSize);
  write_buf = alloc_page_aligned_host_mem(kDefaultBufSize);
  if (!read_buf|| !write_buf || !read_buf2) return -1;
  printf("read_buf address %p write_buf address %p\n", read_buf, write_buf);

  // Allocate the read buffer in HBM for the sequencer
  if (utils::hbm_addr_alloc(kHbmRWBufSize, &read_hbm_buf) < 0) {
    printf("Can't allocate Read HBM buffer \n");
    return -1;
  }
  // Align it to a 4K page for PRP usage
  read_hbm_buf = (read_hbm_buf + 4095) & 0xFFFFFFFFFFFFF000L;

  // Allocate the write buffer in HBM for the sequencer
  if (utils::hbm_addr_alloc(kHbmRWBufSize, &write_hbm_buf) < 0) {
    printf("Can't allocate Write HBM buffer \n");
    return -1;
  }
  // Align it to a 4K page for PRP usage
  write_hbm_buf = (write_hbm_buf + 4095) & 0xFFFFFFFFFFFFF000L;

  printf("HBM read_buf address %lx write_buf address %lx\n",
         read_hbm_buf, write_hbm_buf);

  // Allocate the read buffer2 in HBM for the sequencer
  if (utils::hbm_addr_alloc(kHbmRWBufSize, &read_hbm_buf2) < 0) {
    printf("Can't allocate Read HBM buffer \n");
    return -1;
  }
  // Align it to a 4K page for PRP usage
  read_hbm_buf2 = (read_hbm_buf2 + 4095) & 0xFFFFFFFFFFFFF000L;

  // Allocate the write buffer2 in HBM for the sequencer
  if (utils::hbm_addr_alloc(kHbmRWBufSize, &write_hbm_buf2) < 0) {
    printf("Can't allocate Write HBM buffer \n");
    return -1;
  }
  // Align it to a 4K page for PRP usage
  write_hbm_buf2 = (write_hbm_buf2 + 4095) & 0xFFFFFFFFFFFFF000L;

  printf("HBM read_buf2 address %lx write_buf2 address %lx\n",
         read_hbm_buf2, write_hbm_buf2);

  // Allocate sequencer doorbell data that will be updated by sequencer and read by PVM
  if ((seq_db_data = alloc_host_mem(kSeqDbDataSize)) == nullptr) return -1;
  memset(seq_db_data, 0, kSeqDbDataSize);

  return 0;

}


void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                        uint8_t ring, uint16_t index) {
  uint64_t db_data;
  uint64_t db_addr;

  queues::get_host_doorbell(lif, qtype, qid, ring, index, &db_addr, &db_data);

  printf("Ring Doorbell: Addr %lx Data %lx \n", db_addr, db_data);
  step_doorbell(db_addr, db_data);
}

void test_ring_nvme_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                             uint8_t ring, uint16_t index) {
  uint64_t db_data;
  uint64_t db_addr;

  queues::get_nvme_doorbell(lif, qtype, qid, ring, index, &db_addr, &db_data);

  printf("Ring Doorbell: Addr %lx Data %lx \n", db_addr, db_data);
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

int send_nvme_and_check(uint8_t *send_cmd, uint8_t *recv_cmd, uint32_t size,
                        uint16_t lif, uint8_t qtype, uint32_t qid, uint8_t ring,
                        uint16_t index) {
  int rc;

  printf("Sending data size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, size);
  printf("PRE NVME doorbell cmd comparison %d \n", rc);

  test_ring_nvme_doorbell(lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, size);
  printf("POST NVME doorbell cmd comparison %d \n", rc);

  return rc;
}

int send_and_check_ignore_cid(uint8_t *send_cmd, uint8_t *recv_cmd,
                              uint32_t size, uint16_t lif, uint8_t qtype, 
                              uint32_t qid, uint8_t ring, uint16_t index) {
  int rc;

  printf("Sending data size %u, lif %u, type %u, queue %u, ring %u, index %u"
         "\n", size, lif, qtype, qid, ring, index);

  rc = memcmp(send_cmd, recv_cmd, size);
  printf("PRE doorbell cmd comparison %d \n", rc);

  test_ring_doorbell(lif, qtype, qid, ring, index);

  struct NvmeCmd *send_nvme_cmd = (struct NvmeCmd *) send_cmd;
  struct NvmeCmd *recv_nvme_cmd = (struct NvmeCmd *) recv_cmd;
  recv_nvme_cmd->dw0.cid = send_nvme_cmd->dw0.cid;
  rc = memcmp(send_cmd, recv_cmd, size);
  printf("POST doorbell cmd comparison (ignoring cid) %d \n", rc);

  return rc;
}

int check_ignore_cid(uint8_t *send_cmd, uint8_t *recv_cmd, uint32_t size) {
  int rc;

  struct NvmeCmd *send_nvme_cmd = (struct NvmeCmd *) send_cmd;
  struct NvmeCmd *recv_nvme_cmd = (struct NvmeCmd *) recv_cmd;
  recv_nvme_cmd->dw0.cid = send_nvme_cmd->dw0.cid;
  rc = memcmp(send_cmd, recv_cmd, size);
  printf("Colletive cmd comparison (ignoring cid) %d \n", rc);

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

int consume_r2n_entry(uint16_t r2n_q, uint16_t ssd_handle, uint8_t io_priority,
                      uint8_t is_read, void **r2n_buf,  uint8_t **r2n_wqe_buf,
                      uint8_t **nvme_cmd, uint16_t *r2n_index) {

  if (!r2n_buf || !r2n_wqe_buf || !nvme_cmd || !r2n_index) {
    return -1;
  }
  *r2n_buf = r2n::r2n_buf_alloc();
  if (*r2n_buf == nullptr) {
    printf("can't alloc r2n buf\n");
    return -1;
  }

  *r2n_wqe_buf = (uint8_t *) queues::pvm_sq_consume_entry(r2n_q, r2n_index);
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
  bzero(*nvme_cmd, sizeof(struct NvmeCmd));
  return 0;
}

int form_r2n_seq_wqe(uint16_t ssd_handle, uint8_t io_priority, uint8_t is_read,
                     uint16_t r2n_q, void **r2n_buf,  uint8_t **r2n_wqe_buf, 
                     uint8_t **nvme_cmd) {

  if (!r2n_wqe_buf || !nvme_cmd) {
    return -1;
  }
  *r2n_buf = r2n::r2n_buf_alloc();
  if (*r2n_buf == nullptr) {
    printf("can't alloc r2n buf\n");
    return -1;
  }

  *r2n_wqe_buf = (uint8_t *) alloc_host_mem(kR2nWqeSize);
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
  bzero(*nvme_cmd, sizeof(struct NvmeCmd));
  return 0;
}

int consume_ssd_entry(uint16_t ssd_q, uint8_t **ssd_cmd, uint16_t *ssd_index) {

  if (!ssd_cmd || !ssd_index) {
    return -1;
  }
  *ssd_cmd = (uint8_t *) queues::pvm_sq_consume_entry(ssd_q, ssd_index);
  if (*ssd_cmd == nullptr) {
    printf("can't consume ssd command \n");
    return -1;
  }
  bzero(*ssd_cmd, sizeof(struct NvmeCmd)); 
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
  *((uint64_t *) seq_db_data) = 0;
} 

int form_read_cmd_with_buf(uint8_t *nvme_cmd, uint32_t size, uint16_t cid, 
                           uint64_t slba, uint16_t nlb)
{
  memset(read_buf, 0, kDefaultBufSize);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  // TODO: PRP list based on size
  read_cmd->prp.prp1 = host_mem_v2p(read_buf);
  read_cmd->dw0.cid = cid; 
  read_cmd->nsid = kDefaultNsid; 
  read_cmd->slba = slba;
  read_cmd->dw12.nlb = nlb;

  return 0;
}

int form_write_cmd_with_buf(uint8_t *nvme_cmd, uint32_t size, uint16_t cid, 
                            uint64_t slba, uint16_t nlb)
{
  uint8_t byte_val = get_next_byte();
  memset(write_buf, byte_val, kDefaultBufSize);
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd;
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  // TODO: PRP list based on size
  write_cmd->prp.prp1 = host_mem_v2p(write_buf);
  write_cmd->dw0.cid = cid; 
  write_cmd->nsid = kDefaultNsid; 
  write_cmd->slba = slba;
  write_cmd->dw12.nlb = nlb;

  return 0;
}

int form_read_cmd_with_hbm_buf(uint8_t *nvme_cmd, uint32_t size, uint16_t cid, 
                               uint64_t slba, uint16_t nlb)
{
  // Init the read buf as this will be the final destination despite staging
  // in HBM
  memset(read_buf, 0, kDefaultBufSize);
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  // Use the HBM buffer setup by the sequencer
  read_cmd->prp.prp1 = read_hbm_buf;
  read_cmd->dw0.cid = cid; 
  read_cmd->nsid = kDefaultNsid; 
  read_cmd->slba = slba;
  read_cmd->dw12.nlb = nlb;

  return 0;
}

int form_write_cmd_with_hbm_buf(uint8_t *nvme_cmd, uint32_t size, uint16_t cid, 
                                uint64_t slba, uint16_t nlb)
{
  // Init the write buf as this will be the source despite staging
  // in HBM
  uint8_t byte_val = get_next_byte();
  memset(write_buf, byte_val, kDefaultBufSize);
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd;
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  // Use the HBM buffer setup by the sequencer
  write_cmd->prp.prp1 = write_hbm_buf;
  write_cmd->dw0.cid = cid; 
  write_cmd->nsid = kDefaultNsid; 
  write_cmd->slba = slba;
  write_cmd->dw12.nlb = nlb;

  return 0;
}

int form_read_cmd_no_buf(uint8_t *nvme_cmd)
{
  struct NvmeCmd *read_cmd = (struct NvmeCmd *) nvme_cmd;
  read_cmd->dw0.opc = NVME_READ_CMD_OPCODE;
  read_cmd->dw0.cid = get_next_cid(); 
  read_cmd->nsid = kDefaultNsid; 
  read_cmd->slba = get_next_slba();
  read_cmd->dw12.nlb = kDefaultNlb;

  return 0;
}

int form_write_cmd_no_buf(uint8_t *nvme_cmd)
{
  struct NvmeCmd *write_cmd = (struct NvmeCmd *) nvme_cmd;
  write_cmd->dw0.opc = NVME_WRITE_CMD_OPCODE;
  write_cmd->dw0.cid = get_next_cid(); 
  write_cmd->nsid = kDefaultNsid; 
  write_cmd->slba = get_next_slba();
  write_cmd->dw12.nlb = kDefaultNlb;

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
  admin_cmd->dw0.cid = get_next_cid();
  // These values are not interpretted in DOL testing
  admin_cmd->dw10_11.qid = 1;
  admin_cmd->dw10_11.qsize = 64;

  // Send the NVME admin command and check on PVM side
  rc = send_nvme_and_check(nvme_cmd, pvm_cmd, sizeof(struct NvmeCmd), 
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
  uint8_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 1 entry to post write command
  // Consume PVM queue 1 to check write command
  uint16_t nvme_q = 1, pvm_q = 1;
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
  uint8_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 2 entry to post write command
  // Consume PVM queue 0 (by hashing) to check write command
  uint16_t nvme_q = 2, pvm_q = 0;
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
  uint8_t *nvme_cmd, *pvm_cmd;

  // Consume NVME queue 3 entry to post read command
  // Consume PVM queue 1 (by hashing) to check read command
  uint16_t nvme_q = 3, pvm_q = 1;
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

int pvm_status_trailer_update(struct PvmStatus *status, uint16_t lif, 
                              uint8_t qtype, uint32_t qid) {
  uint64_t dst_qaddr;

  utils::write_bit_fields(status->pvm_trailer, 0, 11, lif);
  utils::write_bit_fields(status->pvm_trailer, 11, 3, qtype);
  utils::write_bit_fields(status->pvm_trailer, 14, 24, qid);
  if (hal_if::get_lif_qstate_addr(lif, qtype, qid, &dst_qaddr) < 0) {
    printf("Can't get the LIF->qstate addr resolved for PVM status\n");
    return -1;
  }
  utils::write_bit_fields(status->pvm_trailer, 38, 34, dst_qaddr);
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
  admin_status->nvme_status.dw3.cid = get_next_cid();
  admin_status->nvme_status.dw3.status_phase = 0;
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
  read_status->nvme_status.dw3.cid = get_next_cid();
  read_status->nvme_status.dw3.status_phase = 0;
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
  write_status->nvme_status.dw3.cid = get_next_cid();
  write_status->nvme_status.dw3.status_phase = 0;
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
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t r2n_q = 2, ssd_q = 19;
  uint16_t ssd_handle = 0; uint8_t io_priority = 0; uint8_t is_read = 1;
  uint8_t *nvme_cmd, *ssd_cmd;

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
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t r2n_q = 2, ssd_q = 19;
  uint16_t ssd_handle = 0; uint8_t io_priority = 0; uint8_t is_read = 0;
  uint8_t *nvme_cmd, *ssd_cmd;

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
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t r2n_q = 2, ssd_q = 20;
  uint16_t ssd_handle = 1; uint8_t io_priority = 0; uint8_t is_read = 1;
  uint8_t *nvme_cmd, *ssd_cmd;

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
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t r2n_q = 2, ssd_q = 34;
  uint16_t ssd_handle = 15; uint8_t io_priority = 0; uint8_t is_read = 0;
  uint8_t *nvme_cmd, *ssd_cmd;

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
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t r2n_q = 2, ssd_q = 27;
  uint16_t ssd_handle = 8; uint8_t io_priority = 1; uint8_t is_read = 1;
  uint8_t *nvme_cmd, *ssd_cmd;

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
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index, ssd_index;
  uint16_t r2n_q = 2, ssd_q = 19;
  uint16_t ssd_handle = 0; uint8_t io_priority = 2; uint8_t is_read = 0;
  uint8_t *nvme_cmd, *ssd_cmd;

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
                     uint8_t is_read, uint8_t **nvme_cmd) {
  void *r2n_buf;
  uint8_t *r2n_wqe_buf;
  uint16_t r2n_index;
  uint16_t r2n_q = 2;

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
  uint8_t *nvme_cmd[3], *ssd_cmd;

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

  return run_nvme_be_wrr(19, 3,     // ssd_q, nvme_be_q,
                         0, 0, 1,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         1, 4, 2,   // hi_running, med_running, lo_running
                         7, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr2() {

  return run_nvme_be_wrr(19, 3,     // ssd_q, nvme_be_q,
                         0, 1, 1,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 0, 2,   // hi_running, med_running, lo_running
                         8, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr3() {

  return run_nvme_be_wrr(34, 18,    // ssd_q, nvme_be_q,
                         15, 2, 1,  // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 4, 1,   // hi_running, med_running, lo_running
                         11, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr4() {

  return run_nvme_be_wrr(22, 6,     // ssd_q, nvme_be_q,
                         3, 1, 0,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 3, 2,   // hi_running, med_running, lo_running
                         11, 63);   // num_running, max_cmds
}

int test_run_nvme_be_wrr5() {

  return run_nvme_be_wrr(27, 11,    // ssd_q, nvme_be_q,
                         8, 0, 0,   // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         2, 4, 2,   // hi_running, med_running, lo_running
                         8, 63);    // num_running, max_cmds
}

int test_run_nvme_be_wrr6() {

  return run_nvme_be_wrr(34, 18,    // ssd_q, nvme_be_q,
                         15, 2, 0,  // ssd_handle, io_priority, is_read,
                         6, 4, 2,   // hi_weight,  med_weight,  lo_weight
                         6, 4, 0,   // hi_running, med_running, lo_running
                         10, 63);   // num_running, max_cmds
}

int test_run_nvme_e2e_io(uint16_t io_priority, uint16_t is_read) {
  int rc;
  uint8_t *cmd_buf;
  uint16_t ssd_handle = 2;
  uint16_t pvm_index;
  uint8_t *status_buf;
  uint16_t pvm_q = 2;

  // Reset the SLBA for this test
  reset_slba();

  // Consume status entry from PVM CQ
  status_buf = (uint8_t *) queues::pvm_cq_consume_entry(pvm_q, &pvm_index);
  if (status_buf == nullptr) {
    printf("can't consume entries \n");
    return -1;
  }
  bzero(status_buf, kR2nStatusSize);

  // Send the R2N Command
  if (send_r2n_pri_cmd(ssd_handle, io_priority, is_read, &cmd_buf) < 0) {
    printf("cant send pri cmd %d \n", io_priority);
    return -1;
  }
  struct NvmeCmd *nvme_cmd = (struct NvmeCmd *) cmd_buf;

#if 0
  // Test code to check the doorbell (now done in P4)
  // Ring the SSD doorbell.
  queues::ring_nvme_e2e_ssd();
#endif

  // Wait for a bit as SSD backend runs in a different thread
  sleep(4);

  //printf("Recv status \n");
  //utils::dump(status_buf);
  
  // Process the status
  struct NvmeStatus *nvme_status = 
              (struct NvmeStatus *) (status_buf + kR2nStatusNvmeOffset);
  printf("nvme status: cid %x, sq head %x, status_phase %x \n", 
         nvme_status->dw3.cid, nvme_status->dw2.sq_head, 
         nvme_status->dw3.status_phase);
  if (nvme_status->dw3.cid != nvme_cmd->dw0.cid || 
      NVME_STATUS_GET_STATUS(*nvme_status)) {
    rc = -1;
    printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n", 
           nvme_status->dw3.cid, nvme_status->dw3.status_phase,
           nvme_cmd->dw0.cid);
  } else {
    rc = 0;
  }
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

  rc = memcmp(read_buf, write_buf, kDefaultBufSize);
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
                         uint8_t is_read, uint8_t **cmd_buf, uint16_t r2n_q, 
                         uint8_t **r2n_wqe_buf) {
  void *r2n_buf;

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
  uint8_t *seq_pdma_desc;
  uint16_t seq_r2n_index;
  uint8_t *seq_r2n_desc;
  uint64_t db_data;
  uint64_t db_addr;
  uint16_t r2n_q = 51;
  uint8_t *r2n_wqe_buf;
  uint8_t *cmd_buf;
  uint16_t pvm_status_q = 2;
  uint16_t pvm_status_index;
  uint8_t *status_buf;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = (uint8_t *) queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  bzero(status_buf, kR2nStatusSize);

  // Form the r2n wqe
  if (form_seq_r2n_wqe_cmd(seq_r2n_q, ssd_handle, io_priority, 0, /* is_read */
                           &cmd_buf, r2n_q, &r2n_wqe_buf) < 0) {
    printf("Can't form sequencer R2N wqe + cmd \n");
    return -1;
  }
  struct NvmeCmd *nvme_cmd = (struct NvmeCmd *) cmd_buf;

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  memset(seq_pdma_desc, 0, kSeqDescSize);

  // Sequencer #2: R2N descriptor
  seq_r2n_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_r2n_q, &seq_r2n_index);
  memset(seq_r2n_desc, 0, kSeqDescSize);

  // Fill the PDMA descriptor
  queues::get_capri_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_r2n_q, 0, 
                             seq_r2n_index, &db_addr, &db_data);
  utils::write_bit_fields(seq_pdma_desc, 0, 64, db_addr);
  utils::write_bit_fields(seq_pdma_desc, 64, 64, bswap_64(db_data));
  utils::write_bit_fields(seq_pdma_desc, 128, 64, host_mem_v2p(write_buf));
  utils::write_bit_fields(seq_pdma_desc, 192, 64, write_hbm_buf);
  utils::write_bit_fields(seq_pdma_desc, 256, 32, kDefaultBufSize);

  // Fill the R2N descriptor
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, r2n_q, &qaddr) < 0) {
    printf("Can't get R2N qaddr \n");
    return -1;
  }
  utils::write_bit_fields(seq_r2n_desc, 0, 64, host_mem_v2p(r2n_wqe_buf));
  utils::write_bit_fields(seq_r2n_desc, 64, 32, kR2nWqeSize);
  utils::write_bit_fields(seq_r2n_desc, 96, 11, queues::get_pvm_lif());
  utils::write_bit_fields(seq_r2n_desc, 107, 3, SQ_TYPE);
  utils::write_bit_fields(seq_r2n_desc, 110, 24, r2n_q);
  utils::write_bit_fields(seq_r2n_desc, 134, 34, qaddr);

  // Kickstart the sequencer 
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_pdma_q, 0, seq_pdma_index);
  
  // Wait for a bit as SSD backend runs in a different thread
  sleep(4);

  //printf("Recv status \n");
  //utils::dump(status_buf);
  
  // Process the status
  struct NvmeStatus *nvme_status = 
              (struct NvmeStatus *) (status_buf + kR2nStatusNvmeOffset);
  printf("nvme status: cid %x, sq head %x, status_phase %x \n", 
         nvme_status->dw3.cid, nvme_status->dw2.sq_head, 
         nvme_status->dw3.status_phase);
  if (nvme_status->dw3.cid != nvme_cmd->dw0.cid || 
      NVME_STATUS_GET_STATUS(*nvme_status)) {
    printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n", 
           nvme_status->dw3.cid, nvme_status->dw3.status_phase,
           nvme_cmd->dw0.cid);
    return -1;
  }
  return 0;
}

int test_seq_read_r2n(uint16_t seq_pdma_q, uint16_t ssd_handle, 
                      uint16_t io_priority) {
  uint16_t seq_pdma_index;
  uint8_t *seq_pdma_desc;
  uint16_t r2n_q = 2;
  uint16_t r2n_index;
  uint8_t *r2n_wqe_buf;
  void *r2n_buf;
  uint8_t *cmd_buf;
  uint16_t pvm_status_q = 2;
  uint16_t pvm_status_index;
  uint8_t *status_buf;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = (uint8_t *) queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  bzero(status_buf, kR2nStatusSize);

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
  struct NvmeCmd *nvme_cmd = (struct NvmeCmd *) cmd_buf;


  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  memset(seq_pdma_desc, 0, kSeqDescSize);

  // Fill the PDMA descriptor
  utils::write_bit_fields(seq_pdma_desc, 0, 64, host_mem_v2p(seq_db_data));
  utils::write_bit_fields(seq_pdma_desc, 64, 64, kSeqDbDataMagic);
  utils::write_bit_fields(seq_pdma_desc, 128, 64, read_hbm_buf);
  utils::write_bit_fields(seq_pdma_desc, 192, 64, host_mem_v2p(read_buf));
  utils::write_bit_fields(seq_pdma_desc, 256, 32, kDefaultBufSize);

  // Update the R2N WQE with the doorbell to the PDMA descriptor
  r2n::r2n_wqe_db_update(r2n_wqe_buf, queues::get_pvm_lif(), SQ_TYPE, 
                         seq_pdma_q, seq_pdma_index);

  // Kickstart the R2N module with the read command (whose completion will 
  // trigger the sequencer) 
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);
  
  // Wait for a bit as SSD backend runs in a different thread
  sleep(4);

  //printf("Recv status \n");
  //utils::dump(status_buf);
  
  // Process the status
  struct NvmeStatus *nvme_status = 
              (struct NvmeStatus *) (status_buf + kR2nStatusNvmeOffset);
  printf("nvme status: cid %x, sq head %x, status_phase %x \n", 
         nvme_status->dw3.cid, nvme_status->dw2.sq_head, 
         nvme_status->dw3.status_phase);
  if (nvme_status->dw3.cid != nvme_cmd->dw0.cid || 
      NVME_STATUS_GET_STATUS(*nvme_status)) {
    printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n", 
           nvme_status->dw3.cid, nvme_status->dw3.status_phase,
           nvme_cmd->dw0.cid);
    return -1;
  }
  if  (*((uint64_t *) seq_db_data) != kSeqDbDataMagic) {
    printf("Sequencer magic incorrect %lx \n", *((uint64_t *) seq_db_data));
    return -1;
  }
  return 0;
}

int test_seq_e2e_r2n(uint16_t seq_pdma_q, uint16_t seq_r2n_q, 
                     uint16_t ssd_handle, uint16_t io_priority) {
  int rc;

  // Send the write command and check status
  rc = test_seq_write_r2n(seq_pdma_q, seq_r2n_q, ssd_handle, 
                          io_priority);
  if (rc < 0) return -1;

  // Memcmp before reading
  rc = memcmp(read_buf, write_buf, kDefaultBufSize);
  printf("\nE2E: pre memcmp %d \n", rc);

  // Send the read command and check status
  rc = test_seq_read_r2n(seq_pdma_q, ssd_handle, io_priority);
  if (rc < 0) return -1;

  // Memcmp after reading and verify its the same
  rc = memcmp(read_buf, write_buf, kDefaultBufSize);
  printf("\nE2E: post memcmp %d \n", rc);

  return rc;
}


int test_run_seq_write1() {
  return test_seq_write_r2n(35, 43,  // seq_pdma_q, seq_r2n_q
                            2, 0);   // ssd_handle, io_priority
}

int test_run_seq_write2() {
  return test_seq_write_r2n(36, 50,  // seq_pdma_q, seq_r2n_q
                            2, 1);   // ssd_handle, io_priority
}

int test_run_seq_write3() {
  return test_seq_write_r2n(35, 49,  // seq_pdma_q, seq_r2n_q
                            2, 2);   // ssd_handle, io_priority
}

int test_run_seq_write4() {
  return test_seq_write_r2n(42, 49,  // seq_pdma_q, seq_r2n_q
                            2, 2);   // ssd_handle, io_priority
}

int test_run_seq_read1() {
  return test_seq_read_r2n(35,      // seq_pdma_q
                           2, 0);   // ssd_handle, io_priority
}

int test_run_seq_read2() {
  return test_seq_read_r2n(38,      // seq_pdma_q
                           2, 1);   // ssd_handle, io_priority
}

int test_run_seq_read3() {
  return test_seq_read_r2n(35,      // seq_pdma_q
                           2, 2);   // ssd_handle, io_priority
}

int test_run_seq_read4() {
  return test_seq_read_r2n(41,      // seq_pdma_q
                           2, 1);   // ssd_handle, io_priority
}

int test_run_seq_e2e1() {
  return test_seq_e2e_r2n(35, 43,  // seq_pdma_q, seq_r2n_q
                          2, 0);   // ssd_handle, io_priority
}

int test_run_seq_e2e2() {
  return test_seq_e2e_r2n(35, 44,  // seq_pdma_q, seq_r2n_q
                          2, 1);   // ssd_handle, io_priority
}

int test_run_seq_e2e3() {
  return test_seq_e2e_r2n(36, 44,  // seq_pdma_q, seq_r2n_q
                          2, 2);   // ssd_handle, io_priority
}

int test_run_seq_e2e4() {
  return test_seq_e2e_r2n(40, 48,  // seq_pdma_q, seq_r2n_q
                          2, 0);   // ssd_handle, io_priority
}

int verify_prot_info(char *out_buf, uint32_t num_aols, xts::xts_aol_t **out_aol,
  uint32_t sector_size, uint32_t sec_num_start, uint16_t app_tag) {
  uint32_t data_size = 0;
  for(uint32_t i = 0; i < num_aols; i++) {
    data_size += out_aol[i]->l0;
    assert((uint64_t)out_buf == (uint64_t)host_mem_p2v(out_aol[i]->a0));
    if(out_aol[i]->a1) {
      assert((uint64_t)out_buf == (uint64_t)host_mem_p2v(out_aol[i]->a1));
      data_size += out_aol[i]->l1;
    }
    if(out_aol[i]->a2) {
      assert((uint64_t)out_buf == (uint64_t)host_mem_p2v(out_aol[i]->a2));
      data_size += out_aol[i]->l2;
    }
  }
  assert(data_size >= sector_size);
  char* buf = (char *)malloc(data_size);
  uint32_t offset = 0;
  // Copy data to be a contiguous block
  for(uint32_t i = 0; i < num_aols; i++) {
    memcpy(buf+offset, out_buf + out_aol[i]->o0, out_aol[i]->l0);
    offset += out_aol[i]->l0;
    if(out_aol[i]->a1) {
      memcpy(buf+offset, out_buf + out_aol[i]->o1, out_aol[i]->l1);
      offset += out_aol[i]->l1;
    }
    if(out_aol[i]->a2) {
      memcpy(buf+offset, out_buf + out_aol[i]->o2, out_aol[i]->l2);
      offset += out_aol[i]->l2;
    }
  }
  uint32_t num_sectors = data_size / sector_size; // Works roughly as output data_size is more than just data (contains prot_info for all sectors)
  for(uint32_t i = 0; i < num_sectors; i++) {
    xts::xts_prot_info_t* prot_info = (xts::xts_prot_info_t*) &buf[(i+1)*sector_size + i*PROT_INFO_SIZE];
    if(prot_info->app_tag != htons(app_tag) ||
      prot_info->sector_num != htonl(sec_num_start + i)) {
      printf("Verify prot_info failed \n");
      free(buf);
      return -1;
    }
  }
  free(buf);
  return 0;
}

bool fill_aol(void* buf, uint64_t& a, uint32_t& o, uint32_t& l, uint32_t& offset,
    uint32_t& pending_size, uint32_t len)
{
  a = (uint64_t)buf;
  o = offset;
  l = len;
  offset += (len + INTER_SUB_AOL_GAP); //Update offset to next data position
  assert(((int)pending_size - (int)len) >= 0);
  pending_size -= len;
  if(pending_size > 0)
    return true;
  else
    return false;
}

uint32_t key128_desc_idx = 0;
uint32_t key256_desc_idx = 0;
bool key_desc_inited = false;
uint32_t exp_opaque_tag = 99; //some random number that keeps gettign incremented

class XtsCtx {
public:
  void init(bool chain = false) {
    xts_db = (uint64_t*)alloc_host_mem(sizeof(uint64_t));
    *xts_db = 0;
    if(!is_src_hbm_buf) {
      if(!chain) {
        srand(time(NULL));
        for(uint32_t i = 0; i < kDefaultBufSize/sizeof(int); i++) {
          ((int*)src_buf)[i] = rand();
        }
      }
      src_buf_phy = (void*)host_mem_v2p(src_buf);
    } else {
      src_buf_phy = src_buf;
    }
    if(!is_dst_hbm_buf) {
      memset(dst_buf, 0, kDefaultBufSize);
      dst_buf_phy = (void*)host_mem_v2p(dst_buf);
    } else {
      dst_buf_phy = dst_buf;
    }
  }

  std::string get_name(bool chain = false) {
    std::string name;
    if(!chain) {
      if(key_size == AES128_KEY_SIZE)
        name += "_K128";
      else
        name += "_K256";
      if(is_src_hbm_buf)
        name += "_hbm";
      else
        name += "_h";
      if(is_dst_hbm_buf)
        name += "_hbm";
      else
        name += "_h";
      name += "_Secs" + std::to_string(num_sectors);
      name += "_MDs" + std::to_string(num_aols);
      name += "_AoLs" + std::to_string(num_sub_aols);
    }
    switch(op) {
    case xts::AES_ENCR_ONLY:
      name += "_E";
    break;
    case xts::T10_ONLY:
      name += "_T10";
    break;
    case xts::AES_ENCR_N_T10:
      name += "_E_T10";
    break;
    case xts::AES_DECR_ONLY:
      name += "_D";
    break;
    case xts::AES_DECR_N_T10:
      name += "_D_T10";
    break;
    default:
      assert(0);
    break;
    }

    return name;
  }

  XtsCtx() {
    for(uint32_t i = 0; i < MAX_AOLS; i++) {
      in_aol[i] = NULL;
      out_aol[i] = NULL;
    }
  }
  ~XtsCtx() {
    if(op == xts::INVALID) {
      //if(xts_db) free_host_mem(xts_db);
      return;
    }
    if(xts_desc_addr) free_host_mem(xts_desc_addr);
    if(status) free_host_mem(status);
    // TODO: This seems to be crashing - needs investigation
    // if(xts_db) free_host_mem(xts_db);
    if(iv) free_host_mem(iv);
    for(uint32_t i = 0; i < num_aols; i++) {
      if(in_aol[i]) free_host_mem(in_aol[i]);
      if(out_aol[i]) free_host_mem(out_aol[i]);
    }
  }
  int test_seq_xts();
  int ring_db_n_verify();

  void* src_buf = write_buf;
  bool is_src_hbm_buf = false;
  void* dst_buf = read_buf;
  bool is_dst_hbm_buf = false;
  void* src_buf_phy = NULL;
  void* dst_buf_phy = NULL;

  uint16_t seq_xts_q = XTS_SEQ_Q;
  uint32_t num_sectors = 1;
  xts::Op op;
  uint32_t key_size = AES128_KEY_SIZE;
  uint32_t sector_size = SECTOR_SIZE;
  uint32_t num_aols = 1;
  uint32_t num_sub_aols = 1;
  uint16_t app_tag = 0xbeef;
  uint32_t start_sec_num = 5;
  xts::xts_aol_t* in_aol[MAX_AOLS];
  xts::xts_aol_t* out_aol[MAX_AOLS];
  bool ring_seq_db = true;
  uint16_t seq_xts_index = 0;
  xts::xts_desc_t* xts_desc_addr = NULL;
  unsigned char* iv = NULL;

  // ctx data needed for verification of op completion
  uint64_t xts_db_addr = 0;
  uint64_t* xts_db = NULL;
  uint64_t exp_db_data = 0xdeadbeefdeadbeef;
  uint64_t* status = NULL;
  bool t10_en = false;
  bool decr_en = false;
};

int XtsCtx::test_seq_xts() {
  uint8_t *seq_xts_desc;

  // Sequencer #1: XTS descriptor
  seq_xts_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_xts_q, &seq_xts_index);
  memset(seq_xts_desc, 0, kSeqDescSize);

  unsigned char iv_src[IV_SIZE] = {0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
  iv = (unsigned char*)alloc_host_mem(IV_SIZE);
  memcpy(iv, iv_src, IV_SIZE);
  xts::xts_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.token3 = 0x0;     // xts
  cmd.token4 = 0x4;     // xts

  switch(op) {
  case(xts::AES_ENCR_ONLY):
    cmd.enable_crc = 0x0; // Disable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x0; // Encrypt
  break;
  case(xts::AES_DECR_ONLY):
    cmd.enable_crc = 0x0; // Disable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x1; // Decrypt
    decr_en = true;
  break;
  case(xts::T10_ONLY):
    cmd.enable_crc = 0x1; // Enable CRC
    cmd.bypass_aes = 0x1; // Bypass AES
    t10_en = true;
  break;
  case(xts::AES_ENCR_N_T10):
    cmd.enable_crc = 0x1; // Enable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x0; // Encrypt
    t10_en = true;
  break;
  case(xts::AES_DECR_N_T10):
     cmd.enable_crc = 0x1; // Enable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x1; // Decrypt
    t10_en = true;
    decr_en = true;
  break;
  default:
    printf(" Unknown xts operation \n");
    assert(0);
    return -1;
  break;
  }

  assert(num_aols <= MAX_AOLS); // Currently we only support upto 2 - min required to validate aol chaining
  assert(num_sub_aols <= MAX_SUB_AOLS); // Currently we only support upto 4 - min required to validate aol chaining
  assert(sizeof(xts::xts_desc_t) == kXtsDescSize);
  xts::xts_desc_t* xts_desc_addr = (xts::xts_desc_t*)alloc_host_mem(sizeof(xts::xts_desc_t));

  assert(sizeof(xts::xts_aol_t) == kAolSize);
  memset(in_aol, 0x0, sizeof(in_aol));
  memset(out_aol, 0x0, sizeof(out_aol));
  for(uint32_t i = 0; i < num_aols; i++) {
    // AOLs need to be aligned at 512-bit boundary. For now page aligning them
    in_aol[i] = (xts::xts_aol_t*)alloc_page_aligned_host_mem(sizeof(xts::xts_aol_t));
    memset(in_aol[i], 0x0, sizeof(xts::xts_aol_t));
    out_aol[i] = (xts::xts_aol_t*)alloc_page_aligned_host_mem(sizeof(xts::xts_aol_t));
    memset(out_aol[i], 0x0, sizeof(xts::xts_aol_t));
  }
  uint32_t data_size;
  if(op == xts::AES_DECR_N_T10) {
    data_size = (num_sectors * sector_size) + (num_sectors * PROT_INFO_SIZE);
  } else {
    data_size = num_sectors * sector_size;
  }
  assert(data_size % num_sub_aols == 0); // For simplicity
  uint32_t sub_aol_data_size = data_size / num_sub_aols;
  uint32_t sub_aol_data_offset = 0;
  uint32_t out_sub_aol_data_size =
    t10_en ? (xts::get_output_size(data_size, sector_size) + num_sub_aols - 1) / num_sub_aols : sub_aol_data_size;
  uint32_t out_sub_aol_data_offset = 0;
  uint32_t in_pending_size = data_size;
  uint32_t out_pending_size = t10_en ? xts::get_output_size(data_size, sector_size) : data_size;
  bool in_cont = true, out_cont = true;

  for(uint32_t i = 0; i < num_aols; i++) {
    // Fill the XTS Msg descriptor
    if(in_cont)
      in_cont = fill_aol(src_buf_phy, in_aol[i]->a0, in_aol[i]->o0, in_aol[i]->l0, sub_aol_data_offset,
        in_pending_size, sub_aol_data_size);
    if(in_cont)
      in_cont = fill_aol(src_buf_phy, in_aol[i]->a1, in_aol[i]->o1, in_aol[i]->l1, sub_aol_data_offset,
        in_pending_size, sub_aol_data_size);
    if(in_cont)
      in_cont = fill_aol(src_buf_phy, in_aol[i]->a2, in_aol[i]->o2, in_aol[i]->l2, sub_aol_data_offset,
        in_pending_size, sub_aol_data_size);
    if(out_cont)
      out_cont = fill_aol(dst_buf_phy, out_aol[i]->a0, out_aol[i]->o0, out_aol[i]->l0, out_sub_aol_data_offset,
        out_pending_size, out_sub_aol_data_size);
    if(out_cont)
      out_cont = fill_aol(dst_buf_phy, out_aol[i]->a1, out_aol[i]->o1, out_aol[i]->l1, out_sub_aol_data_offset,
        out_pending_size, out_sub_aol_data_size);
    if(out_cont)
      out_cont = fill_aol(dst_buf_phy, out_aol[i]->a2, out_aol[i]->o2, out_aol[i]->l2, out_sub_aol_data_offset,
        out_pending_size, out_sub_aol_data_size);

    if(i + 1 < num_aols) {
      in_aol[i]->next = (uint64_t) host_mem_v2p(in_aol[i+1]);
       out_aol[i]->next = (uint64_t) host_mem_v2p(out_aol[i+1]);
    }
  }

  status = (uint64_t*)alloc_host_mem(sizeof(uint64_t));
  *status = STATUS_DEF_VALUE;
  if(!xts_db_addr)
    xts_db_addr = host_mem_v2p(xts_db);

  // Fill the XTS ring descriptor
  xts_desc_addr->in_aol = host_mem_v2p(in_aol[0]);
  xts_desc_addr->out_aol = host_mem_v2p(out_aol[0]);
  xts_desc_addr->iv_addr = host_mem_v2p(iv);
  xts_desc_addr->db_addr = xts_db_addr;
  xts_desc_addr->db_data = exp_db_data;
  xts_desc_addr->opaque_tag_en = 1;
  xts_desc_addr->opaque_tag = exp_opaque_tag;
  xts_desc_addr->sector_num = start_sec_num;
  xts_desc_addr->sector_size = sector_size;
  xts_desc_addr->app_tag = app_tag;
  xts_desc_addr->cmd = cmd;
  xts_desc_addr->status = host_mem_v2p(status);

  // Ideally below key initialization lines need to be commented out if operation is T10 only
  // but barco is not fixing this bug - https://github.com/pensando/asic/issues/669
  if(!key_desc_inited) {
    unsigned char key[64] = {0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
      0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
      0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
      0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
    if(hal_if::get_key_index((char*)key, types::CRYPTO_KEY_TYPE_AES128, AES256_KEY_SIZE*2, &key128_desc_idx)) {
      printf("can't create or update xts 128bit key index \n");
      return -1;
    }
    if(hal_if::get_key_index((char*)key, types::CRYPTO_KEY_TYPE_AES256, AES256_KEY_SIZE*2, &key256_desc_idx)) {
      printf("can't create or update xts 256 key index \n");
      return -1;
    }
    key_desc_inited = true;
  }
  if(key_size == AES128_KEY_SIZE)
    xts_desc_addr->key_desc_idx = key128_desc_idx;
  else
    xts_desc_addr->key_desc_idx = key256_desc_idx;

  // Fill the XTS Seq descriptor
  utils::write_bit_fields(seq_xts_desc, 0, 64, host_mem_v2p(xts_desc_addr));
  utils::write_bit_fields(seq_xts_desc, 64, 32, 7);  //2^7 which will be 128 - xts desc size
  utils::write_bit_fields(seq_xts_desc, 96, 16, 2);  //2^2 which will be 4 - prod index size

  // Fill xts producer index addr
  if(decr_en)
    utils::write_bit_fields(seq_xts_desc, 112, 34, CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX);
  else
    utils::write_bit_fields(seq_xts_desc, 112, 34, CAPRI_BARCO_MD_HENS_REG_XTS0_PRODUCER_IDX);

  uint64_t xts_ring_base_addr;

  if(hal_if::get_xts_ring_base_address(decr_en, &xts_ring_base_addr) < 0) {
    printf("can't get xts ring base address \n");
    return -1;
  }
  // Fill xts ring base addr
  utils::write_bit_fields(seq_xts_desc, 146, 34, xts_ring_base_addr);

  if(ring_seq_db) {
    return ring_db_n_verify();
  }

  return 0;
}

int XtsCtx::ring_db_n_verify() {
  // Kickstart the sequencer
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_xts_q, 0, seq_xts_index);

  // Poll for doorbell data as XTS which runs in a different thread
  auto func = [this] () {
    if(*xts_db != exp_db_data) {
      printf("Doorbell data not yet there - try again \n");
      return -1;
    }
    return 0;
  };

  Poller poll;
  int rv = poll(func);

  if(0 == rv && t10_en && !decr_en && !is_dst_hbm_buf)
    rv = verify_prot_info((char *)dst_buf, num_aols, out_aol, sector_size, start_sec_num, app_tag);

  if(0 == rv) {
    if(STATUS_DEF_VALUE != *status) {
      printf(" status check failed - status value %lu", *status);
      rv = -1;
    }
  }

  uint64_t opaque_tag_addr = 0;
  if(hal_if::get_xts_opaque_tag_addr(decr_en, &opaque_tag_addr)) {
    printf("get_xts_opaque_tag_addr failed \n");
    return -1;
  }

  uint32_t opaque_tag = 0;
  if(!read_mem(opaque_tag_addr, (uint8_t*)&opaque_tag, sizeof(opaque_tag))) {
    printf("Reading opaque tag hbm mem failed \n");
    return -1;
  }

  if(exp_opaque_tag != opaque_tag) {
    printf("Opaque tag expected value %u rcvd %u", exp_opaque_tag, opaque_tag);
    return -1;
  }
  exp_opaque_tag++;

  return rv;
}

class XtsChainCtx {
public:
  int operator()(void) {
    int rv = xts_ctx1.test_seq_xts();
    if(0 == rv && num_ops == 2) {
      printf(" Executing second part of chain\n");
      rv = xts_ctx2.test_seq_xts();
      if(0 == rv) {
        for(uint32_t i = 0; i < xts_ctx1.num_aols; i++) {
          for(uint32_t j = 0; j < xts_ctx1.num_sub_aols; j++) {
            uint32_t off, len;
            if(0 == j) {
              off = xts_ctx1.in_aol[i]->o0;
              len = xts_ctx1.in_aol[i]->l0;
            }
            else if(1 == j) {
              off = xts_ctx1.in_aol[i]->o1;
              len = xts_ctx1.in_aol[i]->l1;
            }
            else if(2 == j) {
              off = xts_ctx1.in_aol[i]->o2;
              len = xts_ctx1.in_aol[i]->l2;
            }
            rv = memcmp((char*)xts_ctx1.src_buf+off, (char*)xts_ctx2.dst_buf+off, len);
            if(0 != rv)
              break;
          }
        }
        if(0 != rv) {
          rv = -1;
          printf(" Memcmp failed \n");
        }
      }
    }
    return rv;
  }
  std::string get_name() {
    std::string name = "XTS";
    name += xts_ctx1.get_name();
    if(num_ops == 2)
      name += xts_ctx2.get_name(true);
    return name;
  }
  XtsCtx xts_ctx1, xts_ctx2;
  int num_ops = 1;
};

int add_xts_tests(std::vector<TestEntry>& test_suite) {
  for(auto const &ent : xts::xts_tests) {
    XtsChainCtx *ctx = new XtsChainCtx;
    ctx->xts_ctx1.op = ent.op1;
    ctx->xts_ctx2.op = ent.op2;
    ctx->xts_ctx1.key_size = ent.key_size;
    ctx->xts_ctx2.key_size = ent.key_size;
    ctx->xts_ctx1.num_sectors = ent.num_sectors;
    ctx->xts_ctx1.num_aols = ent.num_mds;
    ctx->xts_ctx1.num_sub_aols = ent.num_aols;
    ctx->xts_ctx2.num_sectors = ent.num_sectors;
    ctx->xts_ctx2.num_aols = ent.num_mds;
    ctx->xts_ctx2.num_sub_aols = ent.num_aols;
    if(ent.stage_in_hbm == true) {
      ctx->xts_ctx1.dst_buf = (void*)write_hbm_buf;
      ctx->xts_ctx1.is_dst_hbm_buf = true;
      ctx->xts_ctx2.src_buf = (void*)write_hbm_buf;
      ctx->xts_ctx2.is_src_hbm_buf = true;
    } else {
      ctx->xts_ctx2.src_buf = (void*)read_buf;
      ctx->xts_ctx2.is_src_hbm_buf = false;
      ctx->xts_ctx2.dst_buf = (void*)read_buf2;
      ctx->xts_ctx2.is_dst_hbm_buf = false;
    }
    if(xts::INVALID != ent.op2)
      ctx->num_ops = 2;
    ctx->xts_ctx1.init();
    ctx->xts_ctx2.init(true);
    test_suite.push_back({*ctx, ctx->get_name(), false});
    delete ctx;
  }

  return 0;
}

int check_nvme_status(struct NvmeStatus *nvme_status, struct NvmeCmd *nvme_cmd) {
  // Process the status

  if (nvme_status->dw3.cid != nvme_cmd->dw0.cid ||
      NVME_STATUS_GET_STATUS(*nvme_status)) {
    /*printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n",
           nvme_status->dw3.cid, nvme_status->dw3.status_phase,
           nvme_cmd->dw0.cid);*/
    return -1;
  }
  printf("nvme status: cid %x, sq head %x, status_phase %x \n",
         nvme_status->dw3.cid, nvme_status->dw2.sq_head,
         nvme_status->dw3.status_phase);
  return 0;
}

int test_seq_write_xts_r2n(uint16_t seq_pdma_q, uint16_t seq_r2n_q,
                           uint16_t ssd_handle, uint16_t io_priority, XtsCtx& xts_ctx) {
  uint16_t seq_pdma_index;
  uint8_t *seq_pdma_desc;
  uint16_t seq_r2n_index;
  uint8_t *seq_r2n_desc;
  uint64_t db_data;
  uint64_t db_addr;
  uint16_t r2n_q = 51;
  uint8_t *r2n_wqe_buf;
  uint8_t *cmd_buf;
  uint16_t pvm_status_q = 2;
  uint16_t pvm_status_index;
  uint8_t *status_buf;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = (uint8_t *) queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  bzero(status_buf, kR2nStatusSize);

  // Form the r2n wqe
  if (form_seq_r2n_wqe_cmd(seq_r2n_q, ssd_handle, io_priority, 0, /* is_read */
                           &cmd_buf, r2n_q, &r2n_wqe_buf) < 0) {
    printf("Can't form sequencer R2N wqe + cmd \n");
    return -1;
  }
  struct NvmeCmd *nvme_cmd = (struct NvmeCmd *) cmd_buf;

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  memset(seq_pdma_desc, 0, kSeqDescSize);

  // Sequencer #3: R2N descriptor
  seq_r2n_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_r2n_q, &seq_r2n_index);
  memset(seq_r2n_desc, 0, kSeqDescSize);

  // Fill the PDMA descriptor
  utils::write_bit_fields(seq_pdma_desc, 128, 64, host_mem_v2p(write_buf));
  utils::write_bit_fields(seq_pdma_desc, 192, 64, write_hbm_buf2);
  utils::write_bit_fields(seq_pdma_desc, 256, 32, kDefaultBufSize);

  xts_ctx.op = xts::AES_ENCR_ONLY;
  xts_ctx.src_buf = (void*)write_hbm_buf2;
  xts_ctx.is_src_hbm_buf = true;
  xts_ctx.dst_buf = (void*)write_hbm_buf;
  xts_ctx.is_dst_hbm_buf = true;
  xts_ctx.ring_seq_db = false;
  xts_ctx.init();
  queues::get_capri_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_r2n_q, 0,
                             seq_r2n_index, &xts_ctx.xts_db_addr, &xts_ctx.exp_db_data);
  //xts_ctx.exp_db_data = bswap_64(xts_ctx.exp_db_data);
  printf("r2n_db_addr %lx r2n_db_data %lu\n", xts_ctx.xts_db_addr, xts_ctx.exp_db_data);
  xts_ctx.test_seq_xts();
  queues::get_capri_doorbell(queues::get_pvm_lif(), SQ_TYPE, xts_ctx.seq_xts_q, 0,
                             xts_ctx.seq_xts_index, &db_addr, &db_data);
  utils::write_bit_fields(seq_pdma_desc, 0, 64, db_addr);
  utils::write_bit_fields(seq_pdma_desc, 64, 64, bswap_64(db_data));

  // Fill the R2N descriptor
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, r2n_q, &qaddr) < 0) {
    printf("Can't get R2N qaddr \n");
    return -1;
  }
  utils::write_bit_fields(seq_r2n_desc, 0, 64, host_mem_v2p(r2n_wqe_buf));
  utils::write_bit_fields(seq_r2n_desc, 64, 32, kR2nWqeSize);
  utils::write_bit_fields(seq_r2n_desc, 96, 11, queues::get_pvm_lif());
  utils::write_bit_fields(seq_r2n_desc, 107, 3, SQ_TYPE);
  utils::write_bit_fields(seq_r2n_desc, 110, 24, r2n_q);
  utils::write_bit_fields(seq_r2n_desc, 134, 34, qaddr);

  // Kickstart the sequencer
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_pdma_q, 0, seq_pdma_index);

  // Process the status
  struct NvmeStatus *nvme_status =
              (struct NvmeStatus *) (status_buf + kR2nStatusNvmeOffset);
  auto func = [nvme_status, nvme_cmd] () {
    return check_nvme_status(nvme_status, nvme_cmd);
  };
  Poller poll;
  int rv = poll(func);
  return rv;
}

int test_seq_read_xts_r2n(uint16_t seq_pdma_q, uint16_t ssd_handle,
                      uint16_t io_priority, XtsCtx& xts_ctx) {
  uint16_t seq_pdma_index;
  uint8_t *seq_pdma_desc;
  uint16_t r2n_q = 2;
  uint16_t r2n_index;
  uint8_t *r2n_wqe_buf;
  void *r2n_buf;
  uint8_t *cmd_buf;
  uint16_t pvm_status_q = 2;
  uint16_t pvm_status_index;
  uint8_t *status_buf;

  // Reset the SLBA and sequencer doorbell data for this test
  reset_slba();
  reset_seq_db_data();

  // Consume status entry from PVM CQ
  status_buf = (uint8_t *) queues::pvm_cq_consume_entry(pvm_status_q, &pvm_status_index);
  if (status_buf == nullptr) {
    printf("can't consume status entries \n");
    return -1;
  }
  bzero(status_buf, kR2nStatusSize);

  // Consume pdma entry
  seq_pdma_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  memset(seq_pdma_desc, 0, kSeqDescSize);

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
  struct NvmeCmd *nvme_cmd = (struct NvmeCmd *) cmd_buf;

  // Sequencer #1: XTS
  xts_ctx.op = xts::AES_DECR_ONLY;
  xts_ctx.src_buf = (void*)read_hbm_buf;
  xts_ctx.is_src_hbm_buf = true;
  xts_ctx.dst_buf = (void*)read_hbm_buf2;
  xts_ctx.is_dst_hbm_buf = true;
  xts_ctx.ring_seq_db = false;
  xts_ctx.init();
  queues::get_capri_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_pdma_q, 0,
      seq_pdma_index, &xts_ctx.xts_db_addr, &xts_ctx.exp_db_data);
  //xts_ctx.exp_db_data = bswap_64(xts_ctx.exp_db_data);
  xts_ctx.test_seq_xts();

  // Sequencer #2: PDMA descriptor

  // Fill the PDMA descriptor
  utils::write_bit_fields(seq_pdma_desc, 0, 64, host_mem_v2p(seq_db_data));
  utils::write_bit_fields(seq_pdma_desc, 64, 64, kSeqDbDataMagic);
  utils::write_bit_fields(seq_pdma_desc, 128, 64, read_hbm_buf2);
  utils::write_bit_fields(seq_pdma_desc, 192, 64, host_mem_v2p(read_buf));
  utils::write_bit_fields(seq_pdma_desc, 256, 32, kDefaultBufSize);

  // Update the R2N WQE with the doorbell to the PDMA descriptor
  r2n::r2n_wqe_db_update(r2n_wqe_buf, queues::get_pvm_lif(), SQ_TYPE,
                         xts_ctx.seq_xts_q, xts_ctx.seq_xts_index);

  // Kickstart the R2N module with the read command (whose completion will
  // trigger the sequencer)
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, r2n_q, 0, r2n_index);

  // Process the status
  struct NvmeStatus *nvme_status =
              (struct NvmeStatus *) (status_buf + kR2nStatusNvmeOffset);

  auto func1 = [nvme_status, nvme_cmd] () {
    return check_nvme_status(nvme_status, nvme_cmd);
  };
  Poller poll;
  int rv = poll(func1);

  auto func2 = [] () {
    if  (*((uint64_t *) seq_db_data) != kSeqDbDataMagic) {
      //printf("Sequencer magic incorrect %lx \n", *((uint64_t *) seq_db_data));
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

  uint8_t byte_val = get_next_byte() + 1;
  memset(write_buf, byte_val, kDefaultBufSize);
  memset(read_buf, byte_val, kDefaultBufSize);
  write_mem(write_hbm_buf, (uint8_t *)read_buf, kDefaultBufSize);
  write_mem(write_hbm_buf2, (uint8_t *)read_buf, kDefaultBufSize);
  write_mem(read_hbm_buf, (uint8_t *)read_buf, kDefaultBufSize);
  write_mem(read_hbm_buf2, (uint8_t *)read_buf, kDefaultBufSize);

  memset(read_buf, 0x0, SECTOR_SIZE);

  // Send the write command and check status
  rc = test_seq_write_xts_r2n(seq_pdma_q, seq_r2n_q, ssd_handle,
                          io_priority, xts_ctx_write);
  if (rc < 0) return -1;

  // Memcmp before reading
  rc = memcmp(read_buf, write_buf, kDefaultBufSize);
  printf("\nE2E: pre memcmp %d \n", rc);

  // Send the read command and check status
  rc = test_seq_read_xts_r2n(seq_pdma_q, ssd_handle, io_priority, xts_ctx_read);
  if (rc < 0) return -1;

  // Memcmp after reading and verify its the same
  rc = memcmp(read_buf, write_buf, kDefaultBufSize);
  printf("\nE2E: post memcmp %d \n", rc);

  return rc;
}

int test_seq_e2e_xts_r2n1() {
  return test_seq_e2e_xts_r2n(40, 48,  // seq_pdma_q, seq_r2n_q
      2, 0);   // ssd_handle, io_priority
}

int test_seq_write_roce(uint32_t seq_pdma_q, uint32_t seq_roce_q, 
      uint32_t pvm_roce_sq, uint64_t pdma_src_addr, 
      uint64_t pdma_dst_addr, uint32_t pdma_data_size,
      uint64_t roce_wqe_addr, uint32_t roce_wqe_size) {

  uint16_t seq_pdma_index;
  uint8_t *seq_pdma_desc;
  uint16_t seq_roce_index;
  uint8_t *seq_roce_desc;
  uint64_t db_data;
  uint64_t db_addr;

  printf("pdma_q %u, roce_q %u, roce_sq %u, pdma_src_addr %lx, "
         "pdma_dst_addr %lx, pdma_data_size %u, "
         "roce_wqe_addr %lx, roce_wqe_size %u \n",
         seq_pdma_q, seq_roce_q, pvm_roce_sq, pdma_src_addr, 
         pdma_dst_addr, pdma_data_size, roce_wqe_addr, roce_wqe_size);

  // Sequencer #1: PDMA descriptor
  seq_pdma_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_pdma_q, &seq_pdma_index);
  memset(seq_pdma_desc, 0, kSeqDescSize);

  // Sequencer #2: R2N descriptor
  seq_roce_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_roce_q, &seq_roce_index);
  memset(seq_roce_desc, 0, kSeqDescSize);

  // Fill the PDMA descriptor
  queues::get_capri_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_roce_q, 0, 
           seq_roce_index, &db_addr, &db_data);
  utils::write_bit_fields(seq_pdma_desc, 0, 64, db_addr);
  utils::write_bit_fields(seq_pdma_desc, 64, 64, bswap_64(db_data));
  utils::write_bit_fields(seq_pdma_desc, 128, 64, pdma_src_addr);
  utils::write_bit_fields(seq_pdma_desc, 192, 64, pdma_dst_addr);
  utils::write_bit_fields(seq_pdma_desc, 256, 32, pdma_data_size);

  // Fill the Sequencer ROCE descriptor
  uint64_t qaddr;
  if (qstate_if::get_qstate_addr(queues::get_pvm_lif(), SQ_TYPE, pvm_roce_sq, &qaddr) < 0) {
    printf("Can't get PVM's ROCE SQ qaddr \n");
    return -1;
  }
  utils::write_bit_fields(seq_roce_desc, 0, 64, roce_wqe_addr);
  utils::write_bit_fields(seq_roce_desc, 64, 32, roce_wqe_size);
  utils::write_bit_fields(seq_roce_desc, 96, 11, queues::get_pvm_lif());
  utils::write_bit_fields(seq_roce_desc, 107, 3, SQ_TYPE);
  utils::write_bit_fields(seq_roce_desc, 110, 24, pvm_roce_sq);
  utils::write_bit_fields(seq_roce_desc, 134, 34, qaddr);
  utils::write_bit_fields(seq_roce_desc, 168, 8, 1);

  // Kickstart the sequencer 
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_pdma_q, 0, seq_pdma_index);
  
  return 0;
}

int test_run_rdma_e2e_write() {
  uint16_t ssd_handle = 2; // the SSD handle
  uint8_t *cmd_buf = NULL;
  int rc;


  StartRoceSeq(ssd_handle, get_next_byte(), &cmd_buf);
  printf("Started sequencer to PDMA + send over ROCE \n");

  struct NvmeCmd *nvme_cmd = (struct NvmeCmd *) cmd_buf;
  //printf("Dumping NVME command sent \n");
  //utils::dump(cmd_buf);

  sleep(6);

  uint8_t *rcv_buf = (uint8_t *) rdma_get_initiator_rcv_buf();
  //printf("Dumping initator rcv buf which contains NVME status \n");
  //utils::dump(rcv_buf);

  // Process the status
  struct NvmeStatus *nvme_status = 
              (struct NvmeStatus *) (rcv_buf + kR2nStatusNvmeOffset);
  printf("nvme status: cid %x, sq head %x, status_phase %x \n", 
         nvme_status->dw3.cid, nvme_status->dw2.sq_head, 
         nvme_status->dw3.status_phase);
  if (nvme_status->dw3.cid != nvme_cmd->dw0.cid || 
      NVME_STATUS_GET_STATUS(*nvme_status)) {
    rc = -1;
    printf("nvme status: cid %x, status_phase %x nvme_cmd: cid %x\n", 
           nvme_status->dw3.cid, nvme_status->dw3.status_phase,
           nvme_cmd->dw0.cid);
  } else {
    rc = 0;
  }

  PostTargetRcvBuf1();
  PostInitiatorRcvBuf1();

  return rc;
}
}  // namespace tests
