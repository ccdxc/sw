#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct route_ipv6_data_k   k;
struct route_ipv6_data_d   d;

#define prog_name          search_retrieve
#define key                k.p4_to_txdma_header_lpm_dst
#define keys(a)            d.search_retrieve_d.key ## a
#define data(a)            d.search_retrieve_d.data ## a
#define result             p.txdma_to_p4e_header_nexthop_index

#include "../include/lpm64b_data.h"
