#include "../../../p4/apollo2/include/lpm_defines.h"
#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_            p;
struct route_lpm_s0_k  k;
struct route_lpm_s0_d  d;

#define table           d.route_lpm_s0_d
#define key             k.p4_to_txdma_header_lpm_dst
#define next_addr       p.p4_to_txdma_header_lpm_addr
#define s3_offset       p.txdma_control_lpm_s3_offset
#define curr_addr       k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                           p4_to_txdma_header_lpm_addr_sbit2_ebit33}

%%

route_lpm_s0:
    slt        c1,         key,            table.ip7  //if key < table[7]
    bcf        [c1],       lessthan7                  //then goto lessthan7
    slt        c1,         key,            table.ip11 //if key < table[11]
    bcf        [c1],       lessthan11                 //then goto lessthan11
    slt        c1,         key,            table.ip13 //if key < table[13]
    bcf        [c1],       lessthan13                 //then goto lessthan13
    slt        c1,         key,            table.ip14 //if key < table[14]
    bcf        [c1],       lessthan14                 //then goto lessthan14
ge14:
    add        r1,         curr_addr,      LPM_S0_B15_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK15_OFFSET
    // Total 11; 0 branch delay slot waste
lessthan14:
    add        r1,         curr_addr,      LPM_S0_B14_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK14_OFFSET
    // Total 12; 1 branch delay slot waste
lessthan13:
    slt        c1,         key,            table.ip12 //if key < table[12]
    bcf        [c1],       lessthan12                 //then goto lessthan12
    add        r1,         curr_addr,      LPM_S0_B13_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK13_OFFSET
    // Total 12; 1 branch delay slot waste
lessthan12:
    add        r1,         curr_addr,      LPM_S0_B12_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK12_OFFSET
    // Total 13; 2 branch delay slot waste
lessthan11:
    slt        c1,         key,            table.ip9  //if key < table[9]
    bcf        [c1],       lessthan9                  //then goto lessthan9
    slt        c1,         key,            table.ip10 //if key < table[10]
    bcf        [c1],       lessthan10                   //then goto lessthan10
ge10:
    add        r1,         curr_addr,      LPM_S0_B11_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK11_OFFSET
    // Total 12; 1 branch delay slot waste
lessthan10:
    add        r1,         curr_addr,      LPM_S0_B10_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK10_OFFSET
    // Total 13; 2 branch delay slot waste
lessthan9:
    slt        c1,         key,            table.ip8  //if key < table[8]
    bcf        [c1],       lessthan8                  //then goto lessthan8
    add        r1,         curr_addr,      LPM_S0_B9_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK9_OFFSET
    // Total 13; 2 branch delay slot waste
lessthan8:
    add        r1,         curr_addr,      LPM_S0_B8_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK8_OFFSET
    // Total 14; 3 branch delay slot waste
lessthan7:
    slt        c1,         key,            table.ip3  //if key < table[3]
    bcf        [c1],       lessthan3                   //then goto lessthan3
    slt        c1,         key,            table.ip5  //if key < table[5]
    bcf        [c1],       lessthan5                   //then goto lessthan5
    slt        c1,         key,            table.ip6  //if key < table[6]
    bcf        [c1],       lessthan6                   //then goto lessthan6
ge6:
    add        r1,         curr_addr,      LPM_S0_B7_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK7_OFFSET
    // Total 12; 1 branch delay slot waste
lessthan6:
    add        r1,         curr_addr,      LPM_S0_B6_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK6_OFFSET
    // Total 13; 2 branch delay slot waste
lessthan5:
    slt        c1,         key,            table.ip4  //if key < table[4]
    bcf        [c1],       lessthan4                   //then goto lessthan4
    add        r1,         curr_addr,      LPM_S0_B5_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK5_OFFSET
    // Total 13; 2 branch delay slot waste
lessthan4:
    add        r1,         curr_addr,      LPM_S0_B4_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK4_OFFSET
    // Total 14; 3 branch delay slot waste
lessthan3:
    slt        c1,         key,            table.ip1  //if key < table[1]
    bcf        [c1],       lessthan1                  //then goto lessthan1
    slt        c1,         key,            table.ip2  //if key < table[2]
    bcf        [c1],       lessthan2                  //then goto lessthan2
ge2:
    add        r1,         curr_addr,      LPM_S0_B3_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK3_OFFSET
    // Total 13; 2 branch delay slot waste
lessthan2:
    add        r1,         curr_addr,      LPM_S0_B2_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK2_OFFSET
    // Total 14; 3 branch delay slot waste
lessthan1:
    slt        c1,         key,            table.ip0  //if key < table[0]
    bcf        [c1],       lessthan0                  //then goto lessthan0
    add        r1,         curr_addr,      LPM_S0_B1_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK1_OFFSET
    // Total 14; 3 branch delay slot waste
lessthan0:
    add        r1,         curr_addr,      LPM_S0_B0_OFFSET
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  LPM_S2_BLOCK0_OFFSET
    // Total 15; 4 branch delay slot waste

