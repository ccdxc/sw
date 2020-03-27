#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_lif_k.h"

struct lif_k_ k;
struct lif_d  d;
struct phv_   p;

%%

.align
lif_info:
    sne             c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
    seq.c1          c1, d.lif_info_d.ingress_mirror_en, 1
    nop.e
    phvwr.c1        p.capri_intrinsic_tm_span_session, \
                        d.lif_info_d.ingress_mirror_session_id

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lif_error:
  nop.e
  nop
