// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PAL_INTERNAL_HPP__
#define __PAL_INTERNAL_HPP__

namespace sdk {
namespace lib {

typedef struct pal_rwvectors_s {
    pal_ret_t   (*reg_read)(uint64_t addr,
                            uint32_t *data,
                            uint32_t num_words);
    pal_ret_t   (*reg_write)(uint64_t addr,
                             uint32_t *data,
                             uint32_t num_words);
    pal_ret_t   (*mem_read)(uint64_t addr, uint8_t *data,
                            uint32_t size);
    pal_ret_t   (*mem_write)(uint64_t addr, uint8_t *data,
                             uint32_t size);
    pal_ret_t   (*ring_doorbell)(uint64_t addr, uint64_t data);
    pal_ret_t   (*step_cpu_pkt)(const uint8_t* pkt, size_t pkt_len);
} __PACK__ pal_rwvectors_t;

typedef struct pal_info_s {
    bool                sim;
    pal_rwvectors_t    rwvecs;
} __PACK__ pal_info_t;

typedef struct pal_mmap_regions_s {
    uint64_t        phy_addr_base;
    uint64_t        size;
    uint64_t        virtual_addr_base;
} __PACK__ pal_mmap_regions_t;

pal_ret_t pal_init_sim(void);
pal_ret_t pal_hw_init(void);

}    // namespace lib
}    // namespace sdk

#endif // __PAL_INTERNAL_HPP__

