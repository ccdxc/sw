#ifndef _NVME_DP_HPP_
#define _NVME_DP_HPP_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace nvme_dp {

int test_setup();

int config ();

uint64_t get_io_map_base_addr();

uint64_t get_io_map_num_entries();

uint64_t get_io_buf_base_addr();

uint64_t get_iob_ring_base_addr();

}  // namespace nvme_dp


#endif  // _NVME_DP_HPP_
