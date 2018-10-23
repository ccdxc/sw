// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PAL_INTERNAL_HPP__
#define __PAL_INTERNAL_HPP__

namespace sdk {
namespace lib {

typedef struct pal_mmap_regions_s {
    uint64_t        phy_addr_base;
    uint64_t        size;
    uint64_t        virtual_addr_base;
} __PACK__ pal_mmap_regions_t;

pal_ret_t pal_default_init(void);
pal_ret_t pal_init_sim(void);
pal_ret_t pal_hw_init(void);
pal_ret_t pal_mock_init(void);

}    // namespace lib
}    // namespace sdk

#endif // __PAL_INTERNAL_HPP__

