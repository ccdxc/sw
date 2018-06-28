// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __CRC_FAST_HPP__
#define __CRC_FAST_HPP__

#include "nic/include/base.hpp"
#include "nic/include/hal_lock.hpp"

namespace hal {
namespace utils {

typedef uint32_t crc;

class crcFast {
public:
    static crcFast *factory(uint8_t num_polys, bool thread_safe=false);
    static void destroy(crcFast *crc);
    hal_ret_t init_poly(uint8_t poly_index, uint32_t poly);
    crc compute_crc(uint8_t const message[], int nBytes,
                    uint8_t poly_index);

private:
    hal_spinlock_t    slock_;          // lock for thread safety
    bool              thread_safe_;    // TRUE if thread safety is needed
    uint8_t           num_polys_;
    uint32_t          **crcTable_;
    // crc               crcTable[CRC_POLY_MAX][256];

    crcFast() {};
    ~crcFast();
    hal_ret_t init(uint8_t num_polys, bool thread_safe);

    void lock_(void) {
        if (thread_safe_) {
          HAL_SPINLOCK_LOCK(&slock_);
        }
    }

    void unlock_(void) {
        if (thread_safe_) {
          HAL_SPINLOCK_UNLOCK(&slock_);
        }
    }

};

}   // namespace utils
}   // namespace hal
#endif    //__CRC_FAST_HPP__
