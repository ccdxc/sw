#pragma once

enum {
    CPUCB_ID_FLOW_MISS = 0,
    CPUCB_ID_FIN,
    CPUCB_ID_RST,
    CPUCB_ID_ALG,
    CPUCB_ID_MAX
};

// Change the FIN and RST QID to CPUCB_ID_FIN and CPUCB_ID_RST once
// we have the pipeline defined
#define HAL_FTE_FIN_QID               0
#define HAL_FTE_RST_QID               0
#define HAL_FTE_FLOW_REL_COPY_QID     CPUCB_ID_ALG
