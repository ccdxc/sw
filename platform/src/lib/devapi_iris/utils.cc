//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/devapi_iris/utils.hpp"

namespace iris {

bool is_multicast(uint64_t mac) {
    return ((mac & 0x010000000000) == 0x010000000000);
}

} // namepsace iris

