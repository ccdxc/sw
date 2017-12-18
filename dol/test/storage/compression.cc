// Compression DOLs.
#include "dol/test/storage/compression.hpp"
#include "dol/test/storage/compression_test.hpp"
#include "dol/test/storage/tests.hpp"
#include "dol/test/storage/utils.hpp"
#include "nic/asic/capri/design/common/cap_addr_define.h"
#include "nic/asic/capri/model/cap_he/readonly/cap_hens_csr_define.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <byteswap.h>
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

namespace tests {

static const uint64_t cfg_glob = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_GLB_BYTE_ADDRESS;

static const uint64_t cfg_dist = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS;

static const uint64_t cfg_ueng = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_UENG_W0_BYTE_ADDRESS;

static const uint64_t cfg_q_base = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_BASE_ADR_W0_BYTE_ADDRESS;

static const uint64_t cfg_hotq_base = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_BASE_ADR_W0_BYTE_ADDRESS;

static const uint64_t cfg_q_pd_idx = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_PD_IDX_BYTE_ADDRESS;

static const uint64_t cfg_hotq_pd_idx = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_PD_IDX_BYTE_ADDRESS;

static const uint64_t cfg_host = CAP_ADDR_BASE_MD_HENS_OFFSET +
    CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOST_BYTE_ADDRESS;

static const uint32_t kNumSubqEntries = 4096;
static const uint32_t kQueueMemSize = sizeof(cp_desc_t) * kNumSubqEntries;
static void *queue_mem;
static uint64_t queue_mem_pa;
static uint16_t queue_index = 0;

static const uint32_t kStatusBufSize = 4096;
static void *status_buf;
static uint64_t status_buf_pa;

static const uint32_t kDatainBufSize = 4096;
static void *datain_buf;
static uint64_t datain_buf_pa;

// Size defined in compression_test.hpp
static void *dataout_buf;
static uint64_t dataout_buf_pa;

static const uint32_t kSGLBufSize = 4096;
static uint8_t *sgl_buf;
static uint64_t sgl_buf_pa;

static uint64_t hbm_datain_buf_pa;
static uint64_t hbm_dataout_buf_pa;
static uint64_t hbm_status_buf_pa;
static uint64_t hbm_sgl_buf_pa;

static bool status_poll(bool in_hbm) {
  cp_status_sha512_t *s = (cp_status_sha512_t *)status_buf;
  for (int i = 0; i < 1000; i++) {
    if (in_hbm) {
      read_mem(hbm_status_buf_pa, (uint8_t *)status_buf, kStatusBufSize);
    }
    if (s->valid) {
      if (in_hbm) {
        usleep(100);
        // Do it once more as status is copied 1st and DB is rung next.
        read_mem(hbm_status_buf_pa, (uint8_t *)status_buf, kStatusBufSize);
      }
      printf("Got status %llx\n", *((unsigned long long *)s));
      return true;
    }
    usleep(10000);
  }
  return false;
}

void
compression_init()
{
  queue_mem = alloc_page_aligned_host_mem(kQueueMemSize);
  assert(queue_mem != nullptr);
  queue_mem_pa = host_mem_v2p(queue_mem);

  status_buf = alloc_page_aligned_host_mem(kStatusBufSize);
  assert(status_buf != nullptr);
  status_buf_pa = host_mem_v2p(status_buf);

  datain_buf = alloc_page_aligned_host_mem(kDatainBufSize);
  assert(datain_buf != nullptr);
  datain_buf_pa = host_mem_v2p(datain_buf);

  dataout_buf = alloc_page_aligned_host_mem(kDataoutBufSize);
  assert(dataout_buf != nullptr);
  dataout_buf_pa = host_mem_v2p(dataout_buf);

  sgl_buf = (uint8_t *)alloc_page_aligned_host_mem(kSGLBufSize);
  assert(sgl_buf != nullptr);
  sgl_buf_pa = host_mem_v2p(sgl_buf);

  assert(utils::hbm_addr_alloc_page_aligned(kDatainBufSize, &hbm_datain_buf_pa) == 0);
  assert(utils::hbm_addr_alloc_page_aligned(kDataoutBufSize, &hbm_dataout_buf_pa) == 0);
  assert(utils::hbm_addr_alloc_page_aligned(kStatusBufSize, &hbm_status_buf_pa) == 0);
  assert(utils::hbm_addr_alloc_page_aligned(kSGLBufSize, &hbm_sgl_buf_pa) == 0);

  // Pre-fill input buffers.
  bcopy(uncompressed_data, datain_buf, kDatainBufSize);
  write_mem(hbm_datain_buf_pa, (uint8_t *)datain_buf, kDatainBufSize);

  // Write queue base.
  uint32_t cfg_glob_data;
  read_reg(cfg_glob, cfg_glob_data);
  write_reg(cfg_glob, (cfg_glob_data & 0xFFFF0000u) | kCPVersion);
  write_reg(cfg_q_base, queue_mem_pa & 0xFFFFFFFFu);
  write_reg(cfg_q_base + 4, (queue_mem_pa >> 32) & 0xFFFFFFFFu);
  // Enable all 16 engines.
  write_reg(cfg_ueng, 0xFFFF);
  // Enable cold/warm queue.
  write_reg(cfg_dist, 0x1);

  queue_index = 0;

  printf("Compression init done\n");
}

static void populate_sgls(uint16_t data_len, uint16_t num_entries,
                          void *sgl_va_in, uint64_t sgl_pa, uint64_t src_pa) {
  bool sgl_in_host_mem = sgl_pa & 0x8000000000000000ull;
  uint16_t chunk_size = data_len / num_entries;
  uint16_t last_chunk_size = chunk_size + (data_len - (chunk_size * num_entries));
  assert(chunk_size > 0);
  uint16_t count = num_entries;
  cp_sgl_t *sgl_va = (cp_sgl_t *)sgl_va_in;
  cp_sgl_t sgl;
  while (count > 0) {
    bzero(&sgl, sizeof(sgl));
    sgl.link = sgl_pa + sizeof(cp_sgl_t);
    sgl.addr0 = src_pa;
    sgl.len0 = (count == 1) ? last_chunk_size : chunk_size;
    src_pa += sgl.len0;
    count--;
    if (count == 0)
      break;

    sgl.addr1 = src_pa;
    sgl.len1 = (count == 1) ? last_chunk_size : chunk_size;
    src_pa += sgl.len1;
    count--;
    if (count == 0)
      break;

    sgl.addr2 = src_pa;
    sgl.len2 = (count == 1) ? last_chunk_size : chunk_size;
    src_pa += sgl.len2;
    count--;
    if (count == 0)
      break;
    if (sgl_in_host_mem) {
      bcopy(&sgl, sgl_va, sizeof(sgl));
      sgl_va++;
    } else {
      write_mem(sgl_pa, (uint8_t *)&sgl, sizeof(sgl));
    }
    sgl_pa = sgl.link;
  }
  sgl.link = 0;
  if (sgl_in_host_mem) {
    bcopy(&sgl, sgl_va, sizeof(sgl));
  } else {
    write_mem(sgl_pa, (uint8_t *)&sgl, sizeof(sgl));
  }
}

static int run_cp_test(comp_test_t *params) {
  static uint32_t counter = 0;
  counter++;
  cp_desc_t d;
  bzero(&d, sizeof(d));

  printf("Starting testcase %s\n", params->test_name.c_str());
  d.cmd = params->cmd;
  uint64_t bufin_pa = params->src_is_hbm ? hbm_datain_buf_pa : datain_buf_pa;
  if (params->num_src_sgls == 1) {
    d.src = bufin_pa;
  } else {
    if (params->src_sgl_is_hbm) {
      populate_sgls(params->datain_len, params->num_src_sgls, nullptr,
                    hbm_sgl_buf_pa, bufin_pa);
      d.src = hbm_sgl_buf_pa;
    } else {
      populate_sgls(params->datain_len, params->num_src_sgls, sgl_buf,
                    sgl_buf_pa, bufin_pa);
      d.src = sgl_buf_pa;
    }
    d.cmd_bits.src_is_list = 1;
  }

  uint64_t bufout_pa = params->dst_is_hbm ? hbm_dataout_buf_pa : dataout_buf_pa;
  bzero(dataout_buf, kDataoutBufSize);
  if (params->dst_is_hbm) {
    write_mem(hbm_dataout_buf_pa, all_zeros, kDataoutBufSize);
  }
  if (params->num_dst_sgls == 1) {
    d.dst = bufout_pa;
  } else {
    if (params->dst_sgl_is_hbm) {
      populate_sgls(params->dataout_len, params->num_dst_sgls, nullptr,
                    hbm_sgl_buf_pa + 2048, bufout_pa);
      d.dst = hbm_sgl_buf_pa + 2048;
    } else {
      populate_sgls(params->dataout_len, params->num_dst_sgls, sgl_buf + 2048,
                    sgl_buf_pa + 2048, bufout_pa);
      d.dst = sgl_buf_pa + 2048;
    }
    d.cmd_bits.dst_is_list = 1;
  }

  bzero(status_buf, kStatusBufSize);
  if (params->status_is_hbm) {
    write_mem(hbm_status_buf_pa, all_zeros, kStatusBufSize);
  }
  const uint64_t kDBData = 0x11223344556677ull;
  const uint32_t kTagData = 0x8899aabbu;
  d.input_len = params->datain_len;
  d.expected_len = params->dataout_len - 8;
  d.status_addr = params->status_is_hbm ? hbm_status_buf_pa : status_buf_pa;
  d.doorbell_addr = d.status_addr + 1024;
  d.doorbell_data = kDBData;
  d.opaque_tag_addr = d.status_addr + 2048;
  d.opaque_tag_data = kTagData;
  d.status_data = counter;

  cp_desc_t *dst_d = (cp_desc_t *)queue_mem;
  bcopy(&d, &dst_d[queue_index], sizeof(d));
  queue_index++;
  if (queue_index == 4096)
    queue_index = 0;
  write_reg(cfg_q_pd_idx, queue_index);

  cp_status_sha512_t *st = (cp_status_sha512_t *)status_buf;
  if (!status_poll(params->status_is_hbm)) {
    printf("ERROR: Compression status never came\n");
    return -1;
  }
  if (!st->valid) {
    printf("ERROR: Compression valid bit not set\n");
    return -1;
  }
  if (st->err) {
    printf("ERROR: Compression generated err = 0x%x\n", st->err);
    return -1;
  }
  uint16_t expected_data_len = kCompressedDataSize;
  if (params->cmd_bits.insert_header) {
    expected_data_len += 8;
  }
  if (st->output_data_len != expected_data_len) {
    printf("ERROR: output data len mismatch, expected %u, received %u\n",
           expected_data_len, st->output_data_len);
    return -1;
  }
  if (params->dst_is_hbm) {
    read_mem(hbm_dataout_buf_pa, (uint8_t *)dataout_buf, expected_data_len);
  }
  uint8_t *data_start = (uint8_t *)dataout_buf;
  if (params->cmd_bits.insert_header) {
    // HACK, temporary fix until model/RTL is fixed.
    //printf("HACK!! Swapping the header while waiting for model/RTL to get fixed\n");
    //uint64_t *h = (uint64_t *)dataout_buf;
    //*h = bswap_64(*h);
    cp_hdr_t *hdr = (cp_hdr_t *)dataout_buf;
    if (params->cmd_bits.cksum_en) {
      uint32_t expected_cksum =
          params->cmd_bits.cksum_adler ? kADLER32Sum : kCRC32Sum;
      if (hdr->cksum != expected_cksum) {
        printf("ERROR: CKSUM mismatch. Expected 0x%x, Got 0x%x\n",
               expected_cksum, hdr->cksum);
        return -1;
      }
    }
    if (hdr->data_len != kCompressedDataSize) {
      printf("ERROR: wrong data len in the compression header. "
             "Expected %d, got %d\n", kCompressedDataSize, hdr->data_len);
      return -1;
    }
    if (hdr->version != kCPVersion) {
      printf("ERROR: wrong version in the compression header. "
             "Expected 0x%x, got 0x%x\n", kCPVersion, hdr->version);
      return -1;
    }
    data_start += 8;
  }
  if (st->partial_data != counter) {
    printf("ERROR: partial data in status does not match the expected value\n");
    return -1;
  }
  if (params->cmd_bits.sha_en) {
    int sha_len = params->cmd_bits.sha256 ? 32 : 64;
    uint8_t *expected_sha = params->cmd_bits.sha256 ? sha256_post : sha512_post;
    if (bcmp(st->sha512, expected_sha, sha_len) != 0) {
      printf("ERROR: SHA mismatch.\nExpected:\n");
      utils::dump(expected_sha, sha_len);
      printf("Received:\n");
      utils::dump(st->sha512, sha_len);
      return -1;
    }
  }
  if (bcmp(data_start, compressed_data, kCompressedDataSize) != 0) {
    printf("ERROR: compressed data does not match with expected output.\n");
    return -1;
  }
  uint64_t *db_data = (uint64_t *)(((uint8_t *)status_buf) + 1024);
  if (params->cmd_bits.doorbell_on) {
    if (*db_data != kDBData) {
      printf("ERROR: doorbell is not rung\n");
      return -1;
    }
  } else {
    if (*db_data == kDBData) {
      printf("ERROR: doorbell is rung when not requested\n");
      return -1;
    }
  }
  uint32_t *o = (uint32_t *)(((uint8_t *)status_buf) + 2048);
  if (params->cmd_bits.opaque_tag_on) {
    if (*o != kTagData) {
      printf("ERROR: Opaque tag not written\n");
      return -1;
    }
  } else {
    if (*o == kTagData) {
      printf("ERROR: Opaque tag is written when not requested\n");
      return -1;
    }
  }

  printf("Testcase %s passed\n", params->test_name.c_str());
  return 0;
}

int compress_host_flat() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;

  return run_cp_test(&spec);
}

int compress_hbm_flat() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.src_is_hbm   = 1;
  spec.dst_is_hbm   = 1;

  return run_cp_test(&spec);
}

int compress_host_to_hbm_flat() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.dst_is_hbm   = 1;

  return run_cp_test(&spec);
}

int compress_hbm_to_host_flat() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.src_is_hbm   = 1;

  return run_cp_test(&spec);
}

int compress_host_sgl() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 3;
  spec.num_dst_sgls = 2;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;

  return run_cp_test(&spec);
}

int compress_hbm_sgl() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 3;
  spec.num_dst_sgls = 2;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.src_is_hbm   = 1;
  spec.dst_is_hbm   = 1;

  return run_cp_test(&spec);
}

int compress_host_nested_sgl() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 4;
  spec.num_dst_sgls = 4;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;

  return run_cp_test(&spec);
}

int compress_hbm_nested_sgl() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 4;
  spec.num_dst_sgls = 4;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.src_is_hbm   = 1;
  spec.dst_is_hbm   = 1;

  return run_cp_test(&spec);
}

int compress_nested_sgl_in_hbm() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 4;
  spec.num_dst_sgls = 4;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.src_is_hbm   = 1;
  spec.src_sgl_is_hbm   = 1;
  spec.dst_is_hbm   = 1;
  spec.dst_sgl_is_hbm   = 1;

  return run_cp_test(&spec);
}

int compress_return_through_hbm() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.num_src_sgls = 4;
  spec.num_dst_sgls = 4;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.dst_is_hbm   = 1;
  spec.dst_sgl_is_hbm   = 1;
  spec.status_is_hbm = 1;

  return run_cp_test(&spec);
}

int compress_adler_sha256() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.cmd_bits.sha_en = 1;
  spec.cmd_bits.sha256 = 1;
  spec.cmd_bits.cksum_en = 1;
  spec.cmd_bits.cksum_adler = 1;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;

  return run_cp_test(&spec);
}

int compress_crc_sha512() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.cmd_bits.sha_en = 1;
  spec.cmd_bits.sha256 = 0;
  spec.cmd_bits.cksum_en = 1;
  spec.cmd_bits.cksum_adler = 0;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;

  return run_cp_test(&spec);
}

int compress_doorbell_odata() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 5;
  spec.cmd_bits.doorbell_on = 1;
  spec.cmd_bits.opaque_tag_on = 1;
  spec.num_src_sgls = 1;
  spec.num_dst_sgls = 1;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;

  return run_cp_test(&spec);
}

int compress_max_features() {
  comp_test_t spec;
  bzero(&spec, sizeof(spec));
  spec.test_name    = __func__;
  spec.cmd          = 0x7FD;
  spec.num_src_sgls = 7;
  spec.num_dst_sgls = 8;
  spec.datain_len   = 4096;
  spec.dataout_len  = 4096;
  spec.src_sgl_is_hbm = 1;
  spec.dst_is_hbm = 1;
  spec.status_is_hbm = 1;

  return run_cp_test(&spec);
}

// Run the compression output through sequencer.
int compress_output_through_sequencer() {
  cp_desc_t d;
  bzero(&d, sizeof(d));

  printf("Starting testcase compress_output_through_sequencer\n");
  d.cmd = 5;
  d.src = datain_buf_pa;
  d.dst = hbm_dataout_buf_pa;
  write_mem(hbm_dataout_buf_pa, all_zeros, kDataoutBufSize);
  bzero(status_buf, kStatusBufSize);
  write_mem(hbm_status_buf_pa, all_zeros, kStatusBufSize);
  d.input_len = 4096;
  d.expected_len = 4096 - 8;
  d.status_addr = hbm_status_buf_pa;

  // Lets divide the output into 4 buffers of variable size.
  // 199, 537, 1123, 2237
  cp_sq_ent_sgl_t seq_sgl;
  bzero(&seq_sgl, sizeof(seq_sgl));
  seq_sgl.status_host_pa = status_buf_pa;
  seq_sgl.addr[0] = dataout_buf_pa;
  seq_sgl.len[0] = 199;
  seq_sgl.addr[1] = dataout_buf_pa + 199;
  seq_sgl.len[1] = 537;
  seq_sgl.addr[2] = dataout_buf_pa + 199 + 537;
  seq_sgl.len[2] = 1123;
  seq_sgl.addr[3] = dataout_buf_pa + 199 + 537 + 1123;
  seq_sgl.len[3] = 2237;
  write_mem(hbm_sgl_buf_pa, (uint8_t *)&seq_sgl, sizeof(seq_sgl));

  cp_seq_params_t seq_params;
  bzero(&seq_params, sizeof(seq_params));
  seq_params.seq_ent.status_hbm_pa = hbm_status_buf_pa;
  seq_params.seq_ent.src_hbm_pa = hbm_dataout_buf_pa;
  seq_params.seq_ent.sgl_pa = hbm_sgl_buf_pa;
  seq_params.seq_ent.intr_pa = status_buf_pa + 1024;
  seq_params.seq_ent.intr_data = 0x11223344;
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
  d.cmd_bits.sha_en = 1;
  d.cmd_bits.cksum_en = 1;
  cp_desc_t *dst_d = (cp_desc_t *)queue_mem;
  bcopy(&d, &dst_d[queue_index], sizeof(d));
  queue_index++;
  if (queue_index == 4096)
    queue_index = 0;
  write_reg(cfg_q_pd_idx, queue_index);
  step_doorbell(seq_params.ret_doorbell_addr, seq_params.ret_doorbell_data);

  if (!status_poll(false)) {
    printf("ERROR: Compression status never came\n");
    return -1;
  }
  cp_status_sha512_t *st = (cp_status_sha512_t *)status_buf;
  if (!st->valid) {
    printf("ERROR: Compression valid bit not set\n");
    return -1;
  }
  if (st->err) {
    printf("ERROR: Compression generated err = 0x%x\n", st->err);
    return -1;
  }
  uint16_t expected_data_len = kCompressedDataSize + 8;
  if (st->output_data_len != expected_data_len) {
    printf("ERROR: output data len mismatch, expected %u, received %u\n",
           expected_data_len, st->output_data_len);
    return -1;
  }
  uint8_t *data_start = ((uint8_t *)dataout_buf) + 8;
  if (bcmp(data_start, compressed_data, kCompressedDataSize) != 0) {
    printf("ERROR: compressed data does not match with expected output.\n");
    return -1;
  }

  int sha_len = 64;
  uint8_t *expected_sha = sha512_post;
  if (bcmp(st->sha512, expected_sha, sha_len) != 0) {
    printf("ERROR: SHA mismatch.\nExpected:\n");
    utils::dump(expected_sha, sha_len);
    printf("Received:\n");
    utils::dump(st->sha512, sha_len);
    return -1;
  }
  
  printf("testcase compress_output_through_sequencer passed\n");

  return 0;
}

}  // namespace tests
