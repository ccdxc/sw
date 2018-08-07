#ifndef _NVME_DP_TESTS_HPP_
#define _NVME_DP_TESTS_HPP_

namespace tests {

int test_run_nvme_dp_write_cmd();

int test_run_nvme_dp_read_cmd();

int test_run_nvme_dp_e2e_test();

int test_run_nvme_dp_write_perf();

int test_run_nvme_dp_write_scale();

int test_run_nvme_dp_read_perf();

int test_run_nvme_dp_read_scale();

int test_run_nvme_dp_e2e_scale();

}  // namespace tests

#endif   // _NVME_DP_TESTS_HPP_
