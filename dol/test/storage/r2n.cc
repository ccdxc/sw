#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include <byteswap.h>
#include <stddef.h>
#include "dol/test/storage/utils.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "dol/test/storage/r2n.hpp"

namespace r2n {

void *r2n_buf_alloc() {
  return alloc_host_mem(sizeof(r2n_buf_t));
}

void r2n_nvme_be_cmd_init(void *r2n_buf, uint32_t src_queue_id, uint16_t ssd_handle,
                          uint8_t io_priority, uint8_t is_read, uint8_t is_local) {
  if (!r2n_buf) return;
  nvme_be_cmd_t *nvme_be_cmd = (nvme_be_cmd_t *) &(((r2n_buf_t *) r2n_buf)->cmd_buf);
  nvme_be_cmd->s.src_queue_id = bswap_32(src_queue_id);
  nvme_be_cmd->s.ssd_handle = bswap_16(ssd_handle);
  nvme_be_cmd->s.r2n_buf_handle = bswap_64(host_mem_v2p(r2n_buf));
  nvme_be_cmd->s.io_priority = io_priority;
  nvme_be_cmd->s.is_read = is_read;
  nvme_be_cmd->s.is_local = is_local;
}

uint8_t *r2n_nvme_cmd_ptr(void *r2n_buf) {
  if (!r2n_buf) return nullptr;
  nvme_be_cmd_t *nvme_be_cmd = (nvme_be_cmd_t *) &(((r2n_buf_t *) r2n_buf)->cmd_buf);
  return((uint8_t *) &nvme_be_cmd->s.nvme_cmd); 
}
void r2n_wqe_init(void *r2n_wqe_buf, void *r2n_buf, uint16_t opcode) {
  if (!r2n_wqe_buf) return;
  r2n_wqe_t *r2n_wqe = (r2n_wqe_t *) r2n_wqe_buf;
  bzero(r2n_wqe_buf, sizeof(*r2n_wqe));
  r2n_wqe->handle = bswap_64(host_mem_v2p((void *)&(((r2n_buf_t *) r2n_buf)->cmd_buf)));
  uint32_t size = sizeof(r2n_buf_t) - offsetof(r2n_buf_t, cmd_buf);
  r2n_wqe->data_size = bswap_32(size);
  r2n_wqe->opcode = bswap_16(opcode);

  // is_remote, db_enable fields are initialized to their default values with the 
  // bzero above
}

void r2n_wqe_db_update(void *r2n_wqe_buf, uint16_t lif, uint8_t qtype, 
                       uint32_t qid, uint16_t index) {
  if (!r2n_wqe_buf) return;
  
  utils::write_bit_fields(r2n_wqe_buf, 128, 2, 1);
  utils::write_bit_fields(r2n_wqe_buf, 130, 11, lif);
  utils::write_bit_fields(r2n_wqe_buf, 141, 3, qtype);
  utils::write_bit_fields(r2n_wqe_buf, 144, 24, qid);
  utils::write_bit_fields(r2n_wqe_buf, 168, 16, index);
  
}

}  // namespace r2n
