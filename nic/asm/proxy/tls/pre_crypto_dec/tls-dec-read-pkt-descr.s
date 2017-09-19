/*
 * 	Implements the reading of packet descriptor
 *  Stage 2, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct phv_                 p;
struct tx_table_s2_t0_d     d;
struct tx_table_s2_t0_k     k;

%%
    .param      tls_dec_rx_serq_process
    .align

tls_dec_pkt_descriptor_process:
    phvwr   p.s2_s3_t0_phv_idesc_aol0_addr, d.{u.tls_read_pkt_descr_aol_d.A0}.dx
    phvwr   p.s2_s3_t0_phv_idesc_aol0_offset, d.{u.tls_read_pkt_descr_aol_d.O0}.wx 
    phvwr   p.s2_s3_t0_phv_idesc_aol0_len, d.{u.tls_read_pkt_descr_aol_d.L0}.wx

	CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_rx_serq_process,
	                    k.tls_global_phv_qstate_addr, TLS_TCB_CRYPT_OFFSET,
                        TABLE_SIZE_512_BITS)

tls_pkt_descriptor_process_done:
	nop.e
	nop
