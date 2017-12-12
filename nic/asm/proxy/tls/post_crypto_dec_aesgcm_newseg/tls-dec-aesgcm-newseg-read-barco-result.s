/*
 * 	Implements the reading of Barco results and the output descriptor ref
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.barco_result_read_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_d d;
%%

tls_dec_post_crypto_aesgcm_newseg_get_barco_result:

    add         r1, D(status), r0
    add         r2, D(output_list_address), r0

tls_dec_post_crypto_aesgcm_newseg_get_barco_result_done:
    CAPRI_CLEAR_TABLE0_VALID
    nop.e
    nop

