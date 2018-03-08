#include <assert.h>
#include <netinet/in.h>
#include <math.h>
#include <unistd.h>

#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "dol/test/storage/tests.hpp"
#include "dol/test/storage/xts.hpp"
#include "dol/test/storage/qstate_if.hpp"
#include "dol/test/storage/queues.hpp"
#include "dol/test/storage/dp_mem.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "nic/model_sim/include/lib_model_client.h"

uint32_t key128_desc_idx = 0;
uint32_t key256_desc_idx = 0;
bool key_desc_inited = false;
uint32_t exp_opaque_tag_encr = 0;
uint32_t exp_opaque_tag_decr = 0;
uint32_t gcm_exp_opaque_tag_encr = 0;
uint32_t gcm_exp_opaque_tag_decr = 0;

const static uint32_t  kAolSize              = 64;
const static uint32_t  kXtsDescSize          = 128;
const static uint32_t  kXtsPISize            = 4;
const static uint32_t  kXtsQueueSize         = 1024;

using namespace dp_mem;
extern size_t tcid;

namespace xts {
extern std::vector<TestCtx> xts_tests;
}

namespace tests {

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


int verify_opaque_tag(uint32_t exp_opaque_tag, bool decr_en, uint64_t poll_interval=FLAGS_poll_interval, bool is_gcm=false) {

  uint64_t opaque_tag_addr = 0;
  if(hal_if::get_xts_opaque_tag_addr(decr_en, &opaque_tag_addr, is_gcm)) {
    printf("get_xts_opaque_tag_addr failed \n");
    return -1;
  }

  uint32_t opaque_tag = 0;
  auto func = [opaque_tag_addr, opaque_tag, exp_opaque_tag] () {
    if(!read_mem(opaque_tag_addr, (uint8_t*)&opaque_tag, sizeof(opaque_tag))) {
      printf("Reading opaque tag hbm mem failed \n");
      return -1;
    }

    if(exp_opaque_tag > opaque_tag) {
      //printf("Opaque tag expected value %u rcvd %u\n", exp_opaque_tag, opaque_tag);
      return -1;
    }
    return 0;
  };
  Poller poll(poll_interval);
  int rv = poll(func);
  if(0 == rv) {
    if(decr_en) printf("Decr Opaque tag exp %d addr %lx returned successfully \n", exp_opaque_tag, opaque_tag_addr);
    else printf("Encr Opaque tag exp %d addr %lx returned successfully \n", exp_opaque_tag, opaque_tag_addr);
  } else {
    printf("Opaque tag expected value %u rcvd %u addr %lx\n", exp_opaque_tag, opaque_tag, opaque_tag_addr);
  }

  return rv;
}

void XtsCtx::init(uint32_t size, bool chain) {
  xts_db = (uint64_t*)alloc_host_mem(sizeof(uint64_t));
  *xts_db = 0;
  if(!is_src_hbm_buf) {
    if(!chain) {
      srand(time(NULL));
      for(uint32_t i = 0; i < size/sizeof(int); i++) {
        ((int*)src_buf)[i] = rand();
      }
    }
    src_buf_phy = (void*)host_mem_v2p(src_buf);
  } else {
    src_buf_phy = src_buf;
  }
  if(!is_dst_hbm_buf) {
    memset(dst_buf, 0, size);
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

unsigned char key[64] = {0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
  0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
  0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
  0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
unsigned char iv_src[IV_SIZE] = {0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29, 0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};

int XtsCtx::test_seq_xts() {
  dp_mem_t *seq_xts_desc;

  // Sequencer #1: XTS descriptor
  seq_xts_desc = queues::pvm_sq_consume_entry(seq_xts_q, &seq_xts_index);
  seq_xts_desc->clear();

  iv = (unsigned char*)alloc_host_mem(IV_SIZE);
  memcpy(iv, iv_src, IV_SIZE);
  xts::xts_cmd_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.token3 = 0x0;     // xts
  if(!is_gcm) cmd.token4 = 0x4;     // xts
  else cmd.token4 = 0x3;     // gcm

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
  xts_desc_addr = (xts::xts_desc_t*)alloc_host_mem(sizeof(xts::xts_desc_t));
  memset(xts_desc_addr, 0, sizeof(xts::xts_desc_t));

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
  if(is_gcm) {
    if(!decr_en) {
      assert(NULL == auth_tag_addr);
      auth_tag_addr = alloc_host_mem(64);
      memset(auth_tag_addr, 0, 64);
      xts_desc_addr->auth_tag = host_mem_v2p(auth_tag_addr);
    } else {
      assert(NULL != auth_tag_addr);
      xts_desc_addr->auth_tag = host_mem_v2p(auth_tag_addr);
    }
  }
  xts_desc_addr->db_addr = xts_db_addr;
  xts_desc_addr->db_data = exp_db_data;
  if(opa_tag_en) xts_desc_addr->opaque_tag_en = 1;
  else xts_desc_addr->opaque_tag_en = 0;
  if(!opaque_tag) {
    if(!is_gcm)
      xts_desc_addr->opaque_tag = decr_en? ++exp_opaque_tag_decr : ++exp_opaque_tag_encr;
    else
      xts_desc_addr->opaque_tag = decr_en? ++gcm_exp_opaque_tag_decr : ++gcm_exp_opaque_tag_encr;
  } else {
    xts_desc_addr->opaque_tag = opaque_tag;
  }

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

  // Fill xts producer index addr
  if(decr_en)
    if(!is_gcm) xts_ring_pi_addr = CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX;
    else xts_ring_pi_addr = CAPRI_BARCO_MD_HENS_REG_GCM1_PRODUCER_IDX;
  else
    if(!is_gcm) xts_ring_pi_addr = CAPRI_BARCO_MD_HENS_REG_XTS0_PRODUCER_IDX;
    else xts_ring_pi_addr = CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX;

  if(hal_if::get_xts_ring_base_address(decr_en, &xts_ring_base_addr, is_gcm) < 0) {
    printf("can't get xts ring base address \n");
    return -1;
  }

  if(use_seq) {
    // Fill the XTS Seq descriptor
    seq_xts_desc->write_bit_fields(0, 64, host_mem_v2p(xts_desc_addr));
    seq_xts_desc->write_bit_fields(64, 32, (uint64_t) log2(kXtsDescSize));  //2^7 which will be 128 - xts desc size
    seq_xts_desc->write_bit_fields(96, 16, (uint64_t) log2(kXtsPISize));  //2^2 which will be 4 - prod index size
    seq_xts_desc->write_bit_fields(146, 34, xts_ring_base_addr);
    seq_xts_desc->write_bit_fields(112, 34, xts_ring_pi_addr);
    seq_xts_desc->write_thru();
  }

  int rv = 0;
  if(copy_desc || ring_db) {
    rv = ring_doorbell();
    if(rv == 0 && verify_db)
      rv = verify_doorbell();
  }

  return rv;
}

uint32_t pi;
bool pi_inited = false;
int XtsCtx::ring_doorbell() {
  if(use_seq) {
    // Kickstart the sequencer
    test_ring_doorbell(queues::get_pvm_lif(), SQ_TYPE, seq_xts_q, 0, seq_xts_index);
  } else {
    if(!pi_inited) {
      read_reg(xts_ring_pi_addr, pi);
      pi_inited = true;
    }

    uint64_t ring_addr = xts_ring_base_addr + kXtsDescSize * pi;
    write_mem(ring_addr, (uint8_t*)xts_desc_addr, kXtsDescSize);
    pi += 1;
    if(pi == kXtsQueueSize) pi = 0; // roll-over case
    if(ring_db) {
      write_reg(xts_ring_pi_addr, pi);
      pi_inited = false;
    }
  }
  return 0;
}

int XtsCtx::verify_doorbell() {

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
  uint32_t exp_opaque_tag = decr_en? exp_opaque_tag_decr : exp_opaque_tag_encr;
  return verify_opaque_tag(exp_opaque_tag, decr_en);
}

int e2e_verify(XtsCtx& xts_ctx1, XtsCtx& xts_ctx2) {
  int rv = 0;
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
    printf(" Memcmp failed %d \n", rv);
  } else {
    printf(" e2e verify memcmp passed\n");
  }

  return rv;
}

class XtsChainCtx {
public:
  int operator()(void) {
    int rv = xts_ctx1.test_seq_xts();
    if(0 == rv && num_ops == 2) {
      printf(" Executing second part of chain\n");
      rv = xts_ctx2.test_seq_xts();
      if(0 == rv && verify_e2e) {
        return e2e_verify(xts_ctx1, xts_ctx2);
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
  bool verify_e2e = true;
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
      ctx->xts_ctx1.dst_buf = (void*)write_hbm_buf->va();
      ctx->xts_ctx1.is_dst_hbm_buf = true;
      ctx->xts_ctx2.src_buf = (void*)write_hbm_buf->va();
      ctx->xts_ctx2.is_src_hbm_buf = true;
    } else {
      ctx->xts_ctx2.src_buf = (void*)read_buf->va();
      ctx->xts_ctx2.is_src_hbm_buf = false;
      ctx->xts_ctx2.dst_buf = (void*)read_buf2->va();
      ctx->xts_ctx2.is_dst_hbm_buf = false;
    }
    if(xts::INVALID != ent.op2)
      ctx->num_ops = 2;
    ctx->xts_ctx1.init(kDefaultBufSize);
    ctx->xts_ctx2.init(kDefaultBufSize, true);
    test_suite.push_back({*ctx, ctx->get_name(), false});
    delete ctx;
  }
  //test_suite.push_back({&tests::xts_multi_blk, "XTS Multi Block", false});
  test_suite.push_back({&tests::xts_in_place, "XTS HBM  Buffer In-Place", false});
  //test_suite.push_back({&tests::xts_netapp_data, "XTS Netapp Data", false});

  return 0;
}

int add_xts_perf_tests(std::vector<TestEntry>& test_suite) {
  test_suite.push_back({&tests::xts_multi_blk_1req, "XTS Multi Block 1 req", false});
  test_suite.push_back({&tests::xts_multi_blk_64req, "XTS Multi Block 64 req", false});
  test_suite.push_back({&tests::xts_multi_blk_128req, "XTS Multi Block 128 req", false});

  return 0;
}

const static uint32_t  kMaxReqs = 128;
uint32_t  kTotalReqs = 128;
uint32_t  kInitReqs = 64;
uint32_t  kBatchSize = 32;
const static uint32_t  kSectorSize = SECTOR_SIZE;
const static uint32_t  kBufSize = kMaxReqs*(kDefaultBufSize);

int fill_chain_ctx(XtsChainCtx* ctx, char* in_buff[kMaxReqs], char *stg_buff[kMaxReqs], char *out_buff[kMaxReqs]) {
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    ctx[i].xts_ctx1.op = xts::AES_ENCR_ONLY;
    ctx[i].xts_ctx1.use_seq = false;
    ctx[i].xts_ctx1.verify_db = false;
    ctx[i].xts_ctx1.ring_db = false;
    ctx[i].xts_ctx1.num_sectors = kDefaultBufSize/kSectorSize;

    ctx[i].xts_ctx2.op = xts::AES_DECR_ONLY;
    ctx[i].xts_ctx2.use_seq = false;
    ctx[i].xts_ctx2.verify_db = false;
    ctx[i].xts_ctx2.ring_db = false;
    ctx[i].xts_ctx2.num_sectors = kDefaultBufSize/kSectorSize;

    ctx[i].xts_ctx1.src_buf = (void*)in_buff[i];
    ctx[i].xts_ctx1.is_src_hbm_buf = false;
    ctx[i].xts_ctx1.dst_buf = (void*)stg_buff[i];
    ctx[i].xts_ctx1.is_dst_hbm_buf = false;

    ctx[i].xts_ctx2.src_buf = (void*)stg_buff[i];
    ctx[i].xts_ctx2.is_src_hbm_buf = false;
    ctx[i].xts_ctx2.dst_buf = (void*)out_buff[i];
    ctx[i].xts_ctx2.is_dst_hbm_buf = false;

    ctx[i].num_ops = 2;
    ctx[i].xts_ctx1.init(kDefaultBufSize);
    ctx[i].xts_ctx2.init(kDefaultBufSize, true);
  }
  return 0;
}

int xts_multi_blk_1req() {
  kTotalReqs = 1;
  kInitReqs = 1;
  kBatchSize = 1;
  exp_opaque_tag_encr = 0;
  exp_opaque_tag_decr = 0;
  return xts_multi_blk();
}

int xts_multi_blk_64req() {
  kTotalReqs = 64;
  kInitReqs = 64;
  kBatchSize = 64;
  exp_opaque_tag_encr = 0;
  exp_opaque_tag_decr = 0;
  return xts_multi_blk();
}

int xts_multi_blk_128req() {
  kTotalReqs = 128;
  kInitReqs = 128;
  kBatchSize = 128;
  exp_opaque_tag_encr = 0;
  exp_opaque_tag_decr = 0;
  return xts_multi_blk();
}

int xts_multi_blk() {
  int rv;
  char *in_buff[kMaxReqs], *stg_buff[kMaxReqs], *out_buff[kMaxReqs];
  char* in_buffer = (char *)alloc_host_mem(kBufSize);
  char* stg_buffer = (char *)alloc_host_mem(kBufSize);
  char* out_buffer = (char *)alloc_host_mem(kBufSize);

  for(uint32_t i = 0; i < kTotalReqs; i++) {
    in_buff[i] = in_buffer + i * (kDefaultBufSize);
    stg_buff[i] = stg_buffer + i * (kDefaultBufSize);
    out_buff[i] = out_buffer + i * (kDefaultBufSize);
  }

  XtsChainCtx *ctx = new XtsChainCtx[kTotalReqs];
  fill_chain_ctx(ctx, in_buff, stg_buff, out_buff);

  uint32_t exp_encr_opaque_tag = kBatchSize, exp_decr_opaque_tag = 0;
  uint32_t encr_reqs_comp = 0, decr_reqs_comp = 0;
  uint32_t pending_encr_reqs = kTotalReqs -kInitReqs,  pending_decr_reqs = kTotalReqs;

  int iter = 1;
  testcase_begin(tcid, iter);
  //Queue initial set of requests
  for(uint32_t i = 0; i < kInitReqs; i++) {
    if(i == kInitReqs - 1) ctx[i].xts_ctx1.ring_db = true;
    rv = ctx[i].xts_ctx1.test_seq_xts();
    if(0 != rv) goto done;
  }

  while(1) {
    //Wait for batch size encr to complete
    if(exp_encr_opaque_tag && encr_reqs_comp < kTotalReqs) {
      rv = verify_opaque_tag(exp_encr_opaque_tag, false, FLAGS_long_poll_interval);
      if(0 != rv) {
        printf("pending %u comp %u\n", pending_encr_reqs, encr_reqs_comp);
        goto done;
      }
      testcase_end(tcid, iter++);
      encr_reqs_comp = exp_encr_opaque_tag;
      exp_encr_opaque_tag += kBatchSize;
    }

    //Queue next batch size of encr
    if(pending_encr_reqs >= kBatchSize) {
      for(uint32_t i = kTotalReqs - pending_encr_reqs;
          i < kTotalReqs - pending_encr_reqs + kBatchSize;
          i++) {
        if(i == kTotalReqs - pending_encr_reqs + kBatchSize - 1) 
          ctx[i].xts_ctx1.ring_db = true;
        rv = ctx[i].xts_ctx1.test_seq_xts();
        if(0 != rv) goto done;
      }
      pending_encr_reqs -= kBatchSize;
    }

    //Wait for batch size decr to complete
    if(exp_decr_opaque_tag && decr_reqs_comp < kTotalReqs) {
      rv = verify_opaque_tag(exp_decr_opaque_tag, true, FLAGS_long_poll_interval);
      if(0 != rv) {
        printf("pending %u comp %u\n", pending_decr_reqs, decr_reqs_comp);
        goto done;
      }
      testcase_end(tcid, iter++);
      decr_reqs_comp = exp_decr_opaque_tag;
    }

    //Queue next batch size of decr
    if(pending_decr_reqs >= kBatchSize) {
      testcase_begin(tcid, iter);
      for(uint32_t i = kTotalReqs - pending_decr_reqs;
          i < kTotalReqs - pending_decr_reqs + kBatchSize;
          i++) {
        if(i == kTotalReqs - pending_decr_reqs + kBatchSize - 1) 
          ctx[i].xts_ctx2.ring_db = true;
        rv = ctx[i].xts_ctx2.test_seq_xts();
        if(0 != rv) goto done;
      }
      pending_decr_reqs -= kBatchSize;
      exp_decr_opaque_tag += kBatchSize;
    }

    if(encr_reqs_comp >= kTotalReqs &&
       decr_reqs_comp >= kTotalReqs) {
      break;
    }
  }

  for(uint32_t i = 0; i < kTotalReqs; i++) {
    rv = e2e_verify(ctx->xts_ctx1, ctx->xts_ctx2);
    if(0 != rv) goto done;
  }

done:
  delete ctx;
  free_host_mem(in_buffer);
  free_host_mem(stg_buffer);
  free_host_mem(out_buffer);

  return rv;
}

int xts_in_place() {

  XtsChainCtx ctx;
  ctx.verify_e2e = false;
  ctx.xts_ctx1.op = xts::AES_ENCR_ONLY;
  ctx.xts_ctx2.op = xts::AES_DECR_ONLY;

  srand(time(NULL));
  dp_mem_t *buf_sector = write_buf->fragment_find(0, kSectorSize);
  for(uint32_t i = 0; i < kSectorSize/sizeof(int); i++) {
    ((int*)buf_sector->read())[i] = rand();
  }
  buf_sector->write_thru();

  dp_mem_t *hbm_sector = write_hbm_buf->fragment_find(0, kSectorSize);
  memcpy(hbm_sector->read(), buf_sector->read(), kSectorSize);
  hbm_sector->write_thru();

  ctx.xts_ctx1.src_buf = (void*)write_hbm_buf->va();
  ctx.xts_ctx1.is_src_hbm_buf = true;
  ctx.xts_ctx1.dst_buf = (void*)write_hbm_buf->va();
  ctx.xts_ctx1.is_dst_hbm_buf = true;

  ctx.xts_ctx2.src_buf = (void*)write_hbm_buf->va();
  ctx.xts_ctx2.is_src_hbm_buf = true;
  ctx.xts_ctx2.dst_buf = (void*)write_hbm_buf->va();
  ctx.xts_ctx2.is_dst_hbm_buf = true;

  ctx.num_ops = 2;
  ctx.xts_ctx1.init(kDefaultBufSize);
  ctx.xts_ctx2.init(kDefaultBufSize, true);
  int rv = ctx();
  if(0 == rv) {
    rv = memcmp(buf_sector->read_thru(), hbm_sector->read_thru(), kSectorSize);
    if(0 != rv) {
      rv = -1;
      printf(" Memcmp failed %d \n", rv);
    }
  }

  return rv;
}

uint32_t netapp_plain_text[1024] = {
    0x6c6c6548, 0x6f57206f, 0x31646c72, 0x33323120,
    0x37363534, 0x20303938, 0x6c6c6548, 0x6f57206f,
    0x32646c72, 0x33323120, 0x37363534, 0x20303938,
    0x6c6c6548, 0x6f57206f, 0x33646c72, 0x33323120,
    0x37363534, 0x20303938, 0x6c6c6548, 0x6f57206f,
    0x34646c72, 0x33323120, 0x37363534, 0x20303938,
    0x6c6c6548, 0x6f57206f, 0x35646c72, 0x33323120,
    0x37363534, 0x20303938, 0x6c6c6548, 0x6f57206f,
    0x36646c72, 0x33323120, 0x37363534, 0x20303938,
    0x6c6c6548, 0x6f57206f, 0x37646c72, 0x33323120,
    0x37363534, 0x20303938, 0x6c6c6548, 0x6f57206f,
    0x38646c72, 0x33323120, 0x37363534, 0x20303938,
    0x6c6c6548, 0x6f57206f, 0x39646c72, 0x33323120,
    0x37363534, 0x20303938, 0x6c6c6548, 0x6f57206f,
    0x31646c72, 0x32312030, 0x36353433, 0x30393837,
    0x0000000a, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

};

uint32_t netapp_cipher_text[1024] = {
    0xf87891c2, 0x855cb824, 0x3de6673e, 0xe3ac9471,
    0x8fd147cb, 0xddf0e8a4, 0xd8b345b0, 0x99e23666,
    0x91ad3fa3, 0x01e05526, 0x07080258, 0xd8a5a294,
    0x5796c34a, 0xb247bded, 0x998844bf, 0x3ff481e0,
    0x4a2e6790, 0x859759d3, 0xf969c719, 0xa01c17e5,
    0x1096b81e, 0x122105f7, 0xe047c83f, 0x0900a527,
    0x2ba07cbb, 0xa02b5560, 0x72e81bb3, 0xa2297745,
    0x884a26fc, 0x16fa77ac, 0x468ec667, 0x598635f1,
    0x925c091a, 0xa11c3a6d, 0x1322887a, 0x0b5bf90e,
    0x67fa486f, 0x5557651d, 0x824674bd, 0xec3f929d,
    0x6f389e58, 0xbcbb07d8, 0x5e01b354, 0x38e4ba12,
    0xf4e63075, 0xc1548948, 0xa520a81e, 0xe7c1058f,
    0x6f5cb3c6, 0xe79a198d, 0xc26d3290, 0xd25d1fcb,
    0x1da2f41d, 0x423b6528, 0x1a8b441d, 0xf371d2ad,
    0xd5bbb3ca, 0x77c09c8f, 0x1c076164, 0x3d9e16ae,
    0xc3dfce8e, 0x58a6b6a1, 0xd145ffca, 0x0f20d0f0,
    0xa76a04f1, 0xdc2f3229, 0x56cd34ed, 0x8b27ccf1,
    0x61f1c57d, 0xab375e38, 0x43b37166, 0xa65bcd04,
    0xf2c343aa, 0x7921f0ef, 0x09c3d665, 0x186ef58e,
    0x74c3e4ff, 0xc329c8f2, 0xed84ba7d, 0xfd7930eb,
    0xab36c75d, 0xb4ced5e7, 0x37f21dda, 0x34d6ea98,
    0x2bc22c2d, 0x7ea29e1f, 0x52184665, 0xbee4791a,
    0x5e1220eb, 0x8b3da9ae, 0x3958f87c, 0xb1ae6e0f,
    0xdf6ba6c7, 0xf54111e1, 0xbf41c57e, 0x83c43e61,
    0xe532fd57, 0x18280fd6, 0xf3f5b5c3, 0x5a0694a1,
    0x432a4e24, 0xb75d0353, 0x664c62ea, 0xfee8e7f5,
    0x2e95075b, 0x1e3f2e9e, 0x848914ee, 0xcffb08f3,
    0x659f0c27, 0xf500dc4d, 0xb8c90294, 0x93fe1459,
    0xfba66791, 0x42c56bab, 0x279b374e, 0x08f40ea6,
    0xab533108, 0xb2841bce, 0x720ff700, 0xddeb24da,
    0x8e5d5df9, 0x8267f45e, 0xc5256187, 0x2cd728b5,
    0x9238f707, 0xebdf09fb, 0x25dad967, 0x5409602e,
    0x0c5b702f, 0xfc2e774d, 0x69f1042f, 0xcbf7a30f,
    0xd4fe6a79, 0xa793b6e1, 0xa786200e, 0x06360949,
    0xb6e02454, 0xd4431cd8, 0xba0c0e33, 0x4c71ba2a,
    0x7fcf3e6c, 0x1a2f07ce, 0x6057cc49, 0xa8a9bebe,
    0xda085f0e, 0x5df09ee7, 0xb65b8e57, 0x7559c40a,
    0xac02ec46, 0x2a408eb9, 0x0804a4d0, 0xd79a86a5,
    0x389e5897, 0xe550ed00, 0x964e21e8, 0x5cf06b5e,
    0xe83c85ec, 0x2cd7a2ef, 0xad1eae24, 0x09b4440d,
    0xc3e05ac1, 0x19d508ba, 0x1c4f8603, 0x9bdff9e8,
    0xb2d4f7a5, 0xdf33b37b, 0x1bbbdd2c, 0xc96741be,
    0x9c826d1e, 0x694ce529, 0x0887370d, 0x79ecb9a9,
    0x074379d7, 0x609af301, 0xc3773907, 0x16be7d38,
    0x6f295fe7, 0xbd913b06, 0x3d4ed678, 0x6aae3a5a,
    0xab3cc3a2, 0xd9bbc4ae, 0x4a289fea, 0x863fdab4,
    0xe2d28d87, 0x965634b6, 0x57c45fea, 0xf9ffd148,
    0x23213037, 0x9cdc1de5, 0x2e78b9ed, 0x9188e6a5,
    0x41794b17, 0x266785f7, 0xc180a88e, 0x9fddf7ce,
    0xd4e96e34, 0x04b7bfa0, 0x48efa861, 0xe0de399c,
    0xd0bb5ce6, 0x3a4811d6, 0x4f7cf17a, 0x616c4383,
    0xb1afaee2, 0x83271b9e, 0x0bebaf5e, 0xebcfb8ba,
    0x5b281715, 0x61ac23e1, 0xc9314d0f, 0xb599afa4,
    0x60a82de9, 0xd7205f89, 0x7f2430ea, 0x368283c8,
    0x2fe7f406, 0x6a40ae5a, 0xf6d2d62b, 0x3f7713d5,
    0xdfe10861, 0x48c623d1, 0x883f0abe, 0x57a4875e,
    0x7c218d6f, 0x689bd6d3, 0xa7bf3b6b, 0x3869ec84,
    0x90b0e520, 0x1e5ebd83, 0x98a0ff53, 0x4075337f,
    0x6a0a5227, 0x60114b9e, 0xfa49435f, 0x0df65d24,
    0xf493f014, 0xca9f66d0, 0x3f6793b0, 0x85fbec7c,
    0xc9c592a0, 0x2b18c5ef, 0x2050c6bd, 0xe44c4e71,
    0x0167a7ac, 0xf4d4676e, 0x03dda594, 0x1ee065cf,
    0x204ae574, 0x5849749e, 0xefdefcf4, 0x7513e9c1,
    0xdf0d1874, 0xfbafd1de, 0x4d7e8b80, 0x329acecc,
    0xf0ae175b, 0xeaca5c26, 0x450f3e99, 0x5b3ba202,
    0xab18d8c7, 0x5ae6fc39, 0x3ccbb068, 0xb1539628,
    0x33be5a2c, 0xf44a7b9d, 0x5be8edf0, 0x4876b20b,
    0x621d7e37, 0xea7b6703, 0x91edb60d, 0x8ac5a8ae,
    0xe6882507, 0xfc0598ea, 0xca544501, 0xcf793dc6,
    0x097d558d, 0x14fa15be, 0x8958ea84, 0x69124261,
    0x7b3ac936, 0xe9dccb2d, 0xc687ef10, 0xf921cf46,
    0x27be136f, 0x5a995fd4, 0xc6cd1ad8, 0x76ce053b,
    0xbe842c3a, 0x9962b08f, 0x8a8ce2de, 0x3a2d9eeb,
    0x79701ff0, 0x0b79d375, 0x58ed09fd, 0x15f92119,
    0xe7f26f34, 0x1b173b07, 0x513f0dae, 0x47a81050,
    0xb3eea443, 0x6cf62f58, 0x9080ebb8, 0xccc0a80c,
    0x1e56b2f6, 0x397bfd5d, 0x513db4b5, 0x55567a89,
    0xaf45d7c3, 0x37b7bac2, 0x9425ee32, 0x79fb8328,
    0x1c4b803a, 0x605dc22f, 0x5cdfb572, 0xb8f8e902,
    0x28cff20e, 0x066a5014, 0x7b1de0a7, 0x5122ada4,
    0xdd8c9b69, 0xc3af277e, 0x6dc2ff80, 0x4dc3aa57,
    0x4919f3cc, 0xdeb4b157, 0xa414ae43, 0x0005add4,
    0x9c0a5103, 0x21e848a6, 0x72e56752, 0x677a5e97,
    0xa1bd16c3, 0x460bcceb, 0x373427fe, 0x8817705d,
    0x4fdb296e, 0x60f61ee0, 0xabadb944, 0xe8b18be7,
    0x590c4e10, 0x4076031e, 0x7e32bc2d, 0x54da1f49,
    0xa67a3833, 0xa964dbdc, 0xe6d00f46, 0xd72fe40f,
    0x3ecdafb7, 0xf2bf2a03, 0xe09ce70e, 0x403f852f,
    0x0e6837a0, 0xe31babd8, 0xb93ec99f, 0x8b95a21f,
    0x97abd75d, 0x393b1a54, 0xa5d60678, 0x1c302515,
    0xcc430a8a, 0xb1a363bf, 0x09e22924, 0x3b392fce,
    0x45047237, 0x34b9b5da, 0xe86f43b9, 0xf0eb811d,
    0x0f66fd34, 0x33ded541, 0x717505a0, 0xc2ef6856,
    0x28109428, 0x1daf424f, 0x083252a9, 0xaefdd946,
    0x86e45828, 0xcdb5cf05, 0x57c8d952, 0xdcbab30e,
    0xe039f2df, 0x37f0247c, 0x8a41e151, 0xb2234bbe,
    0xea0a1bf4, 0x620e2792, 0xd29ab90e, 0xc15ea6b0,
    0x5d16f7a7, 0x1deac3d8, 0x0e7b1462, 0xcaff3425,
    0x2db74497, 0xa9778cba, 0x02833383, 0x5ed5763d,
    0x0e03d142, 0xa26622cc, 0xcfab6abd, 0xf85c8dd0,
    0x88497c52, 0xd4f00a1b, 0x0e337741, 0x350a6c43,
    0x1b95ed7b, 0x4f42776a, 0x67e38c23, 0x29bce2b4,
    0x03ab49a0, 0xea945ca6, 0x42559dc0, 0xd9e25f65,
    0x795c8385, 0x98305fa4, 0x2f8d6f20, 0xe383749d,
    0xb93877fc, 0x616de029, 0x11a0d958, 0x3af38d3a,
    0x257a4245, 0x4cd13bde, 0xed6ac6a8, 0xd14ccf53,
    0x23333487, 0x2baf8a91, 0xf8ae7020, 0x7b164c21,
    0x3ba6bd41, 0xd874f598, 0xff6e0e92, 0x4a99e2fe,
    0x1315bf2a, 0x7e641684, 0x242993d4, 0x029d715d,
    0x9f21dca2, 0x8ebea1f5, 0xd6dfe2ed, 0xf39dc45e,
    0x4fc03301, 0x50c65348, 0x9e555fbb, 0xb03498a1,
    0x9da6754e, 0x22a2964b, 0x36184f6b, 0xa2b8c0ea,
    0x1d31e4dd, 0x15840187, 0xbb3e842e, 0x52447a64,
    0x3c42043b, 0x526d0fa2, 0x2db73e42, 0x47e0af6c,
    0xed337f90, 0xab8ffcd6, 0x2476479f, 0xf1a0a026,
    0x8ae04a11, 0x51340e62, 0x26f54d20, 0x24002256,
    0x41218eec, 0x30bdc5b7, 0xcb704ee4, 0xf727baa5,
    0xf8f0c695, 0x3a9d8abc, 0xac89000f, 0xee940116,
    0xc490e13e, 0xd7e9c878, 0x70322cf4, 0xb10ac9e7,
    0x0b3f5ecb, 0x92e996f8, 0x04396406, 0x2cdba618,
    0xb0dd1bbf, 0x8144fcc3, 0x50c82faf, 0x32b8c6f0,
    0xb233c126, 0x9af2ed43, 0x43b3f6a2, 0xafb8a4dc,
    0x642058b0, 0x9ea0537e, 0x34174566, 0x163e0504,
    0xd562ff64, 0xe59ee674, 0x3141f79a, 0x5f50d3c2,
    0xd7170157, 0x5d18bbc3, 0xbe94acc5, 0xcf276a07,
    0x07b9eed6, 0x39b0c925, 0xc8bc540b, 0x955cf96c,
    0x8763952d, 0x0560c3d3, 0x353cd5d9, 0xe8730500,
    0x425c74a3, 0xf15cc2ff, 0x79164067, 0x42cad55b,
    0x66a22784, 0x063fcc40, 0x29666093, 0x04125495,
    0xa67b7d36, 0x889dd9c0, 0xd29ad558, 0xa18b2cf4,
    0x10eeed58, 0xcbd82b32, 0xfb382796, 0xffda20b4,
    0xdbe78989, 0x8eefe271, 0x873e38c4, 0xc0e155d3,
    0x7c9dd7bc, 0x141c7489, 0xc02b3207, 0x1a6726ec,
    0xac1b1326, 0xfd0296fc, 0x16dfaec2, 0xfd614f30,
    0x4360ba77, 0x346a4847, 0xc4879e6b, 0xaf03b0ee,
    0x9047c715, 0x532ad6ba, 0x9f79c775, 0xead3066b,
    0xb1a712d3, 0xf4f696e9, 0xdca29cbe, 0x37963130,
    0x66fccda4, 0x44a16776, 0x3f09f5aa, 0xe8ddb69b,
    0x7dd8335a, 0xef254692, 0xefcfa270, 0xb1986867,
    0xbb73a271, 0x8cebeeed, 0xf469ee40, 0x7e809c76,
    0x80498207, 0x245c2454, 0xa5dce72b, 0xfae887b0,
    0xe1f06f22, 0x850e4562, 0x4502d325, 0x60e0b720,
    0x4e0523fb, 0xe56a37ec, 0x693dc9df, 0x43f723c2,
    0x04209b39, 0x25715838, 0x19a58832, 0x553ee717,
    0xd92537a6, 0x2451bcdd, 0x5ee61892, 0xfd73af14,
    0x9ded6e76, 0x22818231, 0xb12f5d3d, 0x1bfed9a6,
    0x4618e81d, 0x595073fb, 0x9686f9ce, 0xd46e3ecf,
    0x6403fa64, 0x91792760, 0xcaee7022, 0xfcfb94f9,
    0x9aa901b9, 0x4ec3541f, 0x2f5d6db4, 0xf0fd36d7,
    0xb8562a49, 0x85491dce, 0xf736c515, 0x6272d754,
    0xe8c3dd76, 0xc924387f, 0xd68fd872, 0x8a2c11fa,
    0x81255e8c, 0xa9b1abd5, 0x5e2745d6, 0x7896a0c1,
    0x8582160b, 0x556b944e, 0x9b1f9010, 0x05df30d6,
    0x28381e67, 0x723acafc, 0xcd0fa8f0, 0xf4092c90,
    0x30bcd367, 0xc9074728, 0x353aa7ce, 0xbb9ee585,
    0xc6f3fb4f, 0x87f2ac9e, 0x41fe4121, 0x0c554fb5,
    0x97cfe2b9, 0x2aea9735, 0xdadcac6e, 0x88b4853c,
    0xffc0e0d7, 0xa83879c5, 0xebee56ea, 0xcc7f2b4a,
    0x39e881a9, 0xe8fea910, 0x750264de, 0x7dd816d0,
    0xe5e83777, 0x71f25b77, 0x36f57443, 0x8946f020,
    0xbc935cfc, 0xabff2161, 0x7228432d, 0xa9044cea,
    0x5ff3f42d, 0x1349ba16, 0xd2edfefb, 0x88c673fe,
    0xddb2d604, 0x7cdcb0a9, 0x9bf83ab9, 0x959bf0e0,
    0x18d2436b, 0xaa736abc, 0xb23b595e, 0x763f1f60,
    0xffcd679e, 0x7b13f212, 0x2ef1c616, 0xc2c944b6,
    0x4855e252, 0x42d565d5, 0xa5aad5ce, 0xd987fe08,
    0x692ae882, 0x152c0d19, 0x16dd9f76, 0x58af765a,
    0x2c55bd65, 0xfa75601b, 0xfd95b9d8, 0x40552f19,
    0x70ea6525, 0xdd955142, 0xd00f7c4f, 0x118ee157,
    0x4089f8cf, 0xbe745337, 0xb6dce355, 0x8a41f5dc,
    0x28b85e28, 0x624a5e20, 0x8b9b9ecb, 0x53e17386,
    0x60aa9e21, 0xaca34892, 0x1d498ca3, 0x943912a6,
    0xde032a8f, 0xf360f323, 0xdf000514, 0xce217dd5,
    0xf0a4f1fd, 0xb2746fe7, 0xb98b8226, 0x18161495,
    0x5485113d, 0x91bc2d7e, 0x7fd02dca, 0x775e2d16,
    0x13b1adbc, 0xde0ff197, 0xe591f586, 0xe6803433,
    0x6a27c579, 0x55dee50e, 0xc04bfbcc, 0x1f635753,
    0x1dacefb1, 0x09c675bb, 0x34e7bc91, 0xacf2605a,
    0xd7c98272, 0x7490b1ab, 0xb82f710d, 0xc6334c1e,
    0x58636b97, 0x9d2531dc, 0x1b2b66b1, 0xacc80d5d,
    0xbf31865e, 0xff43823d, 0x37b7b92f, 0xfcc24f60,
    0x53e0c4e0, 0x033d018f, 0x4facf201, 0xa4447a8e,
    0x78935382, 0x50eaeb49, 0xac158488, 0xc2262e88,
    0xc11205ef, 0xf95c0ed0, 0x479f7db0, 0xae98673e,
    0x2c2a9169, 0xffb6ff2d, 0xd8db600a, 0x6d94190a,
    0xf902888c, 0xc675b07f, 0x18bcf3f8, 0xb5c84698,
    0x412ce8fe, 0xf21ef98f, 0xdf551e73, 0x9d680a43,
    0xbcaa1731, 0xd764cabb, 0x5e631ca3, 0x90f28fa2,
    0xfaa5c180, 0xd4ce2d0a, 0x428fe8ae, 0x527ceb69,
    0xfb7a2662, 0xe795b23f, 0x25a23b11, 0xa8462aa2,
    0x16f4ddfc, 0x58bfc6cc, 0xe569b389, 0x62204fc3,
    0x8f33a6eb, 0xc9d7d5ba, 0xb1f2c47c, 0xf9237f65,
    0x32ff1663, 0xde157503, 0x90bc0576, 0x2906dc4f,
    0x1325abff, 0xbed04ece, 0x24b9a110, 0xcf3c9f9c,
    0x9111787c, 0x512b1f94, 0x65aa98a7, 0x6d7cde03,
    0xa616950c, 0x65a2359a, 0x0b79a249, 0xfabe35e2,
    0xa636aa67, 0xf2d86acb, 0x37d865ac, 0x97637ae9,
    0x4a658a05, 0xe017b22b, 0xf7f4ec8f, 0x8cabe795,
    0xbbb0a9d6, 0x6da2b6b2, 0x54722146, 0x245ea3ef,
    0x8e473f27, 0xd2913682, 0x5e4b5391, 0xedfe7a7a,
    0xef78be35, 0xf2ece9d6, 0xd15bc61e, 0x3f039852,
    0x3f301efc, 0xa3101d13, 0x79d13278, 0x27b705d2,
    0x12e891a3, 0x21affc10, 0x41ba2c0d, 0xaeeaba58,
    0x48b3bab5, 0xe2390a6e, 0x7666db05, 0x799b3a63,
    0x6fb01f2b, 0x62d13e6f, 0xe8558300, 0x4b141b08,
    0xe2f347f3, 0x65bdc86f, 0xf77aedc7, 0x31e5361e,
    0x1065d673, 0x1415bac8, 0x9d3e36d3, 0x3438fb73,
    0x20a047a4, 0xc89b3817, 0x176a5814, 0xdf4138d3,
    0x797d5ef5, 0x1803084b, 0xa34f3d08, 0xa96bd7aa,
    0x43a17b4a, 0x4c250169, 0xb21d04ec, 0x41720698,
    0xe439e708, 0xc0f6732c, 0x5d5344d1, 0xe2172281,
    0x8fce5741, 0x084b864f, 0x3e9cd765, 0x0011266d,
    0xaaf276a1, 0x21ad303d, 0xc467ebe7, 0xafb4d7f1,
    0x34227721, 0xfb717a2d, 0xefc44d6a, 0xa1eb2be7,
    0xb796bc8f, 0x1f1c5fdc, 0xa050ca9b, 0xcd0003ed,
    0x34d90a34, 0x2a7e937e, 0xbf599a7e, 0xc76932b2,
    0xc124d649, 0x74a0c345, 0xe46bd5f9, 0x52a9ea20,
    0x5c83a3d8, 0x7a549320, 0xc2f38bde, 0xcbe8a825,
    0xe86d3414, 0x198dfcd5, 0x8a1b279d, 0xaefddd6b,
    0x0765cf16, 0x45e06b4e, 0x958c2d57, 0xbdaa04e9,
    0xa830cbe0, 0x626d6deb, 0x5405a848, 0xb8cc719d,
    0xaa6acd14, 0x47cf95cf, 0x59fdc84f, 0xf991e63f,
    0xd5de316b, 0x495542ab, 0x97ed4a9b, 0x0a38f1b7,
    0x8261fced, 0xb32ec086, 0x90eb2806, 0x3def8cd2,
    0x9feefc67, 0xe9d6ab5d, 0xa65b2113, 0x4cf13acf,
    0x898a2f40, 0x6ac38ebf, 0xd88fac6b, 0xffac5c4c,
    0x1fef1a04, 0xaeab5b05, 0xc1c347fa, 0x73cf786d,
    0xb50eff7d, 0x1d6a83f8, 0xe9666c91, 0xc64ee0ac,
    0x4f4090cc, 0x2645859c, 0x152fecba, 0x96662ce4,
    0x29319549, 0xcf5e052c, 0xa2b53de5, 0x25b67b90,
    0x0d66b23f, 0xe1b57e69, 0x56d4126f, 0xe4a2519f,
    0x454ecac8, 0xb88f1cd7, 0xe8e5433e, 0xc502d3f7,
    0x82f74c1b, 0x83fbad06, 0xc9153332, 0xe1d0b8b1,
    0x4ce53c64, 0x499e3fbe, 0xc9228214, 0x8f1de2cc,
    0x45dba97e, 0x871701d0, 0x593ad355, 0xe4b0cd1e,
    0xf3ddd71a, 0x0256e94a, 0x5065f01a, 0xb2756fde,
    0xfc0dedbb, 0x61658139, 0x1a742a85, 0xb0bdef25,
    0x36179757, 0x3e74120c, 0x8defb690, 0x9859f8c0,
    0x2dec34c9, 0x3c911ea0, 0x43525d71, 0x62c19400,
    0x20162c66, 0x8dedc67f, 0x264cea65, 0x43ee74f1,
    0xac94d50c, 0x2381a6cd, 0x5a653aa0, 0x463e546f,
    0xc2f215c7, 0x9376d67e, 0xe886b401, 0xd6d29f12,
    0x8c86ec44, 0xd07ea56d, 0xf658cee3, 0x6c7ac15c,
    0xe6999085, 0xe939b713, 0xa0d239a1, 0xfb4dbef6,
    0x1fe92197, 0x6fdb4f4c, 0xcef91e8e, 0xd2bcd488,
    0x03a56fd7, 0x4da2a4e6, 0xac585d5d, 0xa2fecb40,
    0x50ad51e3, 0x287d2b8a, 0x14835115, 0x3f95bf0a,
    0xf50d29f2, 0x1e91f969, 0xb49bfec5, 0xcde9b0b3,
    0x4e292763, 0x5c9b3932, 0x66c815ba, 0x1678dc16,
    0xc7ec554d, 0xdddac26f, 0x28b3a47a, 0x9ff660a4,
    0x6c9ac10f, 0xac788cea, 0xf6e52bbe, 0x91c5d75e,
    0x4416af8e, 0xa11630a7, 0x71c00d06, 0x36966020,
    0x5291358c, 0x232ee85f, 0x7ab9de0d, 0xc7ff84f1,
    0x437051d7, 0x42ff876f, 0x4adda0c5, 0x136f705d,
    0x2bc80e70, 0x5f0c8790, 0x04bcc2db, 0x1105cd79,
    0xa42aff15, 0xf6de0b1c, 0x53dc11b8, 0xe1e90b36,
};

int xts_netapp_data_util(void* key_cp) {
  XtsChainCtx ctx;
  ctx.xts_ctx1.op = xts::AES_ENCR_ONLY;
  ctx.xts_ctx1.key_size = AES256_KEY_SIZE;
  ctx.xts_ctx2.op = xts::AES_DECR_ONLY;
  ctx.xts_ctx2.key_size = AES256_KEY_SIZE;

  uint8_t iv_src_cp[] = {0x2c, 0x23, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  memcpy(key, key_cp, sizeof(key));
  memcpy(iv_src, iv_src_cp, sizeof(iv_src));
  memcpy(write_buf->read(), netapp_plain_text, kDefaultBufSize);
  write_buf->write_thru();
  key_desc_inited = false;

  srand(time(NULL));
  for(uint32_t i = 0; i < kDefaultBufSize/sizeof(int); i++) {
    ((int*)read_buf->read())[i] = rand();
    ((int*)read_buf2->read())[i] = rand();
  }
  read_buf->write_thru();
  read_buf2->write_thru();

  ctx.xts_ctx1.src_buf = (void*)write_buf->va();
  ctx.xts_ctx1.dst_buf = (void*)read_buf2->va();
  ctx.xts_ctx1.num_sectors = kDefaultBufSize/SECTOR_SIZE;

  ctx.xts_ctx2.src_buf = (void*)read_buf2->va();
  ctx.xts_ctx2.dst_buf = (void*)read_buf->va();
  ctx.xts_ctx2.num_sectors = kDefaultBufSize/SECTOR_SIZE;

  ctx.num_ops = 2;
  ctx.xts_ctx1.init(kDefaultBufSize);
  ctx.xts_ctx2.init(kDefaultBufSize, true);
  int rv = ctx();
  if(0 == rv) {
    rv = memcmp(read_buf2->read_thru(), (void*)netapp_cipher_text, kDefaultBufSize);
    if(0 != rv) {
      rv = -1;
      printf(" Memcmp of cipher text failed %d \n", rv);
    }
  }

  return rv;
}

void swap(uint8_t* this1, uint8_t* that1) {
  uint8_t tmp;
  tmp = *this1;
  *this1 = *that1;
  *that1 = tmp;
}

#include <arpa/inet.h>
void switch_endian(uint32_t* buf, int size) {
  /*for(int i = 0; i < size; i++) {
    uint8_t* tmp = &buf[i];
    swap(&tmp[3], &tmp[0]);
    swap(&tmp[2], &tmp[1]);
  }*/
  for(int i = 0; i < size; i++) {
    uint32_t tmp = htonl(buf[i]);
    buf[i] = tmp;
  }
}

int xts_netapp_data() {
  uint8_t key_cp1[] = {
      0x59, 0x62, 0x0d, 0xe1, 0x86, 0x9a, 0x2e, 0x26, 0xd2, 0x0e, 0xb7, 0xf0, 0xfa, 0x46, 0x47, 0xd6,
      0xbf, 0x0f, 0x10, 0x93, 0x69, 0xb1, 0xe6, 0xfa, 0x3b, 0x6f, 0x45, 0x20, 0x7f, 0x80, 0xd8, 0xc9,
      0x3b, 0xd4, 0xd1, 0x56, 0xd6, 0x34, 0x51, 0xaf, 0x08, 0x62, 0xe5, 0x48, 0xb7, 0x9a, 0xfb, 0xa4,
      0xa6, 0xaf, 0x44, 0x3c, 0x96, 0x9c, 0x03, 0x14, 0x5c, 0x1d, 0x44, 0x27, 0xe6, 0x73, 0x4f, 0xf9,
  };
  uint8_t key_cp2[] = {
      0x3b, 0xd4, 0xd1, 0x56, 0xd6, 0x34, 0x51, 0xaf, 0x08, 0x62, 0xe5, 0x48, 0xb7, 0x9a, 0xfb, 0xa4,
      0xa6, 0xaf, 0x44, 0x3c, 0x96, 0x9c, 0x03, 0x14, 0x5c, 0x1d, 0x44, 0x27, 0xe6, 0x73, 0x4f, 0xf9,
      0x59, 0x62, 0x0d, 0xe1, 0x86, 0x9a, 0x2e, 0x26, 0xd2, 0x0e, 0xb7, 0xf0, 0xfa, 0x46, 0x47, 0xd6,
      0xbf, 0x0f, 0x10, 0x93, 0x69, 0xb1, 0xe6, 0xfa, 0x3b, 0x6f, 0x45, 0x20, 0x7f, 0x80, 0xd8, 0xc9,
  };

  int rv = xts_netapp_data_util((void*)key_cp1);
  if(0 != rv) {
    printf(" Netapp validation failed. Endianness: AsIs, KeyOrder: AES-XTS \n");
  }

  rv = xts_netapp_data_util((void*)key_cp2);
  if(0 != rv) {
    printf(" Netapp validation failed. Endianness: AsIs, KeyOrder: XTS-AES \n");
  }

  switch_endian(netapp_plain_text, 1024);
  switch_endian(netapp_cipher_text, 1024);

  rv = xts_netapp_data_util((void*)key_cp1);
  if(0 != rv) {
    printf(" Netapp validation failed. Endianness: Reversed, KeyOrder: AES-XTS \n");
  }

  rv = xts_netapp_data_util((void*)key_cp2);
  if(0 != rv) {
    printf(" Netapp validation failed. Endianness: Reversed, KeyOrder: XTS-AES \n");
  }

  return rv;
}

int fill_ctx(XtsCtx* ctx1, XtsCtx* ctx2, XtsCtx* ctx3, XtsCtx* ctx4,
             uint64_t in_buffer, uint64_t stg1_buffer, uint64_t stg2_buffer,
             uint64_t stg3_buffer, uint64_t out_buffer, bool is_hbm_buf = false) {
  unsigned char scratch[kDefaultBufSize];
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    ctx1[i].op = xts::AES_ENCR_ONLY;
    ctx1[i].use_seq = false;
    ctx1[i].verify_db = false;
    ctx1[i].ring_db = false;
    ctx1[i].num_sectors = kDefaultBufSize/kSectorSize;
    ctx1[i].opaque_tag = i+1;

    ctx2[i].op = xts::AES_DECR_ONLY;
    ctx2[i].use_seq = false;
    ctx2[i].verify_db = false;
    ctx2[i].ring_db = false;
    ctx2[i].num_sectors = kDefaultBufSize/kSectorSize;
    ctx2[i].opaque_tag = i+1;

    ctx3[i].op = xts::AES_ENCR_ONLY;
    ctx3[i].use_seq = false;
    ctx3[i].verify_db = false;
    ctx3[i].ring_db = false;
    ctx3[i].num_sectors = kDefaultBufSize/kSectorSize;
    ctx3[i].is_gcm = true;
    ctx3[i].opaque_tag = i+1;

    ctx4[i].op = xts::AES_DECR_ONLY;
    ctx4[i].use_seq = false;
    ctx4[i].verify_db = false;
    ctx4[i].ring_db = false;
    ctx4[i].num_sectors = kDefaultBufSize/kSectorSize;
    ctx4[i].is_gcm = true;
    ctx4[i].opaque_tag = i+1;

    ctx1[i].src_buf = (void*)(in_buffer + i*kDefaultBufSize);
    ctx1[i].is_src_hbm_buf = is_hbm_buf;
    ctx1[i].dst_buf = (void*)(stg1_buffer + i*kDefaultBufSize);
    ctx1[i].is_dst_hbm_buf = true;
    if(is_hbm_buf) {
      memset(scratch, i, sizeof(scratch));
      write_mem((uint64_t)ctx1[i].src_buf, scratch, kDefaultBufSize);
    }

    ctx2[i].src_buf = (void*)(stg1_buffer + i*kDefaultBufSize);
    ctx2[i].is_src_hbm_buf = true;
    ctx2[i].dst_buf = (void*)(stg2_buffer + i*kDefaultBufSize);
    ctx2[i].is_dst_hbm_buf = true;

    ctx3[i].src_buf = (void*)(stg2_buffer + i*kDefaultBufSize);
    ctx3[i].is_src_hbm_buf = true;
    ctx3[i].dst_buf = (void*)(stg3_buffer + i*kDefaultBufSize);
    ctx3[i].is_dst_hbm_buf = true;

    ctx4[i].src_buf = (void*)(stg3_buffer + i*kDefaultBufSize);
    ctx4[i].is_src_hbm_buf = true;
    ctx4[i].dst_buf = (void*)(out_buffer + i*kDefaultBufSize);
    ctx4[i].is_dst_hbm_buf = is_hbm_buf;

    ctx1[i].init(kDefaultBufSize);
    ctx2[i].init(kDefaultBufSize, true);
    ctx3[i].init(kDefaultBufSize, true);
    ctx4[i].init(kDefaultBufSize, true);
  }
  return 0;
}

int e2e_verify_hbm_buf(XtsCtx& xts_ctx1, XtsCtx& xts_ctx2) {
  int rv = 0;
  unsigned char src_buf_cp[kDefaultBufSize], dst_buf_cp[kDefaultBufSize];
  memset(src_buf_cp, 0, kDefaultBufSize);
  memset(dst_buf_cp, 0, kDefaultBufSize);

  read_mem((uint64_t)xts_ctx1.src_buf, src_buf_cp, kDefaultBufSize);
  read_mem((uint64_t)xts_ctx2.dst_buf, dst_buf_cp, kDefaultBufSize);

  rv = memcmp(src_buf_cp, dst_buf_cp, kDefaultBufSize);
  if(0 != rv) {
    rv = -1;
    printf(" Memcmp failed %d \n", rv);
  } else {
    printf(" e2e verify memcmp passed\n");
  }

  return rv;
}

int ring_doorbell(uint64_t ring_pi_addr, uint32_t pi_inc) {
  uint32_t pindex = 0;
  read_reg(ring_pi_addr, pindex);
  pindex = (pindex + pi_inc) % (kXtsQueueSize-1);
  write_reg(ring_pi_addr, pindex);
  return 0;
}

uint64_t xts_encr_tag_addr = 0, xts_decr_tag_addr = 0, gcm_encr_tag_addr = 0, gcm_decr_tag_addr = 0;
int get_opaque_tag(uint32_t& opaque_tag, bool decr_en, bool is_gcm=false) {
  uint64_t opaque_tag_addr = 0;
  if(!decr_en && !is_gcm) opaque_tag_addr = xts_encr_tag_addr;
  else if(decr_en && !is_gcm) opaque_tag_addr = xts_decr_tag_addr;
  else if(!decr_en && is_gcm) opaque_tag_addr = gcm_encr_tag_addr;
  else if(decr_en && is_gcm) opaque_tag_addr = gcm_decr_tag_addr;

  if(!opaque_tag_addr) {
    if(hal_if::get_xts_opaque_tag_addr(decr_en, &opaque_tag_addr, is_gcm)) {
      printf("get_xts_opaque_tag_addr failed \n");
      return -1;
    }
    if(!decr_en && !is_gcm) xts_encr_tag_addr = opaque_tag_addr;
    else if(decr_en && !is_gcm) xts_decr_tag_addr = opaque_tag_addr;
    else if(!decr_en && is_gcm) gcm_encr_tag_addr = opaque_tag_addr;
    else if(decr_en && is_gcm) gcm_decr_tag_addr = opaque_tag_addr;
  }

  if(!read_mem(opaque_tag_addr, (uint8_t*)&opaque_tag, sizeof(opaque_tag))) {
    printf("Reading opaque tag hbm mem failed \n");
    return -1;
  }
  return 0;
}

int xts_multi_blk_noc_stress_hw_daisy_chain(bool is_hbm_buf=false) {
  int rv = 0;
  uint64_t in_buffer, out_buffer;
  if(is_hbm_buf) {
    rv = utils::hbm_addr_alloc(kBufSize, &in_buffer);
    if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &out_buffer);
  } else {
    in_buffer = (uint64_t)alloc_host_mem(kBufSize);
    out_buffer = (uint64_t)alloc_host_mem(kBufSize);
  }
  uint64_t stg1_buffer, stg2_buffer, stg3_buffer;
  if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &stg1_buffer);
  if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &stg2_buffer);
  if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &stg3_buffer);
  if(0 != rv) {
    printf("HBM memory allocation failed for buffers\n");
    return rv;
  }

  XtsCtx *ctx1 = new XtsCtx[kTotalReqs];
  XtsCtx *ctx2 = new XtsCtx[kTotalReqs];
  XtsCtx *ctx3 = new XtsCtx[kTotalReqs];
  XtsCtx *ctx4 = new XtsCtx[kTotalReqs];
  fill_ctx(ctx1, ctx2, ctx3, ctx4, in_buffer, stg1_buffer, stg2_buffer, stg3_buffer, out_buffer, is_hbm_buf);
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    ctx1[i].opa_tag_en = false;
    ctx2[i].opa_tag_en = false;
    ctx3[i].opa_tag_en = false;
  }

  uint32_t xts_encr_tag_addr = 0, xts_decr_tag_addr = 0, gcm_encr_tag_addr = 0;
  read_reg(CAPRI_BARCO_MD_HENS_REG_XTS0_OPA_TAG_W0_ADDR, xts_encr_tag_addr);
  read_reg(CAPRI_BARCO_MD_HENS_REG_XTS1_OPA_TAG_W0_ADDR, xts_decr_tag_addr);
  read_reg(CAPRI_BARCO_MD_HENS_REG_GCM0_OPA_TAG_W0_ADDR, gcm_encr_tag_addr);

  write_reg(CAPRI_BARCO_MD_HENS_REG_XTS0_OPA_TAG_W0_ADDR, CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX);
  write_reg(CAPRI_BARCO_MD_HENS_REG_XTS0_OPA_TAG_W1_ADDR, 0);
  write_reg(CAPRI_BARCO_MD_HENS_REG_XTS1_OPA_TAG_W0_ADDR, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX);
  write_reg(CAPRI_BARCO_MD_HENS_REG_XTS1_OPA_TAG_W1_ADDR, 0);
  write_reg(CAPRI_BARCO_MD_HENS_REG_GCM0_OPA_TAG_W0_ADDR, CAPRI_BARCO_MD_HENS_REG_GCM1_PRODUCER_IDX);
  write_reg(CAPRI_BARCO_MD_HENS_REG_GCM0_OPA_TAG_W1_ADDR, 0);
  int iter = 1;
  pi_inited = false;
  //Queue initial set of requests
  uint32_t pindex = 0;
  read_reg(CAPRI_BARCO_MD_HENS_REG_XTS1_PRODUCER_IDX, pindex);
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(!((i+1) % kBatchSize)) {
      ctx1[i].opaque_tag = (pindex+i+1) % (kXtsQueueSize-1);
      ctx1[i].opa_tag_en = true;
    }
    if(0 == rv) rv = ctx1[i].test_seq_xts();
  }
  pi_inited = false;
  read_reg(CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX, pindex);
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(!((i+1) % kBatchSize)) {
      ctx2[i].opaque_tag = (pindex+i+1) % (kXtsQueueSize-1);
      ctx2[i].opa_tag_en = true;
    }
    if(0 == rv) rv = ctx2[i].test_seq_xts();
  }
  pi_inited = false;
  read_reg(CAPRI_BARCO_MD_HENS_REG_GCM1_PRODUCER_IDX, pindex);
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(!((i+1) % kBatchSize)) {
      ctx3[i].opaque_tag = (pindex+i+1) % (kXtsQueueSize-1);
      ctx3[i].opa_tag_en = true;
    }
    if(0 == rv) rv = ctx3[i].test_seq_xts();
  }
  pi_inited = false;
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    ctx4[i].auth_tag_addr = ctx3[i].auth_tag_addr;
    if(0 == rv) rv = ctx4[i].test_seq_xts();
  }
  pi_inited = false;
  if(0 != rv) goto done;

  testcase_begin(tcid, iter);
  rv = ring_doorbell(ctx1[0].xts_ring_pi_addr, kTotalReqs);
  if(0 != rv) goto done;

  rv = verify_opaque_tag(kTotalReqs, true, FLAGS_long_poll_interval, true);
  if(0 != rv) {
    goto done;
  }
  testcase_begin(tcid, iter);

  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(is_hbm_buf) rv = e2e_verify_hbm_buf(ctx1[i], ctx4[i]);
    else rv = e2e_verify(ctx1[i], ctx4[i]);
    if(0 != rv) goto done;
  }

done:
  delete[] ctx1;
  delete[] ctx2;
  delete[] ctx3;
  delete[] ctx4;
  write_reg(CAPRI_BARCO_MD_HENS_REG_XTS0_OPA_TAG_W0_ADDR, xts_encr_tag_addr);
  write_reg(CAPRI_BARCO_MD_HENS_REG_XTS1_OPA_TAG_W0_ADDR, xts_decr_tag_addr);
  write_reg(CAPRI_BARCO_MD_HENS_REG_GCM0_OPA_TAG_W0_ADDR, gcm_encr_tag_addr);
  if(!is_hbm_buf) {
    free_host_mem((void*)in_buffer);
    free_host_mem((void*)out_buffer);
  }

  return rv;
}

int xts_multi_blk_noc_stress(bool is_hbm_buf=false) {
  int rv = 0;
  uint64_t in_buffer, out_buffer;
  if(is_hbm_buf) {
    rv = utils::hbm_addr_alloc(kBufSize, &in_buffer);
    if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &out_buffer);
  } else {
    in_buffer = (uint64_t)alloc_host_mem(kBufSize);
    out_buffer = (uint64_t)alloc_host_mem(kBufSize);
  }
  uint64_t stg1_buffer, stg2_buffer, stg3_buffer;
  if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &stg1_buffer);
  if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &stg2_buffer);
  if(0 == rv) rv = utils::hbm_addr_alloc(kBufSize, &stg3_buffer);
  if(0 != rv) {
    printf("HBM memory allocation failed for buffers\n");
    return rv;
  }

  XtsCtx *ctx1 = new XtsCtx[kTotalReqs];
  XtsCtx *ctx2 = new XtsCtx[kTotalReqs];
  XtsCtx *ctx3 = new XtsCtx[kTotalReqs];
  XtsCtx *ctx4 = new XtsCtx[kTotalReqs];
  fill_ctx(ctx1, ctx2, ctx3, ctx4, in_buffer, stg1_buffer, stg2_buffer, stg3_buffer, out_buffer, is_hbm_buf);

  uint32_t encr_reqs_comp = 0, decr_reqs_comp = 0;
  uint32_t gcm_encr_reqs_comp = 0, gcm_decr_reqs_comp = 0;
  uint32_t queued_decr_reqs = 0, queued_gcm_encr_reqs = 0, queued_gcm_decr_reqs = 0;

  int iter = 1;
  pi_inited = false;
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(0 == rv) rv = ctx1[i].test_seq_xts();
  }
  pi_inited = false;
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(0 == rv) rv = ctx2[i].test_seq_xts();
  }
  pi_inited = false;
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(0 == rv) rv = ctx3[i].test_seq_xts();
  }
  pi_inited = false;
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    ctx4[i].auth_tag_addr = ctx3[i].auth_tag_addr;
    if(0 == rv) rv = ctx4[i].test_seq_xts();
  }
  if(0 != rv) goto done;

  testcase_begin(tcid, iter);
  rv = ring_doorbell(ctx1[0].xts_ring_pi_addr, kTotalReqs);
  if(0 != rv) goto done;

  while(1) {
    //xts encryption
    rv = get_opaque_tag(encr_reqs_comp, false);
    if(0 != rv) goto done;

    //Trigger next batch size of decr
    if((encr_reqs_comp - decr_reqs_comp) >= kBatchSize &&
       (encr_reqs_comp - queued_decr_reqs) >= kBatchSize) {
      rv = ring_doorbell(ctx2[0].xts_ring_pi_addr, kBatchSize);
      if(0 != rv) goto done;
      queued_decr_reqs += kBatchSize;
    }

    //xts decryption
    rv = get_opaque_tag(decr_reqs_comp, true);
    if(0 != rv) goto done;

    //Trigger next batch size of gcm encr
    if((decr_reqs_comp - gcm_encr_reqs_comp) >= kBatchSize &&
       (decr_reqs_comp - queued_gcm_encr_reqs) >= kBatchSize) {
      rv = ring_doorbell(ctx3[0].xts_ring_pi_addr, kBatchSize);
      if(0 != rv) goto done;
      queued_gcm_encr_reqs += kBatchSize;
    }

    //gcm encryption
    rv = get_opaque_tag(gcm_encr_reqs_comp, false, true);
    if(0 != rv) goto done;

    //gcm decryption
    rv = get_opaque_tag(gcm_decr_reqs_comp, true, true);
    if(0 != rv) goto done;

    //Trigger next batch size of gcm decr
    if((gcm_encr_reqs_comp - gcm_decr_reqs_comp) >= kBatchSize &&
       (gcm_encr_reqs_comp - queued_gcm_decr_reqs) >= kBatchSize) {
      rv = ring_doorbell(ctx4[0].xts_ring_pi_addr, kBatchSize);
      if(0 != rv) goto done;
      queued_gcm_decr_reqs += kBatchSize;
    }

    if(encr_reqs_comp >= kTotalReqs &&
       decr_reqs_comp >= kTotalReqs &&
       gcm_encr_reqs_comp >= kTotalReqs &&
       queued_gcm_decr_reqs >= kTotalReqs) {
      break;
    }

    usleep(10000);
  }

  rv = verify_opaque_tag(kTotalReqs, true, FLAGS_long_poll_interval, true);
  if(0 != rv) {
    goto done;
  }
  testcase_end(tcid, iter);
  for(uint32_t i = 0; i < kTotalReqs; i++) {
    if(is_hbm_buf) rv = e2e_verify_hbm_buf(ctx1[i], ctx4[i]);
    else rv = e2e_verify(ctx1[i], ctx4[i]);
    if(0 != rv) goto done;
  }

done:
  delete[] ctx1;
  delete[] ctx2;
  delete[] ctx3;
  delete[] ctx4;
  if(!is_hbm_buf) {
    free_host_mem((void*)in_buffer);
    free_host_mem((void*)out_buffer);
  }

  return rv;
}

int xts_multi_blk_noc_stress_from_host_hw_chain() {
  kTotalReqs = 64;
  kInitReqs = 4;
  kBatchSize = 4;
  return xts_multi_blk_noc_stress_hw_daisy_chain();
}

int xts_multi_blk_noc_stress_from_hbm_hw_chain() {
  kTotalReqs = 64;
  kInitReqs = 4;
  kBatchSize = 4;
  return xts_multi_blk_noc_stress_hw_daisy_chain(true);
}


int xts_multi_blk_noc_stress_from_host() {
  kTotalReqs = 64;
  kInitReqs = 4;
  kBatchSize = 4;
  return xts_multi_blk_noc_stress();
}

int xts_multi_blk_noc_stress_from_hbm() {
  kTotalReqs = 64;
  kInitReqs = 4;
  kBatchSize = 4;
  return xts_multi_blk_noc_stress(true);
}


} // namespace tests
