#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_sacl_sport_lpm_s0_k.h"
#include "../../p4/include/sacl_defines.h"

struct phv_                 p;
struct sacl_sport_lpm_s0_k_ k;
struct sacl_sport_lpm_s0_d  d;

#define keys(a)                 d.sacl_sport_lpm_s0_d.key ## a
#define key                     k.p4_to_rxdma_header_flow_sport
#define sacl_sport_base_addr    k.p4_to_rxdma_header_sacl_base_addr

%%

sacl_sport_lpm_s0:
    seq             c1, k.p4_to_rxdma_header_direction, TX_FROM_HOST
    phvwr.c1        p.sacl_metadata_ip[127:112], \
                        k.p4_to_rxdma_header_flow_dst_s0_e15
    phvwr.c1        p.sacl_metadata_ip[111:0], \
                        k.p4_to_rxdma_header_flow_dst_s16_e127
    phvwr.!c1       p.sacl_metadata_ip, k.p4_to_rxdma_header_flow_src
    add             r1, k.p4_to_rxdma_header_flow_dport, \
                        k.p4_to_rxdma_header_flow_proto, 24
    phvwr           p.sacl_metadata_proto_dport, r1
    add             r1, r0, sacl_sport_base_addr
    add             r2, r1, SACL_IP_TABLE_OFFSET
    phvwr           p.sacl_metadata_ip_table_addr, r2
    add             r2, r1, SACL_PROTO_DPORT_TABLE_OFFSET
    phvwr           p.sacl_metadata_proto_dport_table_addr, r2

    slt             c1, key, keys(15)
    bcf             [c1], lessthan15
    slt             c1, key, keys(23)
    bcf             [c1], lessthan23
    slt             c1, key, keys(27)
    bcf             [c1], lessthan27
    slt             c1, key, keys(29)
    bcf             [c1], lessthan29
    slt             c1, key, keys(30)
    bcf             [c1], lessthan30
ge30:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B31_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan30:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B30_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan29:
    slt             c1, key, keys(28)
    bcf             [c1], lessthan28
ge28:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B29_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan28:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B28_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan27:
    slt             c1, key, keys(25)
    bcf             [c1], lessthan25
    slt             c1, key, keys(26)
    bcf             [c1], lessthan26
ge27:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B27_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan26:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B26_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan25:
    slt             c1, key, keys(24)
    bcf             [c1], lessthan24
ge24:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B25_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan24:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B24_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan23:
    slt             c1, key, keys(19)
    bcf             [c1], lessthan19
    slt             c1, key, keys(21)
    bcf             [c1], lessthan21
    slt             c1, key, keys(22)
    bcf             [c1], lessthan22
ge22:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B23_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan22:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B22_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan21:
    slt             c1, key, keys(20)
    bcf             [c1], lessthan20
ge20:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B21_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan20:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B20_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan19:
    slt             c1, key, keys(17)
    bcf             [c1], lessthan17
    slt             c1, key, keys(18)
    bcf             [c1], lessthan18
ge18:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B19_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan18:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B18_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan17:
    slt             c1, key, keys(16)
    bcf             [c1], lessthan16
ge16:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B17_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan16:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B16_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan15:
    slt             c1, key, keys(7)
    bcf             [c1], lessthan7
    slt             c1, key, keys(11)
    bcf             [c1], lessthan11
    slt             c1, key, keys(13)
    bcf             [c1], lessthan13
    slt             c1, key, keys(14)
    bcf             [c1], lessthan14
ge14:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B15_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan14:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B14_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan13:
    slt             c1, key, keys(12)
    bcf             [c1], lessthan12
ge12:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B13_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan12:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B12_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan11:
    slt             c1, key, keys(9)
    bcf             [c1], lessthan9
    slt             c1, key, keys(10)
    bcf             [c1], lessthan10
ge10:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B11_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan10:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B10_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan9:
    slt             c1, key, keys(8)
    bcf             [c1], lessthan8
ge8:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B9_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan8:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B8_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan7:
    slt             c1, key, keys(3)
    bcf             [c1], lessthan3
    slt             c1, key, keys(5)
    bcf             [c1], lessthan5
    slt             c1, key, keys(6)
    bcf             [c1], lessthan6
ge6:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B7_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan6:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B6_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan5:
    slt             c1, key, keys(4)
    bcf             [c1], lessthan4
ge4:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B5_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan4:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B4_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan3:
    slt             c1, key, keys(1)
    bcf             [c1], lessthan1
    slt             c1, key, keys(2)
    bcf             [c1], lessthan2
ge2:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B3_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan2:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B2_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan1:
    slt             c1, key, keys(0)
    bcf             [c1], lessthan0
ge0:
    add.!c1.e       r1, sacl_sport_base_addr, SACL_SPORT_S1_B1_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

lessthan0:
    add.e           r1, sacl_sport_base_addr, SACL_SPORT_S1_B0_OFFSET
    phvwr           p.sacl_metadata_sport_table_addr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
sacl_sport_lpm_s0_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
