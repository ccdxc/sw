// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "crc_fast.hpp"
#include "include/sdk/mem.hpp"

namespace sdk {
namespace utils {

#define WIDTH           (8 * sizeof(crc))
#define TOPBIT          (1 << (WIDTH - 1))

class crcFast {
public:
    crc compute(uint8_t const message[], int nBytes,
                   uint8_t poly_index);
    crcFast();
private:
    uint8_t  num_polys_;
    uint32_t **crcTable_;
    sdk_ret_t init_(void);
    sdk_ret_t init_poly_(uint8_t poly_index, uint32_t poly);
};


static uint32_t g_crc32_polynomials[] = {
    [CRC32_POLYNOMIAL_TYPE_CRC32]  = 0x04C11DB7,
    [CRC32_POLYNOMIAL_TYPE_CRC32C] = 0x1EDC6F41,
    [CRC32_POLYNOMIAL_TYPE_CRC32K] = 0x741B8CD7,
    [CRC32_POLYNOMIAL_TYPE_CRC32Q] = 0x814141AB,
};

static crcFast g_crcFast;

//
// Wrapper around the global object compute method
//
crc crc32 (uint8_t const message[], int nBytes, uint8_t poly_index)
{
    return g_crcFast.compute(message, nBytes, poly_index);
}

//------------------------------------------------------------------------------
// constructor poluates the tables
//------------------------------------------------------------------------------
crcFast::crcFast() {
    num_polys_ = CRC32_POLYNOMIAL_TYPE_MAX;

    // allocate memory for crc Table
    crcTable_ = (uint32_t **)SDK_CALLOC(SDK_MEM_ALLOC_LIB_CRCFAST,
                                        num_polys_ * sizeof(uint32_t *));
    assert(crcTable_ != NULL);
    // for (int i = 0; i < CRC_POLY_MAX; i++) {
    for (int i = 0; i < num_polys_; i++) {
        crcTable_[i] = (uint32_t *)SDK_CALLOC(SDK_MEM_ALLOC_LIB_CRCFAST,
                                              256 * sizeof(crc));
        assert(crcTable_[i] != NULL);
        init_poly_(i, g_crc32_polynomials[i]);
    }
}

//------------------------------------------------------------------------------
// initialize the polynomial
//------------------------------------------------------------------------------
sdk_ret_t
crcFast::init_poly_(uint8_t poly_index, uint32_t poly)
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
crcFast::compute(uint8_t const message[], int nBytes, uint8_t poly_index)
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
