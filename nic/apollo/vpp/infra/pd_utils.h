//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Contains platform/pipeline related utilities
//------------------------------------------------------------------------------

#ifndef __VPP_INFRA_PD_UTILS_H__
#define __VPP_INFRA_PD_UTILS_H__

#include <nic/sdk/include/sdk/platform.hpp>

#ifdef APULU
#define PDS_PIPELINE "apulu"
#include <gen/p4gen/apulu/include/p4pd.h>
#elif APOLLO
#define PDS_PIPELINE "apollo"
#include <gen/p4gen/apollo/include/p4pd.h>
#elif ARTEMIS
#define PDS_PIPELINE "artemis"
#include <gen/p4gen/artemis/include/p4pd.h>
#endif

#endif    // __VPP_INFRA_PD_UTILS_H__

