// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_UTILS_HPP__
#define __SDK_UTILS_HPP__

#include "sdk/catalog.hpp"

namespace sdk {
namespace lib {

uint16_t set_bits_count(uint64_t mask);
int ffs_msb(uint64_t mask);

}
}

#endif // __SDK_UTILS_HPP__

