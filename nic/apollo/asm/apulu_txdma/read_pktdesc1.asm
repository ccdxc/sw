#include "../../p4/include/apulu_sacl_defines.h"
#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct read_pktdesc1_k   k;
struct read_pktdesc1_d   d;
struct phv_              p;

%%

read_pktdesc1:
    /* Initialize rx_to_tx_hdr */
    phvwr         p.{rx_to_tx_hdr_remote_ip, \
                     rx_to_tx_hdr_route_base_addr, \
                     rx_to_tx_hdr_sacl_base_addr0, \
                     rx_to_tx_hdr_sip_classid0, \
                     rx_to_tx_hdr_dip_classid0, \
                     rx_to_tx_hdr_pad0, \
                     rx_to_tx_hdr_sport_classid0, \
                     rx_to_tx_hdr_dport_classid0, \
                     rx_to_tx_hdr_sacl_base_addr1, \
                     rx_to_tx_hdr_sip_classid1, \
                     rx_to_tx_hdr_dip_classid1, \
                     rx_to_tx_hdr_pad1, \
                     rx_to_tx_hdr_sport_classid1, \
                     rx_to_tx_hdr_dport_classid1, \
                     rx_to_tx_hdr_sacl_base_addr2, \
                     rx_to_tx_hdr_sip_classid2, \
                     rx_to_tx_hdr_dip_classid2, \
                     rx_to_tx_hdr_pad2, \
                     rx_to_tx_hdr_sport_classid2, \
                     rx_to_tx_hdr_dport_classid2, \
                     rx_to_tx_hdr_sacl_base_addr3, \
                     rx_to_tx_hdr_sip_classid3, \
                     rx_to_tx_hdr_dip_classid3, \
                     rx_to_tx_hdr_pad3, \
                     rx_to_tx_hdr_sport_classid3, \
                     rx_to_tx_hdr_dport_classid3, \
                     rx_to_tx_hdr_pad7}, \
                     d[511:(512-(offsetof(p,rx_to_tx_hdr_remote_ip) + \
                          sizeof(p.rx_to_tx_hdr_remote_ip) - \
                          offsetof(p, rx_to_tx_hdr_pad7)))]

    /* Setup for route LPM lookup */
    sne        c1, d.read_pktdesc1_d.route_base_addr, r0
    phvwr.c1   p.txdma_control_lpm1_key[127:64], d.read_pktdesc1_d.remote_ip[127:64]
    phvwr.c1   p.txdma_control_lpm1_key[63:0], d.read_pktdesc1_d.remote_ip[63:0]
    phvwr.c1   p.txdma_control_lpm1_base_addr, d.read_pktdesc1_d.route_base_addr
    phvwr.c1   p.txdma_predicate_lpm1_enable, TRUE

    /* Load sacl base addr to r1 */
    add        r1, r0, d.read_pktdesc1_d.sacl_base_addr0

    /* Stop if sacl base addr0 == NULL */
    seq        c1, r1, r0
    nop.c1.e

    /* Add SACL_P1_1_TABLE_OFFSET to sacl base address. */
    addi       r1, r1, SACL_P1_1_TABLE_OFFSET
    /* P1 table index = (sport_classid0 | (sip_classid0 << 7)). */
    add        r2, d.read_pktdesc1_d.sport_classid0, d.read_pktdesc1_d.sip_classid0, \
                                                   SACL_SPORT_CLASSID_WIDTH
    /* Write P1 table index to PHV */
    phvwr      p.txdma_control_rfc_index, r2
    /* Compute the byte offset for P1 table index */
    div        r2, r2, SACL_P1_ENTRIES_PER_CACHE_LINE
    mul        r2, r2, SACL_CACHE_LINE_SIZE
    /* Add the byte offset to table base */
    add        r1, r1, r2
    /* Write the address back to phv for P1 lookup */
    phvwr      p.txdma_control_rfc_table_addr, r1
    /* Enable RFC lookups */
    phvwr.e    p.txdma_predicate_rfc_enable, TRUE
    /* Initialize rule priority to invalid */
    phvwr      p.txdma_control_rule_priority, SACL_PRIORITY_INVALID


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_pktdesc1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
