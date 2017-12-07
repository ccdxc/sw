#include <asm/byteorder.h>
#include <stdio.h>
#include <strings.h>
#include <string>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "dol/test/storage/tests.hpp"
#include "dol/test/storage/rdma.hpp"
#include "dol/test/storage/compression_test.hpp"

namespace queues {
void queues_shutdown();
}

uint64_t hal_port = 50052;

std::vector<tests::TestEntry> test_suite = {
  {&tests::test_run_nvme_pvm_admin_cmd, "NVME->PVM Admin Cmd", false},
  {&tests::test_run_nvme_pvm_read_cmd, "NVME->PVM Read Cmd", false},
  {&tests::test_run_nvme_pvm_write_cmd, "NVME->PVM Write Cmd", false},
  {&tests::test_run_nvme_pvm_hashing1, "NVME->PVM Hashing 1", false},
  {&tests::test_run_nvme_pvm_hashing2, "NVME->PVM Hashing 2", false},
  {&tests::test_run_pvm_nvme_admin_status, "PVM->NVME Admin Status", false},
  {&tests::test_run_pvm_nvme_read_status, "PVM->NVME Read Status", false},
  {&tests::test_run_pvm_nvme_write_status, "PVM->NVME Write Status", false},
  {&tests::test_run_r2n_read_cmd, "R2N -> SSD Read Cmd", false},
  {&tests::test_run_r2n_write_cmd, "R2N -> SSD Write Cmd", false},
  {&tests::test_run_r2n_ssd_pri1, "R2N -> SSD Pri Cmd 1", false},
  {&tests::test_run_r2n_ssd_pri2, "R2N -> SSD Pri Cmd 2", false},
  {&tests::test_run_r2n_ssd_pri3, "R2N -> SSD Pri Cmd 3", false},
  {&tests::test_run_r2n_ssd_pri4, "R2N -> SSD Pri Cmd 4", false},
  {&tests::test_run_nvme_be_wrr1, "NVME Backend WRR 1", false},
  {&tests::test_run_nvme_be_wrr2, "NVME Backend WRR 2", false},
  {&tests::test_run_nvme_be_wrr3, "NVME Backend WRR 3", false},
  {&tests::test_run_nvme_be_wrr4, "NVME Backend WRR 4", false},
  {&tests::test_run_nvme_be_wrr5, "NVME Backend WRR 5", false},
  {&tests::test_run_nvme_be_wrr6, "NVME Backend WRR 6", false},
  {&tests::test_run_nvme_read_comp1, "PVM Local Read Comp 1", false},
  {&tests::test_run_nvme_write_comp1, "PVM Local Write Comp 1", false},
  {&tests::test_run_nvme_read_comp2, "PVM Local Read Comp 2", false},
  {&tests::test_run_nvme_write_comp2, "PVM Local Write Comp 2", false},
  {&tests::test_run_nvme_read_comp3, "PVM Local Read Comp 3", false},
  {&tests::test_run_nvme_write_comp3, "PVM Local Write Comp 3", false},
  {&tests::test_run_nvme_local_e2e1, "NVME Local Tgt E2E 1", false},
  {&tests::test_run_nvme_local_e2e2, "NVME Local Tgt E2E 2", false},
  {&tests::test_run_nvme_local_e2e3, "NVME Local Tgt E2E 3", false},
  {&tests::test_run_seq_write1, "Seq Local Tgt Write 1", false},
  {&tests::test_run_seq_write2, "Seq Local Tgt Write 2", false},
  {&tests::test_run_seq_write3, "Seq Local Tgt Write 3", false},
  {&tests::test_run_seq_write4, "Seq Local Tgt Write 4", false},
  {&tests::test_run_seq_read1, "Seq Local Tgt Read 1", false},
  {&tests::test_run_seq_read2, "Seq Local Tgt Read 2", false},
  {&tests::test_run_seq_read3, "Seq Local Tgt Read 3", false},
  {&tests::test_run_seq_read4, "Seq Local Tgt Read 4", false},
  {&tests::test_run_seq_e2e1, "Seq Local Tgt E2E 1", false},
  {&tests::test_run_seq_e2e2, "Seq Local Tgt E2E 2", false},
  {&tests::test_run_seq_e2e3, "Seq Local Tgt E2E 3", false},
  {&tests::test_run_seq_e2e4, "Seq Local Tgt E2E 4", false},
  {&tests::test_seq_e2e_xts_r2n1, "PDMA->XTS->R2N", false},
  {&tests::compress_host_flat, "Compress Host->Host flat buf", false},
  {&tests::compress_hbm_flat, "Compress HBM->HBM flat buf", false},
  {&tests::compress_host_to_hbm_flat, "Compress Host->HBM flat buf", false},
  {&tests::compress_hbm_to_host_flat, "Compress HBM->Host flat buf", false},
  {&tests::compress_host_sgl, "Compress Host->Host sgl buf", false},
  {&tests::compress_hbm_sgl, "Compress HBM->HBM sgl buf", false},
  {&tests::compress_host_nested_sgl, "Compress Host->Host nested sgl buf", false},
  {&tests::compress_hbm_nested_sgl, "Compress HBM->HBM nested sgl buf", false},
  {&tests::compress_nested_sgl_in_hbm, "Compress Nested sgl buf in HBM", false},
  {&tests::compress_return_through_hbm, "Compress Status/dest-buf in HBM", false},
  {&tests::compress_adler_sha256, "Compress with Adler32 and SHA256", false},
  {&tests::compress_crc_sha512, "Compress with CRC32 and SHA512", false},
  {&tests::compress_doorbell_odata, "Compress with DMA end writes", false},
  {&tests::compress_max_features, "Compress with multiple features", false},
};

std::vector<tests::TestEntry> rdma_tests = {
  {&tests::test_run_rdma_e2e_write, "E2E write over RDMA", false},
  {&tests::test_run_rdma_e2e_read, "E2E read over RDMA", false},
};

void sig_handler(int sig) {
  void *array[16];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 16);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char**argv) {
  signal(SIGSEGV, sig_handler);
  if (argc != 2) {
    printf("Usage: storage_test <hal_port>\n");
    exit(1);
  }
  hal_port = strtoul(argv[1], nullptr, 0);

  if (tests::test_setup() < 0) {
    printf("Setup failed\n");
    return 1;
  }
  printf("Setup completed\n");

  printf("Going to init compression\n");
  tests::compression_init();

  if (rdma_init() < 0) {
    printf("RDMA Setup failed\n");
    return 1;
  }

  // Add xts tests
  tests::add_xts_tests(test_suite);
  printf("Added XTS tests \n");

  // Add rdma tests
  for (size_t i = 0; i < rdma_tests.size(); i++) {
    test_suite.push_back(rdma_tests[i]);
  }
  printf("Added RDMA tests \n");

  for (size_t i = 0; i < test_suite.size(); i++) {
    printf(" Starting test #: %d name: %s \n", (int) i, test_suite[i].test_name.c_str());
    if (test_suite[i].test_fn() < 0)
      test_suite[i].test_succeded = false;
    else
      test_suite[i].test_succeded = true;
  }
  queues::queues_shutdown();

  printf("\nConsolidated Test Report \n");
  printf("--------------------------------------------------------------\n");
  printf("Number\t\tName\t\t\tResult\n");
  printf("--------------------------------------------------------------\n");
  
  int rc = 0;
  for (size_t i = 0; i < test_suite.size(); i++) {
    printf("%lu\t", i+1);
    printf("%s\t\t\t\t", test_suite[i].test_name.c_str());
    printf("%s\n", test_suite[i].test_succeded ? "Success" : "Failure");
    if (!test_suite[i].test_succeded) rc = 1;
  }
  fflush(stdout);
  if (rc != 0) return rc;
  exit(0);
}
