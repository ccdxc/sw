/*
 * 	Implements the reading of odesc and opage from the cache (idesc scratch)
 *  Stage 6, Table 1
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_cached_odesc_opage_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s6_t1_d d;
struct tx_table_s6_t1_k k;

%%
    .param          tls_dec_aesgcm_read_barco_pidx

/*
    GPR Usage:
    r3  - Auth Tag Low
    r4  - Auth Tag High
    r5  - TLS Record Pending Length
*/

tls_dec_aesgcm_read_odesc_opage_cache:

    phvwr           p.to_s7_odesc, D(odesc)
    phvwr           p.to_s7_opage, D(opage)

tls_dec_aesgcm_read_odesc_opage_cache_done:

    CAPRI_CLEAR_TABLE1_VALID
    nop.e
    nop
