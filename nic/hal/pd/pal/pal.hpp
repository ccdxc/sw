#ifndef __PAL_H_
#define __PAL_H_

#include <stddef.h>

#include "nic/include/hal_cfg.hpp"
using namespace hal;

typedef enum pal_ret_s {
    PAL_RET_OK,
    PAL_RET_NOK,
} pal_ret_t;

#define IS_PAL_API_SUCCESS(_ret) ((_ret) == PAL_RET_OK)
#define IS_PAL_API_FAILURE(_ret) ((_ret) != PAL_RET_OK)

typedef struct pal_rwvectors_s {
    pal_ret_t   (*reg_read)(uint64_t addr, uint32_t *data);
    pal_ret_t   (*reg_write)(uint64_t addr, uint32_t data);
    pal_ret_t   (*mem_read)(uint64_t addr, uint8_t *data,
                            uint32_t size);
    pal_ret_t   (*mem_write)(uint64_t addr, uint8_t *data,
                             uint32_t size);
    pal_ret_t   (*ring_doorbell)(uint64_t addr, uint64_t data);
    pal_ret_t   (*step_cpu_pkt)(const uint8_t* pkt, size_t pkt_len);
} pal_rwvectors_t;

typedef struct pal_info_s {
    bool                sim;
    void                *baseaddr;
    int                 devfd;
    pal_rwvectors_t    rwvecs;
} pal_info_t;

pal_ret_t
pal_init(bool sim);

pal_ret_t
pal_init_sim();

pal_ret_t
pal_reg_read(uint64_t addr, uint32_t *data);

pal_ret_t
pal_reg_write(uint64_t addr, uint32_t data);

pal_ret_t
pal_mem_read(uint64_t addr, uint8_t *data, uint32_t size);

pal_ret_t
pal_mem_write(uint64_t addr, uint8_t *data, uint32_t size);

pal_ret_t
pal_ring_doorbell(uint64_t addr, uint64_t data);

pal_ret_t
pal_step_cpu_pkt(const uint8_t* pkt, size_t pkt_len);

#endif // __PAL_H_
