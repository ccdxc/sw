#ifndef __AQ_RX_H
#define __AQ_RX_H
#include "capri.h"
#include "types.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define AQ_RX_DMA_CMD_START_FLIT_ID 7
#define AQ_RX_DMA_CMD_START_FLIT_CMD_ID 2
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

#define AQ_RX_MAX_DMA_CMDS        16

#define AQ_RX_DMA_CMD_PYLD_BASE   2
#define AQ_RX_DMA_CMD_START       0
#define AQ_RX_DMA_CMD_CQ          (AQ_RX_MAX_DMA_CMDS - 4)
#define AQ_RX_DMA_CMD_EQ          (AQ_RX_MAX_DMA_CMDS - 3)
#define AQ_RX_DMA_CMD_AQ_BUSY     (AQ_RX_MAX_DMA_CMDS - 2)
//wakeup dpath and EQ are mutually exclusive
#define AQ_RX_DMA_CMD_WAKEUP_DPATH  AQ_RX_DMA_CMD_EQ
#define AQ_RX_DMA_CMD_EQ_INT      (AQ_RX_MAX_DMA_CMDS - 1)


#define AQ_RX_CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi) \
    CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi);

#define AQ_RX_EQCB_ADDR_GET(_r, _tmp_r, _eqid, _aqcb_base_addr_hi, _log_num_aq_entries) \
    EQCB_ADDR_GET(_r, _tmp_r, _eqid, _aqcb_base_addr_hi, _log_num_aq_entries);

// phv 
struct aq_rx_phv_t {
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
    union {
        struct {
        int_assert_data : 32;
            struct eqwqe_t eqwqe;
        };
        wakeup_dpath_data   : 64;
    };
    struct cqe_t cqe;
    busy     : 1;
    pad6     : 191;
    
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

#endif //__AQ_RX_H
