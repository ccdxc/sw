/*
 * 	Implements the reading of SERQ entry
 *  Stage 1, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

#define D(field)    d.{u.read_serq_entry_d.##field}
#define K(field)    k.{##field}

struct phv_             p;
struct tx_table_s1_t0_d d;
struct tx_table_s1_t0_k k;

%%
    .param          tls_dec_aesgcm_check_tls_hdr


tls_dec_aesgcm_read_serq_entry_process:

    phvwr           p.to_s3_idesc, D(idesc)
    phvwr           p.to_s4_idesc, D(idesc)

    phvwr           p.tls_global_phv_A0, D(A0).dx
    phvwr           p.tls_global_phv_O0, D(O0).wx
    phvwr           p.tls_global_phv_L0, D(L0).wx

	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_aesgcm_check_tls_hdr,
	                    K(tls_global_phv_qstate_addr),
                            TLS_TCB_OPER_DATA_OFFSET,
                            TABLE_SIZE_512_BITS)

tls_dec_aesgcm_read_serq_entry_process_done:
	
	nop.e
	nop
