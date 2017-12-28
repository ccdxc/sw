// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PAL_HPP__
#define __PAL_HPP__

#include "sdk/base.hpp"

namespace sdk {
namespace lib {

typedef enum pal_ret_s {
    PAL_RET_OK,
    PAL_RET_NOK,
} pal_ret_t;

#define IS_PAL_API_SUCCESS(_ret) ((_ret) == sdk::lib::PAL_RET_OK)
#define IS_PAL_API_FAILURE(_ret) ((_ret) != sdk::lib::PAL_RET_OK)

pal_ret_t pal_init(bool sim);
pal_ret_t pal_reg_read(uint64_t addr, uint32_t *data);
pal_ret_t pal_reg_write(uint64_t addr, uint32_t data);
pal_ret_t pal_mem_read(uint64_t addr, uint8_t *data, uint32_t size);
pal_ret_t pal_mem_write(uint64_t addr, uint8_t *data, uint32_t size);
pal_ret_t pal_ring_doorbell(uint64_t addr, uint64_t data);
pal_ret_t pal_step_cpu_pkt(const uint8_t* pkt, size_t pkt_len);

}    // namespace lib
}    // namespace sdk

#endif // __PAL_HPP__

