#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_ipsec_encap_txdma_deque_head_desc_d d;
struct phv_ p;

%%
        .align
ipsec_encap_txdma_deque_head_desc:
    phvwr p.ipsec_to_stage1_head_desc_addr, d.rsvd
    add r1, r0, k.txdma1_global_ipsec_cb_addr
    addi r1, r1, IPSEC_CB_HEAD_DESC_ADDR_OFFSET
    phvwri p.head_desc_addr_update_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.head_desc_addr_update_dma_cmd_addr, r1
    phvwri p.head_desc_addr_update_dma_cmd_phv_start_addr, IPSEC_TXDMA1_HEAD_DESC_PHV_OFFSET_START
    phvwri p.head_desc_addr_update_dma_cmd_phv_end_addr, IPSEC_TXDMA1_HEAD_DESC_PHV_OFFSET_END 
 
    phvwri p.app_header_table1_valid, 0
