#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_iflow_rx2tx_k.h"

struct iflow_rx2tx_k_       k;
struct iflow_rx2tx_d        d;
struct phv_                 p;

#define FLOW_IPV4_ENTRY_SHIFT 5
#define FLOW_IPV6_ENTRY_SHIFT 6

%%
    .param          flow
    .param          flow_ohash
    .param          ipv4_flow
    .param          ipv4_flow_ohash

iflow:
    seq             c1, d.iflow_rx2tx_d.ipaf, 0
    seq             c2, d.iflow_rx2tx_d.parent_is_hint, 0

    add.c2          r2, d.iflow_rx2tx_d.flow_hash[22:0], r0
    add.!c2         r2, d.iflow_rx2tx_d.parent_hint_index, r0

    sll.c1          r2, r2, FLOW_IPV4_ENTRY_SHIFT
    sll.!c1         r2, r2, FLOW_IPV6_ENTRY_SHIFT

    setcf           c3, [c1 & c2]
    addi.c3         r3, r2, loword(ipv4_flow)

    setcf           c4, [c1 & !c2]
    addi.c4         r3, r2, loword(ipv4_flow_ohash)

    setcf           c5, [!c1 & c2]
    addi.c5         r3, r2, loword(flow)

    setcf           c6, [!c1 & !c2]
    addi.c6         r3, r2, loword(flow_ohash)

    phvwr.e         p.txdma_control_pktdesc_addr, r3
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
iflow_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
