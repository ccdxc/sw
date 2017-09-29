#ifndef _TESTS_HPP_
#define _TESTS_HPP_

#include <stdint.h>

namespace tests {

int test_setup();

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

}  // namespace tests

#endif   // _TESTS_HPP_
