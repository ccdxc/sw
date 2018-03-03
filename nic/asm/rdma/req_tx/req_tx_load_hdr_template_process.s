#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_hdr_template_t d;

%%

.align
req_tx_load_hdr_template_process:

    phvwr         p.pad1, d.data
    phvwr         p.pad2[15:0], r0[15:0]

    nop.e
    CAPRI_SET_TABLE_2_VALID(0)
