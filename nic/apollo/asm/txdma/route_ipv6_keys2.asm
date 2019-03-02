#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct route_ipv6_keys2_k  k;
struct route_ipv6_keys2_d  d;

#define prog_name          search2
#define keys(a)            d.search2_d.key ## a
#define key                k.p4_to_txdma_header_lpm_dst
#define base_addr          k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                              p4_to_txdma_header_lpm_addr_sbit2_ebit33}
#define curr_addr          k.{txdma_control_lpm_addr_sbit0_ebit1...\
                              txdma_control_lpm_addr_sbit2_ebit33}
#define next_addr          p.txdma_control_lpm_addr

#include "../include/lpm64b_keys.h"
