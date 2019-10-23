// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
 
#ifndef __PDSA_UTIL_HPP__
#define __PDSA_UTIL_HPP__

#include <nbase.h>

namespace pdsa_stub {

constexpr size_t VRF_PREF_LEN = 4;

static inline unsigned long vrfname_2_vrfid (const NBB_BYTE* vrfname, NBB_ULONG len)
{
    assert (len > VRF_PREF_LEN);
    return strtol (((const char*)vrfname)+VRF_PREF_LEN, nullptr, 10);
}

}

#endif
