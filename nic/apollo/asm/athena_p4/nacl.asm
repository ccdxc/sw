#include "athena.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct nacl_d   d;
struct phv_     p;

%%

nacl_permit:
    nop.e
    nop

.align
nacl_redirect:
    phvwr           p.p4i_to_p4e_header_nacl_redir_oport, d.u.nacl_redirect_d.oport
    phvwr           p.p4i_to_p4e_header_nacl_redir_lif, d.u.nacl_redirect_d.lif
    phvwr           p.p4i_to_p4e_header_nacl_redir_qtype, d.u.nacl_redirect_d.qtype
    phvwr           p.p4i_to_p4e_header_nacl_redir_qid, d.u.nacl_redirect_d.qid
    phvwr           p.p4i_to_p4e_header_nacl_redir_app_id, d.u.nacl_redirect_d.app_id
    phvwr.e         p.capri_intrinsic_drop, 0
    phvwr           p.control_metadata_p4i_drop_reason, 0

.align
nacl_drop:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr           p.control_metadata_p4i_drop_reason[P4I_DROP_NACL], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
