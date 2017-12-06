#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "../../../app-redir-p4+/common/include/app_redir_shared.h"
        
struct tx_table_s5_t1_k     k;
struct phv_                 p;
struct tx_table_s5_t1_d     d;
	
%%

tls_dec_queue_l7q_process:
    CAPRI_CLEAR_TABLE1_VALID
    addi		r5, r0, TLS_PHV_DMA_COMMANDS_START
	add		    r4, r5, r0
	phvwr		p.p4_txdma_intr_dma_cmd_ptr, r4

    sne         c1, k.tls_global_phv_l7_proxy_en, r0
    bcf         [!c1], tls_queue_l7q_process_done
    nop

    sne         c2, k.tls_global_phv_l7_proxy_type_span, r0
    bcf         [c2], dma_cmd_l7_desc_span
    nop

dma_cmd_l7_desc_redir:
    // redir: dma odesc as desc
    add         r3, r0, k.to_s5_odesc
    addi        r3, r3, PKT_DESC_AOL_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd0_dma_cmd, r3, odesc_A0, odesc_next_pkt)

    phvwr       p.l7_ring_entry_descr_addr, k.to_s5_odesc
    b           dma_cmd_l7q_slot
    nop

dma_cmd_l7_desc_span:
    // redir: dma l7_desc desc
    add         r3, r0, k.to_s5_l7_desc
    addi        r3, r3, PKT_DESC_AOL_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd3_dma_cmd, r3, l7_desc_A0, l7_desc_next_pkt)
    
    phvwr       p.l7_ring_entry_descr_addr, k.to_s5_l7_desc

dma_cmd_l7q_slot:
    // Set DMA command to do L7Q write
    CAPRI_OPERAND_DEBUG(d.{u.tls_queue_l7q_d.sw_l7q_pi}.hx)
	sll		    r5, d.{u.tls_queue_l7q_d.sw_l7q_pi}.hx, NIC_L7Q_ENTRY_SIZE_SHIFT
	add		    r1, r5, d.{u.tls_queue_l7q_d.l7q_base}.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd4_dma_cmd, r1, l7_ring_entry_descr_addr,l7_ring_entry_descr_addr)

    smeqb       c1, k.to_s5_debug_dol, TLS_DDOL_SESQ_STOP, TLS_DDOL_SESQ_STOP
    bcf         [c1], tls_l7q_produce_skip
    nop

tls_l7q_produce:
    add         r7, k.to_s5_other_fid, PROXYR_OPER_CB_OFFSET(PROXYR_TLS_PROXY_DIR)
    CAPRI_DMA_CMD_RING_DOORBELL(dma_cmd5_dma_cmd,
                                LIF_APP_REDIR, 
                                APP_REDIR_PROXYR_QTYPE, 
                                r7, 
                                0,
                                d.{u.tls_queue_l7q_d.sw_l7q_pi}.hx,
                                db_data_data)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd5_dma_cmd)
    phvwri      p.dma_cmd5_dma_cmd_eop, 1
    tbladd      d.{u.tls_queue_l7q_d.sw_l7q_pi}.hx, 1
    b           tls_queue_l7q_process_done
    nop
tls_l7q_produce_skip:
    phvwri      p.dma_cmd4_dma_cmd_eop, 1
    phvwri      p.dma_cmd4_dma_cmd_wr_fence, 1

tls_queue_l7q_process_done:
	nop.e
	nop
