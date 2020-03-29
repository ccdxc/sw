//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_STUB_SESSION_H__
#define __VPP_IMPL_STUB_SESSION_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t
pds_session_get_timestamp (uint32_t session_id)
{
    ASSERT(0);

    return ~0;
}
#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_STUB_SESSION_H__
