#ifndef _TESTS_HPP_
#define _TESTS_HPP_

#include <stdint.h>
#include <functional>
#include <vector>
#include "gflags/gflags.h"
#include "dp_mem.hpp"
#include "chain_params.hpp"

DECLARE_uint64(poll_interval);
DECLARE_uint64(long_poll_interval);
DECLARE_uint64(num_pdma_queues);
DECLARE_uint64(acc_scale_chain_replica);
DECLARE_uint64(acc_scale_submissions);
DECLARE_uint64(acc_scale_blk_size);
DECLARE_uint64(acc_scale_iters);
DECLARE_string(acc_scale_verify_method);
DECLARE_bool(with_rtl_skipverify);
DECLARE_bool(combined);

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
                                  ACC_SCALE_TEST_CHKSUM_DECOMP,
  ACC_SCALE_TEST_MAX            = 5,

  /*
   * IMPORTANT: specify the number of scale tests that truly deploy P4+
   * chaining using sequencer status queues. For example, Decrypt-decomp
   * is one such test, but checksum-decompress is not (because there is
   * no inter-dependency between checksum and decompression so both
   * operations would be submitted simultaneously to HW).
   */
   ACC_SCALE_TEST_NUM_TRUE_CHAINING_TESTS = 3,

  /*
   * Number of tests requiring dual sequencer queues:
   * - checksum-decompress: 1 SQ for chksum and 1 SQ for decomp
   */
    ACC_SCALE_TEST_NUM_DUAL_SQ_TESTS = 1,
};

extern uint32_t run_acc_scale_tests_map;

using namespace dp_mem;

namespace tests {

/*
 * Method for certain types of HW resource query
 */
typedef enum {
    TEST_RESOURCE_NON_BLOCKING_QUERY,
    TEST_RESOURCE_BLOCKING_QUERY
} test_resource_query_method_t;

const static uint32_t  kDefaultNlb           = 0;
const static uint32_t  kDefaultNsid          = 1;
const static uint32_t  kDefaultBufSize       = 4096;

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
