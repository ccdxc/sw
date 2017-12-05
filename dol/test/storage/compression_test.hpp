// Configuration of compression test.

#ifndef _COMPRESSION_TEST_HPP_
#define _COMPRESSION_TEST_HPP_

#include "dol/test/storage/compression.hpp"

#include <stdint.h>
#include <string>

namespace tests {

static constexpr uint16_t kUncompressedDataSize = 4096;
extern uint8_t uncompressed_data[kUncompressedDataSize];

static constexpr uint16_t kCompressedDataSize = 2208;
extern uint8_t compression_hdr[8];
extern uint8_t compressed_data[kCompressedDataSize];

// Testcase spec.
// Src and dst are always 4K buffers. The test code will create
// SGLs (or use plain lists) based on datain_len and dataout_len.
typedef struct comp_test {
  std::string test_name;
  union {
    ccmd_t   cmd_bits;
    uint16_t cmd;
  };
  uint8_t num_src_sgls;
  uint8_t num_dst_sgls;
  uint16_t datain_len;
  uint16_t dataout_len;
  uint16_t ring_db:1,
           write_opaque_tag:1,
           src_is_hbm:1,
           dst_is_hbm:1,
           unused:1;
} comp_test_t;

void compression_init();
int compress_host_flat();
int compress_hbm_flat();
int compress_host_to_hbm_flat();
int compress_hbm_to_host_flat();
int compress_host_sgl();
int compress_hbm_sgl();
int compress_host_nested_sgl();
int compress_hbm_nested_sgl();

}  // namespace tests

#endif  // _COMPRESSION_TEST_HPP_
