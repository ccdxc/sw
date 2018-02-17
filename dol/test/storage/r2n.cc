#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include <byteswap.h>
#include <stddef.h>
#include "dol/test/storage/utils.hpp"
#include "nic/utils/host_mem/c_if.h"
#include "dol/test/storage/r2n.hpp"

namespace r2n {

dp_mem_t *r2n_buf_alloc() {
  return new dp_mem_t(1, sizeof(r2n_buf_t));
}

void r2n_nvme_be_cmd_buf_init(dp_mem_t *nvme_cmd_buf, dp_mem_t *r2n_buf,
                              uint32_t src_queue_id, uint16_t ssd_handle, 
                              uint8_t io_priority, uint8_t is_read, 
                              uint8_t is_local, dp_mem_t **nvme_cmd_ptr) {
  if (!nvme_cmd_buf) return;
  nvme_be_cmd_t *nvme_be_cmd = (nvme_be_cmd_t *)nvme_cmd_buf->read();
  if (r2n_buf) {
    nvme_be_cmd->s.r2n_buf_handle = bswap_64(r2n_buf->pa());
  } else {
    nvme_be_cmd->s.r2n_buf_handle = 0;
  }
  nvme_be_cmd->s.src_queue_id = bswap_32(src_queue_id);
  nvme_be_cmd->s.ssd_handle = bswap_16(ssd_handle);
  nvme_be_cmd->s.io_priority = io_priority;
  nvme_be_cmd->s.is_read = is_read;
  nvme_be_cmd->s.is_local = is_local;
  if (nvme_cmd_ptr) {
    *nvme_cmd_ptr = nvme_cmd_buf->fragment_find(offsetof(nvme_be_cmd_t, s.nvme_cmd),
                                                sizeof(struct NvmeCmd));
  }
  nvme_cmd_buf->write_thru();
}

void r2n_nvme_be_cmd_init(dp_mem_t *r2n_buf, uint32_t src_queue_id, uint16_t ssd_handle,
                          uint8_t io_priority, uint8_t is_read, uint8_t is_local) {
  dp_mem_t *nvme_be_cmd;

  if (!r2n_buf) return;

  nvme_be_cmd = r2n_buf->fragment_find(offsetof(r2n_buf_t, cmd_buf), sizeof(nvme_be_cmd_t));
  r2n_nvme_be_cmd_buf_init(nvme_be_cmd, r2n_buf,
                           src_queue_id, ssd_handle, io_priority, is_read, is_local,
                           NULL);
}

dp_mem_t *r2n_nvme_cmd_ptr(dp_mem_t *r2n_buf) {
 dp_mem_t *nvme_be_cmd;
 dp_mem_t *nvme_cmd;

  if (!r2n_buf) return nullptr;

  nvme_be_cmd = r2n_buf->fragment_find(offsetof(r2n_buf_t, cmd_buf),
                                       sizeof(nvme_be_cmd_t));
  nvme_cmd = nvme_be_cmd->fragment_find(offsetof(nvme_be_cmd_t, s.nvme_cmd),
                                        sizeof(struct NvmeCmd));
  return nvme_cmd;
}

void r2n_wqe_init(dp_mem_t *r2n_wqe_buf, dp_mem_t *r2n_buf, uint16_t opcode) {
  dp_mem_t *cmd_buf;

  if (!r2n_wqe_buf) return;
  r2n_wqe_t *r2n_wqe = (r2n_wqe_t *) r2n_wqe_buf->read();
  bzero(r2n_wqe, sizeof(*r2n_wqe));

  cmd_buf = r2n_buf->fragment_find(offsetof(r2n_buf_t, cmd_buf),
                                   sizeof(nvme_be_cmd_t));
  r2n_wqe->handle = bswap_64(cmd_buf->pa());
  uint32_t size = sizeof(r2n_buf_t) - offsetof(r2n_buf_t, cmd_buf);
  r2n_wqe->data_size = bswap_32(size);
  r2n_wqe->opcode = bswap_16(opcode);

  // is_remote, db_enable fields are initialized to their default values with the 
  // bzero above
  r2n_wqe_buf->write_thru();
}

void r2n_wqe_db_update(dp_mem_t *r2n_wqe_buf, uint16_t lif, uint8_t qtype, 
                       uint32_t qid, uint16_t index) {
  if (!r2n_wqe_buf) return;
  
  r2n_wqe_buf->write_bit_fields(128, 2, 1);
  r2n_wqe_buf->write_bit_fields(130, 11, lif);
  r2n_wqe_buf->write_bit_fields(141, 3, qtype);
  r2n_wqe_buf->write_bit_fields(144, 24, qid);
  r2n_wqe_buf->write_bit_fields(168, 16, index);
  r2n_wqe_buf->write_thru();
  
}

}  // namespace r2n
