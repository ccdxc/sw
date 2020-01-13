#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_nacl_k.h"

struct nacl_k_  k;
struct nacl_d   d;
struct phv_     p;

%%

nacl_permit:
    nop.e
    nop

.align
nacl_redirect:
    bbeq            k.arm_to_p4i_nexthop_valid, TRUE, nacl_redirect_from_arm
    phvwr           p.p4i_i2e_session_id, -1
    phvwr           p.p4i_i2e_nexthop_type, d.u.nacl_redirect_d.nexthop_type
    phvwr           p.p4i_i2e_nexthop_id, d.u.nacl_redirect_d.nexthop_id
    b               nacl_redirect2
    phvwr           p.p4i_i2e_copp_policer_id, \
                        d.u.nacl_redirect_d.copp_policer_id
nacl_redirect_from_arm:
    phvwr           p.p4i_i2e_nexthop_type, k.arm_to_p4i_nexthop_type
    or              r1, k.arm_to_p4i_nexthop_id_s8_e15, \
                        k.arm_to_p4i_nexthop_id_s0_e7, 8
    phvwr           p.p4i_i2e_nexthop_id, r1

nacl_redirect2:
    phvwr           p.p4i_i2e_mapping_bypass, TRUE
    phvwr.e         p.capri_intrinsic_drop, 0
    phvwr.f         p.control_metadata_p4i_drop_reason, 0

.align
nacl_redirect_to_arm:
    phvwr           p.p4i_i2e_session_id, -1
    phvwr           p.p4i_i2e_nexthop_type, \
                        d.u.nacl_redirect_to_arm_d.nexthop_type
    phvwr           p.p4i_i2e_copp_policer_id, \
                        d.u.nacl_redirect_to_arm_d.copp_policer_id
    phvwr           p.p4i_to_arm_nacl_data, d.u.nacl_redirect_to_arm_d.data
    phvwr.e         p.p4i_i2e_nexthop_id, d.u.nacl_redirect_to_arm_d.nexthop_id
    phvwr.f         p.control_metadata_redirect_to_arm, TRUE

.align
nacl_drop:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr.f         p.control_metadata_p4i_drop_reason[P4I_DROP_NACL], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
