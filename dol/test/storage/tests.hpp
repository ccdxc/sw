#ifndef _TESTS_HPP_
#define _TESTS_HPP_

#include <stdint.h>

namespace tests {

int test_setup();

int test_run_nvme_pvm_admin_cmd();

int test_run_nvme_pvm_read_cmd();

}  // namespace tests

#endif
