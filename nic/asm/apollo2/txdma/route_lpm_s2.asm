#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_            p;
struct route_lpm_s2_k  k;
struct route_lpm_s2_d  d;

#define prog_name      route_lpm_s2
#define key            k.p4_to_txdma_header_lpm_dst
#define keys(a)        d.route_lpm_s2_d.key ## a
#define data(a)        d.route_lpm_s2_d.data ## a
#define result         p.txdma_to_p4e_header_nexthop_index

#define LPM_KEY_SIZE   4
#define LPM_DATA_SIZE  2

#include "../include/lpm2.h"
