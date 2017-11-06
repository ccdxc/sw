#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s3_t3_k k;
struct phv_ p;
struct tx_table_s3_t3_d d;

%%
    .align
tls_read_l7_descr_alloc:
    CAPRI_CLEAR_TABLE3_VALID
    
    CAPRI_OPERAND_DEBUG(d.u.tls_l7_desc_alloc_d.desc)
    phvwr       p.to_s5_l7_desc, d.u.tls_l7_desc_alloc_d.desc
    nop.e
    nop
