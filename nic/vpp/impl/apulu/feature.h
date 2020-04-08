//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_FEATURE_H__
#define __VPP_IMPL_FEATURE_H__

#include <stdbool.h>

static inline bool
pds_flow_age_supported (void)
{
    // we need this to resolved at compile time as this function gets called
    // per packet in flow plugin. Compiler would optimize this function call
    // at build time, so that there is no performance penalty per packet.
#ifdef FLOW_AGEING_DISABLE
    return false;
#else
    return true;
#endif
}

#endif  // __VPP_IMPL_FEATURE_H__
