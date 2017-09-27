/*
 *    Implements the Write ARQ stage of the TLS TxDMA P4+ pre decrypt pipeline
 */
#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s6_t1_k     k;
struct phv_                 p;
struct tx_table_s6_t1_tls_write_arq_d     d;


%%
    .align    
tls_dec_write_arq:
    CAPRI_CLEAR_TABLE1_VALID


    /* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
    sne         c1, k.tls_global_phv_write_arq, r0
    bcf         [!c1], tls_dec_write_arq_done
    nop


    smeqb       c1, k.s5_s6_t1_s2s_debug_dol, TLS_DDOL_LEAVE_IN_ARQ, TLS_DDOL_LEAVE_IN_ARQ

dma_cmd_arq_slot:
    CPU_ARQ_PIDX_READ_INC(r6, 0, struct tx_table_s6_t1_tls_write_arq_d, pi_0)
	addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r5
    phvwr       p.ring_entry_descr_addr, k.to_s6_idesc
    CPU_RX_ENQUEUE(r5,
                   k.to_s6_idesc,
                   r6,
                   k.s5_s6_t1_s2s_arq_base,
                   ring_entry_descr_addr,
                   dma_cmd0_dma_cmd, 
                   1, 
                   1, 
                   c1)  

    
tls_dec_write_arq_done:
    nop.e
    nop

