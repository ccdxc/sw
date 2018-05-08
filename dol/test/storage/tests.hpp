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
DECLARE_uint64(acc_scale_chain_replica);
DECLARE_uint64(acc_scale_submissions);
DECLARE_uint64(acc_scale_blk_size);
DECLARE_uint64(acc_scale_iters);
DECLARE_string(acc_scale_verify_method);
DECLARE_bool(with_rtl_skipverify);

extern bool run_pdma_tests;

enum {
  ACC_SCALE_TEST_NONE           = 0U,
  ACC_SCALE_TEST_DECRYPT_DECOMP = 1U << 0,
  ACC_SCALE_TEST_COMP_ENCRYPT   = 1U << 1,
  ACC_SCALE_TEST_COMP_HASH      = 1U << 2,
  ACC_SCALE_TEST_CHKSUM_DECOMP  = 1U << 3,
  ACC_SCALE_TEST_ENCRYPT_ONLY   = 1U << 4,
  ACC_SCALE_TEST_ALL            = ACC_SCALE_TEST_DECRYPT_DECOMP     |
                                  ACC_SCALE_TEST_COMP_ENCRYPT       |
                                  ACC_SCALE_TEST_COMP_HASH          |
                                  ACC_SCALE_TEST_ENCRYPT_ONLY       |
                                  0, // ACC_SCALE_TEST_CHKSUM_DECOMP      |
  ACC_SCALE_TEST_MAX            = 5,

  /*
   * IMPORTANT: specify the number of scale tests that truly deploy P4+
   * chaining using sequencer status queues. For example, Decrypt-decomp
   * is one such test, but checksum-decompress is not (because there is
   * no inter-dependency between checksum and decompression so both
   * operations would be submitted simultaneously to HW).
   */
   ACC_SCALE_TEST_NUM_TRUE_CHAINING_TESTS = 3
};

extern uint32_t run_acc_scale_tests_map;

using namespace dp_mem;

namespace tests {

const static uint32_t  kDefaultNlb           = 0;
const static uint32_t  kDefaultNsid          = 1;
const static uint32_t  kDefaultBufSize       = 4096;

// Accelerator chaining

typedef struct {
  uint64_t next_doorbell_addr;	// Next capri doorbell address (if chaining)
  uint64_t next_doorbell_data;	// Next capri doorbell data (if chaining)
} acc_chain_next_db_entry_t;

typedef struct {
  uint64_t barco_ring_addr;     // ring address
  uint64_t barco_pndx_addr;     // producer index address
  uint64_t barco_pndx_shadow_addr;// producer index shadow address
  uint64_t barco_desc_addr;     // descriptor to push
  uint8_t  barco_desc_size;     // descriptor size (power of 2 exponent)
  uint8_t  barco_pndx_size;     // producer index size (power of 2 exponent)
  uint8_t  barco_ring_size;     // ring size (power of 2 exponent)
} acc_chain_barco_push_entry_t;

typedef struct acc_chain_entry {
  union {
      acc_chain_next_db_entry_t    db_entry;
      acc_chain_barco_push_entry_t push_entry;
  };
  uint64_t status_hbm_pa;	// Status address in HBM. Provide this even if data_len is provided in desc.
  uint64_t status_host_pa;	// Destination for the PDMA of status.
  uint64_t src_hbm_pa;		// Address of compression source buffer (needed if copy_src_dst_on_error is set)
  uint64_t dst_hbm_pa;		// Address of compression destination buffer (see above)

  // post compression, options available are:
  // - PDMA compressed data to pdma_out_sgl_pa (sgl_pdma_en), or
  // - chain to next accelerator service which uses input/output AOL (aol_pad_en),
  // - chain to next accelerator service which uses SGL input (sgl_pad_hash_en),
  //   where P4+ will modify addr/length fields in the AOL/SGL based on compression result
  union {
      uint64_t sgl_pdma_in_pa;	// Address of the input SGL
      uint64_t barco_aol_in_pa;
  };
  union {
      uint64_t sgl_pdma_out_pa;	// Address of the output SGL
      uint64_t barco_aol_out_pa;
  };
  uint64_t sgl_vec_pa;	    // SGL vector for multi-block hash
  uint64_t pad_buf_pa;	    // pad buffer address
  uint64_t intr_pa;		    // MSI-X Interrupt address
  uint32_t intr_data;		// MSI-X Interrupt data
  uint16_t status_len;		// Length of the status header
  uint16_t data_len;		// Remaining data length of compression buffer
  uint8_t  pad_len_shift;   // Max padding length (power of 2)
  uint8_t  unused;
  // TODO: These bitfields are interpretted in big endian 
  //       fashion by P4+. For DOL it won't matter as we set bitfields.
  //       For driver, need to define the order properly.
  uint16_t data_len_from_desc   :1,	// use desc data_len rather than output_data_len
           status_dma_en        :1,	// enable DMA of status to status_hbm_pa
  // NOTE: intr_en and next_doorbell_en can be enabled together.
  // When comp/decomp succeeds, Order of evaluation: 1. next_doorbell_en 2. intr_en.
  // When comp/decomp fails and stop_chain_on_error is set, intr_en will be honored
           next_doorbell_en     :1,	// enable chain doorbell
           intr_en              :1,	// enable intr_data write to intr_pa
           next_db_action_barco_push:1,	// next_db action is actually a Barco push
           stop_chain_on_error  :1, // stop chaining on error
           copy_src_dst_on_error:1,
  // NOTE: sgl_xfer_en and aol_len_pad_en are mutually exclusive.
  // Order of evaluation: 1. aol_len_pad_en 2. sgl_xfer_en
           aol_pad_en           :1, // enable AOL length padding
           sgl_pad_hash_en      :1, // enable SGL length padding for multi-block hash
           sgl_pdma_en          :1, // enable data transfer from src_hbm_pa to sgl_pa
           sgl_pdma_pad_only    :1; // enable pad-only fill mode, i.e., Comp engine writes
                                    // compressed output according to SGL, P4+ will fill
                                    // the last block with the right amount of pad data.
                                    // This mode requires sgl_pad_hash_en as P4+ will glean
                                    // the buffers info from the supplied sgl_vec_pa.
} acc_chain_entry_t;

typedef struct cq_sq_ent_sgl {
  uint64_t addr[4];		// Destination Address in the SGL for compression data PDMA
  uint16_t len[4];		// Length of the SGL element for compression data PDMA
  uint64_t pad[3];
} cp_sq_ent_sgl_t;

typedef int (*acc_chain_desc_format_fn_t)(acc_chain_entry_t &chain_ent,
                                          dp_mem_t *seq_status_desc);

typedef struct acc_chain_params {
  acc_chain_entry_t chain_ent;	// Accelerator chaining sequencer descriptor
  acc_chain_desc_format_fn_t desc_format_fn;
  uint32_t seq_q;	            // Sequencer queue
  uint32_t seq_status_q;	    // Status sequencer queue
  uint32_t seq_next_q;   	    // Next sequencer queue in chain
  uint32_t seq_next_status_q;	// Next status sequencer queue in chain
  uint64_t ret_doorbell_addr;	// Doorbell address that is formed for the Status sequencer (filled by API)
  uint64_t ret_doorbell_data;	// Doorbell data that is formed for the Status sequencer (filled by API)
  uint16_t ret_seq_status_index;
} acc_chain_params_t;

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
int test_setup_seq_acc_chain_entry(acc_chain_params_t& params);
int test_setup_post_comp_seq_status_entry(acc_chain_entry_t &chain_ent,
                                          dp_mem_t *seq_status_desc);
int test_setup_post_xts_seq_status_entry(acc_chain_entry_t &chain_ent,
                                         dp_mem_t *seq_status_desc);
int test_data_verify_and_dump(uint8_t *expected_data,
                              uint8_t *actual_data,
                              uint32_t len);

int test_setup();

int alloc_buffers();

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

uint8_t get_next_byte();
uint16_t get_next_cid();
uint64_t get_next_slba();
void test_ring_nvme_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                             uint8_t ring, uint16_t index);

}  // namespace tests

#endif   // _TESTS_HPP_
