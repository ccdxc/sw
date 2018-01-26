#include <assert.h>
#include <netinet/in.h>

#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/tests.hpp"
#include "dol/test/storage/xts.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/queues.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

uint32_t key128_desc_idx = 0;
uint32_t key256_desc_idx = 0;
bool key_desc_inited = false;
uint32_t exp_opaque_tag = 99; //some random number that keeps gettign incremented

const static uint32_t  kAolSize              = 64;
const static uint32_t  kXtsDescSize          = 128;

namespace xts {
extern std::vector<TestCtx> xts_tests;
}

namespace tests {
extern void test_ring_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid,
                        uint8_t ring, uint16_t index);

int verify_prot_info(char *out_buf, uint32_t num_aols, xts::xts_aol_t **out_aol,
  uint32_t sector_size, uint32_t sec_num_start, uint16_t app_tag) {
  uint32_t data_size = 0;
  for(uint32_t i = 0; i < num_aols; i++) {
    data_size += out_aol[i]->l0;
    assert((uint64_t)out_buf == (uint64_t)host_mem_p2v(out_aol[i]->a0));
    if(out_aol[i]->a1) {
      assert((uint64_t)out_buf == (uint64_t)host_mem_p2v(out_aol[i]->a1));
      data_size += out_aol[i]->l1;
    }
    if(out_aol[i]->a2) {
      assert((uint64_t)out_buf == (uint64_t)host_mem_p2v(out_aol[i]->a2));
      data_size += out_aol[i]->l2;
    }
  }
  assert(data_size >= sector_size);
  char* buf = (char *)malloc(data_size);
  uint32_t offset = 0;
  // Copy data to be a contiguous block
  for(uint32_t i = 0; i < num_aols; i++) {
    memcpy(buf+offset, out_buf + out_aol[i]->o0, out_aol[i]->l0);
    offset += out_aol[i]->l0;
    if(out_aol[i]->a1) {
      memcpy(buf+offset, out_buf + out_aol[i]->o1, out_aol[i]->l1);
      offset += out_aol[i]->l1;
    }
    if(out_aol[i]->a2) {
      memcpy(buf+offset, out_buf + out_aol[i]->o2, out_aol[i]->l2);
      offset += out_aol[i]->l2;
    }
  }
  uint32_t num_sectors = data_size / sector_size; // Works roughly as output data_size is more than just data (contains prot_info for all sectors)
  for(uint32_t i = 0; i < num_sectors; i++) {
    xts::xts_prot_info_t* prot_info = (xts::xts_prot_info_t*) &buf[(i+1)*sector_size + i*PROT_INFO_SIZE];
    if(prot_info->app_tag != htons(app_tag) ||
      prot_info->sector_num != htonl(sec_num_start + i)) {
      printf("Verify prot_info failed \n");
      free(buf);
      return -1;
    }
  }
  free(buf);
  return 0;
}

bool fill_aol(void* buf, uint64_t& a, uint32_t& o, uint32_t& l, uint32_t& offset,
    uint32_t& pending_size, uint32_t len)
{
  a = (uint64_t)buf;
  o = offset;
  l = len;
  offset += (len + INTER_SUB_AOL_GAP); //Update offset to next data position
  assert(((int)pending_size - (int)len) >= 0);
  pending_size -= len;
  if(pending_size > 0)
    return true;
  else
    return false;
}

void XtsCtx::init(bool chain) {
  xts_db = (uint64_t*)alloc_host_mem(sizeof(uint64_t));
  *xts_db = 0;
  if(!is_src_hbm_buf) {
    if(!chain) {
      srand(time(NULL));
      for(uint32_t i = 0; i < kDefaultBufSize/sizeof(int); i++) {
        ((int*)src_buf)[i] = rand();
      }
    }
    src_buf_phy = (void*)host_mem_v2p(src_buf);
  } else {
    src_buf_phy = src_buf;
  }
  if(!is_dst_hbm_buf) {
    memset(dst_buf, 0, kDefaultBufSize);
    dst_buf_phy = (void*)host_mem_v2p(dst_buf);
  } else {
    dst_buf_phy = dst_buf;
  }
}

std::string XtsCtx::get_name(bool chain) {
  std::string name;
  if(!chain) {
    if(key_size == AES128_KEY_SIZE)
      name += "_K128";
    else
      name += "_K256";
    if(is_src_hbm_buf)
      name += "_hbm";
    else
      name += "_h";
    if(is_dst_hbm_buf)
      name += "_hbm";
    else
      name += "_h";
    name += "_Secs" + std::to_string(num_sectors);
    name += "_MDs" + std::to_string(num_aols);
    name += "_AoLs" + std::to_string(num_sub_aols);
  }
  switch(op) {
  case xts::AES_ENCR_ONLY:
    name += "_E";
  break;
  case xts::T10_ONLY:
    name += "_T10";
  break;
  case xts::AES_ENCR_N_T10:
    name += "_E_T10";
  break;
  case xts::AES_DECR_ONLY:
    name += "_D";
  break;
  case xts::AES_DECR_N_T10:
    name += "_D_T10";
  break;
  default:
    assert(0);
  break;
  }

  return name;
}

XtsCtx::XtsCtx() {
  for(uint32_t i = 0; i < MAX_AOLS; i++) {
    in_aol[i] = NULL;
    out_aol[i] = NULL;
  }
  seq_xts_q = queues::get_pvm_seq_xts_sq(0);
}

XtsCtx::~XtsCtx() {
  if(op == xts::INVALID) {
    //if(xts_db) free_host_mem(xts_db);
    return;
  }
  if(xts_desc_addr) free_host_mem(xts_desc_addr);
  if(status) free_host_mem(status);
  // TODO: This seems to be crashing - needs investigation
  // if(xts_db) free_host_mem(xts_db);
  if(iv) free_host_mem(iv);
  for(uint32_t i = 0; i < num_aols; i++) {
    if(in_aol[i]) free_host_mem(in_aol[i]);
    if(out_aol[i]) free_host_mem(out_aol[i]);
  }
}

int XtsCtx::test_seq_xts() {
  uint8_t *seq_xts_desc;

  // Sequencer #1: XTS descriptor
  seq_xts_desc = (uint8_t *) queues::pvm_sq_consume_entry(seq_xts_q, &seq_xts_index);
  memset(seq_xts_desc, 0, kSeqDescSize);

  unsigned char iv_src[IV_SIZE] = {0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
  iv = (unsigned char*)alloc_host_mem(IV_SIZE);
  memcpy(iv, iv_src, IV_SIZE);
  xts::xts_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.token3 = 0x0;     // xts
  cmd.token4 = 0x4;     // xts

  switch(op) {
  case(xts::AES_ENCR_ONLY):
    cmd.enable_crc = 0x0; // Disable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x0; // Encrypt
  break;
  case(xts::AES_DECR_ONLY):
    cmd.enable_crc = 0x0; // Disable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x1; // Decrypt
    decr_en = true;
  break;
  case(xts::T10_ONLY):
    cmd.enable_crc = 0x1; // Enable CRC
    cmd.bypass_aes = 0x1; // Bypass AES
    t10_en = true;
  break;
  case(xts::AES_ENCR_N_T10):
    cmd.enable_crc = 0x1; // Enable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x0; // Encrypt
    t10_en = true;
  break;
  case(xts::AES_DECR_N_T10):
     cmd.enable_crc = 0x1; // Enable CRC
    cmd.bypass_aes = 0x0; // Don't Bypass AES
    cmd.is_decrypt = 0x1; // Decrypt
    t10_en = true;
    decr_en = true;
  break;
  default:
    printf(" Unknown xts operation \n");
    assert(0);
    return -1;
  break;
  }

  assert(num_aols <= MAX_AOLS); // Currently we only support upto 2 - min required to validate aol chaining
  assert(num_sub_aols <= MAX_SUB_AOLS); // Currently we only support upto 4 - min required to validate aol chaining
  assert(sizeof(xts::xts_desc_t) == kXtsDescSize);
  xts::xts_desc_t* xts_desc_addr = (xts::xts_desc_t*)alloc_host_mem(sizeof(xts::xts_desc_t));

  assert(sizeof(xts::xts_aol_t) == kAolSize);
  memset(in_aol, 0x0, sizeof(in_aol));
  memset(out_aol, 0x0, sizeof(out_aol));
  for(uint32_t i = 0; i < num_aols; i++) {
    // AOLs need to be aligned at 512-bit boundary. For now page aligning them
    in_aol[i] = (xts::xts_aol_t*)alloc_page_aligned_host_mem(sizeof(xts::xts_aol_t));
    memset(in_aol[i], 0x0, sizeof(xts::xts_aol_t));
    out_aol[i] = (xts::xts_aol_t*)alloc_page_aligned_host_mem(sizeof(xts::xts_aol_t));
    memset(out_aol[i], 0x0, sizeof(xts::xts_aol_t));
  }
  uint32_t in_data_size, out_data_size;
  if(t10_en && decr_en) {
    in_data_size = num_sectors * sector_size + (num_sectors * PROT_INFO_SIZE);;
    out_data_size = (num_sectors * sector_size);
  } else if(t10_en && !decr_en) {
    in_data_size = (num_sectors * sector_size);
    out_data_size = (num_sectors * sector_size) + (num_sectors * PROT_INFO_SIZE);
  } else {
    in_data_size = num_sectors * sector_size;
    out_data_size = num_sectors * sector_size;
  }

  assert(in_data_size % num_sub_aols == 0); // For simplicity
  assert(out_data_size % num_sub_aols == 0); // For simplicity
  uint32_t sub_aol_data_size = in_data_size / num_sub_aols;
  uint32_t sub_aol_data_offset = 0;
  uint32_t out_sub_aol_data_size = out_data_size / num_sub_aols;
  uint32_t out_sub_aol_data_offset = 0;
  uint32_t in_pending_size = in_data_size;
  uint32_t out_pending_size = out_data_size;
  bool in_cont = true, out_cont = true;

  for(uint32_t i = 0; i < num_aols; i++) {
    // Fill the XTS Msg descriptor
    if(in_cont)
      in_cont = fill_aol(src_buf_phy, in_aol[i]->a0, in_aol[i]->o0, in_aol[i]->l0, sub_aol_data_offset,
        in_pending_size, sub_aol_data_size);
    if(in_cont)
      in_cont = fill_aol(src_buf_phy, in_aol[i]->a1, in_aol[i]->o1, in_aol[i]->l1, sub_aol_data_offset,
        in_pending_size, sub_aol_data_size);
    if(in_cont)
      in_cont = fill_aol(src_buf_phy, in_aol[i]->a2, in_aol[i]->o2, in_aol[i]->l2, sub_aol_data_offset,
        in_pending_size, sub_aol_data_size);
    if(out_cont)
      out_cont = fill_aol(dst_buf_phy, out_aol[i]->a0, out_aol[i]->o0, out_aol[i]->l0, out_sub_aol_data_offset,
        out_pending_size, out_sub_aol_data_size);
    if(out_cont)
      out_cont = fill_aol(dst_buf_phy, out_aol[i]->a1, out_aol[i]->o1, out_aol[i]->l1, out_sub_aol_data_offset,
        out_pending_size, out_sub_aol_data_size);
    if(out_cont)
      out_cont = fill_aol(dst_buf_phy, out_aol[i]->a2, out_aol[i]->o2, out_aol[i]->l2, out_sub_aol_data_offset,
        out_pending_size, out_sub_aol_data_size);

    if(i + 1 < num_aols) {
      in_aol[i]->next = (uint64_t) host_mem_v2p(in_aol[i+1]);
       out_aol[i]->next = (uint64_t) host_mem_v2p(out_aol[i+1]);
    }
  }

  status = (uint64_t*)alloc_host_mem(sizeof(uint64_t));
  *status = STATUS_DEF_VALUE;
  if(!xts_db_addr)
    xts_db_addr = host_mem_v2p(xts_db);

  // Fill the XTS ring descriptor
  xts_desc_addr->in_aol = host_mem_v2p(in_aol[0]);
  xts_desc_addr->out_aol = host_mem_v2p(out_aol[0]);
  xts_desc_addr->iv_addr = host_mem_v2p(iv);
  xts_desc_addr->db_addr = xts_db_addr;
  xts_desc_addr->db_data = exp_db_data;
  xts_desc_addr->opaque_tag_en = 1;
  xts_desc_addr->opaque_tag = exp_opaque_tag;
  if(t10_en) {
    xts_desc_addr->sector_num = start_sec_num;
    xts_desc_addr->sector_size = sector_size;
    xts_desc_addr->app_tag = app_tag;
  } else {
    xts_desc_addr->sector_num = 0;
    xts_desc_addr->sector_size = 0;
    xts_desc_addr->app_tag = 0;
  }
  xts_desc_addr->cmd = cmd;
  xts_desc_addr->status = host_mem_v2p(status);

  // Ideally below key initialization lines need to be commented out if operation is T10 only
  // but barco is not fixing this bug - https://github.com/pensando/asic/issues/669
  if(!key_desc_inited) {
    unsigned char key[64] = {0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
      0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
      0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
      0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
    if(hal_if::get_key_index((char*)key, types::CRYPTO_KEY_TYPE_AES128, AES256_KEY_SIZE*2, &key128_desc_idx)) {
      printf("can't create or update xts 128bit key index \n");
      return -1;
    }
    if(hal_if::get_key_index((char*)key, types::CRYPTO_KEY_TYPE_AES256, AES256_KEY_SIZE*2, &key256_desc_idx)) {
      printf("can't create or update xts 256 key index \n");
      return -1;
    }
    key_desc_inited = true;
  }
  if(key_size == AES128_KEY_SIZE)
    xts_desc_addr->key_desc_idx = key128_desc_idx;
  else
    xts_desc_addr->key_desc_idx = key256_desc_idx;

  // Fill the XTS Seq descriptor
  utils::write_bit_fields(seq_xts_desc, 0, 64, host_mem_v2p(xts_desc_addr));
  utils::write_bit_fields(seq_xts_desc, 64, 32, 7);  //2^7 which will be 128 - xts desc size
  utils::write_bit_fields(seq_xts_desc, 96, 16, 2);  //2^2 which will be 4 - prod index size

  // Fill xts producer index addr
  if(decr_en)
    utils::write_bit_fields(seq_xts_desc, 112, 34, CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX);
  else
    utils::write_bit_fields(seq_xts_desc, 112, 34, CAPRI_BARCO_MD_HENS_REG_XTS0_PRODUCER_IDX);

  uint64_t xts_ring_base_addr;

  if(hal_if::get_xts_ring_base_address(decr_en, &xts_ring_base_addr) < 0) {
    printf("can't get xts ring base address \n");
    return -1;
  }
  // Fill xts ring base addr
  utils::write_bit_fields(seq_xts_desc, 146, 34, xts_ring_base_addr);

  if(ring_seq_db) {
    return ring_db_n_verify();
  }

  return 0;
}

int XtsCtx::ring_db_n_verify() {
  // Kickstart the sequencer
  test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_xts_q, 0, seq_xts_index);

  // Poll for doorbell data as XTS which runs in a different thread
  auto func = [this] () {
    if(*xts_db != exp_db_data) {
      //printf("Doorbell data not yet there - try again \n");
      return -1;
    }
    return 0;
  };

  Poller poll;
  int rv = poll(func);
  if(0 != rv) return rv;

  printf("Doorbell returned successfully \n");

  if(0 == rv && t10_en && !decr_en && !is_dst_hbm_buf)
    rv = verify_prot_info((char *)dst_buf, num_aols, out_aol, sector_size, start_sec_num, app_tag);

  if(0 == rv) {
    if(STATUS_DEF_VALUE != *status) {
      printf(" status check failed - status value %lu\n", *status);
      rv = -1;
    }
  }

  uint64_t opaque_tag_addr = 0;
  if(hal_if::get_xts_opaque_tag_addr(decr_en, &opaque_tag_addr)) {
    printf("get_xts_opaque_tag_addr failed \n");
    return -1;
  }

  uint32_t opaque_tag = 0;
  auto func2 = [this, opaque_tag_addr, opaque_tag] () {
    if(!read_mem(opaque_tag_addr, (uint8_t*)&opaque_tag, sizeof(opaque_tag))) {
      printf("Reading opaque tag hbm mem failed \n");
      return -1;
    }

    if(exp_opaque_tag != opaque_tag) {
      //printf("Opaque tag expected value %u rcvd %u\n", exp_opaque_tag, opaque_tag);
      return -1;
    }
    return 0;
  };
  rv = poll(func2);
  if(0 == rv) printf("Opaque tag returned successfully \n");

  exp_opaque_tag++;

  return rv;
}

class XtsChainCtx {
public:
  int operator()(void) {
    int rv = xts_ctx1.test_seq_xts();
    if(0 == rv && num_ops == 2) {
      printf(" Executing second part of chain\n");
      rv = xts_ctx2.test_seq_xts();
      if(0 == rv) {
        char src_buf_cp[kDefaultBufSize], dst_buf_cp[kDefaultBufSize];
        uint32_t src_off = 0, dst_off = 0;
        memset(src_buf_cp, 0, kDefaultBufSize);
        memset(dst_buf_cp, 0, kDefaultBufSize);
        for(uint32_t i = 0; i < xts_ctx1.num_aols; i++) {
          for(uint32_t j = 0; j < xts_ctx1.num_sub_aols; j++) {
            uint32_t off, len;
            if(0 == j) {
              off = xts_ctx1.in_aol[i]->o0;
              len = xts_ctx1.in_aol[i]->l0;
            }
            else if(1 == j) {
              off = xts_ctx1.in_aol[i]->o1;
              len = xts_ctx1.in_aol[i]->l1;
            }
            else if(2 == j) {
              off = xts_ctx1.in_aol[i]->o2;
              len = xts_ctx1.in_aol[i]->l2;
            }
            memcpy(src_buf_cp + src_off, (char*)xts_ctx1.src_buf+off, len);
            src_off += len;
          }
        }
        for(uint32_t i = 0; i < xts_ctx2.num_aols; i++) {
          for(uint32_t j = 0; j < xts_ctx2.num_sub_aols; j++) {
            uint32_t off, len;
            if(0 == j) {
              off = xts_ctx2.out_aol[i]->o0;
              len = xts_ctx2.out_aol[i]->l0;
            }
            else if(1 == j) {
              off = xts_ctx2.out_aol[i]->o1;
              len = xts_ctx2.out_aol[i]->l1;
            }
            else if(2 == j) {
              off = xts_ctx2.out_aol[i]->o2;
              len = xts_ctx2.out_aol[i]->l2;
            }
            memcpy(dst_buf_cp + dst_off, (char*)xts_ctx2.dst_buf+off, len);
            dst_off += len;
          }
        }
        rv = memcmp(src_buf_cp, dst_buf_cp, xts_ctx1.num_sectors * SECTOR_SIZE);
        if(0 != rv) {
          rv = -1;
          printf(" Memcmp failed \n");
        }
      }
    }
    return rv;
  }
  std::string get_name() {
    std::string name = "XTS";
    name += xts_ctx1.get_name();
    if(num_ops == 2)
      name += xts_ctx2.get_name(true);
    return name;
  }
  XtsCtx xts_ctx1, xts_ctx2;
  int num_ops = 1;
};

int add_xts_tests(std::vector<TestEntry>& test_suite) {
  for(auto const &ent : xts::xts_tests) {
    XtsChainCtx *ctx = new XtsChainCtx;
    ctx->xts_ctx1.op = ent.op1;
    ctx->xts_ctx2.op = ent.op2;
    ctx->xts_ctx1.key_size = ent.key_size;
    ctx->xts_ctx2.key_size = ent.key_size;
    ctx->xts_ctx1.num_sectors = ent.num_sectors;
    ctx->xts_ctx1.num_aols = ent.num_mds;
    ctx->xts_ctx1.num_sub_aols = ent.num_aols;
    ctx->xts_ctx2.num_sectors = ent.num_sectors;
    ctx->xts_ctx2.num_aols = ent.num_mds;
    ctx->xts_ctx2.num_sub_aols = ent.num_aols;
    if(ent.stage_in_hbm == true) {
      ctx->xts_ctx1.dst_buf = (void*)write_hbm_buf;
      ctx->xts_ctx1.is_dst_hbm_buf = true;
      ctx->xts_ctx2.src_buf = (void*)write_hbm_buf;
      ctx->xts_ctx2.is_src_hbm_buf = true;
    } else {
      ctx->xts_ctx2.src_buf = (void*)read_buf;
      ctx->xts_ctx2.is_src_hbm_buf = false;
      ctx->xts_ctx2.dst_buf = (void*)read_buf2;
      ctx->xts_ctx2.is_dst_hbm_buf = false;
    }
    if(xts::INVALID != ent.op2)
      ctx->num_ops = 2;
    ctx->xts_ctx1.init();
    ctx->xts_ctx2.init(true);
    test_suite.push_back({*ctx, ctx->get_name(), false});
    delete ctx;
  }

  return 0;
}

} // namespace tests
