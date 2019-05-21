#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t1_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t1_k_ k;
struct s2_t1_nvme_sessprexts_tx_xtscb_process_d d;

#define DMA_CMD_BASE    r5

%%
    .param      nvme_tx_xts_aol_array_base
    .param      nvme_tx_xts_iv_array_base

.align
nvme_sessprexts_tx_xtscb_process:
    add         r1, r0, d.pi
    mincr       r1, d.log_sz, 1
    seq         c1, r1, d.ci
    bcf         [c1], q_full
    add         r2, r0, d.pi
    tblwr.f     d.pi, r1    //Flush

    //r2 has pi value prior to increment

    //calculate xts descr addr
    sll         r3, r2, LOG_XTS_DESC_SIZE
    //XXX: xts_ring_base_addr is not populated yet !!
    add         r3, d.xts_ring_base_addr, r3

    //DMA instr for xts descr
    DMA_CMD_BASE_GET(DMA_CMD_BASE, xts_desc_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, xts_desc_input_list_address, \
                          xts_desc_second_key_desc_index, r3)

    //calculate aol pair addr
    addui       r3, r0, hiword(nvme_tx_xts_aol_array_base)
    addi        r3, r3, loword(nvme_tx_xts_aol_array_base)
    add         r3, r3, r2, LOG_AOL_PAIR_SIZE

    //DMA instr for ip desc
    DMA_CMD_BASE_GET(DMA_CMD_BASE, ip_desc_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, ip_desc_A0, ip_desc_rsvd, r3)
    phvwr       p.xts_desc_input_list_address, r3
    
    //move addr to point to op desc
    add         r3, r3, AOL_SIZE

    //DMA instr for op desc
    DMA_CMD_BASE_GET(DMA_CMD_BASE, op_desc_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, op_desc_A0, op_desc_rsvd, r3)
    phvwr       p.xts_desc_output_list_address, r3

    //calculate iv addr
    addui       r3, r0, hiword(nvme_tx_xts_iv_array_base)
    addi        r3, r3, loword(nvme_tx_xts_iv_array_base)
    add         r3, r3, r2, LOG_IV_SIZE

    //DMA instr for iv
    DMA_CMD_BASE_GET(DMA_CMD_BASE, iv_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, iv_iv_0, iv_iv_1, r3)
    phvwr       p.xts_desc_iv_address, r3

    //DMA instr for xts db
    DMA_CMD_BASE_GET(DMA_CMD_BASE, xts_db_dma)
    addi        r4, r0, HW_XTS_TX_DB_ADDR
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, xts_db_index, xts_db_index, r4)
    DMA_SET_WR_FENCE_END_OF_CMDS(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)

    //whatever is the PI value we gave to XTS engine doorbell, should come back 
    // as opaque tag value and update the CI. 
    //assumption is that opaque tag address is already setup
    //to point to the address of "CI" field in this xtscb. (check)

    phvwr       p.xts_db_index, r1

    phvwrpair.e p.xts_desc_opaque_tag_value, r1, \
                p.xts_desc_opaque_tag_write_en, 1
    CAPRI_SET_TABLE_1_VALID(0)                      //Exit Slot


    //TBD: set command, status_address
q_full:
    //XXX: TBD
exit:
    phvwr.e p.p4_intr_global_drop, 1
    nop             //Exit Slot
