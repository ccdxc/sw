#ifndef _TESTS_HPP_
#define _TESTS_HPP_

#include <stdint.h>
#include <functional>
#include <vector>
#include "gflags/gflags.h"
#include "dp_mem.hpp"

DECLARE_uint64(poll_interval);
DECLARE_uint64(long_poll_interval);
DECLARE_uint64(num_pdma_queues);

extern bool run_pdma_tests;

using namespace dp_mem;

namespace tests {

const static uint32_t  kDefaultBufSize       = 4096;
const static uint32_t  kSeqDescSize          = 64;

typedef struct cp_seq_entry {
  uint64_t next_doorbell_addr;	// Next capri doorbell address (if chaining)
  uint64_t next_doorbell_data;	// Next capri doorbell data (if chaining)
  uint64_t status_hbm_pa;	// Status address in HBM. Provide this even if data_len is provided in desc.
  uint64_t src_hbm_pa;		// Address of compression buffer in HBM (source of PDMA)
  union {
      uint64_t sgl_pa;		// Address of the SGL in host (destination of PDMA)
      uint64_t aol_pa;      // Address of chain input AOL entry
  };
  uint64_t intr_pa;		// MSI-X Interrupt address
  uint32_t intr_data;		// MSI-X Interrupt data
  uint16_t status_len;		// Length of the status header
  uint16_t data_len;		// Remaining data length of compression buffer
  uint8_t  pad_len_shift;   // Padding length (power of 2)
  // TODO: These bitfields are interpretted in big endian 
  //       fashion by P4+. For DOL it won't matter as we set bitfields.
  //       For driver, need to define the order properly.
  uint8_t  use_data_len         :1,	// use desc data_len rather than output_data_len
           status_dma_en        :1,	// enable DMA of status to status_hbm_pa
  // NOTE: Don't enable intr_en and next_doorbell_en together
  //       as only one will be serviced
  // Order of evaluation: 1. next_doorbell_en 2. intr_en
           next_doorbell_en     :1,	// enable chain doorbell
           intr_en              :1,	// enable intr_data write to intr_pa
           exit_chain_on_error  :1, // skip chain doorbell on error
  // NOTE: sgl_xfer_en and aol_len_pad_en are mutually exclusive.
  // Order of evaluation: 1. aol_len_pad_en 2. sgl_xfer_en
           aol_len_pad_en       :1, // enable length padding in AOL
           sgl_xfer_en          :1; // enable data transfer from src_hbm_pa to sgl_pa
} cp_seq_entry_t;

typedef struct cq_sq_ent_sgl {
  uint64_t status_host_pa;	// Status address in host. Destination for the PDMA of status.
  uint64_t addr[4];		// Destination Address in the SGL for compression data PDMA
  uint16_t len[4];		// Length of the SGL element for compression data PDMA
  uint64_t pad[2];
} cp_sq_ent_sgl_t;

typedef struct cp_seq_params {
  cp_seq_entry_t seq_ent;	    // Compression sequencer descriptor
  uint32_t seq_comp_status_q;	// Compression status sequencer queue
  uint32_t seq_xts_q;   	    // XTS sequencer queue
  uint32_t seq_xts_status_q;	// XTS status sequencer queue
  uint64_t ret_doorbell_addr;	// Doorbell address that is formed for the compression sequencer (filled by API)
  uint64_t ret_doorbell_data;	// Doorbell data that is formed for the compression sequencer (filled by API)
} cp_seq_params_t;

class Poller {
public:
  Poller() : timeout(FLAGS_poll_interval) { }
  Poller(int timeout, bool fast_poll=true) : timeout(timeout), fast_poll(fast_poll) { }
  int operator()(std::function<int(void)> poll_func);
private:
  int timeout; //Default overall timeout
  bool fast_poll = true;
};

// API return values: 0 => successs; < 0 => failure
int test_setup_cp_seq_status_ent(cp_seq_params_t *params);

int test_setup();

int check_ignore_cid(dp_mem_t *send_cmd, dp_mem_t *recv_cmd, uint32_t size);

int form_read_cmd_no_buf(dp_mem_t *nvme_cmd, uint64_t slba);

int form_write_cmd_no_buf(dp_mem_t *nvme_cmd, uint64_t slba);

int test_run_nvme_pvm_admin_cmd();

int test_run_nvme_pvm_read_cmd();

int test_run_nvme_pvm_write_cmd();

int test_run_nvme_pvm_hashing1();

int test_run_nvme_pvm_hashing2();

int test_run_pvm_nvme_admin_status();

int test_run_pvm_nvme_read_status();

int test_run_pvm_nvme_write_status();

int test_run_r2n_read_cmd();

int test_run_r2n_write_cmd();

int test_run_r2n_ssd_pri1();

int test_run_r2n_ssd_pri2();

int test_run_r2n_ssd_pri3();

int test_run_r2n_ssd_pri4();

int test_run_nvme_be_wrr1();

int test_run_nvme_be_wrr2();

int test_run_nvme_be_wrr3();

int test_run_nvme_be_wrr4();

int test_run_nvme_be_wrr5();

int test_run_nvme_be_wrr6();

int test_run_nvme_read_comp1();

int test_run_nvme_write_comp1();

int test_run_nvme_read_comp2();

int test_run_nvme_write_comp2();

int test_run_nvme_read_comp3();

int test_run_nvme_write_comp3();

int test_run_nvme_local_e2e1();

int test_run_nvme_local_e2e2();

int test_run_nvme_local_e2e3();

int test_run_seq_write1();

int test_run_seq_write2();

int test_run_seq_write3();

int test_run_seq_write4();

int test_run_seq_read1();

int test_run_seq_read2();

int test_run_seq_read3();

int test_run_seq_read4();

int test_run_seq_e2e1();

int test_run_seq_e2e2();

int test_run_seq_e2e3();

int test_run_seq_e2e4();

int test_seq_e2e_xts_r2n1();

int test_seq_write_roce(uint32_t seq_pdma_q, uint32_t seq_roce_q, 
			uint32_t pvm_roce_sq, uint64_t pdma_src_addr, 
			uint64_t pdma_dst_addr, uint32_t pdma_data_size,
			uint64_t roce_wqe_addr, uint32_t roce_wqe_size);

int test_seq_read_roce(uint32_t seq_pdma_q, uint32_t seq_roce_q, uint32_t pvm_roce_sq,
                       uint64_t pdma_src_addr, uint64_t pdma_dst_addr, uint32_t pdma_data_size,
                       uint8_t pdma_dst_lif_override, uint16_t pdma_dst_lif,
                       uint64_t roce_wqe_addr, uint32_t roce_wqe_size);
int test_seq_roce_op_pdma_prefilled(uint16_t seq_start_q,
                                    uint16_t seq_start_index,
                                    dp_mem_t *seq_roce_desc,
                                    uint32_t pvm_roce_sq,
                                    dp_mem_t *sqwqe);
int test_run_rdma_e2e_write();

int test_run_rdma_e2e_read();

int test_run_rdma_lif_override();

int test_run_rdma_e2e_xts_write1(void);

int test_run_rdma_e2e_xts_read1(void);

int test_run_seq_pdma_multi_xfers();

void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                        uint8_t ring, uint16_t index,
                        bool suppress_log = false);

struct TestEntry {
  std::function<int(void)> test_fn;
  const std::string test_name;
  bool test_succeded;
};

int add_xts_tests(std::vector<TestEntry>& test_suite);
int add_xts_perf_tests(std::vector<TestEntry>& test_suite);
int xts_multi_blk_noc_stress_from_host();
int xts_multi_blk_noc_stress_from_hbm();
int xts_multi_blk_noc_stress_from_host_hw_chain();
int xts_multi_blk_noc_stress_from_hbm_hw_chain();
int test_run_perf_rdma_e2e_write();

}  // namespace tests

#endif   // _TESTS_HPP_
