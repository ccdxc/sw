#include <asm/byteorder.h>
#include <stdio.h>
#include <strings.h>
#include <string>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <iostream>

#include "gflags/gflags.h"

#include "dol/test/storage/tests.hpp"
#include "dol/test/storage/rdma.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/compression_test.hpp"
#include "dol/test/storage/acc_scale_tests.hpp"
#include "nic/model_sim/include/lib_model_client.h"

namespace queues {
void queues_shutdown();
}

DEFINE_uint64(hal_port, 50054, "TCP port of the HAL's gRPC server");
DEFINE_string(hal_ip, "localhost", "IP of HAL's gRPC server");
DEFINE_string(test_group, "", "Test group to run");
DEFINE_uint64(poll_interval, 60, "Polling interval in seconds");
DEFINE_uint64(long_poll_interval, 300,
              "Polling interval for longer running tests in seconds");

DEFINE_uint64(num_pdma_queues, 3,
              "number of queues for PDMA test (in power of 2)");

// Number of accelerator queues for scale testing
DEFINE_uint64(acc_scale_submissions, 2,
              "number of accelerator queue submissions for scale testing (in power of 2)");
DEFINE_validator(acc_scale_submissions, &queues::seq_queue_acc_sub_num_validate);

// Block size for accelerator scale testing
DEFINE_uint64(acc_scale_blk_size, 13,
              "Block size for accelerator scale testing (in power of 2)");
DEFINE_validator(acc_scale_blk_size, &tests::acc_scale_tests_blk_size_validate);

// Number of iterations for accelerator scale testing
DEFINE_uint64(acc_scale_iters, 1,
              "Number of iterations for accelerator scale testing (0 = infinite)");

// Verification method for accelerator scale testing
DEFINE_string(acc_scale_verify_method, "full",
              "Per-iteration verification method for accelerator scale testing: fast, or full");

bool run_unit_tests = false;
bool run_nvme_tests = false;
bool run_nvme_be_tests = false;
bool run_nvme_wrr_tests = false;
bool run_local_e2e_tests = false;
bool run_comp_tests = false;
bool run_comp_seq_tests = false;
bool run_xts_tests = false;
bool run_rdma_tests = false;
bool run_rdma_lif_override_tests = false;
bool run_rdma_xts_tests = false;
bool run_pdma_tests = false;
bool run_xts_perf_tests = false;
bool run_comp_perf_tests = false;
bool run_noc_perf_tests = false;
bool run_rdma_perf_tests = false;
bool run_acc_scale_tests = false;

std::vector<tests::TestEntry> test_suite;

std::vector<tests::TestEntry> unit_tests = {
  {&tests::test_run_nvme_pvm_admin_cmd, "NVME->PVM Admin Cmd", false},
};

std::vector<tests::TestEntry> nvme_tests = {
  {&tests::test_run_nvme_pvm_read_cmd, "NVME->PVM Read Cmd", false},
  {&tests::test_run_nvme_pvm_write_cmd, "NVME->PVM Write Cmd", false},
  {&tests::test_run_nvme_pvm_hashing1, "NVME->PVM Hashing 1", false},
  {&tests::test_run_nvme_pvm_hashing2, "NVME->PVM Hashing 2", false},
  {&tests::test_run_pvm_nvme_admin_status, "PVM->NVME Admin Status", false},
  {&tests::test_run_pvm_nvme_read_status, "PVM->NVME Read Status", false},
  {&tests::test_run_pvm_nvme_write_status, "PVM->NVME Write Status", false},
};

std::vector<tests::TestEntry> nvme_be_tests = {
  {&tests::test_run_r2n_read_cmd, "R2N -> SSD Read Cmd", false},
  {&tests::test_run_r2n_write_cmd, "R2N -> SSD Write Cmd", false},
  {&tests::test_run_r2n_ssd_pri1, "R2N -> SSD Pri Cmd 1", false},
  {&tests::test_run_r2n_ssd_pri2, "R2N -> SSD Pri Cmd 2", false},
  {&tests::test_run_r2n_ssd_pri3, "R2N -> SSD Pri Cmd 3", false},
  {&tests::test_run_r2n_ssd_pri4, "R2N -> SSD Pri Cmd 4", false},
  {&tests::test_run_nvme_read_comp1, "PVM Local Read Comp 1", false},
  {&tests::test_run_nvme_write_comp1, "PVM Local Write Comp 1", false},
  {&tests::test_run_nvme_read_comp2, "PVM Local Read Comp 2", false},
  {&tests::test_run_nvme_write_comp2, "PVM Local Write Comp 2", false},
  {&tests::test_run_nvme_read_comp3, "PVM Local Read Comp 3", false},
  {&tests::test_run_nvme_write_comp3, "PVM Local Write Comp 3", false},
};

std::vector<tests::TestEntry> nvme_wrr_tests = {
  {&tests::test_run_nvme_be_wrr1, "NVME Backend WRR 1", false},
  {&tests::test_run_nvme_be_wrr2, "NVME Backend WRR 2", false},
  {&tests::test_run_nvme_be_wrr3, "NVME Backend WRR 3", false},
  {&tests::test_run_nvme_be_wrr4, "NVME Backend WRR 4", false},
  {&tests::test_run_nvme_be_wrr5, "NVME Backend WRR 5", false},
  {&tests::test_run_nvme_be_wrr6, "NVME Backend WRR 6", false},
};

std::vector<tests::TestEntry> local_e2e_tests = {
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
};

std::vector<tests::TestEntry> comp_tests = {
  {&tests::compress_flat_64K_buf, "Compress Host->Host flat 64K buf", false},
  {&tests::compress_same_src_and_dst, "Compress with same src and dst", false},
  {&tests::decompress_to_flat_64K_buf, "Decompress Host->Host to flat 64K buf", false},
  {&tests::compress_odd_size_buf, "Compress Host->Host to flat odd size buf", false},
  {&tests::decompress_odd_size_buf, "Decompress Host->Host to flat odd size buf", false},
  {&tests::compress_host_sgl_to_host_sgl, "Compress Host->Host using SGLs", false},
  {&tests::decompress_host_sgl_to_host_sgl, "Decompress Host->Host using SGLs", false},
  {&tests::compress_flat_64K_buf_in_hbm, "Compress HBM->HBM flat 64K buf", false},
  {&tests::decompress_to_flat_64K_buf_in_hbm, "Decompress HBM->HBM to flat 64K buf", false},
  {&tests::compress_output_through_sequencer, "Compress and pull data from HBM through sequencer", false},
  {&tests::compress_dualq_flat_4K_buf, "Compress Host-Host flat 4K buf on hot and cold queues", false},
  {&tests::compress_dualq_flat_4K_buf_in_hbm, "Compress HBM-HBM flat 4K buf on hot and cold queues", false},
  {&tests::compress_output_encrypt_app_min_size, "Compress->XTS encrypt chaining: app min block size", false},
  {&tests::compress_output_encrypt_app_nominal_size, "Compress->XTS encrypt chaining: app nominal block size", false},
  {&tests::compress_output_encrypt_app_max_size, "Compress->XTS encrypt chaining: app max block size", false},
  // Enable when model is fixed.
  //{&tests::verify_integrity_for_gt64K, "Verify integrity calc for data size > 64K", false},
};

std::vector<tests::TestEntry> comp_seq_tests = {
  {&tests::seq_compress_flat_64K_buf, "Sequencer Compress Host->Host flat 64K buf", false},
  {&tests::seq_compress_same_src_and_dst, "Sequencer Compress with same src and dst", false},
  {&tests::seq_decompress_to_flat_64K_buf, "Sequencer Decompress Host->Host to flat 64K buf", false},
  {&tests::seq_compress_host_sgl_to_host_sgl, "Sequencer Compress Host->Host using SGLs", false},
  {&tests::seq_decompress_host_sgl_to_host_sgl, "Sequencer Decompress Host->Host using SGLs", false},
  {&tests::seq_compress_flat_64K_buf_in_hbm, "Sequencer Compress HBM->HBM flat 64K buf", false},
  {&tests::seq_decompress_to_flat_64K_buf_in_hbm, "Sequencer Decompress HBM->HBM to flat 64K buf", false},
  {&tests::seq_compress_output_through_sequencer, "Sequencer Compress and pull data from HBM through sequencer", false},
  {&tests::seq_compress_dualq_flat_4K_buf, "Sequencer Compress Host-Host flat 4K buf on hot and cold queues", false},
  {&tests::seq_compress_dualq_flat_4K_buf_in_hbm, "Sequencer Compress HBM-HBM flat 4K buf on hot and cold queues", false},
  {&tests::seq_compress_output_encrypt_app_min_size, "Sequencer Compress->XTS encrypt chaining: app min block size", false},
  {&tests::seq_decrypt_output_decompress_last_app_blk, "Sequencer XTS decrypt->Decompress chaining: app min block size", false},
  {&tests::seq_compress_output_encrypt_app_nominal_size, "Sequencer Compress->XTS encrypt chaining: app nominal block size", false},
  {&tests::seq_decrypt_output_decompress_last_app_blk, "Sequencer XTS decrypt->Decompress chaining: app nominal block size", false},
  {&tests::seq_compress_output_encrypt_app_max_size, "Sequencer Compress->XTS encrypt chaining: app max block size", false},
  {&tests::seq_decrypt_output_decompress_last_app_blk, "Sequencer XTS decrypt->Decompress chaining: app max block size", false},
};

std::vector<tests::TestEntry> comp_perf_tests = {
  {&tests::compress_flat_64K_buf, "Compress Host->Host flat 64K buf", false},
  {&tests::compress_flat_64K_buf_in_hbm, "Compress HBM->HBM flat 64K buf", false},
  {&tests::max_data_rate, "Test max data rate", false},
  {&tests::seq_max_data_rate, "Sequencer Test max data rate", false},
};

std::vector<tests::TestEntry> rdma_tests = {
  {&tests::test_run_rdma_e2e_write, "E2E write over RDMA", false},
  {&tests::test_run_rdma_e2e_read, "E2E read over RDMA", false},
};

std::vector<tests::TestEntry> rdma_lif_override_tests = {
  {&tests::test_run_rdma_lif_override, "E2E read LIF override", false},
};

std::vector<tests::TestEntry> rdma_xts_tests = {
  {&tests::test_run_rdma_e2e_xts_write1, "E2E write over RDMA with XTS", false},
  {&tests::test_run_rdma_e2e_xts_read1, "E2E read over RDMA with XTS", false},
};

std::vector<tests::TestEntry> pdma_tests = {
  {&tests::test_run_seq_pdma_multi_xfers, "PDMA multiple transfers", false},
};

std::vector<tests::TestEntry> noc_perf_tests = {
  {&tests::xts_multi_blk_noc_stress_from_hbm_hw_chain, "NOC Perf with buffers on hbm HW chain", false},
  //{&tests::xts_multi_blk_noc_stress_from_host_hw_chain, "NOC Perf with buffers on host HW chain", false},
  //{&tests::xts_multi_blk_noc_stress_from_hbm, "NOC Perf with buffers on hbm", false},
  //{&tests::xts_multi_blk_noc_stress_from_host, "NOC Perf with buffers on host", false},
};

std::vector<tests::TestEntry> rdma_perf_tests = {
  {&tests::test_run_perf_rdma_e2e_write, "Perf e2e rdma write", false},
  {&tests::test_run_perf_rdma_e2e_write, "Perf e2e rdma write", false},
};

std::vector<tests::TestEntry> acc_scale_tests = {
  {&tests::acc_scale_tests_comp_encrypt_decrypt_decomp,
   "Accelerator scale compress-encrypt-decrypt-decompress", false},
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

size_t tcid = 0;
int main(int argc, char**argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  signal(SIGSEGV, sig_handler);

  std::cout << "Input - hal_port: "   << FLAGS_hal_port 
            << "\nInput - hal_ip: "   << FLAGS_hal_ip 
            << "\nTest group: "       << FLAGS_test_group
            << "\nPolling interval: " << FLAGS_poll_interval 
            << "\n# PDMA queues (power of 2): " << FLAGS_num_pdma_queues 
            << "\n# Accelerator queue submissions (power of 2): " << FLAGS_acc_scale_submissions 
            << "\nBlock size for accelerator scale testing (in power of 2): " << FLAGS_acc_scale_blk_size 
            << "\n# Iterations for accelerator scale testing (0 = infinite): " << FLAGS_acc_scale_iters 
            << "\nVerification method for accelerator scale testing: " << FLAGS_acc_scale_verify_method 
            << std::endl;

  // Set the test group based on flags. Default is to allow all.
  if (FLAGS_test_group == "") {
      run_unit_tests = true;
      run_nvme_tests = true;
      run_nvme_be_tests = true;
      run_nvme_wrr_tests = true;
      run_local_e2e_tests = true;
      run_comp_tests = true;
      run_comp_seq_tests = true;
      run_xts_tests = true;
      run_rdma_tests = true;
      run_rdma_lif_override_tests = true;
      run_rdma_xts_tests = true;
      run_xts_perf_tests = false;
      run_comp_perf_tests = false;
      run_pdma_tests = true;
      run_acc_scale_tests = false;
  } else if (FLAGS_test_group == "rtl_sanity") {
      run_unit_tests = true;
      run_nvme_tests = true;
      run_nvme_be_tests = true;
      run_nvme_wrr_tests = false;		// Never enable this for RTL sanity
      run_local_e2e_tests = true;
      run_comp_tests = false;       // Enable after s/w debugging 
      run_comp_seq_tests = false;		// Enable after s/w debugging
      run_xts_tests = true;
      run_rdma_tests = true;
      run_rdma_lif_override_tests = false;	// Enable after h/w model changes
      run_rdma_xts_tests = false;		// Enable after s/w debugging
      run_xts_perf_tests = false;		// Never enable this for RTL sanity
      run_comp_perf_tests = false;		// Never enable this for RTL sanity
      run_pdma_tests = false;			// Never enable this for RTL sanity
      run_acc_scale_tests = false;
  } else if (FLAGS_test_group == "unit") {
      run_unit_tests = true;
  } else if (FLAGS_test_group == "nvme") {
      run_nvme_tests = true;
  } else if (FLAGS_test_group == "nvme_be") {
      run_nvme_be_tests = true;
      run_nvme_wrr_tests = true;
  } else if (FLAGS_test_group == "local_e2e") {
      run_local_e2e_tests = true;
  } else if (FLAGS_test_group == "comp") {
      run_comp_tests = true;
      run_comp_seq_tests = true;
  } else if (FLAGS_test_group == "xts") {
      run_xts_tests = true;
  } else if (FLAGS_test_group == "rdma") {
      run_rdma_tests = true;
      run_rdma_lif_override_tests = true;
      run_rdma_xts_tests = true;
  } else if (FLAGS_test_group == "xts_perf") {
      run_xts_perf_tests = true;
  } else if (FLAGS_test_group == "comp_perf") {
      run_comp_perf_tests = true;
  } else if (FLAGS_test_group == "pdma") {
      run_pdma_tests = true;
  } else if (FLAGS_test_group == "noc_perf") {
      run_noc_perf_tests = true;
  } else if (FLAGS_test_group == "rdma_perf") {
      run_rdma_perf_tests = true;
  } else if (FLAGS_test_group == "acc_scale") {
      run_acc_scale_tests = true;
  } else {
    printf("Usage: ./storage_test [--hal_port <xxx>] "
           "[--test_group unit|nvme|nvme_be|local_e2e|comp|xts|rdma|pdma|acc_scale|rtl_sanity] "
           " [--poll_interval <yyy>] \n");
    return -1;
  }

  printf("Starting configuration \n");
  if (tests::test_setup() < 0) {
    printf("Setup failed\n");
    return 1;
  }
  printf("Base configuration completed \n");

  printf("Going to init compression\n");
  tests::compression_init();
  printf("Compression configuration completed \n");

  if (rdma_init() < 0) {
    printf("RDMA Setup failed\n");
    return 1;
  }
  printf("RDMA configuration completed \n");

  // Indicate to model that config is done
  config_done();

  printf("Forming test suite based on configuration\n");
  // Add unit tests
  if (run_unit_tests || run_nvme_tests) {
    for (size_t i = 0; i < unit_tests.size(); i++) {
      test_suite.push_back(unit_tests[i]);
    }
    printf("Added unit tests \n");
  }

  // Add NVME tests
  if (run_nvme_tests) {
    for (size_t i = 0; i < nvme_tests.size(); i++) {
      test_suite.push_back(nvme_tests[i]);
    }
    printf("Added nvme tests \n");
  }

  // Add nvme_be tests
  if (run_nvme_be_tests) {
    for (size_t i = 0; i < nvme_be_tests.size(); i++) {
      test_suite.push_back(nvme_be_tests[i]);
    }
    printf("Added nvme_be tests \n");
  }

  // Add nvme_wrr tests
  if (run_nvme_wrr_tests) {
    for (size_t i = 0; i < nvme_wrr_tests.size(); i++) {
      test_suite.push_back(nvme_wrr_tests[i]);
    }
    printf("Added nvme_wrr tests \n");
  }

  // Add local_e2e tests
  if (run_local_e2e_tests) {
    for (size_t i = 0; i < local_e2e_tests.size(); i++) {
      test_suite.push_back(local_e2e_tests[i]);
    }
    printf("Added local_e2e tests \n");
  }

  // Add comp tests
  if (run_comp_tests) {
    for (size_t i = 0; i < comp_tests.size(); i++) {
      test_suite.push_back(comp_tests[i]);
    }
    printf("Added comp tests \n");
  }
 
  // Add comp_seq tests
  if (run_comp_seq_tests) {
    for (size_t i = 0; i < comp_seq_tests.size(); i++) {
      test_suite.push_back(comp_seq_tests[i]);
    }
    printf("Added comp_seq tests \n");
  }
 
  // Add comp_perf tests
  if (run_comp_perf_tests) {
    for (size_t i = 0; i < comp_perf_tests.size(); i++) {
      test_suite.push_back(comp_perf_tests[i]);
    }
    printf("Added comp perf tests \n");
  }

  // Add xts tests
  if (run_xts_tests) {
    tests::add_xts_tests(test_suite);
    printf("Added XTS tests \n");
  }

  // Add xts perf tests
  if (run_xts_perf_tests) {
    tests::add_xts_perf_tests(test_suite);
    printf("Added XTS perf tests \n");
  }

  // Add rdma tests
  if (run_rdma_tests) {
    for (size_t i = 0; i < rdma_tests.size(); i++) {
      test_suite.push_back(rdma_tests[i]);
    }
    printf("Added RDMA tests \n");
  }

  // NOTE: DO NOT ADD any tests between rdma_tests & rdma_lif_override_tests.
  //       There is an ORDERING dependency.
 
  // Add rdma lif override tests
  if (run_rdma_lif_override_tests) {
    for (size_t i = 0; i < rdma_lif_override_tests.size(); i++) {
      test_suite.push_back(rdma_lif_override_tests[i]);
    }
    printf("Added RDMA LIF override tests \n");
  }

  // Add rdma xts tests
  if (run_rdma_xts_tests) {
    for (size_t i = 0; i < rdma_xts_tests.size(); i++) {
      test_suite.push_back(rdma_xts_tests[i]);
    }
    printf("Added RDMA XTS tests \n");
  }

  // Add pdma tests
  if (run_pdma_tests) {
    for (size_t i = 0; i < pdma_tests.size(); i++) {
      test_suite.push_back(pdma_tests[i]);
    }
    printf("Added PDMA tests \n");
  }

  // Add NOC perf tests
  if (run_noc_perf_tests) {
    for (size_t i = 0; i < noc_perf_tests.size(); i++) {
      test_suite.push_back(noc_perf_tests[i]);
    }
    printf("Added NOC Perf tests \n");
  }

  // Add RDMA perf tests
  if (run_rdma_perf_tests) {
    for (size_t i = 0; i < rdma_perf_tests.size(); i++) {
      test_suite.push_back(rdma_perf_tests[i]);
    }
    printf("Added rdma Perf tests \n");
  }

  // Add accelerator scale tests
  if (run_acc_scale_tests) {
    for (size_t i = 0; i < acc_scale_tests.size(); i++) {
      test_suite.push_back(acc_scale_tests[i]);
    }
    printf("Added accelerator scale tests \n");
  }

  printf("Formed test suite with %d cases \n", (int) test_suite.size());

  printf("Running test cases \n");
  for (tcid = 0; tcid < test_suite.size(); tcid++) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    printf(" Starting test #: %d name: %s \n", (int) tcid, test_suite[tcid].test_name.c_str());

    testcase_begin(tcid, 0);
    if (test_suite[tcid].test_fn() < 0)
      test_suite[tcid].test_succeded = false;
    else
      test_suite[tcid].test_succeded = true;
    testcase_end(tcid, 0);

    gettimeofday(&end, NULL);
    printf(" Finished test #: %d name: %s status %d time %d \n", (int) tcid, test_suite[tcid].test_name.c_str(),
           test_suite[tcid].test_succeded, (int) (end.tv_sec - start.tv_sec));
  }
  printf("Test case run complete, shutting down queues \n");
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
  if (rc != 0) { 
    printf("\nOverall Report: FAILURE \n");
  } else {
    printf("\nOverall Report: SUCCESS \n");
  }
  fflush(stdout);
  if (rc != 0) return rc;
  exit(0);
}
