/*
 * 	Implements the processing of the read Barco ring semaphore (if applicable)
 *  Stage 6, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_barco_semaphore_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s6_t0_d d;
struct tx_table_s6_t0_k k;

%%
    .param          tls_dec_aesgcm_queue_barco

/*
    GPR Usage:
*/

tls_dec_aesgcm_read_barco_pidx:

    add             r1, r0, K(tls_global_phv_skip_dma)
    beqi            r1, 1, tls_dec_aesgcm_no_barco_enqueue

    add             r1, r0, K(tls_global_phv_enqueue_barco)
    bnei            r1, 1, tls_dec_aesgcm_no_barco_enqueue

    /* TODO: Semaphore - ring full condition */
    phvwr           p.s6_t0_to_s7_t0_producer_idx, D(producer_idx).wx

    phvwr           p.s6_t0_to_s7_t0_fid, K(to_s6_fid)

tls_dec_aesgcm_no_barco_enqueue:

tls_dec_aesgcm_read_barco_pidx_done:
	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_aesgcm_queue_barco,
	                    K(tls_global_phv_qstate_addr),
                        TLS_TCB_OFFSET,
                        TABLE_SIZE_512_BITS)

    nop.e
    nop
