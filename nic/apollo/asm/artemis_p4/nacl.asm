#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_nacl_k.h"

struct nacl_k_ k;
struct nacl_d  d;
struct phv_ p;

%%

nacl_permit:
    nop.e
    nop

.align
nacl_redirect:
    phvwr           p.capri_intrinsic_tm_oport, d.u.nacl_redirect_d.oport
    phvwr           p.capri_intrinsic_lif, d.u.nacl_redirect_d.lif
    phvwr           p.capri_rxdma_intrinsic_qtype, d.u.nacl_redirect_d.qtype
    phvwr           p.capri_rxdma_intrinsic_qid, d.u.nacl_redirect_d.qid
    phvwr           p.control_metadata_vlan_strip, d.u.nacl_redirect_d.vlan_strip
    phvwr           p.control_metadata_pipe_id, d.u.nacl_redirect_d.pipe_id
    phvwr.e         p.control_metadata_p4i_drop_reason, 0
    phvwr.f         p.capri_intrinsic_drop, 0

.align
nacl_drop:
    phvwr.e         p.control_metadata_p4i_drop_reason[P4I_DROP_NACL], 1
    phvwr.f         p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
