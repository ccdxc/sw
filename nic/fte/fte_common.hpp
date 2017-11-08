#pragma once

#include "nic/gen/proto/hal/types.pb.h"

// Change the FIN and RST QID to CPUCB_ID_FIN and CPUCB_ID_RST once
// we have the pipeline defined
#define HAL_FTE_FIN_QID               0
#define HAL_FTE_RST_QID               0 
#define HAL_FTE_FLOW_REL_COPY_QID     types::CPUCB_ID_ALG
