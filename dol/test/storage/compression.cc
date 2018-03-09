// Compression DOLs.
#include "compression.hpp"
#include "compression_test.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include "nic/asic/capri/design/common/cap_addr_define.h"
#include "nic/asic/capri/model/cap_he/readonly/cap_hens_csr_define.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <byteswap.h>
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"
#include "gflags/gflags.h"

DECLARE_uint64(long_poll_interval);

namespace tests {

static const uint64_t cp_cfg_glob = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_GLB_BYTE_ADDRESS;

static const uint64_t cp_cfg_dist = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS;

static const uint64_t cp_cfg_ueng = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_UENG_W0_BYTE_ADDRESS;

static const uint64_t cp_cfg_q_base = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_BASE_ADR_W0_BYTE_ADDRESS;

static const uint64_t cp_cfg_hotq_base = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_BASE_ADR_W0_BYTE_ADDRESS;

static const uint64_t cp_cfg_q_pd_idx = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_PD_IDX_BYTE_ADDRESS;

static const uint64_t cp_cfg_hotq_pd_idx = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_PD_IDX_BYTE_ADDRESS;

static const uint64_t cp_cfg_host = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOST_BYTE_ADDRESS;

static const uint64_t dc_cfg_glob = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_GLB_BYTE_ADDRESS;

static const uint64_t dc_cfg_dist = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS;

static const uint64_t dc_cfg_ueng = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_UENG_W0_BYTE_ADDRESS;

static const uint64_t dc_cfg_q_base = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q_BASE_ADR_W0_BYTE_ADDRESS;

static const uint64_t dc_cfg_hotq_base = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOTQ_BASE_ADR_W0_BYTE_ADDRESS;

static const uint64_t dc_cfg_q_pd_idx = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q_PD_IDX_BYTE_ADDRESS;

static const uint64_t dc_cfg_hotq_pd_idx = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOTQ_PD_IDX_BYTE_ADDRESS;

static const uint64_t dc_cfg_host = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOST_BYTE_ADDRESS;

static const uint32_t kNumSubqEntries = 4096;
static const uint32_t kQueueMemSize = sizeof(cp_desc_t) * kNumSubqEntries;
static void *cp_queue_mem;
static uint64_t cp_queue_mem_pa;
static uint16_t cp_queue_index = 0;
static void *dc_queue_mem;
static uint64_t dc_queue_mem_pa;
static uint16_t dc_queue_index = 0;

static void *cp_hotq_mem;
static uint64_t cp_hotq_mem_pa;
static uint16_t cp_hotq_index = 0;
static void *dc_hotq_mem;
static uint64_t dc_hotq_mem_pa;
static uint16_t dc_hotq_index = 0;

// Sample data generated during test.
uint8_t all_zeros[65536] = {0};
static constexpr uint32_t kUncompressedDataSize = 65536;
static uint8_t uncompressed_data[kUncompressedDataSize];
static constexpr uint32_t kCompressedBufSize = 65536 - 4096;
static uint8_t compressed_data_buf[kCompressedBufSize];
static uint16_t compressed_data_size;  // Calculated at run-time;
static uint16_t last_cp_output_data_len;  // Calculated at run-time;

// Buffer (total 128K) layout in both HBM and host memory:
static constexpr uint32_t kTotalBufSize = 128 * 1024;
// 0 - (64K-1)              : Uncompressed data
static constexpr uint32_t kUncompressedDataOffset = 0;
// 64K - 126975 (128K-4K)   : compressed data
static constexpr uint32_t kCompressedDataOffset = 65536;
// 126976                   : Status (max 512 bytes)
static constexpr uint32_t kStatusOffset = 126976;
// 127488                   : Fake opaque tag (if needed)
static constexpr uint32_t kOpaqueTagOffset = 127488;
// 127496                   : Fake doorbell (if needed)
static constexpr uint32_t kDoorbellOffset = 127496;
// 128000                   : Sequencer entries (upto 1024 bytes).
static constexpr uint32_t kSequencerOffset = 128000;
// 129024                   : SGL entries (upto 1024 bytes);
static constexpr uint32_t kSGLOffset = 129024;

static uint64_t host_mem_pa;
static uint8_t *host_mem;
static uint64_t hbm_pa;

// Write zeros to the 1st 8 bytes of compressed data buffer in hostmem.
void InvalidateHdrInHostMem() {
  *((uint64_t *)(host_mem + kCompressedDataOffset)) = 0;
}

// Write zeros to the 1st 8 bytes of compressed data buffer in hbm.
void InvalidateHdrInHBM() {
  write_mem(hbm_pa + kCompressedDataOffset, all_zeros, 8);
}

static bool status_poll(bool in_hbm) {
  auto func = [in_hbm] () -> int {
    cp_status_sha512_t *s = (cp_status_sha512_t *)(host_mem + kStatusOffset);
    if (in_hbm) {
      read_mem(hbm_pa + kStatusOffset, (uint8_t *)s, sizeof(cp_status_sha512_t));
    }
    if (s->valid) {
      if (in_hbm) {
        usleep(100);
        read_mem(hbm_pa + kStatusOffset, (uint8_t *)s, sizeof(cp_status_sha512_t));
      }
      printf("Got status %llx\n", *((unsigned long long *)s));
      return 0;
    }
    return 1;
  };
  tests::Poller poll;
  if (poll(func) == 0)
    return true;
  return false;
}

void
compression_init()
{
  cp_queue_mem = alloc_page_aligned_host_mem(kQueueMemSize);
  assert(cp_queue_mem != nullptr);
  cp_queue_mem_pa = host_mem_v2p(cp_queue_mem);
  dc_queue_mem = alloc_page_aligned_host_mem(kQueueMemSize);
  assert(dc_queue_mem != nullptr);
  dc_queue_mem_pa = host_mem_v2p(dc_queue_mem);

  cp_hotq_mem = alloc_page_aligned_host_mem(kQueueMemSize);
  assert(cp_hotq_mem != nullptr);
  cp_hotq_mem_pa = host_mem_v2p(cp_hotq_mem);
  dc_hotq_mem = alloc_page_aligned_host_mem(kQueueMemSize);
  assert(dc_hotq_mem != nullptr);
  dc_hotq_mem_pa = host_mem_v2p(dc_hotq_mem);

  host_mem = (uint8_t *)alloc_page_aligned_host_mem(kTotalBufSize);
  assert(host_mem != nullptr);
  host_mem_pa = host_mem_v2p(host_mem);
  assert(utils::hbm_addr_alloc_page_aligned(kTotalBufSize, &hbm_pa) == 0);

  // Pre-fill input buffers.
  uint64_t *p64 = (uint64_t *)uncompressed_data;
  for (uint64_t i = 0; i < (kUncompressedDataSize/sizeof(uint64_t)); i++)
    p64[i] = i;
  bcopy(uncompressed_data, host_mem + kUncompressedDataOffset, kUncompressedDataSize);
  // write_mem() cannot operate on more than 8K (12K?) at a time. So copy the data
  // 8K at a time.
  for (uint64_t i = 0; i < (kUncompressedDataSize/8192); i++) {
    write_mem(hbm_pa + kUncompressedDataOffset + (i * 8192),
              (uint8_t *)&uncompressed_data[i*8192],
              std::min(8192ul, kUncompressedDataSize - (i*8192ul)));
  }

  uint32_t lo_reg, hi_reg;
  // Write cp queue base.
  read_reg(cp_cfg_glob, lo_reg);
  write_reg(cp_cfg_glob, (lo_reg & 0xFFFF0000u) | kCPVersion);
  write_reg(cp_cfg_q_base, cp_queue_mem_pa & 0xFFFFFFFFu);
  write_reg(cp_cfg_q_base + 4, (cp_queue_mem_pa >> 32) & 0xFFFFFFFFu);

  write_reg(cp_cfg_hotq_base, cp_hotq_mem_pa & 0xFFFFFFFFu);
  write_reg(cp_cfg_hotq_base + 4, (cp_hotq_mem_pa >> 32) & 0xFFFFFFFFu);

  // Write dc queue base.
  read_reg(dc_cfg_glob, lo_reg);
  write_reg(dc_cfg_glob, (lo_reg & 0xFFFF0000u) | kCPVersion);
  write_reg(dc_cfg_q_base, dc_queue_mem_pa & 0xFFFFFFFFu);
  write_reg(dc_cfg_q_base + 4, (dc_queue_mem_pa >> 32) & 0xFFFFFFFFu);

  write_reg(dc_cfg_hotq_base, dc_hotq_mem_pa & 0xFFFFFFFFu);
  write_reg(dc_cfg_hotq_base + 4, (dc_hotq_mem_pa >> 32) & 0xFFFFFFFFu);

  // Enable all 16 cp engines.
  read_reg(cp_cfg_ueng, lo_reg);
  read_reg(cp_cfg_ueng+4, hi_reg);
  lo_reg |= 0xFFFF;
  hi_reg &= ~(1u << (54 - 32));
  hi_reg &= ~(1u << (53 - 32));
  hi_reg |= 1u << (55 - 32);
  write_reg(cp_cfg_ueng, lo_reg);
  write_reg(cp_cfg_ueng+4, hi_reg);
  // Enable both DC engines.
  read_reg(dc_cfg_ueng, lo_reg);
  read_reg(dc_cfg_ueng+4, hi_reg);
  lo_reg |= 0x3;
  hi_reg &= ~(1u << (54 - 32));
  hi_reg &= ~(1u << (53 - 32));
  hi_reg |= 1u << (55 - 32);
  write_reg(dc_cfg_ueng, lo_reg);
  write_reg(dc_cfg_ueng+4, hi_reg);

  // Enable cold/warm queue.
  read_reg(cp_cfg_dist, lo_reg);
  lo_reg |= 1;
  write_reg(cp_cfg_dist, lo_reg);
  read_reg(dc_cfg_dist, lo_reg);
  lo_reg |= 1;
  write_reg(dc_cfg_dist, lo_reg);

  cp_queue_index = 0;
  dc_queue_index = 0;

  cp_hotq_index = 0;
  dc_hotq_index = 0;

  printf("Compression init done\n");
}

// We only compare err and partial data from exp.
static int run_cp_test(cp_desc_t *desc, bool status_in_hbm, const cp_status_no_hash_t &exp) {
  cp_status_sha512_t *s = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  bzero(s, sizeof(*s));
  if (status_in_hbm) {
    write_mem(hbm_pa + kStatusOffset, all_zeros, sizeof(*s));
  }
  desc->status_addr = (status_in_hbm ? hbm_pa : host_mem_pa) + kStatusOffset;

  cp_desc_t *dst_d = (cp_desc_t *)cp_queue_mem;
  bcopy(desc, &dst_d[cp_queue_index], sizeof(*desc));
  cp_queue_index++;
  if (cp_queue_index == 4096)
    cp_queue_index = 0;
  write_reg(cp_cfg_q_pd_idx, cp_queue_index);

  if (!status_poll(status_in_hbm)) {
    printf("ERROR: status never came\n");
    return -1;
  }
  cp_status_sha512_t *st = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  if (!st->valid) {
    printf("ERROR: Status valid bit not set\n");
    return -1;
  }
  if (st->err != exp.err) {
    printf("ERROR: status err 0x%x is unexpected\n", st->err);
    return -1;
  }
  if (st->partial_data != exp.partial_data) {
    printf("ERROR: status partial data 0x%x is unexpected\n", st->partial_data);
    return -1;
  }
  if (desc->cmd_bits.comp_decomp_en && desc->cmd_bits.insert_header && (st->err == 0)) {
    cp_hdr_t *hdr = (cp_hdr_t *)(host_mem + kCompressedDataOffset);
    // If the dst was hbm, copy over header from there.
    if (!(desc->dst & (1ul << 63))) {
      read_mem(hbm_pa + kCompressedDataOffset, (uint8_t *)hdr, sizeof(*hdr));
    }
    if (hdr->version != kCPVersion) {
      printf("Header version mismatch, expected 0x%x, received 0x%x\n",
             kCPVersion, hdr->version);
      return -1;
    }
    if (hdr->cksum == 0) {
      printf("ERROR: Header with zero cksum\n");
      return -1;
    }
    uint32_t datain_len = desc->datain_len;
    if (datain_len == 0)
      datain_len = 65536;
    if ((hdr->data_len == 0) || (hdr->data_len > (kCompressedBufSize - 8))
        || (hdr->data_len > datain_len)) {
      printf("ERROR: Invalid data_len 0x%x\n", hdr->data_len);
      return -1;
    }
    if (st->output_data_len != (hdr->data_len + 8)) {
      printf("ERROR: output_data_len %u does not match hdr+8 %u\n",
             st->output_data_len, hdr->data_len + 8);
      return -1;
    }
  }
  if (desc->cmd_bits.comp_decomp_en && (st->err == 0) && desc->cmd_bits.sha_en) {
    int sha_size = desc->cmd_bits.sha_type ? 32 : 64;
    if (bcmp(st->sha512, all_zeros, sha_size) == 0) {
      printf("ERROR: Sha is all zero\n");
      return -1;
    }
  }
  if ((st->err == 0) && (st->integrity_data == 0)) {
    printf("Integrity is all zero\n");
    return -1;
  }
  last_cp_output_data_len = st->output_data_len;
  printf("Last output data len = %u\n", last_cp_output_data_len);
  return 0;
}

// Only err, output_data_len and partial_data is compared from exp.
static int run_dc_test(cp_desc_t *desc, bool status_in_hbm, const cp_status_no_hash_t &exp) {
  cp_status_sha512_t *s = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  bzero(s, sizeof(*s));
  if (status_in_hbm) {
    write_mem(hbm_pa + kStatusOffset, all_zeros, sizeof(*s));
  }
  desc->status_addr = (status_in_hbm ? hbm_pa : host_mem_pa) + kStatusOffset;

  cp_desc_t *dst_d = (cp_desc_t *)dc_queue_mem;
  bcopy(desc, &dst_d[dc_queue_index], sizeof(*desc));
  dc_queue_index++;
  if (dc_queue_index == 4096)
    dc_queue_index = 0;
  write_reg(dc_cfg_q_pd_idx, dc_queue_index);

  if (!status_poll(status_in_hbm)) {
    printf("ERROR: status never came\n");
    return -1;
  }
  cp_status_sha512_t *st = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  if (!st->valid) {
    printf("ERROR: Status valid bit not set\n");
    return -1;
  }
  if (st->err != exp.err) {
    printf("ERROR: status err 0x%x is unexpected\n", st->err);
    return -1;
  }
  if (st->partial_data != exp.partial_data) {
    printf("ERROR: status partial data 0x%x is unexpected\n", st->partial_data);
    return -1;
  }
  if ((st->err == 0) && (st->output_data_len != exp.output_data_len)) {
    printf("ERROR: output data len mismatch, expected 0x%x, received 0x%x\n",
           exp.output_data_len, st->output_data_len);
    return -1;
  }
  return 0;
}

int compress_flat_64K_buf() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.src = host_mem_pa + kUncompressedDataOffset;
  d.dst = host_mem_pa + kCompressedDataOffset;
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - 8;
  d.status_data = 0x1234;
  InvalidateHdrInHostMem();
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  if (run_cp_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  // Save compressed data.
  cp_hdr_t *hdr = (cp_hdr_t *)(host_mem + kCompressedDataOffset);
  bcopy(hdr, compressed_data_buf, hdr->data_len + 8);
  compressed_data_size = hdr->data_len + 8;
  return 0;
}

int compress_same_src_and_dst() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.src = host_mem_pa + kCompressedDataOffset;
  bcopy(host_mem + kUncompressedDataOffset,
        host_mem + kCompressedDataOffset, 4096);
  d.dst = host_mem_pa + kCompressedDataOffset;
  d.datain_len = 4096;
  d.threshold_len = 4096 - 8;
  d.status_data = 0x1234;
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  if (run_cp_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_to_flat_64K_buf() {
  bzero(host_mem, kTotalBufSize);
  bcopy(compressed_data_buf, host_mem + kCompressedDataOffset, compressed_data_size);
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.dst = host_mem_pa + kUncompressedDataOffset;
  d.src = host_mem_pa + kCompressedDataOffset;
  d.datain_len = compressed_data_size;
  d.threshold_len = 0;  // 64K, output len
  d.status_data = 0x3456;
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x3456;
  if (run_dc_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Verify data buf
  if (bcmp(host_mem + kUncompressedDataOffset, uncompressed_data, kUncompressedDataSize) != 0) {
    printf("Data does not match after decompress\n");
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_odd_size_buf() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.src = host_mem_pa + kUncompressedDataOffset;
  d.dst = host_mem_pa + kCompressedDataOffset;
  d.datain_len = 567;  // 0 = 64K
  d.threshold_len = 567 - 8;
  d.status_data = 0x1234;
  InvalidateHdrInHostMem();
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  if (run_cp_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_odd_size_buf() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.dst = host_mem_pa + kUncompressedDataOffset;
  d.src = host_mem_pa + kCompressedDataOffset;
  d.datain_len = last_cp_output_data_len;
  d.threshold_len = 567;  // 64K, output len
  d.status_data = 0x3456;
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x3456;
  st.output_data_len = 567;
  if (run_dc_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Verify data buf
  if (bcmp(host_mem + kUncompressedDataOffset, uncompressed_data, 567) != 0) {
    printf("Data does not match after decompress\n");
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_host_sgl_to_host_sgl() {
  // Prepare source SGLs
  uint64_t data_pa = host_mem_pa + kUncompressedDataOffset;
  uint64_t sgl_pa = host_mem_pa + kSGLOffset;
  cp_sgl_t sgl1 = {0}, sgl2 = {0};
  sgl1.len0 = sgl1.len1 = sgl1.len2 = 1000;
  sgl1.addr0 = data_pa + 0;
  sgl1.addr1 = data_pa + 1000;
  sgl1.addr2 = data_pa + 2000;
  sgl1.link = sgl_pa + sizeof(cp_sgl_t);
  sgl2.len0 = sgl2.len1 = sgl2.len2 = 1000;
  sgl2.addr0 = data_pa + 3000;
  sgl2.addr1 = data_pa + 4000;
  sgl2.addr2 = data_pa + 5000;
  bcopy(&sgl1, host_mem + kSGLOffset, sizeof(cp_sgl_t));
  bcopy(&sgl2, host_mem + kSGLOffset + sizeof(cp_sgl_t), sizeof(cp_sgl_t));

  // Prepare destination SGLs
  data_pa = host_mem_pa + kCompressedDataOffset;
  sgl_pa = host_mem_pa + kSGLOffset + (2*sizeof(cp_sgl_t));
  sgl1 = {0}; sgl2 = {0};
  sgl1.len0 = sgl1.len1 = sgl1.len2 = 1024;
  sgl1.addr0 = data_pa + 0;
  sgl1.addr1 = data_pa + 1024;
  sgl1.addr2 = data_pa + 2048;
  sgl1.link = sgl_pa + sizeof(cp_sgl_t);
  sgl2.len0 = sgl2.len1 = sgl2.len2 = 1024;
  sgl2.addr0 = data_pa + (3 * 1024);
  sgl2.addr1 = data_pa + 4096;
  sgl2.addr2 = data_pa + (5 * 1024);
  bcopy(&sgl1, host_mem + kSGLOffset + (2*sizeof(cp_sgl_t)), sizeof(cp_sgl_t));
  bcopy(&sgl2, host_mem + kSGLOffset + (3*sizeof(cp_sgl_t)), sizeof(cp_sgl_t));

  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.cmd_bits.src_is_list = 1;
  d.cmd_bits.dst_is_list = 1;
  d.src = host_mem_pa + kSGLOffset;
  d.dst = host_mem_pa + kSGLOffset + (2*sizeof(cp_sgl_t));
  d.datain_len = 6000;
  d.threshold_len = 6000 - 8;
  d.status_data = 0x1234;
  InvalidateHdrInHostMem();
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  if (run_cp_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_host_sgl_to_host_sgl() {
  // bzero some initial area.
  bzero(host_mem + kUncompressedDataOffset, 1024);
  // Prepare source SGLs
  uint64_t data_pa = host_mem_pa + kCompressedDataOffset;
  uint64_t sgl_pa = host_mem_pa + kSGLOffset;
  cp_sgl_t sgl1 = {0};
  cp_sgl_t sgl2;
  sgl1.len0 = last_cp_output_data_len >> 1;
  sgl1.len1 = last_cp_output_data_len - sgl1.len0;
  sgl1.addr0 = data_pa + 0;
  sgl1.addr1 = data_pa + sgl1.len0;

  bcopy(&sgl1, host_mem + kSGLOffset, sizeof(cp_sgl_t));

  // Prepare destination SGLs
  data_pa = host_mem_pa + kUncompressedDataOffset;
  sgl_pa = host_mem_pa + kSGLOffset + (2*sizeof(cp_sgl_t));
  sgl1 = {0}; sgl2 = {0};
  sgl1.len0 = sgl1.len1 = sgl1.len2 = 1024;
  sgl1.addr0 = data_pa + 0;
  sgl1.addr1 = data_pa + 1024;
  sgl1.addr2 = data_pa + 2048;
  sgl1.link = sgl_pa + sizeof(cp_sgl_t);
  sgl2.len0 = sgl2.len1 = sgl2.len2 = 1024;
  sgl2.addr0 = data_pa + (3 * 1024);
  sgl2.addr1 = data_pa + 4096;
  sgl2.addr2 = data_pa + (5 * 1024);
  bcopy(&sgl1, host_mem + kSGLOffset + (2*sizeof(cp_sgl_t)), sizeof(cp_sgl_t));
  bcopy(&sgl2, host_mem + kSGLOffset + (3*sizeof(cp_sgl_t)), sizeof(cp_sgl_t));

  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.cmd_bits.src_is_list = 1;
  d.cmd_bits.dst_is_list = 1;
  d.src = host_mem_pa + kSGLOffset;
  d.dst = host_mem_pa + kSGLOffset + (2*sizeof(cp_sgl_t));
  d.datain_len = last_cp_output_data_len;
  d.threshold_len = 6000;
  d.status_data = 0x1234;
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  st.output_data_len = 6000;
  if (run_dc_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  if (bcmp(host_mem + kUncompressedDataOffset, uncompressed_data, 6000) != 0) {
    printf("Data does not match after decompress\n");
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_flat_64K_buf_in_hbm() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.src = hbm_pa + kUncompressedDataOffset;
  d.dst = hbm_pa + kCompressedDataOffset;
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - 8;
  d.status_data = 0x1234;
  InvalidateHdrInHBM();
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  if (run_cp_test(&d, true, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_to_flat_64K_buf_in_hbm() {
  write_mem(hbm_pa + kUncompressedDataOffset, all_zeros, 16);
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.dst = hbm_pa + kUncompressedDataOffset;
  d.src = hbm_pa + kCompressedDataOffset;
  d.datain_len = compressed_data_size;
  d.threshold_len = 0;  // 64K, output len
  d.status_data = 0x3456;
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x3456;
  if (run_dc_test(&d, true, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

// Route the compressed output through sequencer to handle output block
// boundry issues of compression engine.
int compress_output_through_sequencer() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.src = host_mem_pa + kUncompressedDataOffset;
  d.dst = hbm_pa + kCompressedDataOffset;
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - 8;
  d.status_data = 0x1234;
  InvalidateHdrInHBM();
  InvalidateHdrInHostMem();
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;

  // Prepare an SGL for the sequencer to output data.
  cp_sq_ent_sgl_t seq_sgl;
  bzero(&seq_sgl, sizeof(seq_sgl));
  seq_sgl.status_host_pa = host_mem_pa + kStatusOffset;
  bzero(host_mem + kStatusOffset, sizeof(cp_status_sha512_t));
  seq_sgl.addr[0] = host_mem_pa + kCompressedDataOffset;
  seq_sgl.len[0] = 199;
  seq_sgl.addr[1] = host_mem_pa + kCompressedDataOffset + 199;
  seq_sgl.len[1] = 537;
  seq_sgl.addr[2] = host_mem_pa + kCompressedDataOffset + 199 + 537;
  seq_sgl.len[2] = 1123;
  seq_sgl.addr[3] = host_mem_pa + kCompressedDataOffset + 199 + 537 + 1123;
  seq_sgl.len[3] = kCompressedBufSize - (199 + 537 + 1123);
  write_mem(hbm_pa + kSGLOffset, (uint8_t *)&seq_sgl, sizeof(seq_sgl));

  const uint32_t kIntrData = 0x11223344;
  cp_seq_params_t seq_params;
  bzero(&seq_params, sizeof(seq_params));
  seq_params.seq_ent.status_hbm_pa = hbm_pa + kStatusOffset;
  seq_params.seq_ent.src_hbm_pa = hbm_pa + kCompressedDataOffset;
  seq_params.seq_ent.sgl_pa = hbm_pa + kSGLOffset;
  seq_params.seq_ent.intr_pa = host_mem_pa + kOpaqueTagOffset;
  seq_params.seq_ent.intr_data = kIntrData;
  bzero(host_mem + kOpaqueTagOffset, 8);  // Clear the area where interrupt from sequencer is going to come.
  seq_params.seq_ent.status_len = sizeof(cp_status_sha512_t);
  seq_params.seq_ent.status_dma_en = 1;
  seq_params.seq_ent.intr_en = 1;
  seq_params.seq_index = 0;
  if (test_setup_cp_seq_ent(&seq_params) != 0) {
    printf("cp_seq_ent failed\n");
    return -1;
  }
  d.doorbell_addr = seq_params.ret_doorbell_addr;
  d.doorbell_data = seq_params.ret_doorbell_data;
  d.cmd_bits.doorbell_on = 1;

  // Verify(wait for) that the status makes it to HBM.
  // We could directly poll for sequencer but this is just additional verification.
  if (run_cp_test(&d, true, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Now poll for sequencer interrupt.
  auto seq_intr_poll_func = [kIntrData] () -> int {
    uint32_t *p = (uint32_t *)(host_mem + kOpaqueTagOffset);
    if (*p == kIntrData)
      return 0;
    return 1;
  };
  tests::Poller intr_poll;
  if (intr_poll(seq_intr_poll_func) != 0) {
    printf("ERROR: Interrupt from sequencer never came.\n");
  }
  // By this time we should have data and status.
  cp_status_sha512_t *st2 = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  if (!st2->valid) {
    printf("ERROR: Status valid bit not set\n");
    return -1;
  }
  if (st2->err) {
    printf("ERROR: status err 0x%x is unexpected\n", st2->err);
    return -1;
  }
  if (st2->partial_data != 0x1234) {
    printf("ERROR: status partial data 0x%x is unexpected\n", st2->partial_data);
    return -1;
  }
  cp_hdr_t *hdr = (cp_hdr_t *)(host_mem + kCompressedDataOffset);
  if (hdr->version != kCPVersion) {
    printf("Header version mismatch, expected 0x%x, received 0x%x\n",
           kCPVersion, hdr->version);
    return -1;
  }
  if (hdr->cksum == 0) {
    printf("ERROR: Header with zero cksum\n");
    return -1;
  }
  uint32_t datain_len = 65536;
  if ((hdr->data_len == 0) || (hdr->data_len > (kCompressedBufSize - 8))
      || (hdr->data_len > datain_len)) {
    printf("ERROR: Invalid data_len 0x%x\n", hdr->data_len);
    return -1;
  }
  if (st2->output_data_len != (hdr->data_len + 8)) {
    printf("ERROR: output_data_len %u does not match hdr+8 %u\n",
           st2->output_data_len, hdr->data_len + 8);
    return -1;
  }
  if ((st2->err == 0) && (st2->integrity_data == 0)) {
    printf("Integrity is all zero\n");
    return -1;
  }
  if (bcmp(host_mem + kCompressedDataOffset, compressed_data_buf, st2->output_data_len) != 0) {
    printf("ERROR: compressed data size mismatch\n");
    return -1;
  }
  // Status verification done.

  printf("Testcase %s passed\n", __func__);
  return 0;
}

// Verify integrity of >64K buffer
int verify_integrity_for_gt64K() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.src = host_mem_pa + kUncompressedDataOffset;
  // Give 68K length
  d.datain_len = 4096;
  d.extended_len = 1;
  d.status_data = 0x7654;
  cp_status_no_hash_t st = {0};
  st.partial_data = 0x7654;
  if (run_cp_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  cp_status_sha512_t *st2 = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  uint64_t old_integiry = st2->integrity_data;

  // Now repeat the test using only 4K data i.e. higher 16 bits are zero.
  bzero(&d, sizeof(d));
  d.src = host_mem_pa + kUncompressedDataOffset;
  d.datain_len = 4096;
  d.status_data = 0x5454;
  st = {0};
  st.partial_data = 0x5454;
  if (run_cp_test(&d, false, st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  if (st2->integrity_data == old_integiry) {
    printf("ERROR: Same integrity data for different data sizes\n");
    return -1;
  }

  printf("Testcase %s passed\n", __func__);
  return 0;
}

int max_data_rate() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  cp_desc_t *dst_d = (cp_desc_t *)cp_queue_mem;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.opaque_tag_on = 1;
  d.src = hbm_pa + kUncompressedDataOffset;
  d.dst = hbm_pa + kUncompressedDataOffset + 4096;
  d.status_addr = hbm_pa + kStatusOffset;
  d.datain_len = 4096;
  d.threshold_len = 4096 - 8;;
  d.status_data = 0x1234;
  // We will use 4K bytes at host_mem + kUncompressedDataOffset + (2 * 4096) to store
  // all interrupts (opaque tags).
  bzero(host_mem + kUncompressedDataOffset + (2 * 4096), 4096);
  for (uint32_t i = 0; i < 16; i++) {
    d.opaque_tag_addr = host_mem_pa + kUncompressedDataOffset + (2 * 4096) + i*8;
    // Why add 0x10000? mem is init to 0 and 1st tag is also zero.
    d.opaque_tag_data = 0x10000 + i;
    bcopy(&d, &dst_d[cp_queue_index], sizeof(d));
    cp_queue_index++;
    if (cp_queue_index == 4096)
      cp_queue_index = 0;
  }
  // Dont ring the doorbell yet.
  // Add descriptors for decompression also.
  dst_d = (cp_desc_t *)dc_queue_mem;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.opaque_tag_on = 1;
  d.src = hbm_pa + kCompressedDataOffset;
  d.dst = hbm_pa + kUncompressedDataOffset + (4096*4);
  d.status_addr = hbm_pa + kStatusOffset;
  d.datain_len = 4096;
  d.threshold_len = 4096*4;
  d.status_data = 0x4321;
  for (uint32_t i = 0; i < 2; i++) {
    d.opaque_tag_addr = host_mem_pa + kUncompressedDataOffset +
                        (2 * 4096) + 2048 + i*8;
    d.opaque_tag_data = 0x10000 + i;
    bcopy(&d, &dst_d[dc_queue_index], sizeof(d));
    dc_queue_index++;
    if (dc_queue_index == 4096)
      dc_queue_index = 0;
  }

  // Now ring doorbells
  write_reg(cp_cfg_q_pd_idx, cp_queue_index);
  write_reg(dc_cfg_q_pd_idx, dc_queue_index);

  // Wait for all the interrupts
  auto func = [] () -> int {
    uint64_t *intr_ptr = (uint64_t *)(host_mem + kUncompressedDataOffset + (2 * 4096));
    for (int i = 0; i < 16; i++) {
      if (intr_ptr[i] == 0)
        return 1;
    }
    intr_ptr = (uint64_t *)(host_mem + kUncompressedDataOffset + (2 * 4096) + 2048);
    for (int i = 0; i < 2; i++) {
      if (intr_ptr[i] == 0)
        return 1;
    }
    return 0;
  };
  tests::Poller poll;
  if (poll(func) == 0) {
    printf("Testcase %s passed\n", __func__);
    return 0;
  }
  uint64_t *intr_ptr = (uint64_t *)(host_mem + kUncompressedDataOffset + (2 * 4096));
  for (int i = 0; i < 16; i++) {
    if (intr_ptr[i] == 0)
      printf("Compression request %d did not complete\n", i);
  }
  intr_ptr = (uint64_t *)(host_mem + kUncompressedDataOffset + (2 * 4096) + 2048);
  for (int i = 0; i < 2; i++) {
    if (intr_ptr[i] == 0)
      printf("Decompression request %d did not complete\n", i);
  }
  printf("ERROR: Timed out waiting for all the commands to complete\n");
  return -1;
}

static int cp_dualq_flat_64K_buf(uint64_t mem_pa, bool in_hbm) {
  // Setup compression descriptor for low prirority queue
  cp_desc_t lq_desc;
  bzero(&lq_desc, sizeof(lq_desc));

  lq_desc.cmd_bits.comp_decomp_en = 1;
  lq_desc.cmd_bits.insert_header = 1;
  lq_desc.cmd_bits.sha_en = 1;

  lq_desc.src = mem_pa + kUncompressedDataOffset;
  lq_desc.dst = mem_pa + kCompressedDataOffset;
  lq_desc.datain_len = 4096;
  lq_desc.threshold_len = 4096 - 8;

  lq_desc.status_addr = mem_pa + kStatusOffset;
  lq_desc.status_data = 0x1234;

  // Setup compression descriptor for high prirority queue
  cp_desc_t hq_desc;
  bzero(&hq_desc, sizeof(hq_desc));

  hq_desc.cmd_bits.comp_decomp_en = 1;
  hq_desc.cmd_bits.insert_header = 1;
  hq_desc.cmd_bits.sha_en = 1;

  hq_desc.src = mem_pa + kUncompressedDataOffset + 4096;
  hq_desc.dst = mem_pa + kCompressedDataOffset + 4096;
  hq_desc.datain_len = 4096;
  hq_desc.threshold_len = 4096 - 8;

  hq_desc.status_addr = mem_pa + kStatusOffset +
	  sizeof(cp_status_sha512_t);
  hq_desc.status_data = 0x5678;

  // Initialize status for both the requests
  cp_status_sha512_t *s;
  s = (cp_status_sha512_t *) (host_mem + kStatusOffset);
  bzero(s, sizeof(*s));

  s = (cp_status_sha512_t *) (host_mem + kStatusOffset +
		  sizeof(cp_status_sha512_t));
  bzero(s, sizeof(*s));

  // Add descriptor for both high and low priority queue
  cp_desc_t *dst_desc;
  dst_desc = (cp_desc_t *) cp_queue_mem;
  bcopy(&lq_desc, &dst_desc[cp_queue_index], sizeof(lq_desc));
  cp_queue_index++;
  if (cp_queue_index == 4096)
      cp_queue_index = 0;

  // Dont ring the doorbell yet
  dst_desc = (cp_desc_t *) cp_hotq_mem;
  bcopy(&hq_desc, &dst_desc[cp_hotq_index], sizeof(hq_desc));
  cp_hotq_index++;
  if (cp_hotq_index == 4096)
      cp_hotq_index = 0;

  // Now ring door bells for both high and low queue
  write_reg(cp_cfg_q_pd_idx, cp_queue_index);
  write_reg(cp_cfg_hotq_pd_idx, cp_hotq_index);

  // Check status update to both the descriptors
  auto func = [mem_pa, in_hbm] () -> int {
    cp_status_sha512_t *s;
    s = (cp_status_sha512_t *) (host_mem + kStatusOffset);
    if (in_hbm) {
	    read_mem(mem_pa + kStatusOffset, (uint8_t *)s,
			    sizeof(cp_status_sha512_t));
    }
    if (!s->valid) {
      printf("Compression request in low queue did not complete "
	  "status: %llx\n", *((unsigned long long *) s));
      return -1;
    }

    s = (cp_status_sha512_t *) (host_mem + kStatusOffset +
				sizeof(cp_status_sha512_t));
    if (in_hbm) {
	    read_mem(mem_pa + kStatusOffset + sizeof(cp_status_sha512_t),
		    (uint8_t *)s, sizeof(cp_status_sha512_t));
    }
    if (!s->valid) {
      printf("Compression request in high/hot queue did not complete "
	  "status: %llx\n", *((unsigned long long *) s));
      return -1;
    }

    return 0;
  };

  tests::Poller poll;
  if (poll(func) == 0) {
    printf("Testcase %s passed\n", __func__);
    return 0;
  }

  return -1;
}

int compress_dualq_flat_64K_buf() {
  printf("Starting testcase %s\n", __func__);

  InvalidateHdrInHostMem();

  int rc = cp_dualq_flat_64K_buf(host_mem_pa, false);
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

int compress_dualq_flat_64K_buf_in_hbm() {
  printf("Starting testcase %s\n", __func__);

  InvalidateHdrInHBM();

  int rc = cp_dualq_flat_64K_buf(hbm_pa, true);
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

}  // namespace tests
