#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_            p;
struct route_lpm_s1_k  k;
struct route_lpm_s1_d  d;

#define prog_name      route_lpm_s1
#define keys(a)        d.route_lpm_s1_d.key ## a
#define key            k.p4_to_txdma_header_lpm_dst
#define next_addr      p.p4_to_txdma_header_lpm_addr
#define s2_offset      k.txdma_control_lpm_s2_offset
#define curr_addr      k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                          p4_to_txdma_header_lpm_addr_sbit2_ebit33}

#define LPM_KEY_SIZE  4
#define LPM_DATA_SIZE 2

#include "../include/lpm1.h"
