#ifndef	COMPRESSION_HPP_
#define	COMPRESSION_HPP_

#include <stdint.h>

namespace tests {

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

}  // namespace tests

#endif  // COMPRESSION_HPP_
