#ifndef __AQ_RX_H
#define __AQ_RX_H
#include "capri.h"
#include "types.h"
#include "rqcb.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define AQ_RX_DMA_CMD_START_FLIT_ID 10
#define AQ_RX_DMA_CMD_START_FLIT_CMD_ID 0
#define TOTAL_DMA_CMD_BITS 16 * 16 * 8 // (cmds * dma_cmd_size * bits_per_byte) 

#define AQ_RX_MAX_DMA_CMDS                  8

#define AQ_RX_DMA_CMD_PYLD_BASE             2
#define AQ_RX_DMA_CMD_START                 0
#define AQ_RX_DMA_CMD_CQ                    (AQ_RX_MAX_DMA_CMDS - 4)
#define AQ_RX_DMA_CMD_AQ_ERR_DIS            (AQ_RX_MAX_DMA_CMDS - 4)
#define AQ_RX_DMA_CMD_EQ                    (AQ_RX_MAX_DMA_CMDS - 3)
#define AQ_RX_DMA_CMD_ASYNC_EQ              AQ_RX_DMA_CMD_EQ
//wakeup dpath and EQ are mutually exclusive
#define AQ_RX_DMA_CMD_WAKEUP_DPATH          AQ_RX_DMA_CMD_EQ
#define AQ_RX_DMA_CMD_EQ_INT                (AQ_RX_MAX_DMA_CMDS - 1)
#define AQ_RX_DMA_CMD_ASYNC_EQ_INT          AQ_RX_DMA_CMD_EQ_INT

#define AQ_RX_DMA_CMD_CREATE_QP_CB          (AQ_RX_MAX_DMA_CMDS - 5)
#define AQ_RX_DMA_CMD_CREATE_QP_RQPT_DST    (AQ_RX_MAX_DMA_CMDS - 6)
#define AQ_RX_DMA_CMD_CREATE_QP_RQPT_SRC    (AQ_RX_MAX_DMA_CMDS - 7)

#define AQ_RX_CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi) \
    CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi);

#define AQ_RX_EQCB_ADDR_GET(_r, _tmp_r, _eqid, _aqcb_base_addr_hi, _log_num_aq_entries) \
    EQCB_ADDR_GET(_r, _tmp_r, _eqid, _aqcb_base_addr_hi, _log_num_aq_entries);

struct aq_rx_dma_cmds_flit_t {
    dma_cmd0 : 128;
    dma_cmd1 : 128;
    dma_cmd2 : 128;
    dma_cmd3 : 128;
};


// phv 
struct aq_rx_phv_t {
    // dma commands
    /* flit 11 */
    struct aq_rx_dma_cmds_flit_t flit_11;
    
    /* flit 10 */
    struct aq_rx_dma_cmds_flit_t flit_10;

    /* flit 9 */
    union {
        struct aq_rx_dma_cmds_flit_t flit_9;
        struct rqcb2_t rqcb2;
    };
     
    /* flit 8 */
    union {
        struct aq_rx_dma_cmds_flit_t flit_8;
        struct rqcb1_t rqcb1;
    };
     
    /* flit 7 */
    union {
        struct aq_rx_dma_cmds_flit_t flit_7;
        struct rqcb0_t rqcb0;
    };

    /* flit 6 */
    union {
        struct {
            async_int_assert_data : 32;
            struct eqwqe_t async_eqwqe;
        };
        struct {
            int_assert_data : 32;
            struct eqwqe_t eqwqe;
        };
        wakeup_dpath_data   : 64;
    };
    struct cqe_t cqe;
    busy     : 8;
    error    : 8;
    pad6     : 176;

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
