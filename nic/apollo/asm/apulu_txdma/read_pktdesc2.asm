#include "../../p4/include/apulu_sacl_defines.h"
#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct read_pktdesc2_k   k;
struct read_pktdesc2_d   d;
struct phv_              p;

%%

read_pktdesc2:
    /* Initialize rx_to_tx_hdr */
    phvwr.e         p.{rx_to_tx_hdr_sacl_base_addr4, \
                       rx_to_tx_hdr_sip_classid4, \
                       rx_to_tx_hdr_dip_classid4, \
                       rx_to_tx_hdr_pad4, \
                       rx_to_tx_hdr_sport_classid4, \
                       rx_to_tx_hdr_dport_classid4, \
                       rx_to_tx_hdr_sacl_base_addr5, \
                       rx_to_tx_hdr_sip_classid5, \
                       rx_to_tx_hdr_dip_classid5, \
                       rx_to_tx_hdr_pad5, \
                       rx_to_tx_hdr_sport_classid5, \
                       rx_to_tx_hdr_dport_classid5, \
                       rx_to_tx_hdr_vpc_id, \
                       rx_to_tx_hdr_vnic_id, \
                       rx_to_tx_hdr_iptype, \
                       rx_to_tx_hdr_rx_packet, \
                       rx_to_tx_hdr_payload_len, \
                       rx_to_tx_hdr_dtag_classid, \
                       rx_to_tx_hdr_stag_classid, \
                       rx_to_tx_hdr_pad6, \
                       rx_to_tx_hdr_local_tag_idx, \
                       rx_to_tx_hdr_remote_tag_idx, \
                       rx_to_tx_hdr_pad8}, \
                       d[511:(512-(offsetof(p,rx_to_tx_hdr_sacl_base_addr4) + \
                            sizeof(p.rx_to_tx_hdr_sacl_base_addr4) - \
                            offsetof(p, rx_to_tx_hdr_pad8)))]
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
read_pktdesc2_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
