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
  seq.e         c1, d.lif_info_d.ingress_mirror_en, 1
  phvwr.c1      p.capri_intrinsic_tm_span_session, \
                    d.lif_info_d.ingress_mirror_session_id 
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
lif_error:
  nop.e
  nop
