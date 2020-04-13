#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"
#include "INGRESS_session_k.h"
#include "asic/cmn/asic_common.hpp"

struct session_k_    k;
struct session_d    d;
struct phv_         p;

%%

session_info:
    crestore        [c7-c1], r0, 0xFF
    seq             c1, k.control_metadata_session_index, r0
    seq             c2, k.p4_to_rxdma_header_sacl_base_addr, r0
    setcf           c3, [!c1 | c2]
    phvwr.c3        p.p4_to_rxdma_header_sacl_bypass, 1
    seq             c1, r5, r0
    nop.c1.e
    add             r5, r5, k.control_metadata_session_index, 5
    seq             c1, k.control_metadata_flow_role, TCP_FLOW_RESPONDER
    add.c1          r5, r5, 16
    add             r7, r0, k.capri_p4_intrinsic_packet_len
    addi            r1, r0, 0x1000001
    or              r7, r7, r1, 32
    addi            r6, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    bbeq            d.session_info_d.drop, TRUE, session_info_drop
    or              r7, r7, r5[32:27], 58
    add.e           r6, r6, r5[26:0]
    memwr.dx        r6, r7

session_info_drop:
    add             r6, r6, r5[26:0]
    memwr.dx        r6, r7
    phvwr.e         p.control_metadata_p4i_drop_reason[P4I_DROP_FLOW_HIT], 1
    phvwr           p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
session_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
