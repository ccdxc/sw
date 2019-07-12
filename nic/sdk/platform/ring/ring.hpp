// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __RING_HPP__
#define __RING_HPP__

#include "platform/utils/mpartition.hpp"

namespace sdk {
namespace platform {

typedef struct ring_meta_s {
    bool        is_global;
    std::string hbm_reg_name;
    std::string obj_hbm_reg_name;
    uint32_t    num_slots;
    uint32_t    slot_size_in_bytes;
    uint32_t    obj_size;
    uint64_t    alloc_semaphore_addr;
    bool        skip_init_slots;
    uint32_t    ring_types_in_region;
    uint32_t    ring_type_offset;
} ring_meta_t;

class ring {
public:
    ring(ring_meta_t *meta, mpartition *mpartition);
    ~ring() {};
    sdk_ret_t init(void);
    uint64_t get_base_addr(uint32_t qid) {
        SDK_ASSERT(!meta_.is_global);
        return base_addr_ + meta_.ring_type_offset +
            (qid * meta_.num_slots * meta_.slot_size_in_bytes *
             meta_.ring_types_in_region);
    }
    uint64_t get_base_addr(void) {
        SDK_ASSERT(meta_.is_global);
        return base_addr_;
    }
private:
    ring_meta_t     meta_;
    mpartition      *mpartition_;
    uint64_t        base_addr_;
    uint64_t        obj_base_addr_;
};

} // namespace platform
} // namespace sdk

#endif    // __RING_HPP__
