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

}  // namespace tests

#endif   // _TESTS_HPP_
