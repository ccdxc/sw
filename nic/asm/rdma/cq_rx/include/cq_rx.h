#ifndef __CQ_RX_H
#define __CQ_RX_H
#include "capri.h"
#include "types.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define CQ_RX_DMA_CMD_START_FLIT_ID 7
#define CQ_RX_DMA_CMD_START_FLIT_CMD_ID 2
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

#define CQ_RX_MAX_DMA_CMDS        16

#define CQ_RX_DMA_CMD_PYLD_BASE   2
#define CQ_RX_DMA_CMD_START       0
#define CQ_RX_DMA_CMD_EQ          (CQ_RX_MAX_DMA_CMDS - 1)


#define CQ_RX_EQCB_ADDR_GET(_r, _tmp_r, _eqid, _cqcb_base_addr_hi, _log_num_cq_entries) \
    EQCB_ADDR_GET(_r, _tmp_r, _eqid, _cqcb_base_addr_hi, _log_num_cq_entries);

// phv 
struct cq_rx_phv_t {
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
    struct eqwqe_t eqwqe; //4 Bytes
    pad6     : 480;
    
    /* flit 0-5 */
    union {
        struct {
            pad5: 512;
            pad4: 512;
            pad3: 512;
            pad2: 512;
            pad1: 512;
            pad0: 512;
        };
        // common rx
        struct phv_ common;
    };
};

#endif //__CQ_RX_H
