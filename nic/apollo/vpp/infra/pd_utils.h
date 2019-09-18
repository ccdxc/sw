//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Contains platform/pipeline related utilities
//------------------------------------------------------------------------------

#ifndef __VPP_INFRA_PD_UTILS_H__
#define __VPP_INFRA_PD_UTILS_H__

#include <nic/sdk/include/sdk/platform.hpp>

#ifdef APULU
#define PDS_PLATFORM "apulu"
#include <gen/p4gen/apulu/include/p4pd.h>
#elif APOLLO
#define PDS_PLATFORM "apollo"
#include <gen/p4gen/apollo/include/p4pd.h>
#elif ARTEMIS
#define PDS_PLATFORM "artemis"
#include <gen/p4gen/artemis/include/p4pd.h>
#endif

platform_type_t
pds_get_platform_type(void)
{
    platform_type_t platform_type;
#ifdef __x86_64__
    platform_type = platform_type_t::PLATFORM_TYPE_SIM;
#else
    platform_type = platform_type_t::PLATFORM_TYPE_HW;
#endif
    return platform_type;
}

#endif    // __VPP_INFRA_PD_UTILS_H__

