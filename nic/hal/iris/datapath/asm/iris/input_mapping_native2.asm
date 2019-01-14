#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_input_mapping_native2_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_mapping_native2_k_ k;
struct phv_ p;

%%

native_ipv6_packet2:
    phvwr.e     p.flow_lkp_metadata_lkp_src, k.ipv6_srcAddr
    phvwr.f     p.flow_lkp_metadata_lkp_dst, k.ipv6_dstAddr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_mapping_native2_error:
  nop.e
  nop
