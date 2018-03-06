// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PACK_BYTES_H__
#define __PACK_BYTES_H__

#include "nic/include/base.h"

namespace hal {
namespace utils {
    void pack_bytes_pack(uint8_t *bytes, uint32_t start, uint32_t width, uint64_t val);
    uint64_t pack_bytes_unpack(uint8_t *bytes, uint32_t start, uint32_t width);
}
}
#endif //__PACK_BYTES_H__
