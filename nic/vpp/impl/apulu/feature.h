//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_FEATURE_H__
#define __VPP_IMPL_FEATURE_H__

static inline uint8_t
pds_flow_age_supported (void)
{
    // we need this to resolved at compile time as this function gets called
    // per packet in flow plugin. Compiler would optimize this function call
    // at build time, so that there is no performance penalty per packet.
#ifdef FLOW_AGEING_DISABLE
    return 0;
#else
    // to enable after some more testing
    return 0;
#endif
}

#endif  // __VPP_IMPL_FEATURE_H__
