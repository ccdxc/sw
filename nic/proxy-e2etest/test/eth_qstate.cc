
#include <zmq.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "nic/model_sim/include/buf_hdr.h"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/proxy-e2etest/lib_driver.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <qstate_base>\n", argv[0]);
    return -1;
  }

  lib_model_connect();

  uint64_t qstate_addr = strtoul(argv[1], NULL, 0);

  // read qstate
  struct qstate qstate, *ptr = &qstate;

  read_mem(qstate_addr, (uint8_t *) &qstate, sizeof(qstate));
  printf("pc_offset=0x%0x\n"
         "rsvd0=0x%0x\n"
         "cosA=0x%0x\ncosB=0x%0x\ncos_sel=0x%0x\n"
         "eval_last=0x%0x\n"
         "host=0x%0x\ntotal=0x%0x\n"
         "pid=0x%0x\n"
         "enable=0x%0x\nring_base=0x%0lx\nring_size=0x%0x\n"
         "p_index=0x%0x\nc_index=0x%0x\n"
         "cq_ring_base=0x%0lx\n",
         ptr->pc_offset,
         ptr->rsvd0,
         ptr->cosA, ptr->cosB, ptr->cos_sel,
         ptr->eval_last,
         ptr->host, ptr->total,
         ptr->pid,
         ptr->enable, ptr->ring_base, ptr->ring_size,
         ptr->p_index0, ptr->c_index0,
         ptr->cq_ring_base);

  lib_model_conn_close();

  return 0;
}
