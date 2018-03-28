// Compression DOLs.
#include <math.h>
#include "compression.hpp"
#include "compression_test.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include "queues.hpp"
#include "xts.hpp"
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

static uint16_t kCPVersion = 0x1234;

// Sample data generated during test.
static const uint8_t all_zeros[65536] = {0};
static constexpr uint32_t kUncompressedDataSize = 65536;
static uint8_t uncompressed_data[kUncompressedDataSize];
static constexpr uint32_t kCompressedBufSize = 65536 - 4096;
static uint8_t compressed_data_buf[kCompressedBufSize];
static uint16_t compressed_data_size;  // Calculated at run-time;
static uint16_t last_cp_output_data_len;  // Calculated at run-time;

static const uint32_t kSeqIntrData = 0x11223344;

static dp_mem_t *uncompressed_buf;
static dp_mem_t *uncompressed_host_buf;
static dp_mem_t *compressed_buf;
static dp_mem_t *compressed_host_buf;

static dp_mem_t *status_buf;
static dp_mem_t *status_buf2;
static dp_mem_t *status_host_buf;
static dp_mem_t *status_host_buf2;
static dp_mem_t *opaque_host_buf;

static dp_mem_t *host_sgl1;
static dp_mem_t *host_sgl2;
static dp_mem_t *host_sgl3;
static dp_mem_t *host_sgl4;
static dp_mem_t *seq_sgl;

static dp_mem_t *xts_encrypt_host_buf;
static dp_mem_t *xts_decrypt_buf;
static dp_mem_t *xts_in_aol;
static dp_mem_t *xts_out_aol;
static dp_mem_t *xts_desc_buf;
static dp_mem_t *xts_status_desc_buf;
static dp_mem_t *xts_status_buf;
static dp_mem_t *xts_status_host_buf;
static dp_mem_t *xts_opaque_host_buf;

static dp_mem_t *pad_buf;

// Forward declaration with default param values
int run_cp_test(cp_desc_t *desc, dp_mem_t *status, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type = COMP_QUEUE_PUSH_HW_DIRECT,
                uint32_t seq_comp_qid = 0);
int run_dc_test(cp_desc_t *desc, dp_mem_t *status, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type = COMP_QUEUE_PUSH_HW_DIRECT,
                uint32_t seq_comp_qid = 0);

// Write zeros to the 1st 8 bytes of compressed data buffer in hostmem.
void InvalidateHdrInHostMem() {
  dp_mem_t *cp_hdr = compressed_host_buf->fragment_find(0, sizeof(cp_hdr_t));
  cp_hdr->clear_thru();
}

// Write zeros to the 1st 8 bytes of compressed data buffer in hbm.
void InvalidateHdrInHBM() {
  dp_mem_t *cp_hdr = compressed_buf->fragment_find(0, sizeof(cp_hdr_t));
  cp_hdr->clear_thru();
}

static bool status_poll(dp_mem_t *status) {
  auto func = [status] () -> int {
    cp_status_sha512_t *s = (cp_status_sha512_t *)status->read_thru();
    if (s->valid) {
      if (status->is_mem_type_hbm()) {
        usleep(100);
        s = (cp_status_sha512_t *)status->read_thru();
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
        seq_comp_desc->write_bit_fields(64, 34, comp_queue.cfg_q_pd_idx);
        seq_comp_desc->write_bit_fields(98, 4, (uint8_t)log2(sizeof(*dst_desc)));
        seq_comp_desc->write_bit_fields(102, 3, (uint8_t)log2(sizeof(uint32_t)));

        // skip 1 filler bit
        seq_comp_desc->write_bit_fields(106, 34, comp_queue.q_base_mem_pa);

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

        seq_comp_desc->write_bit_fields(140, 16, curr_pd_idx);
        seq_comp_desc->write_bit_fields(156, 1, 1); /* set barco_batch_mode */
        if (push_type == COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST) {
            seq_comp_desc->write_bit_fields(157, 1, 1); /* set barco_batch_last */
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
compression_buf_init()
{
    uncompressed_buf = new dp_mem_t(1, kUncompressedDataSize,
                                    DP_MEM_ALIGN_PAGE);
    uncompressed_host_buf = new dp_mem_t(1, kUncompressedDataSize,
                                         DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
    // size of compressed buffers accounts any for compression failure,
    // i.e., must be as large as uncompressed buffer plus cp_hdr_t
    compressed_buf = new dp_mem_t(1, kUncompressedDataSize + sizeof(cp_hdr_t),
                                  DP_MEM_ALIGN_PAGE);
    compressed_host_buf = new dp_mem_t(1, kUncompressedDataSize + sizeof(cp_hdr_t),
                                       DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
    status_buf = new dp_mem_t(1, sizeof(cp_status_sha512_t));
    status_buf2 = new dp_mem_t(1, sizeof(cp_status_sha512_t));
    status_host_buf = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                                   DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    status_host_buf2 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                                    DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    opaque_host_buf = new dp_mem_t(1, sizeof(uint32_t),
                                   DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    host_sgl1 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    host_sgl2 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    host_sgl3 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    host_sgl4 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    seq_sgl = new dp_mem_t(1, sizeof(cp_sq_ent_sgl_t));

    // XTS AOL must be 512 byte aligned
    xts_encrypt_host_buf = new dp_mem_t(1, kUncompressedDataSize + sizeof(cp_hdr_t),
                                        DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM);
    xts_decrypt_buf = new dp_mem_t(1, kUncompressedDataSize + sizeof(cp_hdr_t),
                                   DP_MEM_ALIGN_PAGE);
    xts_in_aol = new dp_mem_t(1, sizeof(xts::xts_aol_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM, 512);
    xts_out_aol = new dp_mem_t(1, sizeof(xts::xts_aol_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM, 512);
    xts_desc_buf = new dp_mem_t(1, sizeof(xts::xts_desc_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                sizeof(xts::xts_desc_t));
    xts_status_desc_buf = new dp_mem_t(1, sizeof(xts::xts_status_desc_t),
                                      DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                      sizeof(xts::xts_status_desc_t));
    xts_status_buf = new dp_mem_t(1, sizeof(uint64_t));
    xts_status_host_buf = new dp_mem_t(1, sizeof(uint64_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    xts_opaque_host_buf = new dp_mem_t(1, sizeof(uint32_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    pad_buf = new dp_mem_t(1, 4096, DP_MEM_ALIGN_PAGE);

    // Pre-fill input buffers.
    uint64_t *p64 = (uint64_t *)uncompressed_data;
    for (uint64_t i = 0; i < (kUncompressedDataSize/sizeof(uint64_t)); i++)
      p64[i] = i;

    bcopy(uncompressed_data, uncompressed_buf->read(),
          uncompressed_buf->line_size_get());
    uncompressed_buf->write_thru();
    bcopy(uncompressed_data, uncompressed_host_buf->read(),
          uncompressed_host_buf->line_size_get());
    uncompressed_host_buf->write_thru();
}

void
compression_init()
{
  comp_queue_alloc(cp_queue, cp_cfg_q_base, cp_cfg_q_pd_idx);
  comp_queue_alloc(cp_hotq, cp_cfg_hotq_base, cp_cfg_hotq_pd_idx);
  comp_queue_alloc(dc_queue, dc_cfg_q_base, dc_cfg_q_pd_idx);
  comp_queue_alloc(dc_hotq, dc_cfg_hotq_base, dc_cfg_hotq_pd_idx);

  compression_buf_init();

  uint32_t lo_reg, hi_reg;
  read_reg(cp_cfg_glob, lo_reg);

  if (comp_inited_by_hal) {
      kCPVersion = lo_reg & 0xffff;
      printf("Comp version is 0x%x\n", kCPVersion);

  } else {

      // Write cp queue base.
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
int run_cp_test(cp_desc_t *desc, dp_mem_t *status, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type, uint32_t seq_comp_qid) {

  status->clear_thru();
  desc->status_addr = status->pa();

  comp_queue_push(desc, cp_queue, push_type, seq_comp_qid);
  if (!status_poll(status)) {
    printf("ERROR: status never came\n");
    return -1;
  }
  cp_status_sha512_t *st = (cp_status_sha512_t *)status->read();
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
    cp_hdr_t *hdr = (cp_hdr_t *)compressed_host_buf->read_thru();
    // If the dst was hbm, copy over header from there.
    if (!(desc->dst & (1ul << 63))) {
      hdr = (cp_hdr_t *)compressed_buf->read_thru();
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
    if ((hdr->data_len == 0) || (hdr->data_len > (kCompressedBufSize - sizeof(cp_hdr_t)))
        || (hdr->data_len > datain_len)) {
      printf("ERROR: Invalid data_len 0x%x\n", hdr->data_len);
      return -1;
    }
    if (st->output_data_len != (hdr->data_len + sizeof(cp_hdr_t))) {
      printf("ERROR: output_data_len %u does not match hdr+sizeof(cp_hdr_t) %d\n",
             st->output_data_len, (int)(hdr->data_len + sizeof(cp_hdr_t)));
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
int run_dc_test(cp_desc_t *desc, dp_mem_t *status, const cp_status_no_hash_t &exp,
                comp_queue_push_t push_type, uint32_t seq_comp_qid) {

  status->clear_thru();
  desc->status_addr = status->pa();

  comp_queue_push(desc, dc_queue, push_type, seq_comp_qid);
  if (!status_poll(status)) {
    printf("ERROR: status never came\n");
    return -1;
  }
  cp_status_sha512_t *st = (cp_status_sha512_t *)status->read();
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
  d.src = uncompressed_host_buf->pa();
  d.dst = compressed_host_buf->pa();
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - sizeof(cp_hdr_t);
  d.status_data = 0x1234;
  InvalidateHdrInHostMem();

  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  if (run_cp_test(&d, status_host_buf, exp_st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  // Save compressed data.
  cp_hdr_t *hdr = (cp_hdr_t *)compressed_host_buf->read_thru();
  bcopy(hdr, compressed_data_buf, hdr->data_len + sizeof(cp_hdr_t));
  compressed_data_size = hdr->data_len + sizeof(cp_hdr_t);
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
  bcopy(uncompressed_host_buf->read(),
        compressed_host_buf->read(), 4096);
  compressed_host_buf->write_thru();
  d.src = compressed_host_buf->pa();
  d.dst = compressed_host_buf->pa();
  d.datain_len = 4096;
  d.threshold_len = 4096 - sizeof(cp_hdr_t);
  d.status_data = 0x1234;

  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  if (run_cp_test(&d, status_host_buf, exp_st, push_type, seq_comp_qid) < 0) {
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
  uncompressed_host_buf->clear_thru();
  bcopy(compressed_data_buf, compressed_host_buf->read(), compressed_data_size);
  compressed_host_buf->write_thru();

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.dst = uncompressed_host_buf->pa();
  d.src = compressed_host_buf->pa();
  d.datain_len = compressed_data_size;
  d.threshold_len = 0;  // 64K, output len
  d.status_data = 0x3456;
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x3456;
  if (run_dc_test(&d, status_host_buf, exp_st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Verify data buf
  if (bcmp(uncompressed_host_buf->read_thru(), uncompressed_data, kUncompressedDataSize) != 0) {
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
  d.src = uncompressed_host_buf->pa();
  d.dst = compressed_host_buf->pa();
  d.datain_len = 567;  // 0 = 64K
  d.threshold_len = 567 - sizeof(cp_hdr_t);
  d.status_data = 0x1234;
  InvalidateHdrInHostMem();
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  if (run_cp_test(&d, status_host_buf, exp_st) < 0) {
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
  d.dst = uncompressed_host_buf->pa();
  d.src = compressed_host_buf->pa();
  d.datain_len = last_cp_output_data_len;
  d.threshold_len = 567;  // 64K, output len
  d.status_data = 0x3456;
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x3456;
  exp_st.output_data_len = 567;
  if (run_dc_test(&d, status_host_buf, exp_st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Verify data buf
  if (bcmp(uncompressed_host_buf->read_thru(), uncompressed_data, 567) != 0) {
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
  host_sgl1->clear();
  host_sgl2->clear();
  cp_sgl_t *cp_sgl1 = (cp_sgl_t *)host_sgl1->read();
  cp_sgl_t *cp_sgl2 = (cp_sgl_t *)host_sgl2->read();

  cp_sgl1->len0 = cp_sgl1->len1 = cp_sgl1->len2 = 1000;
  cp_sgl1->addr0 = uncompressed_host_buf->pa() + 0;
  cp_sgl1->addr1 = uncompressed_host_buf->pa() + 1000;
  cp_sgl1->addr2 = uncompressed_host_buf->pa() + 2000;
  cp_sgl1->link = host_sgl2->pa();
  cp_sgl2->len0 = cp_sgl2->len1 = cp_sgl2->len2 = 1000;
  cp_sgl2->addr0 = uncompressed_host_buf->pa() + 3000;
  cp_sgl2->addr1 = uncompressed_host_buf->pa() + 4000;
  cp_sgl2->addr2 = uncompressed_host_buf->pa() + 5000;
  host_sgl1->write_thru();
  host_sgl2->write_thru();

  // Prepare destination SGLs
  host_sgl3->clear();
  host_sgl4->clear();
  cp_sgl_t *cp_sgl3 = (cp_sgl_t *)host_sgl3->read();
  cp_sgl_t *cp_sgl4 = (cp_sgl_t *)host_sgl4->read();
  cp_sgl3->len0 = cp_sgl3->len1 = cp_sgl3->len2 = 1024;
  cp_sgl3->addr0 = compressed_host_buf->pa() + 0;
  cp_sgl3->addr1 = compressed_host_buf->pa() + 1024;
  cp_sgl3->addr2 = compressed_host_buf->pa() + 2048;
  cp_sgl3->link = host_sgl4->pa();
  cp_sgl4->len0 = cp_sgl4->len1 = cp_sgl4->len2 = 1024;
  cp_sgl4->addr0 = compressed_host_buf->pa() + (3 * 1024);
  cp_sgl4->addr1 = compressed_host_buf->pa() + 4096;
  cp_sgl4->addr2 = compressed_host_buf->pa() + (5 * 1024);
  host_sgl3->write_thru();
  host_sgl4->write_thru();

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.cmd_bits.src_is_list = 1;
  d.cmd_bits.dst_is_list = 1;
  d.src = host_sgl1->pa();
  d.dst = host_sgl3->pa();
  d.datain_len = 6000;
  d.threshold_len = 6000 - sizeof(cp_hdr_t);
  d.status_data = 0x1234;
  InvalidateHdrInHostMem();
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  if (run_cp_test(&d, status_host_buf, exp_st, push_type, seq_comp_qid) < 0) {
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
  dp_mem_t *partial_buf = uncompressed_host_buf->fragment_find(0, 1024);
  partial_buf->clear_thru();

  // Prepare source SGLs
  host_sgl1->clear();
  cp_sgl_t *cp_sgl1 = (cp_sgl_t *)host_sgl1->read();

  cp_sgl1->len0 = last_cp_output_data_len >> 1;
  cp_sgl1->len1 = last_cp_output_data_len - cp_sgl1->len0;
  cp_sgl1->addr0 = compressed_host_buf->pa() + 0;
  cp_sgl1->addr1 = compressed_host_buf->pa() + cp_sgl1->len0;
  host_sgl1->write_thru();

  // Prepare destination SGLs
  host_sgl3->clear();
  host_sgl4->clear();
  cp_sgl_t *cp_sgl3 = (cp_sgl_t *)host_sgl3->read();
  cp_sgl_t *cp_sgl4 = (cp_sgl_t *)host_sgl4->read();

  cp_sgl3->len0 = cp_sgl3->len1 = cp_sgl3->len2 = 1024;
  cp_sgl3->addr0 = uncompressed_host_buf->pa() + 0;
  cp_sgl3->addr1 = uncompressed_host_buf->pa() + 1024;
  cp_sgl3->addr2 = uncompressed_host_buf->pa() + 2048;
  cp_sgl3->link = host_sgl4->pa();
  cp_sgl4->len0 = cp_sgl4->len1 = cp_sgl4->len2 = 1024;
  cp_sgl4->addr0 = uncompressed_host_buf->pa() + (3 * 1024);
  cp_sgl4->addr1 = uncompressed_host_buf->pa() + 4096;
  cp_sgl4->addr2 = uncompressed_host_buf->pa() + (5 * 1024);
  host_sgl3->write_thru();
  host_sgl4->write_thru();

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.cmd_bits.src_is_list = 1;
  d.cmd_bits.dst_is_list = 1;
  d.src = host_sgl1->pa();
  d.dst = host_sgl3->pa();
  d.datain_len = last_cp_output_data_len;
  d.threshold_len = 6000;
  d.status_data = 0x1234;
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  exp_st.output_data_len = 6000;
  if (run_dc_test(&d, status_host_buf, exp_st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  if (bcmp(uncompressed_host_buf->read_thru(), uncompressed_data, 6000) != 0) {
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
  d.src = uncompressed_buf->pa();
  d.dst = compressed_buf->pa();
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - sizeof(cp_hdr_t);
  d.status_data = 0x1234;
  InvalidateHdrInHBM();
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  if (run_cp_test(&d, status_buf, exp_st, push_type, seq_comp_qid) < 0) {
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

  // bzero some initial area.
  dp_mem_t *partial_buf = uncompressed_buf->fragment_find(0, 16);
  partial_buf->clear_thru();
  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.header_present = 1;
  d.cmd_bits.cksum_verify_en = 1;
  d.dst = uncompressed_buf->pa();
  d.src = compressed_buf->pa();
  d.datain_len = compressed_data_size;
  d.threshold_len = 0;  // 64K, output len
  d.status_data = 0x3456;
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x3456;
  if (run_dc_test(&d, status_buf, exp_st, push_type, seq_comp_qid) < 0) {
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
  d.src = uncompressed_host_buf->pa();
  d.dst = compressed_buf->pa();
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - sizeof(cp_hdr_t);
  d.status_data = 0x1234;
  InvalidateHdrInHBM();
  InvalidateHdrInHostMem();
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;

  // Prepare an SGL for the sequencer to output data.
  seq_sgl->clear();
  cp_sq_ent_sgl_t *ssgl = (cp_sq_ent_sgl_t *)seq_sgl->read();

  ssgl->addr[0] = compressed_host_buf->pa();
  ssgl->len[0] = 199;
  ssgl->addr[1] = compressed_host_buf->pa() + 199;
  ssgl->len[1] = 537;
  ssgl->addr[2] = compressed_host_buf->pa() + 199 + 537;
  ssgl->len[2] = 1123;
  ssgl->addr[3] = compressed_host_buf->pa() + 199 + 537 + 1123;
  ssgl->len[3] = kCompressedBufSize - (199 + 537 + 1123);
  seq_sgl->write_thru();

  cp_seq_params_t seq_params;
  bzero(&seq_params, sizeof(seq_params));
  status_host_buf->clear_thru();
  seq_params.seq_ent.status_hbm_pa = status_buf->pa();
  seq_params.seq_ent.status_host_pa = status_host_buf->pa();
  seq_params.seq_ent.src_hbm_pa = d.src;
  seq_params.seq_ent.dst_hbm_pa = d.dst;
  seq_params.seq_ent.sgl_out_aol_pa = seq_sgl->pa();
  seq_params.seq_ent.sgl_xfer_en = 1;
  seq_params.seq_ent.intr_pa = opaque_host_buf->pa();
  seq_params.seq_ent.intr_data = kSeqIntrData;

  // Clear the area where interrupt from sequencer is going to come.
  opaque_host_buf->clear_thru();
  seq_params.seq_ent.status_len = status_buf->line_size_get();
  seq_params.seq_ent.status_dma_en = 1;
  seq_params.seq_ent.intr_en = 1;
  seq_params.seq_comp_status_q = queues::get_pvm_seq_comp_status_sq(0);
  if (test_setup_cp_seq_status_ent(&seq_params) != 0) {
    printf("cp_seq_ent failed\n");
    return -1;
  }
  d.doorbell_addr = seq_params.ret_doorbell_addr;
  d.doorbell_data = seq_params.ret_doorbell_data;
  d.cmd_bits.doorbell_on = 1;

  // Verify(wait for) that the status makes it to HBM.
  // We could directly poll for sequencer but this is just additional verification.
  if (run_cp_test(&d, status_buf, exp_st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Now poll for sequencer interrupt.
  auto seq_intr_poll_func = [] () -> int {
    uint32_t *p = (uint32_t *)opaque_host_buf->read_thru();
    if (*p == kSeqIntrData)
      return 0;
    return 1;
  };
  tests::Poller intr_poll;
  if (intr_poll(seq_intr_poll_func) != 0) {
    printf("ERROR: Interrupt from sequencer never came.\n");
  }
  if (bcmp(compressed_host_buf->read(), compressed_data_buf, last_cp_output_data_len) != 0) {
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

void compress_xts_encrypt_setup(cp_desc_t& d,
                                XtsCtx& xts_ctx,
                                cp_seq_params_t& seq_params,
                                dp_mem_t *src_buf,
                                dp_mem_t *dst_buf) {
  xts::xts_cmd_t cmd;
  xts::xts_desc_t *xts_desc_addr;
  uint32_t datain_len;

  xts_ctx.op = xts::AES_ENCR_ONLY;
  xts_ctx.use_seq = false;
  xts_ctx.seq_xts_status_q = seq_params.seq_xts_status_q;
  xts_ctx.num_sectors = 1;
  xts_ctx.copy_desc = false;
  xts_ctx.ring_db = false;

  xts_in_aol->clear();
  xts::xts_aol_t *xts_in = (xts::xts_aol_t *)xts_in_aol->read();

  // Note: p4+ will modify L0 and L1 below based on compression
  // output_data_len and any required padding
  datain_len = d.datain_len == 0 ? 65536 :  d.datain_len;
  xts_in->a0 = src_buf->pa();
  xts_in->l0 = datain_len;
  xts_in->a1 = pad_buf->pa();
  xts_in->l1 = pad_buf->line_size_get();
  xts_in_aol->write_thru();

  xts_out_aol->clear();
  xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read();
  xts_out->a0 = dst_buf->pa();
  xts_out->l0 = datain_len + sizeof(cp_hdr_t);
  xts_out_aol->write_thru();

  // Set up XTS status chaining;
  // Note: XTS status chaining is not strictly necessary since encryption is the last
  // in the chain. It is used here to validate XTS completion status and interrupt.
  //
  // Clear the area where interrupt from sequencer is going to come.
  xts_opaque_host_buf->clear_thru();
  memset(xts_status_host_buf->read(), 0xff, xts_status_host_buf->line_size_get());
  xts_status_host_buf->write_thru();

  xts_status_desc_buf->clear();
  xts_status_desc_buf->write_bit_fields(128, 64, xts_status_buf->pa());
  xts_status_desc_buf->write_bit_fields(192, 64, xts_status_host_buf->pa());
  xts_status_desc_buf->write_bit_fields(256, 64, xts_opaque_host_buf->pa());
  xts_status_desc_buf->write_bit_fields(320, 32, kSeqIntrData);
  xts_status_desc_buf->write_bit_fields(352, 16, xts_status_host_buf->line_size_get());
  xts_status_desc_buf->write_bit_fields(368, 1, 1); // status_dma_en
  xts_status_desc_buf->write_bit_fields(370, 1, 1); // intr_en
  xts_status_desc_buf->write_bit_fields(372, 1, 1); // stop_chain_on_error
  xts_status_desc_buf->write_thru();
  xts_ctx.desc_write_seq_xts_status(xts_status_desc_buf);

  // Set up XTS encrypt descriptor
  xts_ctx.cmd_eval_seq_xts(cmd);
  xts_desc_addr = xts_ctx.desc_prefill_seq_xts(xts_desc_buf);
  xts_desc_addr->in_aol = xts_in_aol->pa();
  xts_desc_addr->out_aol = xts_out_aol->pa();
  xts_desc_addr->cmd = cmd;
  xts_desc_addr->status = xts_status_buf->pa();
  queues::get_capri_doorbell(queues::get_pvm_lif(), SQ_TYPE,
                             xts_ctx.seq_xts_status_q, 0, xts_ctx.seq_xts_status_index, 
                             &xts_desc_addr->db_addr, &xts_desc_addr->db_data);
  xts_desc_buf->write_thru();
  xts_ctx.desc_write_seq_xts(xts_desc_buf);
}

int _compress_output_encrypt(comp_queue_push_t push_type,
                             uint32_t seq_comp_qid) {
  XtsCtx xts_ctx;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  bcopy(uncompressed_data, uncompressed_host_buf->read(),
        uncompressed_host_buf->line_size_get());
  uncompressed_host_buf->write_thru();

  d.cmd_bits.comp_decomp_en = 1;
  d.cmd_bits.insert_header = 1;
  d.cmd_bits.sha_en = 1;
  d.src = uncompressed_host_buf->pa();
  d.dst = compressed_buf->pa();
  d.datain_len = 0;  // 0 = 64K
  d.threshold_len = kCompressedBufSize - sizeof(cp_hdr_t);
  d.status_data = 0x1234;
  InvalidateHdrInHBM();
  InvalidateHdrInHostMem();
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;

  cp_seq_params_t seq_params;
  bzero(&seq_params, sizeof(seq_params));
  seq_params.seq_comp_status_q = queues::get_pvm_seq_comp_status_sq(0);
  seq_params.seq_xts_q = queues::get_pvm_seq_xts_sq(0);
  seq_params.seq_xts_status_q = queues::get_pvm_seq_xts_status_sq(0);

  // Set up encryption
  compress_xts_encrypt_setup(d, xts_ctx, seq_params, compressed_buf,
                             xts_encrypt_host_buf);

  // encryption will use direct barco push action
  seq_params.seq_ent.next_doorbell_en = 1;
  seq_params.seq_ent.is_next_db_barco_push = 1;
  seq_params.seq_ent.push_entry.barco_ring_addr = xts_ctx.xts_ring_base_addr;
  seq_params.seq_ent.push_entry.barco_pndx_addr = xts_ctx.xts_ring_pi_addr;
  seq_params.seq_ent.push_entry.barco_desc_addr = xts_desc_buf->pa();
  seq_params.seq_ent.push_entry.barco_desc_size = (uint8_t)log2(xts_desc_buf->line_size_get());
  seq_params.seq_ent.push_entry.barco_pndx_size = (uint8_t)log2(xts::kXtsPISize);
  
  status_buf->clear_thru();
  seq_params.seq_ent.status_hbm_pa = status_buf->pa();
  seq_params.seq_ent.status_host_pa = status_host_buf->pa();
  seq_params.seq_ent.status_len = status_buf->line_size_get();

  // Note that encryption data transfer is handled by compress_xts_encrypt_setup()
  // above. The compression status sequencer will use the AOLs given below
  // only to update the length fields with the correct output data length and pad length.
  seq_params.seq_ent.sgl_in_aol_pa = xts_in_aol->pa();
  seq_params.seq_ent.sgl_out_aol_pa = xts_out_aol->pa();
  seq_params.seq_ent.status_dma_en = 1;
  seq_params.seq_ent.stop_chain_on_error = 1;
  seq_params.seq_ent.aol_pad_xfer_en = 1;
  seq_params.seq_ent.pad_len_shift = (uint8_t)log2(pad_buf->line_size_get());

  // Enable interrupt in case compression fails
  opaque_host_buf->clear_thru();
  seq_params.seq_ent.intr_pa = opaque_host_buf->pa();
  seq_params.seq_ent.intr_data = kSeqIntrData;
  seq_params.seq_ent.intr_en = 1;

  if (test_setup_cp_seq_status_ent(&seq_params) != 0) {
    printf("cp_seq_ent failed\n");
    return -1;
  }
  d.doorbell_addr = seq_params.ret_doorbell_addr;
  d.doorbell_data = seq_params.ret_doorbell_data;
  d.cmd_bits.doorbell_on = 1;

  // Verify(wait for) that the status makes it to HBM.
  // We could directly poll for sequencer but this is just additional verification.
  if (run_cp_test(&d, status_buf, exp_st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Now poll for XTS status sequencer interrupt.
  auto seq_intr_poll_func = [] () -> int {
    uint32_t *p = (uint32_t *)xts_opaque_host_buf->read_thru();
    if (*p == kSeqIntrData)
      return 0;
    return 1;
  };
  tests::Poller intr_poll;
  if (intr_poll(seq_intr_poll_func) != 0) {
    printf("ERROR: Interrupt from XTS status sequencer never came.\n");
  }

  // Validate XTS status
  uint64_t curr_xts_status = *((uint64_t *)xts_status_host_buf->read_thru());
  if (curr_xts_status) {
    printf("ERROR: XTS error 0x%lx\n", curr_xts_status);
    return -1;
  }
  // Status verification done.

  printf("Testcase %s passed\n", __func__);
  return 0;
}


int compress_output_encrypt() {
    return _compress_output_encrypt(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_output_encrypt() {
    return _compress_output_encrypt(COMP_QUEUE_PUSH_SEQUENCER, 
                                    queues::get_pvm_seq_comp_sq(0));
}

// Verify integrity of >64K buffer
int verify_integrity_for_gt64K() {
  printf("Starting testcase %s\n", __func__);
  cp_desc_t d;
  bzero(&d, sizeof(d));
  d.src = uncompressed_host_buf->pa();
  // Give 68K length
  d.datain_len = 4096;
  d.extended_len = 1;
  d.status_data = 0x7654;
  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x7654;
  if (run_cp_test(&d, status_host_buf, exp_st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  cp_status_sha512_t *st2 = (cp_status_sha512_t *)status_host_buf->read();
  uint64_t old_integiry = st2->integrity_data;

  // Now repeat the test using only 4K data i.e. higher 16 bits are zero.
  bzero(&d, sizeof(d));
  d.src = uncompressed_host_buf->pa();
  d.datain_len = 4096;
  d.status_data = 0x5454;
  exp_st = {0};
  exp_st.partial_data = 0x5454;
  if (run_cp_test(&d, status_host_buf, exp_st) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  st2 = (cp_status_sha512_t *)status_host_buf->read();
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

  dp_mem_t *partial_buf = uncompressed_buf->fragment_find(0, TMP_BLK_SIZE);
  bcopy(uncompressed_data, partial_buf->read(), TMP_BLK_SIZE);
  partial_buf->write_thru();

  d.src = partial_buf->pa();
  d.dst = compressed_buf->pa();
  d.datain_len = TMP_BLK_SIZE;
  d.threshold_len = TMP_BLK_SIZE - sizeof(cp_hdr_t);

  d.status_data = 0x1234;

  cp_status_no_hash_t exp_st = {0};
  exp_st.partial_data = 0x1234;
  if (run_cp_test(&d, status_buf, exp_st, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }

  printf("Testcase %s passed\n", __func__);

  // Save compressed data.
  cp_hdr_t *hdr = (cp_hdr_t *)compressed_buf->read_thru();
  bcopy(hdr, compressed_data_buf, hdr->data_len + sizeof(cp_hdr_t));
  compressed_data_size = hdr->data_len + sizeof(cp_hdr_t);

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

  uint32_t i;
  dp_mem_t *max_cp_uncompressed_buf = new dp_mem_t(MAX_CP_REQ, TMP_BLK_SIZE,
                                                   DP_MEM_ALIGN_PAGE);
  dp_mem_t *max_cp_compressed_buf = new dp_mem_t(MAX_CP_REQ, TMP_BLK_SIZE,
                                                 DP_MEM_ALIGN_PAGE);
  dp_mem_t *max_cp_status_buf = new dp_mem_t(MAX_CP_REQ, sizeof(cp_status_sha512_t));
  dp_mem_t *max_dc_uncompressed_buf = new dp_mem_t(MAX_DC_REQ, TMP_BLK_SIZE,
                                                   DP_MEM_ALIGN_PAGE);
  dp_mem_t *max_dc_status_buf = new dp_mem_t(MAX_DC_REQ, sizeof(cp_status_sha512_t));

  // allocate opaque tags as a byte stream for easy memcmp
  dp_mem_t *max_cp_opaque_host_buf = new dp_mem_t(1, MAX_CP_REQ * sizeof(uint32_t),
                                                  DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
  dp_mem_t *exp_opaque_data_buf = new dp_mem_t(1, MAX_CP_REQ * sizeof(uint32_t),
                                               DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
  dp_mem_t *max_dc_opaque_host_buf = new dp_mem_t(1, MAX_DC_REQ * sizeof(uint32_t),
                                                  DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
  uint32_t exp_opaque_data = 0xa5a5a5a5;
  memset(exp_opaque_data_buf->read(), 0xa5, exp_opaque_data_buf->line_size_get());
  exp_opaque_data_buf->write_thru();

  // allocate and fill descriptors to load compression engine with requests
  for (i = 0; i < MAX_CP_REQ; i++) {
    bzero(&d, sizeof(d));
 
    d.cmd_bits.comp_decomp_en = 1;
    d.cmd_bits.insert_header = 1;
    d.cmd_bits.opaque_tag_on = 1;

    max_cp_uncompressed_buf->line_set(i);
    d.src = max_cp_uncompressed_buf->pa();
    max_cp_compressed_buf->line_set(i);
    d.dst = max_cp_compressed_buf->pa();

    d.datain_len = TMP_BLK_SIZE;	// TBD: parameterize this
    d.threshold_len = TMP_BLK_SIZE - sizeof(cp_hdr_t);
  
    max_cp_status_buf->line_set(i);
    d.status_addr = max_cp_status_buf->pa();
    d.status_data = 0x1234;
    
    d.opaque_tag_addr = max_cp_opaque_host_buf->pa() + (i * sizeof(uint32_t));
    d.opaque_tag_data = exp_opaque_data;
    comp_queue_push(&d, cp_queue, 
                    i == (MAX_CP_REQ - 1) ? last_push_type : push_type,
                    seq_comp_qid_cp);
  }

  // Ring the doorbell after loading decompression engine with requests
  for (i = 0; i < MAX_DC_REQ; i++) {
    bzero(&d, sizeof(d));

    d.cmd_bits.comp_decomp_en = 1;
    d.cmd_bits.header_present = 1;
    d.cmd_bits.opaque_tag_on = 1;

    // decompress from data obtained from _max_data_rate_compress_flat_buf() above
    d.src = compressed_buf->pa();

    max_dc_uncompressed_buf->line_set(i);
    d.dst = max_dc_uncompressed_buf->pa();
    d.datain_len = compressed_data_size;	// predefined fixed size
    d.threshold_len = TMP_BLK_SIZE;

    max_dc_status_buf->line_set(i);
    d.status_addr = max_dc_status_buf->pa();
    d.status_data = 0x4321;

    d.opaque_tag_addr = max_dc_opaque_host_buf->pa() + (i * sizeof(uint32_t));
    d.opaque_tag_data = exp_opaque_data;
    comp_queue_push(&d, dc_queue, 
                    i == (MAX_DC_REQ - 1) ? last_push_type : push_type,
                    seq_comp_qid_dc);
  }

  // Now ring doorbells
  comp_queue_post_push(cp_queue);
  comp_queue_post_push(dc_queue);

  // Wait for all the interrupts
  auto func = [max_cp_opaque_host_buf, max_dc_opaque_host_buf,
               exp_opaque_data_buf,
               max_cp_status_buf, max_dc_status_buf] () -> int {

      uint32_t              i;
      cp_status_sha512_t    *s;

      if (memcmp(max_cp_opaque_host_buf->read_thru(),
                 exp_opaque_data_buf->read_thru(),
                 max_cp_opaque_host_buf->line_size_get())) {
	      return -1;
      }
      if (memcmp(max_dc_opaque_host_buf->read_thru(),
                 exp_opaque_data_buf->read_thru(),
                 max_dc_opaque_host_buf->line_size_get())) {
	      return -1;
      }

      for (i = 0; i < MAX_CP_REQ; i++) {
          max_cp_status_buf->line_set(i);
          s = (cp_status_sha512_t *)max_cp_status_buf->read_thru();
          if (!s->valid) {
              printf("Compression status for buf %u is invalid! "
                      "status: %llx\n", i, *((unsigned long long *) s));
              return -1;
          }
          if (s->err != 0) {
              printf("ERROR: buf %u compression err 0x%x is unexpected!\n",
                     i, s->err);
              return -1;
          }
      }

      for (i = 0; i < MAX_DC_REQ; i++) {
          max_dc_status_buf->line_set(i);
          s = (cp_status_sha512_t *)max_dc_status_buf->read_thru();
          if (!s->valid) {
              printf("Decompression status for buf %u is invalid! "
                      "status: %llx\n", i, *((unsigned long long *) s));
              return -1;
          }
          if (s->err != 0) {
              printf("ERROR: buf %u decompression err 0x%x is unexpected!\n",
                     i, s->err);
              return -1;
          }
      }

      return 0;
  };

  tests::Poller poll;
  if (poll(func) == 0) {
    printf("Testcase %s passed\n", __func__);

    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be accessing the buffers

    delete max_cp_uncompressed_buf;
    delete max_cp_compressed_buf;
    delete max_cp_status_buf;
    delete max_dc_uncompressed_buf;
    delete max_dc_status_buf;
    delete max_cp_opaque_host_buf;
    delete exp_opaque_data_buf;
    delete max_dc_opaque_host_buf;

    return 0;
  }

  if (memcmp(max_cp_opaque_host_buf->read_thru(),
             exp_opaque_data_buf->read_thru(),
             max_cp_opaque_host_buf->line_size_get())) {
      printf("Compression requests did not complete\n");
  }
  if (memcmp(max_dc_opaque_host_buf->read_thru(),
             exp_opaque_data_buf->read_thru(),
             max_dc_opaque_host_buf->line_size_get())) {
      printf("Decompression requests did not complete\n");
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

static int cp_dualq_flat_4K_buf(dp_mem_t *comp_buf,
                                dp_mem_t *uncomp_buf,
                                dp_mem_t *status_buf1,
                                dp_mem_t *status_buf2,
                                comp_queue_push_t push_type,
                                uint32_t seq_comp_qid_cp,
                                uint32_t seq_comp_qid_hotq) {
  // Setup compression descriptor for low prirority queue
  cp_desc_t lq_desc;
  bzero(&lq_desc, sizeof(lq_desc));

  lq_desc.cmd_bits.comp_decomp_en = 1;
  lq_desc.cmd_bits.insert_header = 1;
  lq_desc.cmd_bits.sha_en = 1;

  lq_desc.src = uncomp_buf->pa();
  lq_desc.dst = comp_buf->pa();
  lq_desc.datain_len = 4096;
  lq_desc.threshold_len = 4096 - sizeof(cp_hdr_t);

  lq_desc.status_addr = status_buf1->pa();
  lq_desc.status_data = 0x1234;

  // Setup compression descriptor for high prirority queue
  cp_desc_t hq_desc;
  bzero(&hq_desc, sizeof(hq_desc));

  hq_desc.cmd_bits.comp_decomp_en = 1;
  hq_desc.cmd_bits.insert_header = 1;
  hq_desc.cmd_bits.sha_en = 1;

  hq_desc.src = uncomp_buf->pa() + 4096;
  hq_desc.dst = comp_buf->pa() + 4096;
  hq_desc.datain_len = 4096;
  hq_desc.threshold_len = 4096 - sizeof(cp_hdr_t);

  hq_desc.status_addr = status_buf2->pa();
  hq_desc.status_data = 0x5678;

  // Initialize status for both the requests
  status_buf1->clear_thru();
  status_buf2->clear_thru();

  // Add descriptor for both high and low priority queue
  comp_queue_push(&lq_desc, cp_queue, push_type, seq_comp_qid_cp);

  // Dont ring the doorbell yet
  comp_queue_push(&hq_desc, cp_hotq, push_type, seq_comp_qid_hotq);

  // Now ring door bells for both high and low queue
  comp_queue_post_push(cp_queue);
  comp_queue_post_push(cp_hotq);

  // Check status update to both the descriptors
  auto func = [status_buf1, status_buf2] () -> int {
    cp_status_sha512_t *s;
    s = (cp_status_sha512_t *)status_buf1->read_thru();
    if (!s->valid) {
      printf("Compression request in low queue did not complete "
	  "status: %llx\n", *((unsigned long long *) s));
      return -1;
    }

    s = (cp_status_sha512_t *)status_buf2->read_thru();
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

  int rc = cp_dualq_flat_4K_buf(compressed_host_buf, uncompressed_host_buf,
                                status_host_buf, status_host_buf2,
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

  int rc = cp_dualq_flat_4K_buf(compressed_host_buf, uncompressed_host_buf,
                                status_host_buf, status_host_buf2,
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

  int rc = cp_dualq_flat_4K_buf(compressed_buf, uncompressed_buf,
                                status_buf, status_buf2,
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

  int rc = cp_dualq_flat_4K_buf(compressed_buf, uncompressed_buf,
                                status_buf, status_buf2,
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
