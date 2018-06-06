#ifndef __CQ_TX_H
#define __CQ_TX_H
#include "capri.h"
#include "types.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define CQ_TX_DMA_CMD_PHV_INTRINSIC 2
#define CQ_TX_DMA_CMD_RDMA_FEEDBACK 19   // This should be at the end
#define CQ_TX_DMA_CMD_START_FLIT_ID 7
#define CQ_TX_DMA_CMD_START_FLIT_CMD_ID 2
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

// phv 
struct cq_tx_phv_t {
    // dma commands
    /* flit 11 */
    dma_cmd16 : 128;
    dma_cmd17 : 128;
    dma_cmd18 : 128;
    dma_cmd19 : 128;


    /* flit 10 */
    dma_cmd12 : 128;
    dma_cmd13 : 128;
    dma_cmd14 : 128;
    dma_cmd15 : 128;

    /* flit 9 */
    dma_cmd8 : 128;
    dma_cmd9 : 128;
    dma_cmd10 : 128;
    dma_cmd11 : 128;

    /* flit 8 */
    dma_cmd4 : 128;
    dma_cmd5 : 128;
    dma_cmd6 : 128;
    dma_cmd7 : 128;
 
    /* flit 7 */
    dma_cmd0 : 128;
    dma_cmd1 : 128;
    dma_cmd2 : 128;
    dma_cmd3 : 128;

    /* flit 6 */
    pad6     : 512;
    
    /* flit 0-5 */
    union {
        struct {
            struct phv_intr_p4_t p4_intr;            //  5 Bytes
            struct phv_intr_rxdma_t p4_intr_rxdma;   // 10 Bytes
            struct p4_to_p4plus_roce_header_t p4_to_p4plus; // 20 Bytes
            struct rdma_feedback_t rdma_feedback;    // 11 Bytes
            pad5: 144;
            pad4: 512;
            pad3: 512;
            pad2: 512;
            pad1: 512;
            pad0: 512;
        };
        // common tx
        struct phv_ common;
    };
};

#endif //__CQ_TX_H
