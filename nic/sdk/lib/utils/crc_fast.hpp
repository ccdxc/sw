// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __CRC_FAST_HPP__
#define __CRC_FAST_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/lock.hpp"

namespace sdk {
namespace utils {

typedef uint32_t crc;

typedef enum crc32_polynomial_type_ {
    CRC32_POLYNOMIAL_TYPE_CRC32     = 0,
    CRC32_POLYNOMIAL_TYPE_CRC32C    = 1,
    CRC32_POLYNOMIAL_TYPE_CRC32K    = 2,
    CRC32_POLYNOMIAL_TYPE_CRC32Q    = 3,
    CRC32_POLYNOMIAL_TYPE_MAX,
} crc32_polynomial_type_t;

extern crc crc32(uint8_t const message[], int nBytes, uint8_t poly_index);

}   // namespace utils
}   // namespace sdk
#endif    //__CRC_FAST_HPP__
