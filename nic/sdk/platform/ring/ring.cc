// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <arpa/inet.h>

#include "include/sdk/base.hpp"
#include "platform/capri/capri_common.hpp"
#include "asic/rw/asicrw.hpp"
#include "ring.hpp"

using sdk::asic::asic_mem_read;
using sdk::asic::asic_mem_write;
using sdk::asic::asic_reg_write;

namespace sdk {
namespace platform {

// ring entry of 8 bytes
#define DEFAULT_SLOT_SIZE 8

ring::ring(ring_meta_t *meta, mpartition *mpartition) {
    meta_ = *meta;
    mpartition_ = mpartition;
    if (!meta_.ring_types_in_region) {
        meta_.ring_types_in_region = 1;
    }
    if (!meta_.slot_size_in_bytes) {
        meta_.ring_types_in_region = DEFAULT_SLOT_SIZE;
    }

    base_addr_ = mpartition_->start_addr(meta_.hbm_reg_name.c_str());
    SDK_ASSERT(base_addr_ != INVALID_MEM_ADDRESS);
    if (meta_.obj_size) {
        obj_base_addr_ = mpartition_->start_addr(meta_.obj_hbm_reg_name.c_str());
        SDK_ASSERT(obj_base_addr_ != INVALID_MEM_ADDRESS);
    }
}

sdk_ret_t
ring::init(void) {
    uint32_t reg_size;
    uint32_t required_size;

    reg_size = mpartition_->size(meta_.hbm_reg_name.c_str());
    required_size = meta_.num_slots * meta_.slot_size_in_bytes *
        meta_.ring_types_in_region;
    SDK_ASSERT(reg_size >= required_size);

    if (!meta_.skip_init_slots) {
        SDK_ASSERT(obj_base_addr_ && meta_.obj_size);
        SDK_TRACE_DEBUG("Initializing ring %s", meta_.hbm_reg_name.c_str());
        for (uint32_t i = 0; i < meta_.num_slots; i++) {
            uint64_t slot_addr = base_addr_ + i * meta_.slot_size_in_bytes;
            uint64_t obj_addr = obj_base_addr_ + i * meta_.obj_size;
            obj_addr = htonll(obj_addr);
            asic_mem_write(slot_addr, (uint8_t *)&obj_addr, meta_.slot_size_in_bytes);
        }
    }

    if (meta_.alloc_semaphore_addr &&
            CAPRI_SEM_RAW_IS_PI_CI(meta_.alloc_semaphore_addr)) {
        uint32_t val32;

        // Set CI = ring size
        SDK_TRACE_DEBUG("Setting ring semaphore ci to %d", meta_.num_slots);
        val32 = meta_.num_slots;
        asic_reg_write(meta_.alloc_semaphore_addr +
                CAPRI_SEM_INC_NOT_FULL_CI_OFFSET, &val32);

        // Set PI to 0
        val32 = 0;
        asic_reg_write(meta_.alloc_semaphore_addr, &val32);
    }


    return SDK_RET_OK;
}

} // namespace platform
} // namespace sdk
