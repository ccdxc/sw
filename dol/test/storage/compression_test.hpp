// Configuration of compression test.

#ifndef _COMPRESSION_TEST_HPP_
#define _COMPRESSION_TEST_HPP_

#include "compression.hpp"

namespace tests {

void compression_init();
int compress_flat_64K_buf();
int seq_compress_flat_64K_buf();
int compress_same_src_and_dst();
int seq_compress_same_src_and_dst();
int decompress_to_flat_64K_buf();
int seq_decompress_to_flat_64K_buf();
int compress_odd_size_buf();
int decompress_odd_size_buf();
int compress_host_sgl_to_host_sgl();
int seq_compress_host_sgl_to_host_sgl();
int decompress_host_sgl_to_host_sgl();
int seq_decompress_host_sgl_to_host_sgl();
int compress_flat_64K_buf_in_hbm();
int seq_compress_flat_64K_buf_in_hbm();
int decompress_to_flat_64K_buf_in_hbm();
int seq_decompress_to_flat_64K_buf_in_hbm();
int compress_output_through_sequencer();
int seq_compress_output_through_sequencer();
int verify_integrity_for_gt64K();
int max_data_rate();
int seq_max_data_rate();
int compress_dualq_flat_4K_buf();
int seq_compress_dualq_flat_4K_buf();
int compress_dualq_flat_4K_buf_in_hbm();
int seq_compress_dualq_flat_4K_buf_in_hbm();
int compress_output_encrypt_app_min_size();
int compress_output_encrypt_app_max_size();
int compress_output_encrypt_app_nominal_size();
int seq_compress_output_encrypt_app_min_size();
int seq_compress_output_encrypt_app_max_size();
int seq_compress_output_encrypt_app_nominal_size();
int seq_decrypt_output_decompress_last_app_blk();

}  // namespace tests

#endif  // _COMPRESSION_TEST_HPP_
