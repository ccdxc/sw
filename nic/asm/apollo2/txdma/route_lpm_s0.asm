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
    add        r1,         curr_addr,      960        //60 + ((15 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  10620                      //60 + 960 + 15 * 704 - 960
    // Total 11; 0 branch delay slot waste
lessthan14:
    add        r1,         curr_addr,      900        //60 + ((14 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  9976                       //60 + 960 + 14 * 704 - 900
    // Total 12; 1 branch delay slot waste
lessthan13:
    slt        c1,         key,            table.ip12 //if key < table[12]
    bcf        [c1],       lessthan12                 //then goto lessthan12
    add        r1,         curr_addr,      840        //60 + ((13 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  9332                       //60 + 960 + 13 * 704 - 840
    // Total 12; 1 branch delay slot waste
lessthan12:
    add        r1,         curr_addr,      780        //60 + ((12 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  8688                       //60 + 960 + 12 * 704 - 780
    // Total 13; 2 branch delay slot waste
lessthan11:
    slt        c1,         key,            table.ip9  //if key < table[9]
    bcf        [c1],       lessthan9                  //then goto lessthan9
    slt        c1,         key,            table.ip10 //if key < table[10]
    bcf        [c1],       lessthan10                   //then goto lessthan10
ge10:
    add        r1,         curr_addr,      720        //60 + ((11 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,                  8044       //60 + 960 + 11 * 704 - 720
    // Total 12; 1 branch delay slot waste
lessthan10:
    add        r1,         curr_addr,      660        //60 + ((10 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  7400                       //60 + 960 + 10 * 704 - 660
    // Total 13; 2 branch delay slot waste
lessthan9:
    slt        c1,         key,            table.ip8  //if key < table[8]
    bcf        [c1],       lessthan8                  //then goto lessthan8
    add        r1,         curr_addr,      600        //60 + ((9 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  6756                       //60 + 960 + 9 * 704 - 600
    // Total 13; 2 branch delay slot waste
lessthan8:
    add        r1,         curr_addr,      540        //60 + ((8 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  6112                       //60 + 960 + 8 * 704 - 540
    // Total 14; 3 branch delay slot waste
lessthan7:
    slt        c1,         key,            table.ip3  //if key < table[3]
    bcf        [c1],       lessthan3                   //then goto lessthan3
    slt        c1,         key,            table.ip5  //if key < table[5]
    bcf        [c1],       lessthan5                   //then goto lessthan5
    slt        c1,         key,            table.ip6  //if key < table[6]
    bcf        [c1],       lessthan6                   //then goto lessthan6
ge6:
    add        r1,         curr_addr,      480        //60 + ((7 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  5468                       //60 + 960 + 7 * 704 - 480
    // Total 12; 1 branch delay slot waste
lessthan6:
    add        r1,         curr_addr,      420        //60 + ((6 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  4824                       //60 + 960 + 6 * 704 - 420
    // Total 13; 2 branch delay slot waste
lessthan5:
    slt        c1,         key,            table.ip4  //if key < table[4]
    bcf        [c1],       lessthan4                   //then goto lessthan4
    add        r1,         curr_addr,      360        //60 + ((5 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  4180                       //60 + 960 + 5 * 704 - 360
    // Total 13; 2 branch delay slot waste
lessthan4:
    add        r1,         curr_addr,      300        //60 + ((4 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  3536                       //60 + 960 + 4 * 704 - 300
    // Total 14; 3 branch delay slot waste
lessthan3:
    slt        c1,         key,            table.ip1  //if key < table[1]
    bcf        [c1],       lessthan1                  //then goto lessthan1
    slt        c1,         key,            table.ip2  //if key < table[2]
    bcf        [c1],       lessthan2                  //then goto lessthan2
ge2:
    add        r1,         curr_addr,      240        //60 + ((3 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  2892                       //60 + 960 + 3 * 704 - 240
    // Total 13; 2 branch delay slot waste
lessthan2:
    add        r1,         curr_addr,      180        //60 + ((2 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr      s3_offset,  2248                       //60 + 960 + 2 * 704 - 180
    // Total 14; 3 branch delay slot waste
lessthan1:
    slt        c1,         key,            table.ip0  //if key < table[0]
    bcf        [c1],       lessthan0                  //then goto lessthan0
    add        r1,         curr_addr,      120        //60 + ((1 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr    s3_offset,    1604                       //60 + 960 + 1 * 704 - 120
    // Total 14; 3 branch delay slot waste
lessthan0:
    add        r1,         curr_addr,      60         //60 + ((0 * 15) * 4)
    phvwr.e    next_addr,  r1                         //next_addr = curr_addr + r1
    phvwr    s3_offset,    960                        //60 + 960 + 0 * 704 - 60
    // Total 15; 4 branch delay slot waste

