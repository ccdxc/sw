/*
 * 	Construct the barco request in this stage for decrypt
 * Stage 5, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s5_t0_k     k;
struct phv_                 p;
struct tx_table_s5_t0_d     d;
	
%%
	    .param      tls_dec_queue_brq_process
	    .param      tls_dec_queue_brq_mpp_process
   	    .param      tls_dec_write_arq
#	    .param		BRQ_QPCB_BASE
        .param      ARQRX_QIDXR_BASE
        .param      ARQTX_BASE
        
tls_dec_bld_barco_req_process:
    seq         c1, k.tls_global_phv_write_arq, r0
    bcf         [!c1], tls_cpu_rx
    nop
table_read_QUEUE_BRQ:
    /* Fill the barco request in the phv to be DMAed later into BRQ slot */
    phvwr       p.barco_desc_status_address, k.{to_s5_idesc}.dx
    addi        r2, r0, PKT_DESC_AOL_OFFSET
    add         r1, r2, k.{to_s5_idesc}
    phvwr       p.barco_desc_input_list_address, r1.dx
    CAPRI_OPERAND_DEBUG(r1.dx)

    add         r1, r2, k.{to_s5_odesc}
    phvwr       p.barco_desc_output_list_address, r1.dx
    CAPRI_OPERAND_DEBUG(r1.dx)

    add         r1, r0, k.{to_s5_odesc}
    phvwr       p.odesc_dma_src_odesc, r1.dx

    phvwr       p.barco_desc_key_desc_index, d.u.tls_bld_brq5_d.barco_key_desc_index
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq5_d.barco_key_desc_index)

    phvwr       p.crypto_iv_salt, d.u.tls_bld_brq5_d.salt
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq5_d.salt)

    /* FIXME: Misnomer, this is actually the sequence number */
	phvwr		p.s4_s6_t0_phv_aad_seq_num, d.u.tls_bld_brq5_d.explicit_iv
    tbladd      d.u.tls_bld_brq5_d.explicit_iv, 1

    phvwr       p.barco_desc_command, d.u.tls_bld_brq5_d.barco_command
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq5_d.barco_command)

    addi        r1, r0, NTLS_AAD_SIZE
    phvwr       p.barco_desc_header_size, r1.wx

	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
    phvwr       p.barco_desc_doorbell_address, r4.dx
    CAPRI_OPERAND_DEBUG(r4.dx)

	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, k.tls_global_phv_fid, TLS_SCHED_RING_BSQ, 0)
    phvwr       p.barco_desc_doorbell_data, r3.dx
    CAPRI_OPERAND_DEBUG(r3.dx)

    /* The barco-command[31:24] is checked for GCM/CCM/CBC. endian-swapped */
    smeqb  c4, d.u.tls_bld_brq5_d.barco_command[7:0], 0xf0, 0x30
    bcf    [!c4], tls_dec_queue_to_brq_mpp_ring
    nop
	
    addi        r3, r0, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX

tls_dec_bld_barco_req_process_done:
    /* FIXME: The Capri model currently does not support a read of 8 bytes from register space
     * enable this once it is fixed
     *  CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_queue_brq_process, r3, TABLE_SIZE_64_BITS);
     */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_dec_queue_brq_process, r3, TABLE_SIZE_32_BITS);
        nop.e
        nop

tls_dec_queue_to_brq_mpp_ring:
    addi        r3, r0, CAPRI_BARCO_MP_MPNS_REG_MPP1_PRODUCER_IDX

tls_dec_bld_barco_req_process_done_2:
    /* FIXME: The Capri model currently does not support a read of 8 bytes from register space
     * enable this once it is fixed
     *  CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_enc_queue_brq_process, r3, TABLE_SIZE_64_BITS);
     */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_dec_queue_brq_mpp_process, r3, TABLE_SIZE_32_BITS);
	nop.e
	nop

tls_cpu_rx:

    phvwri      p.s5_s6_t1_s2s_arq_base, ARQTX_BASE
    phvwr       p.s5_s6_t1_s2s_debug_dol, k.to_s5_debug_dol

    /* Use RxDMA pi (first arg = 1 for TxDMA) */
    CPU_ARQRX_QIDX_ADDR(1, r3, ARQRX_QIDXR_BASE)

    CAPRI_NEXT_TABLE_READ_OFFSET(1,
                                 TABLE_LOCK_EN,
                                 tls_dec_write_arq,
                                 r3,
                                 0, /* TODO: Make it CPU_ARQRX_QIDXR_OFFSET */
                                 TABLE_SIZE_512_BITS)

    b.c4           tls_dec_bld_barco_req_process_done
    nop
    b               tls_dec_bld_barco_req_process_done_2
    nop
