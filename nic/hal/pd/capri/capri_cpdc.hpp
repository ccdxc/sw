#ifndef __CAPRI_CPDC_HPP__
#define __CAPRI_CPDC_HPP__

#include "gen/proto/types.pb.h"
#include "nic/include/base.hpp"
#include "platform/capri/capri_cfg.hpp"

namespace hal {
namespace pd {

#define CAPRI_CPDC_INT_AXI_ERR                  (1 << 0)
#define CAPRI_CPDC_INT_ECC_ERR                  (1 << 1)
#define CAPRI_CPDC_INT_UENG_ERR                 (1 << 2)
#define CAPRI_CPDC_INT_CP_DONE                  (1 << 3)
#define CAPRI_CPDC_INT_FIFO_OVFL                (1 << 4)

#define CAPRI_CPDC_INT_ERR_LOG_MASK             \
    (CAPRI_CPDC_INT_AXI_ERR      |              \
     CAPRI_CPDC_INT_ECC_ERR      |              \
     CAPRI_CPDC_INT_FIFO_OVFL)

#define CAPRI_CPDC_INT_ECC_CORRECTABLE          (0xffff << 0)
#define CAPRI_CPDC_INT_ECC_UNCORRECTABLE        (0xffff << 16)

#define CAPRI_CPDC_INT_ECC_LOG_MASK             \
    (CAPRI_CPDC_INT_ECC_UNCORRECTABLE)

#define CAPRI_CPDC_INT_AXI_UENG_AXI_ERR         (0xffff << 0)
#define CAPRI_CPDC_INT_AXI_UENG_TIMEOUT         (0xffff << 16)

#define CAPRI_CPDC_AXI_ERR_W0_LOG_MASK          \
    (0)
    
#define CAPRI_CPDC_INT_AXI_DIST_AXI_ERR         (1 << 0)
#define CAPRI_CPDC_INT_AXI_DIST_TIMEOUT         (1 << 1)
#define CAPRI_CPDC_INT_AXI_AXI_BR_ERR           (1 << 2)
#define CAPRI_CPDC_INT_AXI_AXI_DR_ERR           (1 << 3)

#define CAPRI_CPDC_AXI_ERR_W1_LOG_MASK          \
    (CAPRI_CPDC_INT_AXI_DIST_AXI_ERR      |     \
     CAPRI_CPDC_INT_AXI_DIST_TIMEOUT)
     
}    // namespace pd
}    // namespace hal

#endif /*  __CAPRI_CPDC_HPP__ */
