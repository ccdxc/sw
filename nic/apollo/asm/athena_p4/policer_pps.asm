#include "athena.h"
#include "egress.h"
#include "EGRESS_p.h"

#if 0
struct policer_pps_d        d;
struct phv_                 p;

%%

policer_pps:
    seq           c1, d.policer_pps_d.entry_valid, TRUE
    seq.c1        c1, d.policer_pps_d.tbkt[39], TRUE
    nop.!c1.e
    //phvwr.c1.e  p.control_metadata_p4e_drop_reason[P4E_DROP_VNIC_POLICER_RX], 1
    phvwr.c1.f.e  p.capri_intrinsic_drop, TRUE
    nop
#else
%%
    nop.e
    nop
#endif
