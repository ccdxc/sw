/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Construct the barco request in this stage
 * Stage 3, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s3_t0_k k                  ;
struct phv_ p	;
struct tx_table_s3_t0_d d	;
	
%%
	    .param      tls_mac_queue_enc_brq_process	
        
tls_mac_bld_barco_enc_req_process:        

table_read_QUEUE_ENC_BRQ:
    CAPRI_SET_DEBUG_STAGE0_3(p.to_s5_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    CAPRI_CLEAR_TABLE0_VALID

    /* Fill the barco request in the phv to be DMAed later into BRQ slot */
    phvwr       p.barco_desc_status_address, k.{to_s3_idesc}.dx
    addi        r2, r0, PKT_DESC_AOL_OFFSET
    add         r1, r2, k.{to_s3_idesc}
    phvwr       p.barco_desc_input_list_address, r1.dx
    CAPRI_OPERAND_DEBUG(r1.dx)

    phvwr       p.barco_desc_key_desc_index, d.u.tls_bld_brq3_d.barco_key_desc_index
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq3_d.barco_key_desc_index)

    phvwri      p.to_s5_enc_requests, 1

    /*
     * The "barco-command" in the TLSCB will have value for "AES-CBC-HMAC-SHA2", but we perform
     * 2 separate passes, HMAC-SHA2 in the 1st pass and then AES-CBC in the 2nd pass (we're in
     * the 2nd pass now). So we'll use the AES-CBC command value instead of the one in TLSCB.
     *
     * NOTE: Also, in order to distinguish the 2 post-barco-crypto stages of AES-CBC-HMAC-SHA2 2 pass
     * pipeline (one for HMAC-SHA2 mac generation and one for AES-CBC encrypt), we currently keep
     * state in the TLSCB barco-command field (we do +1 in pre-mac stage and -1 here in post-mac stage).
     * This assumes only one outstanding request to barco per TLSCB, which needs to be removed -- We'll
     * use a different BSQ ring-id eventually for barco to ring response doorbell on, to distinguish
     * this case.
     */
    //phvwr       p.barco_desc_command, d.u.tls_bld_brq3_d.barco_command
    phvwri      p.barco_desc_command, TLS_WORD_SWAP(CAPRI_BARCO_COMMAND_AES_CBC_ENCRYPT)

    tblsub      d.{u.tls_bld_brq3_d.barco_command}.wx, 1
    CAPRI_OPERAND_DEBUG(d.u.tls_bld_brq3_d.barco_command)

    /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
    phvwr       p.barco_desc_doorbell_address, r4.dx
    CAPRI_OPERAND_DEBUG(r4.dx)

    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.tls_global_phv_fid, TLS_SCHED_RING_BSQ, 0)
    phvwr       p.barco_desc_doorbell_data, r3.dx
    CAPRI_OPERAND_DEBUG(r3.dx)

    /*
     * We use MPP3 barco-ring for AES-CBC encrypt/decrypt requests.
     */
    addi        r3, r0, CAPRI_BARCO_MP_MPNS_REG_MPP3_PRODUCER_IDX
	
    /* FIXME: The Capri model currently does not support a read of 8 bytes from register space
     * enable this once it is fixed
     *  CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_mac_queue_enc_brq_process, r3, TABLE_SIZE_64_BITS);
     */
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, tls_mac_queue_enc_brq_process, r3, TABLE_SIZE_32_BITS);
    nop.e
    nop
