#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct p4plus_to_p4_1_k k;
struct phv_             p;

k = {
  p4plus_to_p4_flags = (P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID |
                        P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO);
  p4plus_to_p4_ip_id_delta = 10;
  p4plus_to_p4_tcp_seq_delta = 100;
};
