// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_UTILS_HPP__
#define __HAL_UTILS_HPP__

#include "sdk/catalog.hpp"

sdk::lib::catalog* hal_lib_catalog_init (void);

uint16_t hal_lib_set_bits_count (uint64_t mask);

#endif // __HAL_UTILS_HPP__
