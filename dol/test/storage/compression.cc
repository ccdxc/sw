// Compression DOLs.
#include <math.h>
#include "compression.hpp"
#include "compression_test.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include "queues.hpp"
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

// compression/decompression blocks initialized by HAL
// or by this DOL module.
static const bool comp_inited_by_hal = true;

static const uint32_t kNumSubqEntries = 1024;
static const uint32_t kQueueMemSize = sizeof(cp_desc_t) * kNumSubqEntries;

typedef enum {
  COMP_QUEUE_PUSH_INVALID,
  COMP_QUEUE_PUSH_SEQUENCER,
  COMP_QUEUE_PUSH_SEQUENCER_BATCH,
  COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST,
  COMP_QUEUE_PUSH_HW_DIRECT,
  COMP_QUEUE_PUSH_HW_DIRECT_BATCH,
} comp_queue_push_t;

// comp_queue_t provides usage flexibility as follows:
// - HW queue configuration performed by this DOL module or elsewhere (such as HAL)
// - queue entry submission via sequencer or directly to HW producer register
typedef struct {
  uint64_t  cfg_q_base;
  uint64_t  cfg_q_pd_idx;
  cp_desc_t *q_base_mem;
  uint64_t  q_base_mem_pa;

  uint32_t  curr_seq_comp_qid;
  uint16_t  curr_seq_comp_pd_idx;
  uint16_t  curr_pd_idx;
  comp_queue_push_t curr_push_type;
} comp_queue_t;

static comp_queue_t cp_queue;
static comp_queue_t dc_queue;

static comp_queue_t cp_hotq;
static comp_queue_t dc_hotq;

// Sample data generated during test.
static const uint8_t all_zeros[65536] = {0};
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

static uint8_t *host_mem;
static uint64_t host_mem_pa;
static uint64_t hbm_pa;

// Forward declaration with default param values
int run_cp_test(cp_desc_t *desc, bool status_in_hbm, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type = COMP_QUEUE_PUSH_HW_DIRECT,
                uint32_t seq_comp_qid = 0);
int run_dc_test(cp_desc_t *desc, bool status_in_hbm, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type = COMP_QUEUE_PUSH_HW_DIRECT,
                uint32_t seq_comp_qid = 0);

// Write zeros to the 1st 8 bytes of compressed data buffer in hostmem.
static void InvalidateHdrInHostMem() {
  *((uint64_t *)(host_mem + kCompressedDataOffset)) = 0;
}

// Write zeros to the 1st 8 bytes of compressed data buffer in hbm.
static void InvalidateHdrInHBM() {
  write_mem(hbm_pa + kCompressedDataOffset, (uint8_t *) all_zeros, 8);
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

uint64_t
queue_mem_pa_get(uint64_t reg_addr)
{
    uint32_t lo_val, hi_val;
    uint64_t full_val;

    read_reg(reg_addr, lo_val);
    read_reg(reg_addr + 4, hi_val);

    full_val = ((uint64_t)hi_val << 32) | lo_val;
    printf("%s 0x%lx\n", __FUNCTION__, full_val);
    return full_val;
}


void
comp_queue_alloc(comp_queue_t &comp_queue,
                 uint64_t cfg_q_base,
                 uint64_t cfg_q_pd_idx)
{
    memset(&comp_queue, 0, sizeof(comp_queue));
    comp_queue.cfg_q_base = cfg_q_base;
    comp_queue.cfg_q_pd_idx = cfg_q_pd_idx;

    // If comp was initialized by HAL, q_base_mem below would be used
    // as descriptor cache for sequencer submission.
    comp_queue.q_base_mem = (cp_desc_t *)alloc_page_aligned_host_mem(kQueueMemSize);
    assert(comp_queue.q_base_mem != nullptr);

    if (comp_inited_by_hal) {
        comp_queue.q_base_mem_pa = queue_mem_pa_get(cfg_q_base);
    } else {
        comp_queue.q_base_mem_pa = host_mem_v2p(comp_queue.q_base_mem);
    }
}

void
comp_queue_push(const cp_desc_t *src_desc,
                comp_queue_t &comp_queue,
                comp_queue_push_t push_type,
                uint32_t seq_comp_qid)
{
    cp_desc_t   *dst_desc;
    dp_mem_t    *seq_comp_desc;
    uint16_t    curr_pd_idx;

    switch (push_type) {

    case COMP_QUEUE_PUSH_SEQUENCER:
    case COMP_QUEUE_PUSH_SEQUENCER_BATCH:
    case COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST:
        curr_pd_idx = comp_queue.curr_pd_idx;
        comp_queue.curr_pd_idx = (comp_queue.curr_pd_idx + 1) % kNumSubqEntries;

        dst_desc = &comp_queue.q_base_mem[curr_pd_idx];
        memcpy(dst_desc, src_desc, sizeof(*dst_desc));

        comp_queue.curr_seq_comp_qid = seq_comp_qid;
        seq_comp_desc = queues::pvm_sq_consume_entry(comp_queue.curr_seq_comp_qid,
                                                     &comp_queue.curr_seq_comp_pd_idx);
        seq_comp_desc->clear();
        seq_comp_desc->write_bit_fields(0, 64, host_mem_v2p(dst_desc));
        seq_comp_desc->write_bit_fields(64, 32, (uint64_t)log2(sizeof(*dst_desc)));
        seq_comp_desc->write_bit_fields(96, 16, (uint64_t)log2(sizeof(uint32_t)));
        seq_comp_desc->write_bit_fields(112, 34, comp_queue.cfg_q_pd_idx);
        seq_comp_desc->write_bit_fields(146, 34, comp_queue.q_base_mem_pa);

        // Sequencer queue depth is limited and should be taken into
        // considerations when using batch mode.
        if (push_type == COMP_QUEUE_PUSH_SEQUENCER) {
            seq_comp_desc->write_thru();
            test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE,
                               comp_queue.curr_seq_comp_qid, 0,
                               comp_queue.curr_seq_comp_pd_idx);
            comp_queue.curr_push_type = COMP_QUEUE_PUSH_INVALID;
            break;
        }

        seq_comp_desc->write_bit_fields(180, 16, curr_pd_idx);
        seq_comp_desc->write_bit_fields(196, 1, 1); /* set barco_batch_mode */
        if (push_type == COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST) {
            seq_comp_desc->write_bit_fields(197, 1, 1); /* set barco_batch_last */
        }

        // defer until caller calls comp_queue_post_push()
        seq_comp_desc->write_thru();
        comp_queue.curr_push_type = push_type;
        break;

    case COMP_QUEUE_PUSH_HW_DIRECT:
    case COMP_QUEUE_PUSH_HW_DIRECT_BATCH:
        write_mem(comp_queue.q_base_mem_pa + (comp_queue.curr_pd_idx * sizeof(cp_desc_t)),
                  (uint8_t *)src_desc, sizeof(cp_desc_t));
        comp_queue.curr_pd_idx = (comp_queue.curr_pd_idx + 1) % kNumSubqEntries;
        if (push_type == COMP_QUEUE_PUSH_HW_DIRECT) {
            write_reg(comp_queue.cfg_q_pd_idx, comp_queue.curr_pd_idx);
            comp_queue.curr_push_type = COMP_QUEUE_PUSH_INVALID;
            break;
        }

        // defer until caller calls comp_queue_post_push()
        comp_queue.curr_push_type = push_type;
        break;

    default:
        printf("%s unsupported push_type %d\n", __FUNCTION__, push_type);
        assert(0);
        break;
    }
}

// Execute any deferred comp_queue_push() on the given comp_queue.
void
comp_queue_post_push(comp_queue_t &comp_queue)
{
    switch (comp_queue.curr_push_type) {

    case COMP_QUEUE_PUSH_SEQUENCER_BATCH:
    case COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST:
        test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, 
                           comp_queue.curr_seq_comp_qid, 0,
                           comp_queue.curr_seq_comp_pd_idx);
        break;

    case COMP_QUEUE_PUSH_HW_DIRECT_BATCH:
        write_reg(comp_queue.cfg_q_pd_idx, comp_queue.curr_pd_idx);
        break;

    default:
        printf("%s nothing to do for curr_push_type %d\n", __FUNCTION__,
               comp_queue.curr_push_type);
        break;
    }

    comp_queue.curr_push_type = COMP_QUEUE_PUSH_INVALID;
}

void
compression_init()
{
  comp_queue_alloc(cp_queue, cp_cfg_q_base, cp_cfg_q_pd_idx);
  comp_queue_alloc(cp_hotq, cp_cfg_hotq_base, cp_cfg_hotq_pd_idx);
  comp_queue_alloc(dc_queue, dc_cfg_q_base, dc_cfg_q_pd_idx);
  comp_queue_alloc(dc_hotq, dc_cfg_hotq_base, dc_cfg_hotq_pd_idx);

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
  if (!comp_inited_by_hal) {

      // Write cp queue base.
      read_reg(cp_cfg_glob, lo_reg);
      write_reg(cp_cfg_glob, (lo_reg & 0xFFFF0000u) | kCPVersion);
      write_reg(cp_cfg_q_base, cp_queue.q_base_mem_pa & 0xFFFFFFFFu);
      write_reg(cp_cfg_q_base + 4, (cp_queue.q_base_mem_pa >> 32) & 0xFFFFFFFFu);

      write_reg(cp_cfg_hotq_base, cp_hotq.q_base_mem_pa & 0xFFFFFFFFu);
      write_reg(cp_cfg_hotq_base + 4, (cp_hotq.q_base_mem_pa >> 32) & 0xFFFFFFFFu);

      // Write dc queue base.
      read_reg(dc_cfg_glob, lo_reg);
      write_reg(dc_cfg_glob, (lo_reg & 0xFFFF0000u) | kCPVersion);
      write_reg(dc_cfg_q_base, dc_queue.q_base_mem_pa & 0xFFFFFFFFu);
      write_reg(dc_cfg_q_base + 4, (dc_queue.q_base_mem_pa >> 32) & 0xFFFFFFFFu);

      write_reg(dc_cfg_hotq_base, dc_hotq.q_base_mem_pa & 0xFFFFFFFFu);
      write_reg(dc_cfg_hotq_base + 4, (dc_hotq.q_base_mem_pa >> 32) & 0xFFFFFFFFu);

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
  }

  printf("Compression init done\n");
}

// We only compare err and partial data from exp.
int run_cp_test(cp_desc_t *desc, bool status_in_hbm, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type, uint32_t seq_comp_qid) {
  cp_status_sha512_t *s = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  bzero(s, sizeof(*s));
  if (status_in_hbm) {
    write_mem(hbm_pa + kStatusOffset, (uint8_t *) all_zeros, sizeof(*s));
  }
  desc->status_addr = (status_in_hbm ? hbm_pa : host_mem_pa) + kStatusOffset;

  comp_queue_push(desc, cp_queue, push_type, seq_comp_qid);
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
int run_dc_test(cp_desc_t *desc, bool status_in_hbm, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type, uint32_t seq_comp_qid) {
  cp_status_sha512_t *s = (cp_status_sha512_t *)(host_mem + kStatusOffset);
  bzero(s, sizeof(*s));
  if (status_in_hbm) {
    write_mem(hbm_pa + kStatusOffset, (uint8_t *) all_zeros, sizeof(*s));
  }
  desc->status_addr = (status_in_hbm ? hbm_pa : host_mem_pa) + kStatusOffset;

  comp_queue_push(desc, dc_queue, push_type, seq_comp_qid);
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

int _compress_flat_64K_buf(comp_queue_push_t push_type,
                           uint32_t seq_comp_qid) {
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
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
  if (run_cp_test(&d, false, st, push_type, seq_comp_qid) < 0) {
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

int compress_flat_64K_buf() {
  return _compress_flat_64K_buf(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_flat_64K_buf() {
  return _compress_flat_64K_buf(COMP_QUEUE_PUSH_SEQUENCER, 
                                queues::get_pvm_seq_comp_sq(0));
}

int _compress_same_src_and_dst(comp_queue_push_t push_type,
                               uint32_t seq_comp_qid) {
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
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
  if (run_cp_test(&d, false, st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_same_src_and_dst() {
    return _compress_same_src_and_dst(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_same_src_and_dst() {
    return _compress_same_src_and_dst(COMP_QUEUE_PUSH_SEQUENCER, 
                                      queues::get_pvm_seq_comp_sq(0));
}

int _decompress_to_flat_64K_buf(comp_queue_push_t push_type,
                                uint32_t seq_comp_qid) {
  bzero(host_mem, kTotalBufSize);
  bcopy(compressed_data_buf, host_mem + kCompressedDataOffset, compressed_data_size);
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
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
  if (run_dc_test(&d, false, st, push_type, seq_comp_qid) < 0) {
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

int decompress_to_flat_64K_buf() {
    return _decompress_to_flat_64K_buf(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_decompress_to_flat_64K_buf() {
    return _decompress_to_flat_64K_buf(COMP_QUEUE_PUSH_SEQUENCER, 
                                       queues::get_pvm_seq_comp_sq(0));
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

int _compress_host_sgl_to_host_sgl(comp_queue_push_t push_type,
                                   uint32_t seq_comp_qid) {
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

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
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
  if (run_cp_test(&d, false, st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_host_sgl_to_host_sgl() {
    return _compress_host_sgl_to_host_sgl(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_host_sgl_to_host_sgl() {
    return _compress_host_sgl_to_host_sgl(COMP_QUEUE_PUSH_SEQUENCER, 
                                          queues::get_pvm_seq_comp_sq(0));
}

int _decompress_host_sgl_to_host_sgl(comp_queue_push_t push_type,
                                     uint32_t seq_comp_qid) {
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

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
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
  if (run_dc_test(&d, false, st, push_type, seq_comp_qid) < 0) {
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

int decompress_host_sgl_to_host_sgl() {
    return _decompress_host_sgl_to_host_sgl(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_decompress_host_sgl_to_host_sgl() {
    return _decompress_host_sgl_to_host_sgl(COMP_QUEUE_PUSH_SEQUENCER, 
                                            queues::get_pvm_seq_comp_sq(0));
}

int _compress_flat_64K_buf_in_hbm(comp_queue_push_t push_type,
                                  uint32_t seq_comp_qid) {
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);
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
  if (run_cp_test(&d, true, st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_flat_64K_buf_in_hbm() {
    return _compress_flat_64K_buf_in_hbm(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_flat_64K_buf_in_hbm() {
    return _compress_flat_64K_buf_in_hbm(COMP_QUEUE_PUSH_SEQUENCER, 
                                         queues::get_pvm_seq_comp_sq(0));
}

int _decompress_to_flat_64K_buf_in_hbm(comp_queue_push_t push_type,
                                       uint32_t seq_comp_qid) {
  write_mem(hbm_pa + kUncompressedDataOffset, (uint8_t *) all_zeros, 16);
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);
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
  if (run_dc_test(&d, true, st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_to_flat_64K_buf_in_hbm() {
    return _decompress_to_flat_64K_buf_in_hbm(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_decompress_to_flat_64K_buf_in_hbm() {
    return _decompress_to_flat_64K_buf_in_hbm(COMP_QUEUE_PUSH_SEQUENCER, 
                                              queues::get_pvm_seq_comp_sq(0));
}

// Route the compressed output through sequencer to handle output block
// boundry issues of compression engine.
int _compress_output_through_sequencer(comp_queue_push_t push_type,
                                       uint32_t seq_comp_qid) {
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
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
  if (test_setup_cp_seq_status_ent(&seq_params) != 0) {
    printf("cp_seq_ent failed\n");
    return -1;
  }
  d.doorbell_addr = seq_params.ret_doorbell_addr;
  d.doorbell_data = seq_params.ret_doorbell_data;
  d.cmd_bits.doorbell_on = 1;

  // Verify(wait for) that the status makes it to HBM.
  // We could directly poll for sequencer but this is just additional verification.
  if (run_cp_test(&d, true, st, push_type, seq_comp_qid) < 0) {
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

int compress_output_through_sequencer() {
    return _compress_output_through_sequencer(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_output_through_sequencer() {
    return _compress_output_through_sequencer(COMP_QUEUE_PUSH_SEQUENCER, 
                                              queues::get_pvm_seq_comp_sq(0));
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


#define MAX_CP_REQ	16
#define MAX_DC_REQ	2
#define MAX_CPDC_REQ	(MAX_CP_REQ + MAX_DC_REQ)
#define TMP_BLK_SIZE	8192

/*
 * TBD: Cleanup -- following function is duplicated for quick-testing.
 * To avoid duplication, size of the buffer and buffers may need to be
 * parameterized.
 *
 */
int _max_data_rate_compress_flat_buf(comp_queue_push_t push_type,
                           uint32_t seq_comp_qid) {
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);

  cp_desc_t d;

  InvalidateHdrInHostMem();
  bzero(&d, sizeof(d));

  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;

  d.src = host_mem_pa + kUncompressedDataOffset;
  d.dst = host_mem_pa + kCompressedDataOffset;
  d.datain_len = TMP_BLK_SIZE;
  d.threshold_len = kCompressedBufSize - 8;

  d.status_data = 0x1234;

  cp_status_no_hash_t st = {0};
  st.partial_data = 0x1234;
  if (run_cp_test(&d, false, st, push_type, seq_comp_qid) < 0) {
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

int _max_data_rate(comp_queue_push_t push_type,
		uint32_t seq_comp_qid_cp,
		uint32_t seq_comp_qid_dc) {
  comp_queue_push_t last_push_type;
  cp_desc_t         d;

  _max_data_rate_compress_flat_buf(COMP_QUEUE_PUSH_HW_DIRECT, 0);

  printf("Starting testcase %s push_type %d seq_comp_qid_cp %u "
         "seq_comp_qid_dc %u\n", __func__, push_type,
         seq_comp_qid_cp, seq_comp_qid_dc);
  switch (push_type) {

  case COMP_QUEUE_PUSH_SEQUENCER_BATCH:
      last_push_type = COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST;
      break;

  default:
      last_push_type = push_type;
      break;
  }

  uint64_t host_mem_pa_ex_tmp;
  uint8_t *host_mem_ex[MAX_CPDC_REQ];
  uint8_t *host_mem_ex_tmp;
  uint64_t hbm_pa_ex[MAX_CPDC_REQ];
  uint64_t hbm_pa_ex_tmp;
  uint32_t i;

  // allocate and fill descriptors to load compression engine with requests
  for (i = 0; i < MAX_CPDC_REQ - MAX_DC_REQ; i++) {
    host_mem_ex_tmp = (uint8_t *) alloc_page_aligned_host_mem(kTotalBufSize);
    assert(host_mem_ex_tmp != nullptr);
    host_mem_pa_ex_tmp = host_mem_v2p(host_mem_ex_tmp);
    assert(utils::hbm_addr_alloc_page_aligned(kTotalBufSize, &hbm_pa_ex_tmp) == 0);

    host_mem_ex[i] = host_mem_ex_tmp;
    hbm_pa_ex[i] = hbm_pa_ex_tmp;

    bzero(&d, sizeof(d));
 
    d.cmd_bits.comp_decomp_en = 1;
    d.cmd_bits.insert_header = 1;
    d.cmd_bits.opaque_tag_on = 1;
    
    d.src = hbm_pa_ex_tmp + kUncompressedDataOffset;
    d.dst = hbm_pa_ex_tmp + kCompressedDataOffset;
    d.datain_len = TMP_BLK_SIZE;	// TBD: parameterize this
    d.threshold_len = TMP_BLK_SIZE - 8;
  
    d.status_addr = host_mem_pa_ex_tmp + kStatusOffset;
    d.status_data = 0x1234;
    
    bzero(host_mem_ex_tmp + kOpaqueTagOffset, 8);
    d.opaque_tag_addr = host_mem_pa_ex_tmp + kOpaqueTagOffset;
    d.opaque_tag_data = 0x10000 + i;
    comp_queue_push(&d, cp_queue, 
                    i == (MAX_CP_REQ - 1) ? last_push_type : push_type,
                    seq_comp_qid_cp);
  }

  // Ring the doorbell after loading decompression engine with requests
  for (; i < MAX_CPDC_REQ; i++) {
    host_mem_ex_tmp = (uint8_t *) alloc_page_aligned_host_mem(kTotalBufSize);
    assert(host_mem_ex_tmp != nullptr);
    host_mem_pa_ex_tmp = host_mem_v2p(host_mem_ex_tmp);
    assert(utils::hbm_addr_alloc_page_aligned(kTotalBufSize, &hbm_pa_ex_tmp) == 0);

    host_mem_ex[i] = host_mem_ex_tmp;
    hbm_pa_ex[i] = hbm_pa_ex_tmp;

    bzero(&d, sizeof(d));

    d.cmd_bits.comp_decomp_en = 1;
    d.cmd_bits.header_present = 1;
    d.cmd_bits.opaque_tag_on = 1;

    d.src = hbm_pa_ex_tmp + kCompressedDataOffset;
    d.dst = hbm_pa_ex_tmp + kUncompressedDataOffset;
    d.datain_len = compressed_data_size;	// predefined fixed size
    d.threshold_len = TMP_BLK_SIZE * 4;

    d.status_addr = host_mem_pa_ex_tmp + kStatusOffset;
    d.status_data = 0x4321;

    bzero(host_mem_ex_tmp + kOpaqueTagOffset, 8);
    d.opaque_tag_addr = host_mem_pa_ex_tmp + kOpaqueTagOffset;
    d.opaque_tag_data = 0x10000 + i;
    comp_queue_push(&d, dc_queue, 
		    i == (MAX_CPDC_REQ - 1) ?
		    last_push_type : push_type,
		    seq_comp_qid_dc);
  }

  // Now ring doorbells
  comp_queue_post_push(cp_queue);
  comp_queue_post_push(dc_queue);

  // Wait for all the interrupts
  auto func = [host_mem_ex, hbm_pa_ex] () -> int {
    int  i;
    uint64_t *intr_ptr;

    // NOTE: Max requests include the request count of both compression
    // and decompression
    for (i = 0; i < MAX_CPDC_REQ - MAX_DC_REQ; i++) {
      intr_ptr = (uint64_t *) (host_mem_ex[i] + kOpaqueTagOffset);
      if (intr_ptr == 0)
        return -1;

      cp_status_sha512_t *s;
      s = (cp_status_sha512_t *) (host_mem_ex[i] + kStatusOffset);
      if (!s->valid) {
	      printf("Compression status is invalid! "
			      "status: %llx\n", *((unsigned long long *) s));
	      return -1;
      }
    }

    // handle remaining decompression requests
    for (; i < MAX_CPDC_REQ; i++) {
      intr_ptr = (uint64_t *) (host_mem_ex[i] + kOpaqueTagOffset);
      if (intr_ptr == 0)
        return -1;

      cp_status_sha512_t *s;
      s = (cp_status_sha512_t *) (host_mem_ex[i] + kStatusOffset);
      if (!s->valid) {
	      printf("Decompression status is invalid! "
			      "status: %llx\n", *((unsigned long long *) s));
	      return -1;
      }
      if (s->err != 0) {
	      printf("ERROR: status err 0x%x is unexpected!\n", s->err);
	      return -1;
      }
      if (s->partial_data != 0x4321) {
	      printf("ERROR: status partial data 0x%x is unexpected\n",
			      s->partial_data);
	      return -1;
      }
    }
    return 0;
  };
  tests::Poller poll;
  if (poll(func) == 0) {
    printf("Testcase %s passed\n", __func__);
    return 0;
  }

  uint64_t *intr_ptr;
  for (i = 0; i < MAX_CPDC_REQ - MAX_DC_REQ; i++) {
    intr_ptr = (uint64_t *) (host_mem_ex[i] + kOpaqueTagOffset);
    if (intr_ptr[i] == 0)
      printf("Compression request %d did not complete\n", i);
  }
  for (; i < MAX_CPDC_REQ; i++) {
    intr_ptr = (uint64_t *) (host_mem_ex[i] + kOpaqueTagOffset);
    if (intr_ptr[i] == 0)
      printf("Decompression request %d did not complete\n", i);
  }
  printf("ERROR: Timed out waiting for all the commands to complete\n");
  return -1;
}

int max_data_rate() {
    return _max_data_rate(COMP_QUEUE_PUSH_HW_DIRECT_BATCH, 0, 0);
}

int seq_max_data_rate() {
    return _max_data_rate(COMP_QUEUE_PUSH_SEQUENCER_BATCH,
                          queues::get_pvm_seq_comp_sq(0),
                          queues::get_pvm_seq_comp_sq(1));
}

static int cp_dualq_flat_4K_buf(uint64_t mem_pa, bool in_hbm,
                                 comp_queue_push_t push_type,
                                 uint32_t seq_comp_qid_cp,
                                 uint32_t seq_comp_qid_hotq) {
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
  comp_queue_push(&lq_desc, cp_queue, push_type, seq_comp_qid_cp);

  // Dont ring the doorbell yet
  comp_queue_push(&hq_desc, cp_hotq, push_type, seq_comp_qid_hotq);

  // Now ring door bells for both high and low queue
  comp_queue_post_push(cp_queue);
  comp_queue_post_push(cp_hotq);

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

int compress_dualq_flat_4K_buf() {
  printf("Starting testcase %s\n", __func__);

  InvalidateHdrInHostMem();

  int rc = cp_dualq_flat_4K_buf(host_mem_pa, false,
                                 COMP_QUEUE_PUSH_HW_DIRECT_BATCH, 0, 0);
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

int seq_compress_dualq_flat_4K_buf() {
  printf("Starting testcase %s\n", __func__);

  InvalidateHdrInHostMem();

  int rc = cp_dualq_flat_4K_buf(host_mem_pa, false,
                                 COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST,
                                 queues::get_pvm_seq_comp_sq(0),
                                 queues::get_pvm_seq_comp_sq(1));
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

int compress_dualq_flat_4K_buf_in_hbm() {
  printf("Starting testcase %s\n", __func__);

  InvalidateHdrInHBM();

  int rc = cp_dualq_flat_4K_buf(hbm_pa, true,
                                 COMP_QUEUE_PUSH_HW_DIRECT_BATCH, 0, 0);
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

int seq_compress_dualq_flat_4K_buf_in_hbm() {
  printf("Starting testcase %s\n", __func__);

  InvalidateHdrInHBM();

  int rc = cp_dualq_flat_4K_buf(hbm_pa, true,
                                 COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST,
                                 queues::get_pvm_seq_comp_sq(0),
                                 queues::get_pvm_seq_comp_sq(1));
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

}  // namespace tests
