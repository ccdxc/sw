// Configuration of compression test.

#ifndef _COMPRESSION_TEST_HPP_
#define _COMPRESSION_TEST_HPP_

#include "compression.hpp"

namespace tests {

void compression_init();
int compress_flat_64K_buf();
int decompress_to_flat_64K_buf();
int compress_odd_size_buf();
int decompress_odd_size_buf();
int compress_host_sgl_to_host_sgl();
int decompress_host_sgl_to_host_sgl();

}  // namespace tests

#endif  // _COMPRESSION_TEST_HPP_
