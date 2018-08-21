// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "crc_fast.hpp"
#include "include/sdk/mem.hpp"

namespace sdk {
namespace utils {

#define WIDTH           (8 * sizeof(crc))
#define TOPBIT          (1 << (WIDTH - 1))
#define CRC_POLY_MAX    5

//------------------------------------------------------------------------------
// initialize an instance of bitmap class
//------------------------------------------------------------------------------
sdk_ret_t
crcFast::init(uint8_t num_polys, bool thread_safe)
{
    thread_safe_ = thread_safe;
    if (thread_safe_) {
        SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }

    num_polys_ = num_polys;

    // allocate memory for crc Table
    crcTable_ = (uint32_t **)SDK_CALLOC(SDK_MEM_ALLOC_LIB_CRCFAST,
                                        num_polys * sizeof(uint32_t *));
    if (crcTable_ == NULL) {
        return SDK_RET_OOM;
    }
    for (int i = 0; i < CRC_POLY_MAX; i++) {
        crcTable_[i] = (uint32_t *)SDK_CALLOC(SDK_MEM_ALLOC_LIB_CRCFAST,
                                              256 * sizeof(crc));
        if (crcTable_[i] == NULL) {
            return SDK_RET_OOM;
        }
    }

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// factory method for this class
//------------------------------------------------------------------------------
crcFast *
crcFast::factory(uint8_t num_polys, bool thread_safe)
{
    sdk_ret_t    ret;
    void         *mem;
    crcFast      *new_crc;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_CRCFAST, sizeof(crcFast));
    if (mem == NULL) {
        return NULL;
    }
    new_crc = new (mem) crcFast();

    ret = new_crc->init(num_polys, thread_safe);
    if (ret != SDK_RET_OK) {
        new_crc->~crcFast();
        SDK_FREE(SDK_MEM_ALLOC_LIB_CRCFAST, mem);
        return NULL;
    }

    return new_crc;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
crcFast::~crcFast()
{
    if (thread_safe_) {
        SDK_SPINLOCK_LOCK(&slock_);
    }
    if (crcTable_) {
        SDK_FREE(SDK_MEM_ALLOC_LIB_CRCFAST, crcTable_);
    }
    if (thread_safe_) {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
}

//------------------------------------------------------------------------------
// initialize the polynomial
//------------------------------------------------------------------------------
sdk_ret_t
crcFast::init_poly(uint8_t poly_index, uint32_t poly)
{
    crc  remainder;

    if (poly_index >= num_polys_) {
        return SDK_RET_OOB;
    }

    // Compute the remainder of each possible dividend.
    for (int dividend = 0; dividend < 256; ++dividend) {
        // Start with the dividend followed by zeros.
        remainder = dividend << (WIDTH - 8);

        // Perform modulo-2 division, a bit at a time.
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            // Try to divide the current data bit.
            if (remainder & TOPBIT) {
                remainder = (remainder << 1) ^ poly;
            }
            else {
                remainder = (remainder << 1);
            }
        }

        // Store the result into the table.
        crcTable_[poly_index][dividend] = remainder;
    }

    return SDK_RET_OK;
}

crc
crcFast::compute_crc(uint8_t const message[], int nBytes, uint8_t poly_index)
{
    uint8_t data;
    crc remainder = 0;

    // Divide the message by the polynomial, a byte at a time.
    for (int byte = 0; byte < nBytes; ++byte) {
        data = message[byte] ^ (remainder >> (WIDTH - 8));
        remainder = crcTable_[poly_index][data] ^ (remainder << 8);
    }

    // The final remainder is the CRC.
    return (remainder);

}

}   // namespace utils
}   // namespace sdk
