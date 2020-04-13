/*
 *      Implements the reading of SERQ to pick up the new descriptor to process
 * Stage 0, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
        

struct phv_ p   ;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

        .param          tls_dec_read_serq_entry_process
        
tls_dec_pre_crypto_process:
    phvwr       p.tls_global_phv_debug_dol, d.u.read_tls_stg0_d.debug_dol	
    CAPRI_OPERAND_DEBUG(d.u.read_tls_stg0_d.debug_dol)

    /* Verify that we do have room in the TLS record ring, if not 
       do not accept the TCP segment from SERQ
    */
    add         r1, d.u.read_tls_stg0_d.recq_pi, 0
    mincr       r1, ASIC_BSQ_RING_SLOTS_SHIFT, 1
    seq         c1, r1, d.u.read_tls_stg0_d.recq_ci /* Ring Full */
    bcf         [c1], tls_dec_pre_crypto_process_defer
    nop

    /* Relay the address to setup the DMA command */
    add         r1, r0, d.u.read_tls_stg0_d.recq_pi
    sll         r2, r1, ASIC_BSQ_RING_SLOT_SIZE_SHFT
    add         r1, r2, d.{u.read_tls_stg0_d.recq_base}.wx

    tblmincri       d.u.read_tls_stg0_d.recq_pi, ASIC_BSQ_RING_SLOTS_SHIFT ,1

    add         r6, r0, d.{u.read_tls_stg0_d.ci_0}.hx
    sll         r6, r6, NIC_SERQ_ENTRY_SIZE_SHIFT

    tblmincri.f d.{u.read_tls_stg0_d.ci_0}.hx, ASIC_SERQ_RING_SLOTS_SHIFT, 1

    /* Setting up DMA command due to accessibility to recq_[base, pi] information */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_bsq_slot_dma_cmd, r1, bsq_slot_desc, bsq_slot_desc)

    phvwr       p.barco_desc_command, d.u.read_tls_stg0_d.barco_command
    /*
     * Check if this is AES-CCM decrypt case, which has some differences in the barco
     * request encoding as compared to GCM (barco-command[31:24] value 0x05,
     * endian-swapped).
     */
    seq         c1, d.u.read_tls_stg0_d.barco_command[7:0], 0x05
    phvwri.c1   p.tls_global_phv_do_pre_ccm_dec, 1

    phvwrpair   p.tls_global_phv_fid, k.p4_txdma_intr_qid[15:0],  \
                p.tls_global_phv_qstate_addr,               \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}[31:0]

    seq         c4, d.{u.read_tls_stg0_d.ci_0}.hx, d.{u.read_tls_stg0_d.pi_0}.hx
    bcf         [!c4], tls_dec_pre_crypto_process_skip_serq_dbell
    nop

    /* CI == PI, hence evaluate scheduler */
    /* address will be in r4 */
    addi    r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_TLS)

    /* data will be in r3 */
    add	    r1, k.p4_txdma_intr_qid[15:0], r0
    add     r5, d.{u.read_tls_stg0_d.ci_0}.hx, r0
    CAPRI_RING_DOORBELL_DATA(0, r1, TLS_SCHED_RING_SERQ, r5)

	memwr.dx  	 r4, r3

tls_dec_pre_crypto_process_skip_serq_dbell:
    phvwr       p.to_s1_serq_ci, d.{u.read_tls_stg0_d.ci_0}.hx
    phvwr       p.to_s1_serq_prod_ci_addr, d.u.read_tls_stg0_d.serq_prod_ci_addr

    add         r3, r6, d.u.read_tls_stg0_d.serq_base

table_read_DESC:
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tls_dec_read_serq_entry_process, r3, TABLE_SIZE_256_BITS)
    nop.e
    nop

tls_dec_pre_crypto_process_defer:
    nop.e
    nop
    
