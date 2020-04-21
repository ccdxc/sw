#ifndef __ELBA_CPDC_HPP__
#define __ELBA_CPDC_HPP__

#include "gen/proto/types.pb.h"
#include "nic/include/base.hpp"
#include "platform/capri/capri_cfg.hpp"

namespace hal {
namespace pd {

/*
 * HW descriptor info
 */
typedef struct {
    uint64_t    src;
    uint64_t    dst;
    uint16_t    cmd;
    uint16_t    datain_len;
    uint16_t    extended_len;
    uint16_t    threshold_len;
    uint64_t    status_addr;
    uint64_t    db_addr;
    uint64_t    db_data;
    uint64_t    otag_addr;
    uint32_t    otag_data;
    uint32_t    status_data;
} __attribute__((packed)) cpdc_descriptor_t;

#define ELBA_CPDC_INT_AXI_ERR                  (1 << 0)
#define ELBA_CPDC_INT_ECC_ERR                  (1 << 1)
#define ELBA_CPDC_INT_UENG_ERR                 (1 << 2)
#define ELBA_CPDC_INT_CP_DONE                  (1 << 3)
#define ELBA_CPDC_INT_FIFO_OVFL                (1 << 4)

#define ELBA_CPDC_INT_ERR_LOG_MASK             \
    (ELBA_CPDC_INT_AXI_ERR      |              \
     ELBA_CPDC_INT_ECC_ERR      |              \
     ELBA_CPDC_INT_FIFO_OVFL)

#define ELBA_CPDC_INT_ECC_CORRECTABLE          (0xffff << 0)
#define ELBA_CPDC_INT_ECC_UNCORRECTABLE        (0xffff << 16)

#define ELBA_CPDC_INT_ECC_LOG_MASK             \
    (ELBA_CPDC_INT_ECC_UNCORRECTABLE)

#define ELBA_CPDC_INT_AXI_UENG_AXI_ERR         (0xffff << 0)
#define ELBA_CPDC_INT_AXI_UENG_TIMEOUT         (0xffff << 16)

#define ELBA_CPDC_AXI_ERR_W0_LOG_MASK          \
    (0)
    
#define ELBA_CPDC_INT_AXI_DIST_AXI_ERR         (1 << 0)
#define ELBA_CPDC_INT_AXI_DIST_TIMEOUT         (1 << 1)
#define ELBA_CPDC_INT_AXI_AXI_BR_ERR           (1 << 2)
#define ELBA_CPDC_INT_AXI_AXI_DR_ERR           (1 << 3)

#define ELBA_CPDC_AXI_ERR_W1_LOG_MASK          \
    (ELBA_CPDC_INT_AXI_DIST_AXI_ERR      |     \
     ELBA_CPDC_INT_AXI_DIST_TIMEOUT)
     
}    // namespace pd
}    // namespace hal

#endif /*  __ELBA_CPDC_HPP__ */
