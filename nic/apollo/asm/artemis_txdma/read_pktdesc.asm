#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct read_pktdesc_k   k;
struct read_pktdesc_d   d;
struct phv_             p;

%%

read_pktdesc:
    phvwr.e.f       p.{rx_to_tx_hdr_remote_ip, \
                        rx_to_tx_hdr_sacl_base_addr, \
                        rx_to_tx_hdr_route_base_addr, \
                        rx_to_tx_hdr_sip_classid, \
                        rx_to_tx_hdr_dip_classid, \
                        rx_to_tx_hdr_tag_classid, \
                        rx_to_tx_hdr_meter_result, \
                        rx_to_tx_hdr_dport_classid, \
                        rx_to_tx_hdr_sport_classid, \
                        rx_to_tx_hdr_tag2_classid, \
                        rx_to_tx_hdr_vnic_id, \
                        rx_to_tx_hdr_vcn_id}, \
                        d[511:(512-(offsetof(p,rx_to_tx_hdr_remote_ip) + \
                            sizeof(p.rx_to_tx_hdr_remote_ip) - \
                            offsetof(p,rx_to_tx_hdr_tag2_classid)))]
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_pktdesc_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
