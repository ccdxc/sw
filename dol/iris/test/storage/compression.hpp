#ifndef	COMPRESSION_HPP_
#define	COMPRESSION_HPP_

#include <stdint.h>
#include "dol/iris/test/storage/acc_ring.hpp"
#include "dol/iris/test/storage/dp_mem.hpp"
#include "dol/iris/test/storage/chain_params.hpp"

using namespace dp_mem;

// Calculate max number of hash blocks required for a given app_max_size
// and app_hash_size.
#define COMP_MAX_HASH_BLKS(app_max_size, app_hash_size)   \
    (((app_max_size) + (app_hash_size) - 1) / (app_hash_size))


namespace tests {

class comp_encrypt_chain_t;
class decrypt_decomp_chain_t;

typedef struct ccmd {
  uint16_t comp_decomp_en:1, // 1-engine enable, 0-only SHA/integrity
           header_present:1, // must be 1 for decompression
           insert_header:1,  // must be 1 for compression
           doorbell_on:1,
           opaque_tag_on:1,
           src_is_list:1,
           dst_is_list:1,
           cksum_verify_en:1,// valid for decompression only
           cksum_adler:1,    // 0-ADLER32, 1-CRC32
           sha_en:1,
           sha_type:2,       // 00-512, 01-256
           // The following is N/A if comp_decomp_en = 0. In that case
           // integrity_src always is input data.
           integrity_src:1,  // 0-Compressed data, 1-Uncompressed data
           integrity_type:3; // 000:M-CRC64, 001:CRC32C, 010:Adler32 011:M-Adler32
} ccmd_t;

#define COMP_DEDUP_SHA512               0
#define COMP_DEDUP_SHA256               1

#define COMP_INTEGRITY_SRC_COMP_DATA    0
#define COMP_INTEGRITY_SRC_UNCOMP_DATA  1

#define COMP_INTEGRITY_M_CRC64          0
#define COMP_INTEGRITY_CRC32C           1
#define COMP_INTEGRITY_ADLER32          2
#define COMP_INTEGRITY_M_ADLER32        3

#define COMP_CHECKSUM_ADLER32           0
#define COMP_CHECKSUM_CRC32C            1

constexpr const char *integrity_type_name[] = {
    "M_CRC64",
    "CRC32C",
    "ADLER32",
    "M_ADLER32"
};

constexpr const char *integrity_src_name[] = {
    "SRC_COMP_DATA",
    "SRC_UNCOMP_DATA"
};

constexpr const char *sha_type_name[] = {
    "SHA512",
    "SHA256"
};

constexpr const char *sha_en_name[] = {
    "disabled",
    "enabled"
};

typedef struct cp_desc {
  uint64_t src;
  uint64_t dst;
  union {
    ccmd_t   cmd_bits;
    uint16_t cmd;
  };
  uint16_t datain_len;  // Length of input data, 0 = 64KB
  uint16_t extended_len;// High order input data len, valid only if comp_decomp_en = 0
  uint16_t threshold_len;
  uint64_t status_addr;
  uint64_t doorbell_addr;
  uint64_t doorbell_data;
  uint64_t opaque_tag_addr;
  uint32_t opaque_tag_data;
  uint32_t status_data;
} cp_desc_t;

#ifdef ELBA
typedef struct cp_sgl {
  uint64_t addr0;
  uint32_t rsvd0;
  uint32_t len0;
  uint64_t addr1;
  uint32_t rsvd1;
  uint32_t len1;
  uint64_t addr2;
  uint32_t rsvd2;
  uint32_t len2;
  uint64_t link;  // next descriptor
  uint64_t rsvd;
} cp_sgl_t;
#else
typedef struct cp_sgl {
  uint64_t addr0;
  uint32_t len0;
  uint32_t rsvd0;
  uint64_t addr1;
  uint32_t len1;
  uint32_t rsvd1;
  uint64_t addr2;
  uint32_t len2;
  uint32_t rsvd2;
  uint64_t link;  // next descriptor
  uint64_t rsvd;
} cp_sgl_t;
#endif

typedef struct cp_hdr {
  uint32_t cksum;
  uint16_t data_len;
  uint16_t version;
} cp_hdr_t;

// Status codes
#define CP_STATUS_SUCCESS		0
#define CP_STATUS_AXI_TIMEOUT		1
#define CP_STATUS_AXI_DATA_ERROR	2
#define CP_STATUS_AXI_ADDR_ERROR	3
#define CP_STATUS_COMPRESSION_FAILED	4
#define CP_STATUS_DATA_TOO_LONG	5
#define CP_STATUS_CHECKSUM_FAILED	6
#define CP_STATUS_SGL_DESC_ERROR	7

#define CP_STATUS_PAD_ALIGNED_SIZE                              \
  (((sizeof(cp_status_sha512_t) + kMinHostMemAllocSize - 1) /   \
   kMinHostMemAllocSize) * kMinHostMemAllocSize)

typedef struct cp_status_no_hash {
  uint16_t rsvd:12,
           err:3,  // See status code above
           valid:1;
  uint16_t output_data_len;
  uint32_t partial_data;
  uint64_t integrity_data;
} cp_status_no_hash_t;

typedef struct cp_status_sha512 {
  uint16_t rsvd:12,
           err:3,  // See status code above
           valid:1;
  uint16_t output_data_len;
  uint32_t partial_data;
  uint64_t integrity_data;
  uint8_t  sha512[64];
} cp_status_sha512_t;

typedef struct cp_status_sha256 {
  uint16_t rsvd:12,
           err:3,  // See status code above
           valid:1;
  uint16_t output_data_len;  // Includes header length.
  uint32_t partial_data;
  uint64_t integrity_data;
  uint8_t  sha256[32];
} cp_status_sha256_t;


// Max block size supported by hardware engine
constexpr uint32_t kCompEngineMaxSize = 65536;

// Typical sizes used by customer application
constexpr uint32_t kCompAppMinSize = 4096;
constexpr uint32_t kCompAppMaxSize = 32768;
constexpr uint32_t kCompAppNominalSize = 8192;
constexpr uint32_t kCompAppTestSize = 16384;

constexpr uint32_t kCompAppHashBlkSize = 4096;

constexpr uint32_t kCompSeqIntrData = 0x11223344;
constexpr uint32_t kCompHashIntrData = 0xaabbccdd;

extern acc_ring_t *cp_ring;
extern acc_ring_t *dc_ring;

extern acc_ring_t *cp_hot_ring;
extern acc_ring_t *dc_hot_ring;

extern dp_mem_t *comp_pad_buf;

void comp_sgl_packed_fill(dp_mem_t *comp_sgl_vec,
                          dp_mem_t *comp_buf,
                          uint32_t blk_size);
void comp_sgl_trace(const char *comp_sgl_name,
                    dp_mem_t *comp_sgl_vec,
                    uint32_t max_blks,
                    bool honor_link);
void chain_sgl_pdma_packed_fill(dp_mem_t *seq_sgl_pdma,
                                dp_mem_t *dst_buf);
bool comp_status_poll(dp_mem_t *status,
                      const cp_desc_t &d,
                      bool suppress_log = false);
void compress_cp_desc_template_fill(cp_desc_t &d,
                                    dp_mem_t *src_buf,
                                    dp_mem_t *dst_buf,
                                    dp_mem_t *status_buf,
                                    dp_mem_t *invalidate_hdr_buf,
                                    uint32_t src_len);
void decompress_cp_desc_template_fill(cp_desc_t &d,
                                      dp_mem_t *src_buf,
                                      dp_mem_t *dst_buf,
                                      dp_mem_t *status_buf,
                                      uint32_t src_len,
                                      uint32_t threshold_len);
int compress_status_verify(dp_mem_t *status,
                           dp_mem_t *dst_buf,
                           const cp_desc_t& desc,
                           bool log_error=true,
                           uint32_t expected_status=CP_STATUS_SUCCESS);
int decompress_status_verify(dp_mem_t *status,
                             const cp_desc_t& desc,
                             uint32_t exp_output_data_len,
                             bool log_error=true);
uint32_t comp_status_output_data_len_get(dp_mem_t *status);
uint64_t comp_status_integrity_data_get(dp_mem_t *status);

int seq_comp_status_desc_fill(chain_params_comp_t& chain_params);

static inline const char *
integrity_type_name_get(uint32_t type)
{
    if (type < ARRAYSIZE(integrity_type_name)) {
        return integrity_type_name[type];
    }
    return "unknown";
}

static inline const char *
integrity_src_name_get(uint32_t src)
{
    if (src < ARRAYSIZE(integrity_src_name)) {
        return integrity_src_name[src];
    }
    return "unknown";
}

static inline const char *
sha_type_name_get(uint32_t type)
{
    if (type < ARRAYSIZE(sha_type_name)) {
        return sha_type_name[type];
    }
    return "unknown";
}

static inline const char *
sha_en_name_get(bool sha_en)
{
    return sha_en_name[sha_en & 1];
}

}  // namespace tests

#endif  // COMPRESSION_HPP_
