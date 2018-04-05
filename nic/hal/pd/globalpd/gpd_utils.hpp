// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __GPD_UTILS_HPP__
#define __GPD_UTILS_HPP__

#include "nic/include/base.h"

namespace hal {
namespace pd {

#define OPER_TYPES(ENTRY)                         \
    ENTRY(TABLE_OPER_INSERT,    0, "INSERT")      \
    ENTRY(TABLE_OPER_UPDATE,    1, "UPDATE")      \
    ENTRY(TABLE_OPER_REMOVE,    2, "REMOVE")

DEFINE_ENUM(table_oper_t, OPER_TYPES)
#undef OPER_TYPES

// utils
uint8_t *memrev(uint8_t *block, size_t elnum);


}   // namespace pd
}   // namespace hal
#endif    // __GPD_UTILS_HPP__

