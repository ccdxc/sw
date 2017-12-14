/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * 	Construct the barco request in this stage
 * Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s4_t0_k k                  ;
struct phv_ p	;
struct tx_table_s4_t0_d d	;
	
%%
	    .param      tls_mac_queue_brq_process
#	    .param	BRQ_QPCB_BASE
        
tls_mac_bld_barco_req_process:        

table_read_QUEUE_BRQ:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s6_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_0)

    /* Fill the barco request in the phv to be DMAed later into BRQ slot */
    phvwr       p.barco_desc_status_address, k.{to_s4_idesc}.dx
    addi        r2, r0, PKT_DESC_AOL_OFFSET
    add         r1, r2, k.{to_s4_idesc}
    phvwr       p.barco_desc_input_list_address, r1.dx
    CAPRI_OPERAND_DEBUG(r1.dx)

    phvwr       p.barco_desc_key_desc_index, k.{to_s4_barco_hmac_key_desc_index}
    CAPRI_OPERAND_DEBUG(k.{to_s4_barco_hmac_key_desc_index})

    phvwri      p.to_s6_mac_requests, 1

    /* Setup AAD */
    /* AAD length already setup in Stage 2, Table 3 */
    phvwr       p.s2_s5_t0_phv_aad_seq_num, d.u.tls_bld_brq4_d.explicit_iv
    phvwri      p.s2_s5_t0_phv_aad_type, NTLS_RECORD_DATA
    phvwri      p.s2_s5_t0_phv_aad_version_major, NTLS_TLS_1_2_MAJOR
    phvwri      p.s2_s5_t0_phv_aad_version_minor, NTLS_TLS_1_2_MINOR

    tbladd      d.u.tls_bld_brq4_d.explicit_iv, 1

    /*
     * The "barco-command" in the TLSCB will have value for "AES-CBC-HMAC-SHA2", but we perform
     * 2 separate passes, HMAC-SHA2 in the 1st pass and then AES-CBC in the 2nd pass (we're in
     * the 2nd pass now). So we'll use the HMAC-SHA2 mac-generate command value instead of the
     * one in TLSCB.
     *
     * NOTE: Also, in order to distinguish the 2 post-barco-crypto stages of AES-CBC-HMAC-SHA2 2 pass
     * pipeline (one for HMAC-SHA2 mac generation and one for AES-CBC encrypt), we currently keep
     * state in the TLSCB barco-command field (we do +1 in pre-mac stage and -1 here in post-mac stage).
     * This assumes only one outstanding request to barco per TLSCB, which needs to be removed -- We'll
     * use a different BSQ ring-id eventually for barco to ring response doorbell on, to distinguish
     * this case.
     */
    /* phvwr       p.barco_desc_command, d.u.tls_bld_brq4_d.barco_command
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq4_d.barco_command)*/
    phvwri       p.barco_desc_command, TLS_WORD_SWAP(CAPRI_BARCO_COMMAND_HMAC_SHA256_GENERATE)
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq4_d.barco_command)
    tbladd       d.{u.tls_bld_brq4_d.barco_command}.wx, 1
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq4_d.barco_command)

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

    /*
     * We use MPP2 barco ring for HMAC-SHA2 mac generation/verify.
     */
    addi        r3, r0, CAPRI_BARCO_MP_MPNS_REG_MPP2_PRODUCER_IDX
	
    /* FIXME: The Capri model currently does not support a read of 8 bytes from register space
     * enable this once it is fixed
     *  CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_mac_queue_brq_process, r3, TABLE_SIZE_64_BITS);
     */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_mac_queue_brq_process, r3, TABLE_SIZE_32_BITS);
    nop.e
    nop
