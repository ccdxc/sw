// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <arpa/inet.h>
#include "lib/pal/pal.hpp"
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

sdk_ret_t
ring::init(ring_meta_t *meta, mpartition *mpartition) {
    uint32_t reg_size;
    uint32_t required_size;

    meta_ = *meta;
    mpartition_ = mpartition;
    if (!meta_.ring_types_in_region) {
        meta_.ring_types_in_region = 1;
    }
    if (!meta_.slot_size_in_bytes) {
        meta_.slot_size_in_bytes = DEFAULT_SLOT_SIZE;
    }
    if (meta_.is_global) {
        meta_.max_rings = 1;
    }

    SDK_ASSERT(meta_.max_rings);

    base_addr_ = mpartition_->start_addr(meta_.hbm_reg_name.c_str());
    SDK_ASSERT(base_addr_ != INVALID_MEM_ADDRESS);
    if (meta_.obj_size) {
        obj_base_addr_ = mpartition_->start_addr(meta_.obj_hbm_reg_name.c_str());
        SDK_ASSERT(obj_base_addr_ != INVALID_MEM_ADDRESS);
    }
    
    reg_size = mpartition_->size(meta_.hbm_reg_name.c_str());
    required_size = meta_.num_slots * meta_.slot_size_in_bytes *
        meta_.ring_types_in_region * meta_.max_rings;
    SDK_ASSERT(reg_size >= required_size);

    if (meta_.init_slots) {
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
   
     
    if (sdk::lib::gl_pal_info.platform_type ==
            platform_type_t::PLATFORM_TYPE_HW) {
        virt_base_addr_= (uint64_t)sdk::lib::pal_mem_map(base_addr_,
                                           meta_.num_slots *
                                           meta_.slot_size_in_bytes);

         // If the object ring is present, lets memory-map the object memory
         // region too.
         if (meta_.obj_size) {
             virt_obj_base_addr_ = (uint64_t)sdk::lib::pal_mem_map(
                     obj_base_addr_, meta_.num_slots * meta_.obj_size);
            if (!virt_obj_base_addr_) {
                SDK_TRACE_ERR("Failed to mmap the OBJ Ring:%s",
                   meta_.hbm_reg_name.c_str());
                return SDK_RET_NO_RESOURCE;
            } else {
                SDK_TRACE_ERR("mmap the OBJ Ring %d phy {:#x} @ virt {:#x}",
                   obj_base_addr_, virt_obj_base_addr_);
            }
        }
    }

    return SDK_RET_OK;
}

} // namespace platform
} // namespace sdk
