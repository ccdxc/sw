#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_p4e_device_info_k.h"

struct p4e_device_info_k_   k;
struct p4e_device_info_d    d;
struct phv_                 p;

%%

.assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_generic_valid) == 4)

p4e_device_info:
    phvwr           p.{tcp_option_eol_valid...tcp_option_generic_valid}, 0
    seq             c1, k.egress_recirc_valid, TRUE
    xor             r1, k.egress_recirc_mapping_done, 0x1
    phvwr.c1        p.control_metadata_mapping_ohash_lkp, r1
    phvwr.e         p.rewrite_metadata_device_ipv4_addr, \
                        d.p4e_device_info_d.device_ipv4_addr
    phvwr.f         p.rewrite_metadata_device_ipv6_addr, \
                        d.p4e_device_info_d.device_ipv6_addr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4e_device_info_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr.f         p.{tcp_option_eol_valid...tcp_option_generic_valid}, 0
