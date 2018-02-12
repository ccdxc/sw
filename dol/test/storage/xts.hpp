#ifndef _XTS_HPP_
#define _XTS_HPP_

#include <stdint.h>
#include <string>

#define CAPRI_BARCO_MD_HENS_REG_BASE                    (0x6580000)

#define CAPRI_BARCO_MD_HENS_REG_XTS0_PRODUCER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0xc)
#define CAPRI_BARCO_MD_HENS_REG_XTS0_CONSUMER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x80)
#define CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x10c)
#define CAPRI_BARCO_MD_HENS_REG_XTS1_CONSUMER_IDX       (CAPRI_BARCO_MD_HENS_REG_BASE + 0x180)

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

typedef struct xts_prot_info_ {
  uint16_t crc;
  uint16_t app_tag;
  uint32_t sector_num;
} __attribute__((packed)) xts_prot_info_t;

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

extern void *read_buf;
extern  void *write_buf;
extern  void *read_buf2;

extern  uint64_t read_hbm_buf;
extern uint64_t write_hbm_buf;
extern uint64_t read_hbm_buf2;
extern uint64_t write_hbm_buf2;

class XtsCtx {
public:

  void init(uint32_t size, bool chain = false);
  std::string get_name(bool chain = false);
  XtsCtx();
  ~XtsCtx();
  int test_seq_xts();
  int ring_doorbell();
  int verify_doorbell();
  int queue_req_n_ring_db_from_host();

  void* src_buf = write_buf;
  bool is_src_hbm_buf = false;
  void* dst_buf = read_buf;
  bool is_dst_hbm_buf = false;
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
  bool ring_db = true;
  bool verify_db = true;
  uint16_t seq_xts_index = 0;
  xts::xts_desc_t* xts_desc_addr = NULL;
  unsigned char* iv = NULL;

  // ctx data needed for verification of op completion
  uint64_t xts_db_addr = 0;
  uint64_t* xts_db = NULL;
  uint64_t exp_db_data = 0xdeadbeefdeadbeef;
  uint64_t* status = NULL;
  bool t10_en = false;
  bool decr_en = false;

  uint64_t xts_ring_base_addr;
  uint64_t xts_ring_pi_addr;
  bool use_seq = true;
};


int xts_multi_blk();
int xts_in_place();
int xts_netapp_data();


}  // namespace tests

#endif   // _XTS_HPP_
