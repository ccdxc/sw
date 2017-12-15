#ifndef _TESTS_HPP_
#define _TESTS_HPP_

#include <stdint.h>
#include <functional>

namespace tests {

typedef struct cp_seq_entry {
  uint64_t next_doorbell_addr;	// Next capri doorbell address (if chaining)
  uint64_t next_doorbell_data;	// Next capri doorbell data (if chaining)
  uint64_t status_hbm_pa;	// Status address in HBM. Provide this even if data_len is provided in desc.
  uint64_t src_hbm_pa;		// Address of compression buffer in HBM (source of PDMA)
  uint64_t sgl_pa;		// Address of the SGL in host (destination of PDMA)
  uint64_t intr_pa;		// MSI-X Interrupt address
  uint32_t intr_data;		// MSI-X Interrupt data
  uint16_t status_len;		// Length of the status header
  uint16_t data_len;		// Remaining data length of compression buffer
  // NOTE: Don't enable intr_en and next_doorbell_en together
  //       as only one will be serviced
  // Order of evaluation: 1. next_doorbell_en 2. intr_en
  // TODO: These bitfields are interpretted in big endian 
  //       fashion by P4+. For DOL it won't matter as we set bitfields.
  //       For driver, need to define the order properly.
  uint8_t  use_data_len:1;	// 0 = DIS, 1 =EN
  uint8_t  status_dma_en:1;	// 0 = DIS, 1 =EN
  uint8_t  next_doorbell_en:1;	// 0 = DIS, 1 =EN
  uint8_t  intr_en:1;		// 0 = DIS, 1 =EN
} cp_seq_entry_t;

typedef struct cq_sq_ent_sgl {
  uint64_t status_host_pa;	// Status address in host. Destination for the PDMA of status.
  uint64_t addr[4];		// Destination Address in the SGL for compression data PDMA
  uint16_t len[4];		// Length of the SGL element for compression data PDMA
} cp_esq_ent_sgl_t;

typedef struct cp_seq_params {
  cp_seq_entry_t seq_ent;	// Compression sequencer descriptor
  uint32_t seq_index;		// Compression sequencer queue (0 ... 15)
  uint64_t ret_doorbell_addr;	// Doorbell address that is formed for the compression sequencer (filled by API)
  uint64_t ret_doorbell_data;	// Doorbell data that is formed for the compression sequencer (filled by API)
} cp_seq_params_t;

// API return values: 0 => successs; < 0 => failure
int test_setup_cp_seq_ent(cp_seq_params_t *params);

int test_setup();

int check_ignore_cid(uint8_t *send_cmd, uint8_t *recv_cmd, uint32_t size);

int form_read_cmd_no_buf(uint8_t *nvme_cmd, uint64_t slba);

int form_write_cmd_no_buf(uint8_t *nvme_cmd, uint64_t slba);

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

int test_seq_read_roce(uint32_t seq_pdma_q, uint64_t pdma_src_addr, 
                       uint64_t pdma_dst_addr, uint32_t pdma_data_size,
                       uint8_t pdma_dst_lif_override, uint16_t pdma_dst_lif,
                       uint16_t db_lif, uint16_t db_qtype, uint32_t db_qid,
                       uint16_t db_ring, uint16_t db_index);

int test_run_rdma_e2e_write();

int test_run_rdma_e2e_read();

int test_run_rdma_lif_override();

struct TestEntry {
  std::function<int(void)> test_fn;
  const std::string test_name;
  bool test_succeded;
};

int add_xts_tests(std::vector<TestEntry>& test_suite);

}  // namespace tests

#endif   // _TESTS_HPP_
