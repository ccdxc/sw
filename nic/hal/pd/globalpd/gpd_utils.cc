// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "gpd_utils.hpp"

using namespace hal;

namespace hal {
namespace pd {

uint8_t *
memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

}   // namespace pd
}   // namespace hal
