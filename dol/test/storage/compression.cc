// Compression DOLs.
#include "dol/test/storage/compression.hpp"
#include "dol/test/storage/compression_test.hpp"
#include "dol/test/storage/utils.hpp"
#include "nic/asic/capri/design/common/cap_addr_define.h"
#include "nic/asic/capri/model/cap_he/readonly/cap_hens_csr_define.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
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

static const uint32_t kDataoutBufSize = 4096;
static void *dataout_buf;
static uint64_t dataout_buf_pa;

static const uint32_t kSGLBufSize = 4096;
static uint8_t *sgl_buf;
static uint64_t sgl_buf_pa;

static uint64_t hbm_datain_buf_pa;
static uint64_t hbm_dataout_buf_pa;
static uint64_t hbm_status_buf_pa;
static uint64_t hbm_sgl_buf_pa;

bool status_poll(cp_status_sha512_t *ret_status) {
  cp_status_sha512_t *s = (cp_status_sha512_t *)status_buf;
  for (int i = 0; i < 1000; i++) {
    if (s->valid) {
      printf("Got status %llx\n", *((unsigned long long *)s));
      bcopy(s, ret_status, sizeof (*s));
      return true;
    }
    usleep(10000);
  }
  return false;
}

void
compression_init()
{
  queue_mem = alloc_host_mem(kQueueMemSize);
  assert(queue_mem != nullptr);
  queue_mem_pa = host_mem_v2p(queue_mem);

  status_buf = alloc_host_mem(kStatusBufSize);
  assert(status_buf != nullptr);
  status_buf_pa = host_mem_v2p(status_buf);

  datain_buf = alloc_host_mem(kDatainBufSize);
  assert(datain_buf != nullptr);
  datain_buf_pa = host_mem_v2p(datain_buf);

  dataout_buf = alloc_host_mem(kDataoutBufSize);
  assert(dataout_buf != nullptr);
  dataout_buf_pa = host_mem_v2p(dataout_buf);

  sgl_buf = (uint8_t *)alloc_host_mem(kSGLBufSize);
  assert(sgl_buf != nullptr);
  sgl_buf_pa = host_mem_v2p(sgl_buf);

  assert(utils::hbm_addr_alloc(kDatainBufSize, &hbm_datain_buf_pa) == 0);
  assert(utils::hbm_addr_alloc(kDataoutBufSize, &hbm_dataout_buf_pa) == 0);
  assert(utils::hbm_addr_alloc(kStatusBufSize, &hbm_status_buf_pa) == 0);
  assert(utils::hbm_addr_alloc(kSGLBufSize, &hbm_sgl_buf_pa) == 0);

  // Pre-fill input buffers.
  bcopy(uncompressed_data, datain_buf, kDatainBufSize);
  write_mem(hbm_datain_buf_pa, (uint8_t *)datain_buf, kDatainBufSize);

  // Write queue base.
  write_reg(cfg_q_base, queue_mem_pa & 0xFFFFFFFFu);
  write_reg(cfg_q_base + 4, (queue_mem_pa >> 32) & 0xFFFFFFFFu);
  // Enable all 16 engines.
  write_reg(cfg_ueng, 0xFFFF);
  // Enable cold/warm queue.
  write_reg(cfg_dist, 0x1);

  queue_index = 0;

  printf("Compression init done\n");
}

void populate_sgls(uint16_t data_len, uint16_t num_entries,
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
    sgl_pa += sizeof(cp_sgl_t);
    sgl.link = sgl_pa;
  }
  if (sgl_in_host_mem) {
    bcopy(&sgl, sgl_va, sizeof(sgl));
  } else {
    write_mem(sgl_pa, (uint8_t *)&sgl, sizeof(sgl));
  }
}

int run_cp_test(comp_test_t *params) {
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
    populate_sgls(params->datain_len, params->num_src_sgls, sgl_buf,
                  sgl_buf_pa, bufin_pa);
    d.src = sgl_buf_pa;
    d.cmd_bits.src_is_list = 1;
  }

  uint64_t bufout_pa = params->src_is_hbm ? hbm_dataout_buf_pa : dataout_buf_pa;
  if (params->num_dst_sgls == 1) {
    d.dst = bufout_pa;
  } else {
    populate_sgls(params->dataout_len, params->num_dst_sgls, sgl_buf + 2048,
                  sgl_buf_pa + 2048, bufout_pa);
    d.dst = sgl_buf_pa + 2048;
    d.cmd_bits.dst_is_list = 1;
  }

  const uint64_t kDBData = 0x11223344556677ull;
  const uint32_t kTagData = 0x8899aabbu;
  d.input_len = params->datain_len;
  d.expected_len = params->dataout_len;
  bzero(status_buf, kStatusBufSize);
  d.status_addr = status_buf_pa;
  d.doorbell_addr = status_buf_pa + 1024;
  d.doorbell_data = kDBData;
  d.opaque_tag_addr = status_buf_pa + 2048;
  d.opaque_tag_data = kTagData;
  d.status_data = counter;

  cp_desc_t *dst_d = (cp_desc_t *)queue_mem;
  bcopy(&d, &dst_d[queue_index], sizeof(d));
  queue_index++;
  if (queue_index == 4096)
    queue_index = 0;
  write_reg(cfg_q_pd_idx, queue_index);

  cp_status_sha512_t st;
  if (!status_poll(&st)) {
    printf("ERROR: Compression status never came\n");
    return -1;
  }
  if (!st.valid) {
    printf("ERROR: Compression valid bit not set\n");
    return -1;
  }
  if (st.err) {
    printf("ERROR: Compression generated err = 0x%x\n", st.err);
    return -1;
  }
  uint16_t expected_data_len = kCompressedDataSize;
  if (params->cmd_bits.insert_header) {
    expected_data_len += 8;
  }
  if (st.output_data_len != expected_data_len) {
    printf("ERROR: output data len mismatch, expected %u, received %u\n",
           expected_data_len, st.output_data_len);
    return -1;
  }
  if (params->dst_is_hbm) {
    read_mem(hbm_dataout_buf_pa, (uint8_t *)dataout_buf, expected_data_len);
  }
  uint8_t *data_start = (uint8_t *)dataout_buf;
  if (params->cmd_bits.insert_header) {
    if (bcmp(dataout_buf, compression_hdr, 8) != 0) {
      printf("ERROR: compression hdr mismatch\n");
      printf("Expected : "); utils::dump(compression_hdr, 8);
      printf("Received : "); utils::dump((uint8_t *)dataout_buf, 8);
      return -1;
    }
    data_start += 8;
  }
  if (st.partial_data != counter) {
    printf("ERROR: partial data in status does not match the expected value\n");
    return -1;
  }
  if (bcmp(data_start, compressed_data, kCompressedDataSize) != 0) {
    printf("ERROR: compressed data does not match with expected output.\n");
    return -1;
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

}  // namespace tests
