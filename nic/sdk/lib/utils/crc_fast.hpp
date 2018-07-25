// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __CRC_FAST_HPP__
#define __CRC_FAST_HPP__

#include "sdk/base.hpp"
#include "sdk/lock.hpp"

namespace sdk {
namespace utils {

typedef uint32_t crc;

class crcFast {
public:
    static crcFast *factory(uint8_t num_polys, bool thread_safe=false);
    static void destroy(crcFast *crc);
    sdk_ret_t init_poly(uint8_t poly_index, uint32_t poly);
    crc compute_crc(uint8_t const message[], int nBytes,
                    uint8_t poly_index);

private:
    sdk_spinlock_t    slock_;          // lock for thread safety
    bool              thread_safe_;    // TRUE if thread safety is needed
    uint8_t           num_polys_;
    uint32_t          **crcTable_;
    // crc               crcTable[CRC_POLY_MAX][256];

    crcFast() {};
    ~crcFast();
    sdk_ret_t init(uint8_t num_polys, bool thread_safe);

    void lock_(void) {
        if (thread_safe_) {
          SDK_SPINLOCK_LOCK(&slock_);
        }
    }

    void unlock_(void) {
        if (thread_safe_) {
          SDK_SPINLOCK_UNLOCK(&slock_);
        }
    }

};

}   // namespace utils
}   // namespace sdk
#endif    //__CRC_FAST_HPP__
