#include "../../p4/include/lpm_defines.h"

#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct sacl_ip_sport_p1_k   k;
struct sacl_ip_sport_p1_d   d;

%%

sacl_ip_sport_p1:
    mod             r7, k.{sacl_metadata_ip_sport_class_id_sbit0_ebit0, \
                           sacl_metadata_ip_sport_class_id_sbit1_ebit16}, 51
    mul             r7, r7, 10
    tblrdp.e        r1, r7, 0, 10
    phvwr           p.sacl_metadata_p1_class_id, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
sacl_ip_sport_p1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
