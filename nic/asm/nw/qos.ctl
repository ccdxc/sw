#include "ingress.h"
#include "INGRESS_p.h"

struct qos_k k;
struct qos_d d;
struct phv_  p;

d = {
  qos_d.cos_en= 1;
  qos_d.cos = 3;
  qos_d.dscp_en = 1;
  qos_d.dscp = 32;
  qos_d.egress_tm_oqueue = 2;
};

