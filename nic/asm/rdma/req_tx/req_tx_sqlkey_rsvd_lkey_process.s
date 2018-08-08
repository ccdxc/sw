#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s4_t0_k k;
struct key_entry_aligned_t d;

#define IN_P t0_s2s_sge_to_lkey_info
#define IN_TO_S_P to_s4_dcqcn_bind_mw_info


#define K_SGE_VA CAPRI_KEY_RANGE(IN_P, sge_va_sbit0_ebit7, sge_va_sbit56_ebit63)
#define K_SGE_BYTES CAPRI_KEY_RANGE(IN_P, sge_bytes_sbit0_ebit7, sge_bytes_sbit8_ebit15)
#define K_SGE_INDEX CAPRI_KEY_FIELD(IN_P, sge_index)
#define K_SGE_DMA_CMD_START_INDEX CAPRI_KEY_FIELD(IN_P, dma_cmd_start_index)

#define K_PD CAPRI_KEY_FIELD(IN_TO_S_P, header_template_addr_or_pd)

%%

.align
req_tx_sqlkey_rsvd_lkey_process:

     // get DMA cmd entry based on dma_cmd_index
     DMA_CMD_I_BASE_GET(r3, r6, REQ_TX_DMA_CMD_START_FLIT_ID, K_SGE_DMA_CMD_START_INDEX)

     // setup mem2pkt cmd to transfer data from host memory to pkt payload
     DMA_MEM2PKT_SETUP(r3, c2, K_SGE_BYTES, K_SGE_VA)

     add        r1, K_SGE_INDEX, r0
     CAPRI_SET_TABLE_I_VALID(r1, 0)

     nop.e
     nop
