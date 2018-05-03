// Compression DOLs.
#include <math.h>
#include "compression.hpp"
#include "compression_test.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include "queues.hpp"
#include "xts.hpp"
#include "decrypt_decomp_chain.hpp"
#include "comp_encrypt_chain.hpp"
#include "comp_hash_chain.hpp"
#include "chksum_decomp_chain.hpp"
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
const static uint32_t kMaxSubqEntries = 4096;

comp_queue_t *cp_queue;
comp_queue_t *dc_queue;

comp_queue_t *cp_hotq;
comp_queue_t *dc_hotq;

static uint32_t cp_queue_size;
static uint32_t cp_hotq_size;
static uint32_t dc_queue_size;
static uint32_t dc_hotq_size;

// These constants equate to the number of 
// hardware compression/decompression engines.
#define MAX_CP_REQ	16
#define MAX_DC_REQ	2

// Sample data generated during test.
static const uint8_t all_zeros[kCompEngineMaxSize] = {0};
static constexpr uint32_t kUncompressedDataSize = kCompEngineMaxSize;
static uint8_t uncompressed_data[kUncompressedDataSize];
static constexpr uint32_t kCompressedBufSize = kCompEngineMaxSize - 4096;
static uint8_t compressed_data_buf[kCompressedBufSize];
static uint16_t compressed_data_size;  // Calculated at run-time;
static uint16_t last_cp_output_data_len;

static uint16_t kCPVersion = 0x1234;

static dp_mem_t *uncompressed_buf;
static dp_mem_t *uncompressed_host_buf;
static dp_mem_t *compressed_buf;
static dp_mem_t *compressed_host_buf;

static dp_mem_t *status_buf;
static dp_mem_t *status_buf2;
static dp_mem_t *status_host_buf;
static dp_mem_t *status_host_buf2;
static dp_mem_t *opaque_host_buf;
static dp_mem_t *hash_status_host_vec;
static dp_mem_t *hash_opaque_host_vec;
static dp_mem_t *decomp_status_host;

static dp_mem_t *host_sgl1;
static dp_mem_t *host_sgl2;
static dp_mem_t *host_sgl3;
static dp_mem_t *host_sgl4;
static dp_mem_t *seq_sgl;

static dp_mem_t *xts_status_host_buf;

static comp_encrypt_chain_t   *comp_encrypt_chain;
static comp_hash_chain_t      *comp_hash_chain;
static decrypt_decomp_chain_t *decrypt_decomp_chain;
static chksum_decomp_chain_t  *chksum_decomp_chain;

dp_mem_t *comp_pad_buf;

// Forward declaration with default param values
int run_cp_test(cp_desc_t& desc,
                dp_mem_t *dst_buf,
                dp_mem_t *status,
                comp_queue_push_t push_type = COMP_QUEUE_PUSH_HW_DIRECT,
                uint32_t seq_comp_qid = 0);
int run_dc_test(cp_desc_t& desc,
                dp_mem_t *status,
                uint32_t exp_output_data_len,
                comp_queue_push_t push_type = COMP_QUEUE_PUSH_HW_DIRECT,
                uint32_t seq_comp_qid = 0);

bool
comp_status_poll(dp_mem_t *status,
                 const cp_desc_t& desc,
                 bool suppress_log)
{
  uint32_t poll_factor;
  uint32_t datain_len;

  auto func = [status, suppress_log] () -> int {
    cp_status_sha512_t *s = (cp_status_sha512_t *)status->read_thru();
    if (s->valid) {
      if (status->is_mem_type_hbm()) {
        usleep(100);
        s = (cp_status_sha512_t *)status->read_thru();
      }
      if (!suppress_log) {
          printf("Got status %llx\n", *((unsigned long long *)s));
      }
      return 0;
    }
    return 1;
  };

  datain_len = desc.datain_len == 0 ?
               kCompEngineMaxSize : desc.datain_len;
  poll_factor = (datain_len + kCompAppMinSize - 1) / kCompAppMinSize;
  tests::Poller poll(FLAGS_long_poll_interval * poll_factor);
  if (poll(func) == 0)
    return true;
  return false;
}

#define LOG_CHECK_PRINTF(fmt, ...)  \
    if (log_error) {printf(fmt, __VA_ARGS__);}

int
compress_status_verify(dp_mem_t *status,
                       dp_mem_t *dst_buf,
                       const cp_desc_t& desc,
                       bool log_error)
{
    cp_status_sha512_t *st = (cp_status_sha512_t *)status->read_thru();

    if (!st->valid) {
        LOG_CHECK_PRINTF("%s ERROR: Status valid bit not set\n", __func__);
        return -1;
    }
    if (st->err) {
        LOG_CHECK_PRINTF("%s ERROR: status err 0x%x is unexpected\n",
                         __func__, st->err);
        return -1;
    }
    if (st->partial_data != desc.status_data) {
        LOG_CHECK_PRINTF("%s ERROR: partial data mismatch, expected 0x%x, "
                         "received 0x%x\n", __func__, desc.status_data,
                         st->partial_data);
        return -1;
    }
    if (desc.cmd_bits.comp_decomp_en && desc.cmd_bits.insert_header) {
        dp_mem_t *hdr_buf = dst_buf->fragment_find(0, sizeof(cp_hdr_t));
        cp_hdr_t *hdr = (cp_hdr_t *)hdr_buf->read_thru();

        if (hdr->version != kCPVersion) {
            LOG_CHECK_PRINTF("%s Header version mismatch, expected 0x%x, "
                             "received 0x%x\n", __func__, kCPVersion, hdr->version);
            return -1;
        }
        if (hdr->cksum == 0) {
            LOG_CHECK_PRINTF("%s ERROR: Header with zero cksum\n", __func__);
            return -1;
        }
        uint32_t datain_len = desc.datain_len == 0 ?
                              kCompEngineMaxSize : desc.datain_len;
        if ((hdr->data_len == 0) ||
            (hdr->data_len > (dst_buf->line_size_get() - sizeof(cp_hdr_t))) ||
            (hdr->data_len > datain_len)) {

            LOG_CHECK_PRINTF("%s ERROR: Invalid data_len 0x%x\n",
                             __func__, hdr->data_len);
            return -1;
        }
        if (st->output_data_len != (hdr->data_len + sizeof(cp_hdr_t))) {
            LOG_CHECK_PRINTF("%s ERROR: output_data_len %u mismatches hdr size %d\n",
                             __func__, st->output_data_len,
                             (int)(hdr->data_len + sizeof(cp_hdr_t)));
            return -1;
        }
    }
    if (desc.cmd_bits.sha_en) {
        int sha_size = desc.cmd_bits.sha_type ? 32 : 64;
        if (memcmp(st->sha512, all_zeros, sha_size) == 0) {
            LOG_CHECK_PRINTF("%s ERROR: Sha is all zero\n", __func__);
            return -1;
        }
    }
    if (st->integrity_data == 0) {
        LOG_CHECK_PRINTF("%s Integrity is all zero\n", __func__);
        return -1;
    }
    return 0;
}

int
decompress_status_verify(dp_mem_t *status,
                         const cp_desc_t& desc,
                         uint32_t exp_output_data_len,
                         bool log_error)
{
    cp_status_sha512_t *st = (cp_status_sha512_t *)status->read_thru();

    if (!st->valid) {
        LOG_CHECK_PRINTF("%s ERROR: Status valid bit not set\n", __func__);
        return -1;
    }
    if (st->err) {
        LOG_CHECK_PRINTF("%s ERROR: status err 0x%x is unexpected\n",
                         __func__, st->err);
        return -1;
    }
    if (st->partial_data != desc.status_data) {
        LOG_CHECK_PRINTF("%s ERROR: partial data mismatch, expected 0x%x, "
                         "received 0x%x\n",__func__, desc.status_data,
                         st->partial_data);
        return -1;
    }
    if (st->output_data_len != exp_output_data_len) {
        LOG_CHECK_PRINTF("%s ERROR: output data len mismatch, expected %u, "
                         "received %u\n", __func__, exp_output_data_len,
                         st->output_data_len);
        return -1;
    }
    return 0;
}

uint32_t
comp_status_output_data_len_get(dp_mem_t *status)
{
    cp_status_sha512_t *st = (cp_status_sha512_t *)status->read_thru();
    return st->output_data_len;
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


comp_queue_t::comp_queue_t(uint64_t cfg_q_base,
                           uint64_t cfg_q_pd_idx,
                           uint32_t size) :
    cfg_q_base(cfg_q_base),
    cfg_q_pd_idx(cfg_q_pd_idx),
    curr_seq_comp_qid(0),
    curr_seq_comp_pd_idx(0),
    curr_pd_idx(0),
    curr_push_type(COMP_QUEUE_PUSH_INVALID)
{
    q_size = size == 0 ? kMaxSubqEntries : size;
    shadow_pd_idx_mem = new dp_mem_t(1, sizeof(uint32_t));

    // If comp was initialized by HAL, q_base_mem below would be used
    // as descriptor cache for sequencer submission.
    q_base_mem = (cp_desc_t *)alloc_page_aligned_host_mem(sizeof(cp_desc_t) * q_size);
    assert(q_base_mem != nullptr);

    if (comp_inited_by_hal) {
        q_base_mem_pa = queue_mem_pa_get(cfg_q_base);
    } else {
        q_base_mem_pa = host_mem_v2p(q_base_mem);
    }
}

void
comp_queue_t::push(const cp_desc_t& src_desc,
                   comp_queue_push_t push_type,
                   uint32_t seq_comp_qid)
{
    cp_desc_t   *dst_desc;
    dp_mem_t    *seq_comp_desc;
    uint16_t    pd_idx;

    switch (push_type) {

    case COMP_QUEUE_PUSH_SEQUENCER:
    case COMP_QUEUE_PUSH_SEQUENCER_DEFER:
    case COMP_QUEUE_PUSH_SEQUENCER_BATCH:
    case COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST:
        pd_idx = curr_pd_idx;
        curr_pd_idx = (curr_pd_idx + 1) % q_size;

        dst_desc = &q_base_mem[pd_idx];
        memcpy(dst_desc, &src_desc, sizeof(*dst_desc));

        curr_seq_comp_qid = seq_comp_qid;
        seq_comp_desc = queues::seq_sq_consume_entry(curr_seq_comp_qid,
                                                     &curr_seq_comp_pd_idx);
        seq_comp_desc->clear();
        seq_comp_desc->write_bit_fields(0, 64, host_mem_v2p(dst_desc));
        seq_comp_desc->write_bit_fields(64, 34, cfg_q_pd_idx);
        seq_comp_desc->write_bit_fields(98, 34, shadow_pd_idx_mem->pa());
        seq_comp_desc->write_bit_fields(132, 4, (uint8_t)log2(sizeof(*dst_desc)));
        seq_comp_desc->write_bit_fields(136, 3, (uint8_t)log2(sizeof(uint32_t)));
        seq_comp_desc->write_bit_fields(139, 5, (uint8_t)log2(q_size));
        seq_comp_desc->write_bit_fields(144, 34, q_base_mem_pa);

        // Sequencer queue depth is limited and should be taken into
        // considerations when using batch mode.
        if (push_type == COMP_QUEUE_PUSH_SEQUENCER) {
            seq_comp_desc->write_thru();
            test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE,
                               curr_seq_comp_qid, 0,
                               curr_seq_comp_pd_idx);
            curr_push_type = COMP_QUEUE_PUSH_INVALID;
            break;
        }

        // Indicate Barco batch mode only as instructed;
        // Otherwise, simply defer the push as seen below
        if (push_type != COMP_QUEUE_PUSH_SEQUENCER_DEFER) {
            seq_comp_desc->write_bit_fields(178, 16, pd_idx);
            seq_comp_desc->write_bit_fields(194, 1, 1); /* set barco_batch_mode */
            if (push_type == COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST) {
                seq_comp_desc->write_bit_fields(195, 1, 1); /* set barco_batch_last */
            }
        }

        // defer until caller calls post_push()
        seq_comp_desc->write_thru();
        curr_push_type = push_type;
        break;

    case COMP_QUEUE_PUSH_HW_DIRECT:
    case COMP_QUEUE_PUSH_HW_DIRECT_BATCH:
        write_mem(q_base_mem_pa + (curr_pd_idx * sizeof(cp_desc_t)),
                  (uint8_t *)&src_desc, sizeof(cp_desc_t));
        curr_pd_idx = (curr_pd_idx + 1) % q_size;

        // since we didn't go thru sequencer here, ensure the shadow pindex
        // maintains up-to-date value
        *((uint32_t *)shadow_pd_idx_mem->read()) = curr_pd_idx;
        shadow_pd_idx_mem->write_thru();
        if (push_type == COMP_QUEUE_PUSH_HW_DIRECT) {
            write_reg(cfg_q_pd_idx, curr_pd_idx);
            curr_push_type = COMP_QUEUE_PUSH_INVALID;
            break;
        }

        // defer until caller calls post_push()
        curr_push_type = push_type;
        break;

    default:
        printf("%s unsupported push_type %d\n", __FUNCTION__, push_type);
        assert(0);
        break;
    }
}

// Execute any deferred push() on the given comp_queue.
void
comp_queue_t::post_push(void)
{
    switch (curr_push_type) {

    case COMP_QUEUE_PUSH_SEQUENCER_BATCH:
    case COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST:

        // maintain up-to-date value for shadow pindex in batch mode
        // as P4+ code takes a different path that does not read/update
        // the shadow pindex
        *((uint32_t *)shadow_pd_idx_mem->read()) = curr_pd_idx;
        shadow_pd_idx_mem->write_thru();

        /*
         * Fall through!!!
         */

    case COMP_QUEUE_PUSH_SEQUENCER_DEFER:
        test_ring_doorbell(queues::get_seq_lif(), SQ_TYPE, 
                           curr_seq_comp_qid, 0,
                           curr_seq_comp_pd_idx);
        break;

    case COMP_QUEUE_PUSH_HW_DIRECT_BATCH:
        write_reg(cfg_q_pd_idx, curr_pd_idx);
        break;

    default:
        printf("%s nothing to do for curr_push_type %d\n", __FUNCTION__,
               curr_push_type);
        break;
    }

    curr_push_type = COMP_QUEUE_PUSH_INVALID;
}

void
compression_buf_init()
{
    uint32_t    max_hash_blks;

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
    status_buf = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                              kMinHostMemAllocSize);
    status_buf2 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                               kMinHostMemAllocSize);
    status_host_buf = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                   kMinHostMemAllocSize);
    status_host_buf2 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                                    DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                    kMinHostMemAllocSize);
    opaque_host_buf = new dp_mem_t(1, sizeof(uint64_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                   kMinHostMemAllocSize);
    host_sgl1 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                             sizeof(cp_sgl_t));
    host_sgl2 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                             sizeof(cp_sgl_t));
    host_sgl3 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                             sizeof(cp_sgl_t));
    host_sgl4 = new dp_mem_t(1, sizeof(cp_sgl_t),
                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                             sizeof(cp_sgl_t));
    seq_sgl = new dp_mem_t(1, sizeof(cp_sq_ent_sgl_t),
                           DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                           sizeof(cp_sq_ent_sgl_t));

    xts_status_host_buf = new dp_mem_t(1, sizeof(uint64_t),
                                       DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                       kMinHostMemAllocSize);
    comp_pad_buf = new dp_mem_t(1, 4096, DP_MEM_ALIGN_PAGE);

    // Pre-fill input buffers.
    uint64_t *p64 = (uint64_t *)uncompressed_data;
    for (uint64_t i = 0; i < (kUncompressedDataSize/sizeof(uint64_t)); i++)
      p64[i] = i;

    memcpy(uncompressed_buf->read(), uncompressed_data,
           uncompressed_buf->line_size_get());
    uncompressed_buf->write_thru();
    memcpy(uncompressed_host_buf->read(), uncompressed_data,
          uncompressed_host_buf->line_size_get());
    uncompressed_host_buf->write_thru();

    // Create and initialize compression->XTS-encrypt chaining
    comp_encrypt_chain_params_t cec_ctor;
    comp_encrypt_chain = 
         new comp_encrypt_chain_t(cec_ctor.app_max_size(kCompAppMaxSize).
                                           uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                           comp_mem_type(DP_MEM_TYPE_HBM).
                                           comp_status_mem_type1(DP_MEM_TYPE_HBM).
                                           comp_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                           encrypt_mem_type(DP_MEM_TYPE_HOST_MEM).
                                           destructor_free_buffers(true));
    comp_encrypt_chain_pre_push_params_t cec_pre_push;
    comp_encrypt_chain->pre_push(cec_pre_push.caller_comp_pad_buf(comp_pad_buf).
                                              caller_xts_status_buf(xts_status_host_buf).
                                              caller_xts_opaque_buf(nullptr).
                                              caller_xts_opaque_data(0));

    // Create and initialize XTS-decrypt->decompression chaining
    decrypt_decomp_chain_params_t ddc_ctor;
    decrypt_decomp_chain = 
         new decrypt_decomp_chain_t(ddc_ctor.app_max_size(kCompAppMaxSize).
                                             uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                             xts_status_mem_type1(DP_MEM_TYPE_HBM).
                                             xts_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                             decrypt_mem_type(DP_MEM_TYPE_HBM).
                                             destructor_free_buffers(true));
    decrypt_decomp_chain_pre_push_params_t ddc_pre_push;
    decrypt_decomp_chain->pre_push(ddc_pre_push.caller_comp_status_buf(status_host_buf).
                                                caller_comp_opaque_buf(nullptr).
                                                caller_comp_opaque_data(0));
    // Create and initialize compression->hash chaining
    comp_hash_chain_params_t chc_ctor;
    comp_hash_chain = 
         new comp_hash_chain_t(chc_ctor.app_max_size(kCompAppMaxSize).
                                        uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                        comp_mem_type1(DP_MEM_TYPE_HBM).
                                        comp_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                        comp_status_mem_type1(DP_MEM_TYPE_HBM).
                                        comp_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                        destructor_free_buffers(true));
    max_hash_blks = COMP_HASH_CHAIN_MAX_HASH_BLKS(kCompAppMaxSize, sizeof(cp_hdr_t),
                                                  kCompAppHashBlkSize);
    hash_status_host_vec = new dp_mem_t(max_hash_blks, CP_STATUS_PAD_ALIGNED_SIZE,
                                        DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                        kMinHostMemAllocSize);
    hash_opaque_host_vec = new dp_mem_t(max_hash_blks, sizeof(uint64_t),
                                        DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                        kMinHostMemAllocSize);
    comp_hash_chain_pre_push_params_t chc_pre_push;
    comp_hash_chain->pre_push(chc_pre_push.caller_comp_pad_buf(comp_pad_buf).
                                           caller_hash_status_vec(hash_status_host_vec).
                                           caller_hash_opaque_vec(hash_opaque_host_vec).
                                           caller_hash_opaque_data(kCompHashIntrData));
    // Create and initialize checksum-decompression chaining
    chksum_decomp_chain_params_t cdc_ctor;
    chksum_decomp_chain = 
         new chksum_decomp_chain_t(cdc_ctor.app_max_size(kCompAppMaxSize).
                                            uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                            destructor_free_buffers(true));
    decomp_status_host = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                                      DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                      kMinHostMemAllocSize);
    chksum_decomp_chain_pre_push_params_t cdc_pre_push;
    chksum_decomp_chain->pre_push(cdc_pre_push.caller_decomp_status_buf(decomp_status_host));
}

void
compression_init()
{
  uint32_t lo_reg, hi_reg;

  read_reg(cp_cfg_glob, lo_reg);
  if (comp_inited_by_hal) {
      kCPVersion = lo_reg & 0xffff;
      printf("Comp version is 0x%x\n", kCPVersion);

      read_reg(cp_cfg_dist, lo_reg);
      cp_queue_size = (lo_reg >> 2) & 0xfff;
      cp_hotq_size = (lo_reg >> 14) & 0xfff;
      read_reg(dc_cfg_dist, lo_reg);
      dc_queue_size = (lo_reg >> 2) & 0xfff;
      dc_hotq_size = (lo_reg >> 14) & 0xfff;
      printf("cp_queue_size %u cp_hotq_size %u dc_queue_size %u dc_hotq_size %u\n",
             cp_queue_size, cp_hotq_size, dc_queue_size, dc_hotq_size);
  }

  cp_queue = new comp_queue_t(cp_cfg_q_base, cp_cfg_q_pd_idx, cp_queue_size);
  cp_hotq = new comp_queue_t(cp_cfg_hotq_base, cp_cfg_hotq_pd_idx, cp_hotq_size);
  dc_queue = new comp_queue_t(dc_cfg_q_base, dc_cfg_q_pd_idx, dc_queue_size);
  dc_hotq = new comp_queue_t(dc_cfg_hotq_base, dc_cfg_hotq_pd_idx, dc_hotq_size);

  compression_buf_init();

  if (!comp_inited_by_hal) {

      // Write cp queue base.
      write_reg(cp_cfg_glob, (lo_reg & 0xFFFF0000u) | kCPVersion);
      write_reg(cp_cfg_q_base, cp_queue->q_base_mem_pa_get() & 0xFFFFFFFFu);
      write_reg(cp_cfg_q_base + 4, (cp_queue->q_base_mem_pa_get() >> 32) & 0xFFFFFFFFu);

      write_reg(cp_cfg_hotq_base, cp_hotq->q_base_mem_pa_get() & 0xFFFFFFFFu);
      write_reg(cp_cfg_hotq_base + 4, (cp_hotq->q_base_mem_pa_get() >> 32) & 0xFFFFFFFFu);

      // Write dc queue base.
      read_reg(dc_cfg_glob, lo_reg);
      write_reg(dc_cfg_glob, (lo_reg & 0xFFFF0000u) | kCPVersion);
      write_reg(dc_cfg_q_base, dc_queue->q_base_mem_pa_get() & 0xFFFFFFFFu);
      write_reg(dc_cfg_q_base + 4, (dc_queue->q_base_mem_pa_get() >> 32) & 0xFFFFFFFFu);

      write_reg(dc_cfg_hotq_base, dc_hotq->q_base_mem_pa_get() & 0xFFFFFFFFu);
      write_reg(dc_cfg_hotq_base + 4, (dc_hotq->q_base_mem_pa_get() >> 32) & 0xFFFFFFFFu);

      // Enable all 16 cp engines.
      read_reg(cp_cfg_ueng, lo_reg);
      read_reg(cp_cfg_ueng+4, hi_reg);
      lo_reg |= 0xFFFF;
      hi_reg &= ~(1u << (54 - 32));
      hi_reg &= ~(1u << (53 - 32));
      hi_reg |= (1u << (36 - 32)) |
                (1u << (55 - 32));
      write_reg(cp_cfg_ueng, lo_reg);
      write_reg(cp_cfg_ueng+4, hi_reg);
      // Enable both DC engines.
      read_reg(dc_cfg_ueng, lo_reg);
      read_reg(dc_cfg_ueng+4, hi_reg);
      lo_reg |= 0x3;
      hi_reg &= ~(1u << (54 - 32));
      hi_reg &= ~(1u << (53 - 32));
      hi_reg |= (1u << (36 - 32)) |
                (1u << (55 - 32));
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

int run_cp_test(cp_desc_t& desc,
                dp_mem_t *dst_buf,
                dp_mem_t *status,
                comp_queue_push_t push_type,
                uint32_t seq_comp_qid)
{
    status->clear_thru();
    cp_queue->push(desc, push_type, seq_comp_qid);
    if (!comp_status_poll(status, desc)) {
      printf("ERROR: status never came\n");
      return -1;
    }

    if (compress_status_verify(status, dst_buf, desc) == 0) {
        last_cp_output_data_len = comp_status_output_data_len_get(status);
        printf("Last output data len = %u\n", last_cp_output_data_len);
        return 0;
    }
    return -1;
}

int run_dc_test(cp_desc_t& desc,
                dp_mem_t *status,
                uint32_t exp_output_data_len,
                comp_queue_push_t push_type,
                uint32_t seq_comp_qid)
{
    status->clear_thru();
    dc_queue->push(desc, push_type, seq_comp_qid);
    if (!comp_status_poll(status, desc)) {
      printf("ERROR: status never came\n");
      return -1;
    }
    return decompress_status_verify(status, desc, exp_output_data_len);
}

void compress_cp_desc_template_fill(cp_desc_t &d,
                                    dp_mem_t *src_buf,
                                    dp_mem_t *dst_buf,
                                    dp_mem_t *status_buf,
                                    dp_mem_t *invalidate_hdr_buf,
                                    uint32_t src_len)
{
    memset(&d, 0, sizeof(d));
    d.cmd_bits.comp_decomp_en = 1;
    d.cmd_bits.insert_header = 1;
    d.src = src_buf->pa();
    d.dst = dst_buf->pa();
    d.status_addr = status_buf->pa();
    d.datain_len = src_len == kCompEngineMaxSize ? 0 : src_len;
    d.threshold_len = src_len - sizeof(cp_hdr_t);
    d.status_data = 0x1234;

    if (invalidate_hdr_buf) {
        invalidate_hdr_buf->fragment_find(0, sizeof(cp_hdr_t))->clear_thru();
    }
}

void decompress_cp_desc_template_fill(cp_desc_t &d,
                                      dp_mem_t *src_buf,
                                      dp_mem_t *dst_buf,
                                      dp_mem_t *status_buf,
                                      uint32_t src_len,
                                      uint32_t threshold_len)
{
    memset(&d, 0, sizeof(d));
    d.cmd_bits.comp_decomp_en = 1;
    d.cmd_bits.header_present = 1;
    d.cmd_bits.cksum_verify_en = 1;
    d.src = src_buf->pa();
    d.dst = dst_buf->pa();
    d.status_addr = status_buf->pa();
    d.datain_len = src_len == kCompEngineMaxSize ? 0 : src_len;
    d.threshold_len = threshold_len == kCompEngineMaxSize ? 0 : threshold_len;
    d.status_data = 0x3456;
}

int _compress_flat_64K_buf(comp_queue_push_t push_type,
                           uint32_t seq_comp_qid) {
  cp_desc_t d;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
  compress_cp_desc_template_fill(d, uncompressed_host_buf, compressed_host_buf,
                                 status_host_buf, compressed_host_buf,
                                 kUncompressedDataSize);
  if (run_cp_test(d, compressed_host_buf, status_host_buf,
                  push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  // Save compressed data.
  cp_hdr_t *hdr = (cp_hdr_t *)compressed_host_buf->read_thru();
  memcpy(compressed_data_buf, hdr, hdr->data_len + sizeof(cp_hdr_t));
  compressed_data_size = hdr->data_len + sizeof(cp_hdr_t);
  return 0;
}

int compress_flat_64K_buf() {
  return _compress_flat_64K_buf(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int seq_compress_flat_64K_buf() {
  return _compress_flat_64K_buf(COMP_QUEUE_PUSH_SEQUENCER, 
                                queues::get_seq_comp_sq(0));
}

int _compress_same_src_and_dst(comp_queue_push_t push_type,
                               uint32_t seq_comp_qid) {
  cp_desc_t d;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);

  memcpy(compressed_host_buf->read(), uncompressed_data, kCompAppMinSize);
  compressed_host_buf->write_thru();
  compress_cp_desc_template_fill(d, compressed_host_buf, compressed_host_buf,
                                 status_host_buf, nullptr, kCompAppMinSize);
  if (run_cp_test(d, compressed_host_buf, status_host_buf, 
                  push_type, seq_comp_qid) < 0) {
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
                                      queues::get_seq_comp_sq(0));
}

int _decompress_to_flat_64K_buf(comp_queue_push_t push_type,
                                uint32_t seq_comp_qid) {
  cp_desc_t d;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);

  uncompressed_host_buf->clear_thru();
  memcpy(compressed_host_buf->read(), compressed_data_buf, compressed_data_size);
  compressed_host_buf->write_thru();

  decompress_cp_desc_template_fill(d, compressed_host_buf, uncompressed_host_buf,
                                   status_host_buf, compressed_data_size,
                                   kUncompressedDataSize);
  if (run_dc_test(d, status_host_buf, 0,
                  push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Verify data buf
  if (test_data_verify_and_dump(uncompressed_data,
                                uncompressed_host_buf->read_thru(),
                                kUncompressedDataSize)) {
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
                                       queues::get_seq_comp_sq(0));
}

int compress_odd_size_buf() {
  cp_desc_t d;

  printf("Starting testcase %s\n", __func__);
  compress_cp_desc_template_fill(d, uncompressed_host_buf, compressed_host_buf,
                                 status_host_buf, compressed_host_buf, 567);
  if (run_cp_test(d, compressed_host_buf, status_host_buf) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_odd_size_buf() {
  cp_desc_t d;

  printf("Starting testcase %s\n", __func__);
  decompress_cp_desc_template_fill(d, compressed_host_buf, uncompressed_host_buf,
                                   status_host_buf, last_cp_output_data_len, 567);
  if (run_dc_test(d, status_host_buf, 567) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Verify data buf
  if (test_data_verify_and_dump(uncompressed_data,
                                uncompressed_host_buf->read_thru(),
                                567)) {
      printf("Testcase %s failed\n", __func__);
      return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int _compress_host_sgl_to_host_sgl(comp_queue_push_t push_type,
                                   uint32_t seq_comp_qid) {
  cp_desc_t d;

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
  compress_cp_desc_template_fill(d, host_sgl1, host_sgl3, status_host_buf,
                                 compressed_host_buf, 6000);
  d.cmd_bits.src_is_list = 1;
  d.cmd_bits.dst_is_list = 1;
  if (run_cp_test(d, compressed_host_buf, status_host_buf, 
                  push_type, seq_comp_qid) < 0) {
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
                                          queues::get_seq_comp_sq(0));
}

int _decompress_host_sgl_to_host_sgl(comp_queue_push_t push_type,
                                     uint32_t seq_comp_qid) {
  cp_desc_t d;

  // clear some initial area.
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
  decompress_cp_desc_template_fill(d, host_sgl1, host_sgl3, status_host_buf,
                                   last_cp_output_data_len, 6000);
  d.cmd_bits.src_is_list = 1;
  d.cmd_bits.dst_is_list = 1;
  if (run_dc_test(d, status_host_buf, 6000, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  if (test_data_verify_and_dump(uncompressed_data,
                                uncompressed_host_buf->read_thru(),
                                6000)) {
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
                                            queues::get_seq_comp_sq(0));
}

int _compress_flat_64K_buf_in_hbm(comp_queue_push_t push_type,
                                  uint32_t seq_comp_qid) {
  cp_desc_t d;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);
  compress_cp_desc_template_fill(d, uncompressed_buf, compressed_buf,
                                 status_buf, compressed_buf, kUncompressedDataSize);
  if (run_cp_test(d, compressed_buf, status_buf, push_type, seq_comp_qid) < 0) {
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
                                         queues::get_seq_comp_sq(0));
}

int _decompress_to_flat_64K_buf_in_hbm(comp_queue_push_t push_type,
                                       uint32_t seq_comp_qid) {

  cp_desc_t d;

  // clear some initial area.
  dp_mem_t *partial_buf = uncompressed_buf->fragment_find(0, 16);
  partial_buf->clear_thru();

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);
  decompress_cp_desc_template_fill(d, compressed_buf, uncompressed_buf,
                                   status_buf, compressed_data_size,
                                   kUncompressedDataSize);
  if (run_dc_test(d, status_buf, 0, push_type, seq_comp_qid) < 0) {
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
                                              queues::get_seq_comp_sq(0));
}

// Route the compressed output through sequencer to handle output block
// boundry issues of compression engine.
int _compress_output_through_sequencer(comp_queue_push_t push_type,
                                       uint32_t seq_comp_qid) {
  cp_desc_t d;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
         __func__, push_type, seq_comp_qid);
  compress_cp_desc_template_fill(d, uncompressed_host_buf, compressed_buf,
                                 status_buf, compressed_buf,
                                 kUncompressedDataSize);
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

  acc_chain_params_t chain_params = {0};
  status_host_buf->clear_thru();
  chain_params.desc_format_fn = test_setup_post_comp_seq_status_entry;
  chain_params.chain_ent.status_hbm_pa = status_buf->pa();
  chain_params.chain_ent.status_host_pa = status_host_buf->pa();
  chain_params.chain_ent.src_hbm_pa = d.src;
  chain_params.chain_ent.dst_hbm_pa = d.dst;
  chain_params.chain_ent.sgl_pdma_out_pa = seq_sgl->pa();
  chain_params.chain_ent.sgl_pdma_en = 1;
  chain_params.chain_ent.intr_pa = opaque_host_buf->pa();
  chain_params.chain_ent.intr_data = kCompSeqIntrData;

  // Clear the area where interrupt from sequencer is going to come.
  opaque_host_buf->clear_thru();
  chain_params.chain_ent.status_len = status_buf->line_size_get();
  chain_params.chain_ent.status_dma_en = 1;
  chain_params.chain_ent.intr_en = 1;
  chain_params.seq_status_q = queues::get_seq_comp_status_sq(0);
  if (test_setup_seq_acc_chain_entry(chain_params) != 0) {
    printf("cp_chain_ent failed\n");
    return -1;
  }
  d.doorbell_addr = chain_params.ret_doorbell_addr;
  d.doorbell_data = chain_params.ret_doorbell_data;
  d.cmd_bits.doorbell_on = 1;

  // Verify(wait for) that the status makes it to HBM.
  // We could directly poll for sequencer but this is just additional verification.
  if (run_cp_test(d, compressed_buf, status_buf, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  // Now poll for sequencer interrupt.
  auto seq_intr_poll_func = [] () -> int {
    uint32_t *p = (uint32_t *)opaque_host_buf->read_thru();
    if (*p == kCompSeqIntrData)
      return 0;
    return 1;
  };
  tests::Poller intr_poll;
  if (intr_poll(seq_intr_poll_func) != 0) {
    printf("ERROR: Interrupt from sequencer never came.\n");
  }
  if (test_data_verify_and_dump(compressed_data_buf,
                                compressed_host_buf->read_thru(),
                                last_cp_output_data_len)) {
      printf("Testcase %s failed\n", __func__);
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
                                              queues::get_seq_comp_sq(0));
}

// Verify integrity of >64K buffer
int verify_integrity_for_gt64K() {
  cp_desc_t d;
  printf("Starting testcase %s\n", __func__);

  compress_cp_desc_template_fill(d, uncompressed_host_buf, compressed_host_buf,
                                 status_host_buf, nullptr, kCompAppMinSize);
  // Give 68K length
  d.extended_len = 1;
  d.status_data = 0x7654;
  if (run_cp_test(d, compressed_host_buf, status_host_buf) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  cp_status_sha512_t *st2 = (cp_status_sha512_t *)status_host_buf->read();
  uint64_t old_integiry = st2->integrity_data;

  // Now repeat the test using only 4K data i.e. higher 16 bits are zero.
  compress_cp_desc_template_fill(d, uncompressed_host_buf, compressed_host_buf,
                                 status_host_buf, nullptr, kCompAppMinSize);
  d.status_data = 0x5454;
  if (run_cp_test(d, compressed_host_buf, status_host_buf) < 0) {
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

int _max_data_rate(comp_queue_push_t push_type,
                   uint32_t seq_comp_qid_cp,
                   uint32_t seq_comp_qid_dc)
{
  comp_queue_push_t last_push_type;
  cp_desc_t         comp_cp_desc[MAX_CP_REQ];
  cp_desc_t         decomp_cp_desc[MAX_DC_REQ];
  cp_desc_t         *d;
  uint32_t          source_compressed_len;

  // Generate source compressed data for use by decompression below
  dp_mem_t *partial_buf = uncompressed_buf->fragment_find(0, kCompAppNominalSize);
  memcpy(partial_buf->read(), uncompressed_data, kCompAppNominalSize);
  partial_buf->write_thru();
  compress_cp_desc_template_fill(comp_cp_desc[0], partial_buf, compressed_buf,
                                 status_buf, compressed_buf, kCompAppNominalSize);
  if (run_cp_test(comp_cp_desc[0], compressed_buf, status_buf) < 0) {
    printf("%s source compressed data generation failed\n", __func__);
    return -1;
  }
  source_compressed_len = last_cp_output_data_len;

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
  dp_mem_t *max_cp_uncompressed_buf = new dp_mem_t(MAX_CP_REQ, kCompAppNominalSize,
                                                   DP_MEM_ALIGN_PAGE);
  dp_mem_t *max_cp_compressed_buf = new dp_mem_t(MAX_CP_REQ, kCompAppNominalSize,
                                                 DP_MEM_ALIGN_PAGE);
  dp_mem_t *max_cp_status_buf = new dp_mem_t(MAX_CP_REQ, CP_STATUS_PAD_ALIGNED_SIZE,
                                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                             kMinHostMemAllocSize);
  dp_mem_t *max_dc_uncompressed_buf = new dp_mem_t(MAX_DC_REQ, kCompAppNominalSize,
                                                   DP_MEM_ALIGN_PAGE);
  dp_mem_t *max_dc_status_buf = new dp_mem_t(MAX_DC_REQ, CP_STATUS_PAD_ALIGNED_SIZE,
                                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                             kMinHostMemAllocSize);
  // allocate opaque tags as a byte stream for easy memcmp
  dp_mem_t *max_cp_opaque_host_buf = new dp_mem_t(1, MAX_CP_REQ * sizeof(uint64_t),
                                                  DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                                  kMinHostMemAllocSize);
  dp_mem_t *exp_opaque_data_buf = new dp_mem_t(1, MAX_CP_REQ * sizeof(uint64_t),
                                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                               kMinHostMemAllocSize);
  dp_mem_t *max_dc_opaque_host_buf = new dp_mem_t(1, MAX_DC_REQ * sizeof(uint64_t),
                                                  DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                                  kMinHostMemAllocSize);
  uint32_t exp_opaque_data = 0xa5a5a5a5;

  // Note that RTL expects each opaque tag as uint64_t and writes
  // exp_opaque_data to the first 4 bytes, followed by 0 in the next 4 bytes
  for (i = 0; i < MAX_CP_REQ; i++) {
      memcpy(exp_opaque_data_buf->read() + (i * sizeof(uint64_t)),
             &exp_opaque_data, sizeof(uint32_t));
  }
  exp_opaque_data_buf->write_thru();

  // allocate and fill descriptors to load compression engine with requests
  for (i = 0, d = &comp_cp_desc[0];
       i < MAX_CP_REQ;
       i++, d++) {

    max_cp_uncompressed_buf->line_set(i);
    max_cp_compressed_buf->line_set(i);
    max_cp_status_buf->line_set(i);
    compress_cp_desc_template_fill(*d, max_cp_uncompressed_buf,
                                   max_cp_compressed_buf, max_cp_status_buf,
                                   nullptr, kCompAppNominalSize);
    d->status_data += i;
    d->cmd_bits.opaque_tag_on = 1;
    d->opaque_tag_addr = max_cp_opaque_host_buf->pa() + (i * sizeof(uint64_t));
    d->opaque_tag_data = exp_opaque_data;
    cp_queue->push(*d, 
                    i == (MAX_CP_REQ - 1) ? last_push_type : push_type,
                    seq_comp_qid_cp);
  }

  // Ring the doorbell after loading decompression engine with requests
  for (i = 0, d = &decomp_cp_desc[0];
       i < MAX_DC_REQ;
       i++, d++) {

    // decompress from data obtained from sourced compressed data
    // created earlier
    max_dc_uncompressed_buf->line_set(i);
    max_dc_status_buf->line_set(i);
    decompress_cp_desc_template_fill(*d, compressed_buf, max_dc_uncompressed_buf,
                                     max_dc_status_buf, source_compressed_len,
                                     kCompAppNominalSize);
    d->status_data += i;
    d->cmd_bits.opaque_tag_on = 1;
    d->opaque_tag_addr = max_dc_opaque_host_buf->pa() + (i * sizeof(uint64_t));
    d->opaque_tag_data = exp_opaque_data;
    dc_queue->push(*d,
                   i == (MAX_DC_REQ - 1) ? last_push_type : push_type,
                   seq_comp_qid_dc);
  }

  // Now ring doorbells
  cp_queue->post_push();
  dc_queue->post_push();

  // Wait for all the interrupts
  auto func = [max_cp_opaque_host_buf, max_dc_opaque_host_buf,
               exp_opaque_data_buf,
               max_cp_status_buf, max_dc_status_buf,
               max_cp_compressed_buf,
               comp_cp_desc, decomp_cp_desc] () -> int {

      const cp_desc_t   *d;
      uint32_t          i;

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

      for (i = 0, d = &comp_cp_desc[0];
           i < MAX_CP_REQ;
           i++, d++) {

          max_cp_status_buf->line_set(i);
          max_cp_compressed_buf->line_set(i);
          if (compress_status_verify(max_cp_status_buf,
                                     max_cp_compressed_buf, *d, false)) {
              return -1;
          }
      }

      for (i = 0, d = &decomp_cp_desc[0];
           i < MAX_DC_REQ;
           i++, d++) {

          max_dc_status_buf->line_set(i);
          if (decompress_status_verify(max_dc_status_buf, *d,
                                       kCompAppNominalSize, false)) {
              return -1;
          }
      }

      return 0;
  };

  tests::Poller poll(FLAGS_long_poll_interval);
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
                          queues::get_seq_comp_sq(0),
                          queues::get_seq_comp_sq(1));
}

static int cp_dualq_flat_4K_buf(dp_mem_t *comp_buf,
                                dp_mem_t *uncomp_buf,
                                dp_mem_t *status_buf1,
                                dp_mem_t *status_buf2,
                                comp_queue_push_t push_type,
                                uint32_t seq_comp_qid_cp,
                                uint32_t seq_comp_qid_hotq) {
  cp_desc_t lq_desc;
  cp_desc_t hq_desc;

  // Setup compression descriptor for low prirority queue
  compress_cp_desc_template_fill(lq_desc, uncomp_buf, comp_buf,
                                 status_buf1, comp_buf, kCompAppMinSize);

  // Setup compression descriptor for high prirority queue
  dp_mem_t *hq_uncomp_buf = uncomp_buf->fragment_find(kCompAppMinSize,
                                                      kCompAppMinSize);
  dp_mem_t *hq_comp_buf = comp_buf->fragment_find(kCompAppMinSize,
                                                  kCompAppMinSize);
  compress_cp_desc_template_fill(hq_desc, hq_uncomp_buf, hq_comp_buf,
                                 status_buf2, hq_comp_buf, kCompAppMinSize);
  hq_desc.status_data = lq_desc.status_data * 2;

  // Initialize status for both the requests
  status_buf1->clear_thru();
  status_buf2->clear_thru();

  // Add descriptor for both high and low priority queues
  cp_queue->push(lq_desc, push_type, seq_comp_qid_cp);

  // Dont ring the doorbell yet
  cp_hotq->push(hq_desc, push_type, seq_comp_qid_hotq);

  // Now ring door bells for both high and low queues
  cp_queue->post_push();
  cp_hotq->post_push();

  // Check status update to both the descriptors
  auto func = [status_buf1, status_buf2,
               comp_buf, hq_comp_buf,
               lq_desc, hq_desc] () -> int {
    if (compress_status_verify(status_buf1, comp_buf, lq_desc, false)) {
      return -1;
    }

    if (compress_status_verify(status_buf2, hq_comp_buf, hq_desc, false)) {
      return -1;
    }

    return 0;
  };

  tests::Poller poll(FLAGS_long_poll_interval);
  if (poll(func) == 0) {
    printf("Testcase %s passed\n", __func__);
    return 0;
  }

  if (compress_status_verify(status_buf1, comp_buf, lq_desc)) {
    printf("Compression request in low queue verification failed\n");
  }

  if (compress_status_verify(status_buf2, hq_comp_buf, hq_desc)) {
    printf("Compression request in high/hot queue verification failed\n");
  }

  return -1;
}

int compress_dualq_flat_4K_buf() {
  printf("Starting testcase %s\n", __func__);

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

  int rc = cp_dualq_flat_4K_buf(compressed_host_buf, uncompressed_host_buf,
                                status_host_buf, status_host_buf2,
                                COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST,
                                queues::get_seq_comp_sq(0),
                                queues::get_seq_comp_sq(1));
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

int compress_dualq_flat_4K_buf_in_hbm() {
  printf("Starting testcase %s\n", __func__);

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

  int rc = cp_dualq_flat_4K_buf(compressed_buf, uncompressed_buf,
                                status_buf, status_buf2,
                                COMP_QUEUE_PUSH_SEQUENCER_BATCH_LAST,
                                queues::get_seq_comp_sq(0),
                                queues::get_seq_comp_sq(1));
  if (rc == 0)
    printf("Testcase %s passed\n", __func__);
  else
    printf("Testcase %s failed\n", __func__);

  return rc;
}

// Accelerator compression to XTS-encrypt chaining DOLs.
int compress_output_encrypt_app_min_size() {
    comp_encrypt_chain_push_params_t    params;
    comp_encrypt_chain->push(params.app_blk_size(kCompAppMinSize).
                                    comp_queue(cp_queue).
                                    push_type(COMP_QUEUE_PUSH_HW_DIRECT).
                                    seq_comp_qid(0).
                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(0)).
                                    seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    comp_encrypt_chain->post_push();
    return comp_encrypt_chain->full_verify();
}

int seq_compress_output_encrypt_app_min_size() {
    comp_encrypt_chain_push_params_t    params;
    comp_encrypt_chain->push(params.app_blk_size(kCompAppMinSize).
                                    comp_queue(cp_queue).
                                    push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                    seq_comp_qid(queues::get_seq_comp_sq(0)).
                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(0)).
                                    seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    comp_encrypt_chain->post_push();
    return comp_encrypt_chain->full_verify();
}

int compress_output_encrypt_app_max_size() {
    comp_encrypt_chain_push_params_t    params;
    comp_encrypt_chain->push(params.app_blk_size(kCompAppMaxSize).
                                    comp_queue(cp_queue).
                                    push_type(COMP_QUEUE_PUSH_HW_DIRECT).
                                    seq_comp_qid(0).
                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(0)).
                                    seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    comp_encrypt_chain->post_push();
    return comp_encrypt_chain->full_verify();
}

int seq_compress_output_encrypt_app_max_size() {
    comp_encrypt_chain_push_params_t    params;
    comp_encrypt_chain->push(params.app_blk_size(kCompAppMaxSize).
                                    comp_queue(cp_queue).
                                    push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                    seq_comp_qid(queues::get_seq_comp_sq(0)).
                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(0)).
                                    seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    comp_encrypt_chain->post_push();
    return comp_encrypt_chain->full_verify();
}

int compress_output_encrypt_app_nominal_size() {
    comp_encrypt_chain_push_params_t    params;
    comp_encrypt_chain->push(params.app_blk_size(kCompAppNominalSize).
                                    comp_queue(cp_queue).
                                    push_type(COMP_QUEUE_PUSH_HW_DIRECT).
                                    seq_comp_qid(0).
                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(0)).
                                    seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    comp_encrypt_chain->post_push();
    return comp_encrypt_chain->full_verify();
}

int seq_compress_output_encrypt_app_nominal_size() {
    comp_encrypt_chain_push_params_t    params;
    comp_encrypt_chain->push(params.app_blk_size(kCompAppNominalSize).
                                    comp_queue(cp_queue).
                                    push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                    seq_comp_qid(queues::get_seq_comp_sq(0)).
                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(0)).
                                    seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    comp_encrypt_chain->post_push();
    return comp_encrypt_chain->full_verify();
}

int _compress_clear_insert_header(comp_queue_push_t push_type,
                                  uint32_t seq_comp_qid) {
  cp_desc_t d;

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);

  /* flat 64K buf in hbm */
  compress_cp_desc_template_fill(d, uncompressed_buf, compressed_buf,
                                 status_buf, compressed_buf, kUncompressedDataSize);

  /* reset the bit not to insert the header */
  d.cmd_bits.insert_header = 0;

  if (run_cp_test(d, compressed_buf, status_buf, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }
  printf("Testcase %s passed\n", __func__);
  return 0;
}

int compress_clear_insert_header() {
    return _compress_clear_insert_header(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

int _decompress_clear_header_present(comp_queue_push_t push_type,
                                       int32_t seq_comp_qid) {

  cp_desc_t d;

  // clear some initial area.
  dp_mem_t *partial_buf = uncompressed_buf->fragment_find(0, 16);
  partial_buf->clear_thru();

  printf("Starting testcase %s push_type %d seq_comp_qid %u\n",
          __func__, push_type, seq_comp_qid);

  /* flat 64K buf in hbm */
  decompress_cp_desc_template_fill(d, compressed_buf, uncompressed_buf,
                                   status_buf, compressed_data_size,
                                   kUncompressedDataSize);

  /* reset the bit to indicate header is not present */
  d.cmd_bits.header_present = 0;

  if (run_dc_test(d, status_buf, 0, push_type, seq_comp_qid) < 0) {
    printf("Testcase %s failed\n", __func__);
    return -1;
  }

  // Verify data buf
  if (test_data_verify_and_dump(uncompressed_data,
                                uncompressed_buf->read_thru(),
                                kUncompressedDataSize)) {
      printf("Testcase %s failed\n", __func__);
      return -1;
  }

  printf("Testcase %s passed\n", __func__);
  return 0;
}

int decompress_clear_header_present() {
    return _decompress_clear_header_present(COMP_QUEUE_PUSH_HW_DIRECT, 0);
}

// Accelerator XTS-decrypt to decompression chaining DOLs.
int seq_decrypt_output_decompress_last_app_blk() {

    // Execute decrypt-decompression on the last compress-pad-encrypted block,
    // i.e., the block size is whatever was last compressed and padded.
    // 
    // Use the xts_ctx default mode of XTS sequencer queue, with chaining
    // to decompression initiated from P4+ handling of XTS status sequencer.
    decrypt_decomp_chain_push_params_t  params;
    decrypt_decomp_chain->push(params.comp_encrypt_chain(comp_encrypt_chain).
                                      decomp_queue(dc_queue).
                                      seq_xts_qid(queues::get_seq_xts_sq(0)).
                                      seq_xts_status_qid(queues::get_seq_xts_status_sq(0)).
                                      seq_comp_status_qid(queues::get_seq_comp_status_sq(0)));
    decrypt_decomp_chain->post_push();
    return decrypt_decomp_chain->full_verify();
}

int seq_compress_output_hash_app_max_size() {
    comp_hash_chain_push_params_t   params;

    // Compression and hash both using cp_queue
    comp_hash_chain->push(params.app_blk_size(kCompAppMaxSize).
                                 app_hash_size(kCompAppHashBlkSize).
                                 integrity_type(COMP_INTEGRITY_M_ADLER32).
                                 comp_queue(cp_queue).
                                 hash_queue(cp_queue).
                                 push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                 seq_comp_qid(queues::get_seq_comp_sq(0)).
                                 seq_comp_status_qid(queues::get_seq_comp_status_sq(0)));
    comp_hash_chain->post_push();
    return comp_hash_chain->full_verify();
}

int seq_compress_output_hash_app_test_size() {
    comp_hash_chain_push_params_t   params;

    // Note: cp_queue being used for compression and cp_hotq for hashing
    comp_hash_chain->push(params.app_blk_size(kCompAppTestSize).
                                 app_hash_size(kCompAppHashBlkSize).
                                 comp_queue(cp_queue).
                                 hash_queue(cp_hotq).
                                 push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                 seq_comp_qid(queues::get_seq_comp_sq(0)).
                                 seq_comp_status_qid(queues::get_seq_comp_status_sq(0)));
    comp_hash_chain->post_push();
    return comp_hash_chain->full_verify();
}

int seq_compress_output_hash_app_nominal_size() {
    comp_hash_chain_push_params_t   params;

    // Note: cp_hotq being used for compression and cp_queue for hashing
    comp_hash_chain->push(params.app_blk_size(kCompAppNominalSize).
                                 app_hash_size(kCompAppHashBlkSize).
                                 integrity_type(COMP_INTEGRITY_M_ADLER32).
                                 comp_queue(cp_hotq).
                                 hash_queue(cp_queue).
                                 push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                 seq_comp_qid(queues::get_seq_comp_sq(0)).
                                 seq_comp_status_qid(queues::get_seq_comp_status_sq(0)));
    comp_hash_chain->post_push();
    return comp_hash_chain->full_verify();
}

// Accelerator checksum to decompression chaining DOLs.
int seq_chksum_decompress_last_app_blk() {

    // Execute checksum-decompression on the last compress-hash block,
    // i.e., the block size is whatever was last compressed.
    chksum_decomp_chain_push_params_t  params;
    chksum_decomp_chain->push(params.comp_hash_chain(comp_hash_chain).
                                     chksum_queue(dc_hotq).
                                     decomp_queue(dc_queue).
                                     push_type(COMP_QUEUE_PUSH_SEQUENCER).
                                     seq_chksum_qid(queues::get_seq_comp_sq(0)).
                                     seq_decomp_qid(queues::get_seq_comp_sq(1)));
    chksum_decomp_chain->post_push();
    return chksum_decomp_chain->full_verify();
}

}  // namespace tests
