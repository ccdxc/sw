#ifndef _XTS_HPP_
#define _XTS_HPP_

#include <stdint.h>
#include <string>
#include "dol/iris/test/storage/dp_mem.hpp"
#include "dol/iris/test/storage/chain_params.hpp"
#include "dol/iris/test/storage/acc_ring.hpp"
#include "gflags/gflags.h"

using namespace dp_mem;

DECLARE_uint64(poll_interval);

extern const uint32_t  kXtsDescSize;
extern const uint32_t  kXtsQueueSize;

#ifndef ELBA
//model/cap_hens/readonly/cap_hese_csr_define.h

#define CAPRI_BARCO_MD_HENS_REG_BASE                    (0x6580000)

#define CAPRI_BARCO_MD_HENS_REG_XTS0_PRODUCER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0xc)
#define CAPRI_BARCO_MD_HENS_REG_XTS0_CONSUMER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x80)
#define CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x10c)
#define CAPRI_BARCO_MD_HENS_REG_XTS1_CONSUMER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x180)

/*
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_PRODUCER_IDX_BYTE_ADDRESS 0x20cu
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_OPA_TAG_ADDR_W0_BYTE_ADDRESS 0x210u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_OPA_TAG_ADDR_W1_BYTE_ADDRESS 0x214u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_CONSUMER_IDX_BYTE_ADDRESS 0x280u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_PRODUCER_IDX_BYTE_ADDRESS 0x30cu
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_OPA_TAG_ADDR_W0_BYTE_ADDRESS 0x310u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_OPA_TAG_ADDR_W1_BYTE_ADDRESS 0x314u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_CONSUMER_IDX_BYTE_ADDRESS 0x380u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_OPA_TAG_ADDR_W0_BYTE_ADDRESS 0x10u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_OPA_TAG_ADDR_W1_BYTE_ADDRESS 0x14u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_OPA_TAG_ADDR_W0_BYTE_ADDRESS 0x110u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_OPA_TAG_ADDR_W1_BYTE_ADDRESS 0x114u
 *
 */

/*
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_RING_BASE_W0_BYTE_ADDRESS 0x200u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_RING_BASE_W1_BYTE_ADDRESS 0x204u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_RING_BASE_W0_BYTE_ADDRESS 0x300u
 * #define CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_RING_BASE_W1_BYTE_ADDRESS 0x304u
 */


#define CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x20c)
#define CAPRI_BARCO_MD_HENS_REG_GCM0_CONSUMER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x280)
#define CAPRI_BARCO_MD_HENS_REG_GCM1_PRODUCER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x30c)
#define CAPRI_BARCO_MD_HENS_REG_GCM1_CONSUMER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x380)

#define CAPRI_BARCO_MD_HENS_REG_XTS0_OPA_TAG_W0_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x10)
#define CAPRI_BARCO_MD_HENS_REG_XTS0_OPA_TAG_W1_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x14)
#define CAPRI_BARCO_MD_HENS_REG_XTS1_OPA_TAG_W0_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x110)
#define CAPRI_BARCO_MD_HENS_REG_XTS1_OPA_TAG_W1_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x114)
#define CAPRI_BARCO_MD_HENS_REG_GCM0_OPA_TAG_W0_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x210)
#define CAPRI_BARCO_MD_HENS_REG_GCM0_OPA_TAG_W1_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x214)
#define CAPRI_BARCO_MD_HENS_REG_GCM1_OPA_TAG_W0_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x310)
#define CAPRI_BARCO_MD_HENS_REG_GCM1_OPA_TAG_W1_ADDR       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x314)
#endif

namespace xts {

#define XTS_SEQ_Q 52
#define PROT_INFO_SIZE 8
#define AES128_KEY_SIZE 16
#define AES256_KEY_SIZE 32
#define IV_SIZE 16
#define MSG_DESC_ALIGN 512
#define MAX_AOLS 2
#define MAX_SUB_AOLS 4
#define SECTOR_SIZE 512
#define INTER_SUB_AOL_GAP SECTOR_SIZE
#define STATUS_DEF_VALUE 0

static constexpr uint32_t kXtsPISize = sizeof(uint32_t);

#ifdef ELBA
typedef struct {
  uint32_t status;
  uint32_t rsvd;
  uint64_t chksum;
} __attribute__((packed)) xts_status_t;
#else
typedef struct {
  uint32_t status;
  uint32_t rsvd;
} __attribute__((packed)) xts_status_t;
#endif

typedef struct xts_aol_ {
  uint64_t a0;
  uint32_t o0;
  uint32_t l0;
  uint64_t a1;
  uint32_t o1;
  uint32_t l1;
  uint64_t a2;
  uint32_t o2;
  uint32_t l2;
  uint64_t next;
  uint64_t resv;
}xts_aol_t;
// C++ does not let packed struct feilds to be passed as references
// Commenting out for now - it doesnt affect functionality
// } __attribute__((packed)) xts_aol_t;

typedef struct xts_cmd_ {
  uint32_t enable_crc :1,
           bypass_aes :1,
           unused1 :18,
           is_decrypt :1,
           unused2 :3,
           token3 :4,
           token4 :4;
} __attribute__((packed)) xts_cmd_t;

#ifdef ELBA
#define XTS_REQ_DESC_PADDING 32
typedef struct xts_desc_ {
  uint64_t     in_aol;
  uint64_t     out_aol;
  xts_cmd_t    cmd;
  uint32_t     key_desc_idx;
  uint64_t     iv_addr;
  uint64_t     auth_tag;
  uint32_t     hdr;
  uint64_t     status;
  uint64_t     opaque_tag_addr;
  uint32_t     opaque_tag;
  uint32_t     opaque_tag_en :1,
               db_bits :1,	/* 1 for 32-bit DB, 0 for 64-bit DB */
               resv :30;
  uint64_t     db_addr;
  uint64_t     db_data;
  uint32_t     sec_key_desc_idx;
  uint16_t     sector_size;
  uint16_t     app_tag;
  uint32_t     sector_num;
  char         resv_pad[XTS_REQ_DESC_PADDING];
} __attribute__((packed)) xts_desc_t;
#else
#define XTS_REQ_DESC_PADDING 44
typedef struct xts_desc_ {
  uint64_t     in_aol;
  uint64_t     out_aol;
  xts_cmd_t    cmd;
  uint32_t     key_desc_idx;
  uint64_t     iv_addr;
  uint64_t     auth_tag;
  uint32_t     hdr;
  uint64_t     status;
  uint32_t     opaque_tag;
  uint32_t     opaque_tag_en :1,
               resv :31;
  uint16_t     sector_size;
  uint16_t     app_tag;
  uint32_t     sector_num;
  uint64_t     db_addr;
  uint64_t     db_data;
  char         resv_pad[XTS_REQ_DESC_PADDING];
} __attribute__((packed)) xts_desc_t;
#endif

typedef struct xts_prot_info_ {
  uint16_t crc;
  uint16_t app_tag;
  uint32_t sector_num;
} __attribute__((packed)) xts_prot_info_t;

typedef struct xts_status_desc_ {
  uint64_t     db_addr;
  uint64_t     db_data;
  uint64_t     status_addr0;
  uint64_t     status_addr1;
  uint64_t     intr_addr;	// MSI-X Interrupt address
  uint32_t     intr_data;	// MSI-X Interrupt data
  uint16_t     status_len;
  uint8_t      status_dma_en:1,
               next_doorbell_en:1,
               intr_en:1,
               is_next_db_barco_push:1,
               stop_chain_on_error:1;
  uint8_t      pad[17];
} __attribute__((packed)) xts_status_desc_t;

inline uint32_t get_output_size(uint32_t ip_size, uint32_t sector_size) {
	return (ip_size + (ip_size/sector_size) * PROT_INFO_SIZE);
}

typedef enum {
  INVALID,
  AES_ENCR_ONLY,
  T10_ONLY,
  AES_ENCR_N_T10,
  AES_DECR_ONLY,
  AES_DECR_N_T10,
}Op;

typedef struct TestCtx_{
  uint32_t key_size;
  Op op1;
  bool stage_in_hbm = false;
  Op op2;
  uint32_t num_sectors;
  uint32_t num_mds;
  uint32_t num_aols;
}TestCtx;


}  // namespace xts

namespace tests {

/*
 * Support 2 types of encryption/decryption (typically post compression):
 *  - Encrypt/decrypt entire application block, or
 *  - Encrypt/decrypt multiple individual blocks, each is of a "hash" block size
 */
typedef enum {
    XTS_ENC_DEC_ENTIRE_APP_BLK,
    XTS_ENC_DEC_PER_HASH_BLK
} xts_enc_dec_blk_type_t;

extern dp_mem_t *read_buf;
extern  dp_mem_t *write_buf;
extern  dp_mem_t *read_buf2;

extern  dp_mem_t *read_hbm_buf;
extern dp_mem_t *write_hbm_buf;
extern dp_mem_t *read_hbm_buf2;
extern dp_mem_t *write_hbm_buf2;

class XtsCtx {
public:

  void init(uint32_t size, bool chain = false);
  std::string get_name(bool chain = false);
  XtsCtx();
  ~XtsCtx();
  int cmd_eval_seq_xts(xts::xts_cmd_t& cmd);
  void desc_prefill_seq_xts(xts::xts_desc_t *xts_desc);
  int desc_write_seq_xts(xts::xts_desc_t *xts_desc);
  int desc_write_seq_xts_status(chain_params_xts_t& chain_params);
  int test_seq_xts();
  void status_invalidate(void);
  int ring_doorbell();
  uint16_t seq_xts_index_get(void);
  int verify_exp_opaque_tag(uint32_t exp_opaque_tag,
                            uint64_t poll_interval=FLAGS_poll_interval);
  int verify_doorbell(bool verify_pi=true,
                      uint64_t poll_interval=FLAGS_poll_interval);
  int queue_req_n_ring_db_from_host();

  void* src_buf = (void*)write_buf->va();
  bool is_src_hbm_buf = write_buf->is_mem_type_hbm();
  void* dst_buf = (void*)read_buf->va();
  bool is_dst_hbm_buf = read_buf->is_mem_type_hbm();
  void* src_buf_phy = NULL;
  void* dst_buf_phy = NULL;

  uint16_t seq_xts_q;
  uint32_t num_sectors = 1;
  xts::Op op;
  uint32_t key_size = AES128_KEY_SIZE;
  uint32_t sector_size = SECTOR_SIZE;
  uint32_t num_aols = 1;
  uint32_t num_sub_aols = 1;
  uint16_t app_tag = 0xbeef;
  uint32_t start_sec_num = 5;
  xts::xts_aol_t* in_aol[MAX_AOLS];
  xts::xts_aol_t* out_aol[MAX_AOLS];
  bool verify_db = true;
  unsigned char* iv = NULL;

  // ctx data needed for verification of op completion
  uint64_t xts_db_addr = 0;
  dp_mem_t* xts_db = NULL;
  uint64_t exp_db_data = 0xdeadbeefdeadbeef;
  dp_mem_t* status = NULL;
  uint32_t opaque_tag = 0;
  uint32_t last_used_opaque_tag = 0;
  bool caller_status_en = false;
  bool caller_xts_db_en = false;
  bool opa_tag_en = true;
  bool t10_en = false;
  bool decr_en = false;
  bool is_gcm=false;
  bool suppress_info_log=false;
  void* auth_tag_addr = NULL;

  acc_ring_t *acc_ring = nullptr;
  acc_ring_push_t push_type = ACC_RING_PUSH_SEQUENCER;
};


int xts_init(void);
int xts_resync();
int xts_multi_blk();
int xts_in_place();
int xts_netapp_data();
int xts_multi_blk_1req();
int xts_multi_blk_64req();
int xts_multi_blk_128req();
int xts_multi_blk_noc_stress_from_host();
int xts_multi_blk_noc_stress_from_hbm();

void xts_aol_sparse_fill(xts_enc_dec_blk_type_t enc_dec_blk_type,
                         dp_mem_t *xts_aol_vec,
                         dp_mem_t *xts_buf,
                         uint32_t blk_size,
                         uint32_t num_blks);
void xts_aol_trace(const char *aol_name,
                   dp_mem_t *xts_aol_vec,
                   uint32_t max_blks,
                   bool honor_link);

}  // namespace tests

#endif   // _XTS_HPP_
