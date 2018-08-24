#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_            p;
struct route_lpm_s2_k  k;
struct route_lpm_s2_d  d;

#define table           d.route_lpm_s2_d
#define key             k.p4_to_txdma_header_lpm_dst
#define nh_idx          p.txdma_to_p4e_header_nexthop_index

%%

route_lpm_s2:
    slt         c1,         key,            table.ip3  //if key < table[3]
    bcf         [c1],       lessthan3                  //then goto lessthan3
    slt         c1,         key,            table.ip5  //if key < table[5]
    bcf         [c1],       lessthan5                  //then goto lessthan5
    slt         c1,         key,            table.ip6  //if key < table[6]
    phvwr.c1.e  nh_idx,     table.nh5                  //then phv.nh_idx = nh5
    phvwr.!c1.e nh_idx,     table.nh6                  //else phv.nh_idx = nh6
    nop
    // Total 8; 0 branch delay slot waste
lessthan5:
    slt         c1,         key,            table.ip4  //if key < table[4]
    phvwr.c1.e  nh_idx,     table.nh3                  //then phv.nh_idx = nh3
    phvwr.!c1.e nh_idx,     table.nh4                  //else phv.nh_idx = nh4
    nop
    // Total 9; 1 branch delay slot waste
lessthan3:
    slt         c1,         key,            table.ip1  //if key < table[1]
    bcf         [c1],       lessthan1                  //then goto lessthan1
    slt         c1,         key,            table.ip2  //if key < table[2]
    phvwr.c1.e  nh_idx,     table.nh1                  //then phv.nh_idx = nh1
    phvwr.!c1.e nh_idx,     table.nh2                  //else phv.nh_idx = nh2
    nop
    // Total 9; 1 branch delay slot waste
lessthan1:
    slt         c1,         key,            table.ip0  //if key < table[0]
    phvwr.c1.e  nh_idx,     table.nh_                  //then phv.nh_idx = nh_
    phvwr.!c1.e nh_idx,     table.nh0                  //else phv.nh_idx = nh0
    nop
    // Total 10; 2 branch delay slot waste

