#include "../../p4/include/artemis_sacl_defines.h"
#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct read_pktdesc_k   k;
struct read_pktdesc_d   d;
struct phv_             p;

%%

read_pktdesc:
    /* Load sacl base addr to r1 */
    add        r1, r0, k.rx_to_tx_hdr_sacl_base_addr
    /* Add SACL_P1_1_TABLE_OFFSET to sacl base address. */
    addi       r1, r1, SACL_P1_1_TABLE_OFFSET
    /* P1 table index = ((sip_classid << 7) | sport_classid). */
    add.c1     r2, d.read_pktdesc_d.sport_classid, d.read_pktdesc_d.sip_classid, 7
    /* Write P1 table index to PHV */
    phvwr      p.txdma_control_rfc_index, r2
    /* Compute the byte offset for P1 table index */
    div        r2, r2, 51
    mul        r2, r2, 64
    /* Add the byte offset to table base */
    add        r1, r1, r2
    /* Write the address back to phv for P1 lookup */
    phvwr      p.txdma_control_rfc_table_addr, r1

    /* Setup for route LPM lookup */
    phvwr      p.txdma_control_lpm1_key[63:0], d.read_pktdesc_d.remote_ip[63:0]
    phvwr      p.txdma_control_lpm1_key[127:64], d.read_pktdesc_d.remote_ip[127:64]
    phvwr      p.txdma_control_lpm1_base_addr, d.read_pktdesc_d.route_base_addr

    /* Initialize rx_to_tx_hdr */
    phvwr.e.f       p.{rx_to_tx_hdr_remote_ip, \
                       rx_to_tx_hdr_sacl_base_addr, \
                       rx_to_tx_hdr_route_base_addr, \
                       rx_to_tx_hdr_meter_result, \
                       rx_to_tx_hdr_sip_classid, \
                       rx_to_tx_hdr_dip_classid, \
                       rx_to_tx_hdr_stag_classid, \
                       rx_to_tx_hdr_dtag_classid, \
                       rx_to_tx_hdr_sport_classid, \
                       rx_to_tx_hdr_dport_classid, \
                       rx_to_tx_hdr_vpc_id, \
                       rx_to_tx_hdr_vnic_id, \
                       rx_to_tx_hdr_payload_len, \
                       rx_to_tx_hdr_iptype, \
                       rx_to_tx_hdr_pad0}, \
                       d[511:(512-(offsetof(p,rx_to_tx_hdr_remote_ip) + \
                            sizeof(p.rx_to_tx_hdr_remote_ip) - \
                            offsetof(p, rx_to_tx_hdr_pad0)))]
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_pktdesc_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
