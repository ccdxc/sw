#ifndef	COMPRESSION_HPP_
#define	COMPRESSION_HPP_

#include <stdint.h>

namespace tests {

typedef struct ccmd {
  uint16_t compression_en:1, // NOP for decompression
           header_present:1, // must be 1 for decompression
           insert_header:1,  // must be 1 for compression
           doorbell_on:1,
           opaque_tag_on:1,
           src_is_list:1,
           dst_is_list:1,
           cksum_en:1,
           cksum_type:1, // 0-ALDER32, 1-CRC32
           sha_en:1,
           sha2_type:1,  // 0-512, 1-256
           rsvd5b:5;
} ccmd_t;

typedef struct cp_desc {
  uint64_t src;
  uint64_t dst;
  union {
    ccmd_t   cmd_bits;
    uint16_t cmd;
  };
  uint16_t rsvd;
  uint16_t input_len;
  uint16_t expected_len;
  uint64_t status_addr;
  uint64_t doorbell_addr;
  uint64_t doorbell_data;
  uint64_t opaque_tag_addr;
  uint32_t opaque_tag_data;
  uint32_t status_data;
} cp_desc_t;

typedef struct cp_sgl {
  uint64_t addr0;
  uint64_t len0:16,
           rsvd0:48;
  uint64_t addr1;
  uint64_t len1:16,
           rsvd1:48;
  uint64_t addr2;
  uint64_t len2:16,
           rsvd2:48;
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
#define	CP_STATUS_AXI_DATA_ERROR	2
#define	CP_STATUS_AXI_ADDR_ERROR	2
#define	CP_STATUS_COMPRESSION_FAILED	4
#define	CP_STATUS_DATA_TOO_LONG		5
#define	CP_STATUS_CHECKSUM_FAILED	6
#define CP_STATUS_SGL_DESC_ERROR	7

typedef struct cp_status_no_hash {
  uint16_t rsvd:12,
           err:3,  // See status code above
           valid:1;
  uint16_t output_data_len;
  uint32_t partial_data;
} cp_status_no_hash_t;

typedef struct cp_status_sha512 {
  uint16_t rsvd:12,
           err:3,  // See status code above
           valid:1;
  uint16_t output_data_len;
  uint32_t partial_data;
  uint8_t  sha512[64];
} cp_status_sha512_t;

typedef struct cp_status_sha256 {
  uint16_t rsvd:12,
           err:3,  // See status code above
           valid:1;
  uint16_t output_data_len;  // Includes header length.
  uint32_t partial_data;
  uint8_t  sha256[32];
} cp_status_sha256_t;

}  // namespace tests

#endif  // COMPRESSION_HPP_
