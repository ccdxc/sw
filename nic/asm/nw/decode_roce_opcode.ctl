#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct decode_roce_opcode_k k;
struct decode_roce_opcode_d d;
struct phv_                 p;

d = {
  u.decode_roce_opcode_d.raw_flags = 0x155;
  u.decode_roce_opcode_d.len = 0x34;
};

k = {
  capri_intrinsic_tm_oport = TM_PORT_DMA;
  roce_bth_destQP = 0x5555;
};
