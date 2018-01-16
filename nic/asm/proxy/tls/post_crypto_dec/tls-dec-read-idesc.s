/*
 * 	Implements the reading of input descriptor to retirve the page addresses
 *  Stage 2, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_                 p;
struct tx_table_s2_t3_d     d;
struct tx_table_s2_t3_k     k;


%%


tls_dec_post_read_idesc:

    seq         c1, k.tls_global_phv_l7_proxy_type_span, 1
    phvwri.!c1  p.app_header_table3_valid, 0;
    phvwr       p.to_s3_ipage,  d.u.tls_read_idesc_d.A0

tls_dec_post_read_idesc_done:
    nop.e
    nop.e

