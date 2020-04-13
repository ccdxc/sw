// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <arpa/inet.h>
#include "lib/pal/pal.hpp"
#include "include/sdk/base.hpp"
#include "asic/cmn/asic_common.hpp"
#include "asic/rw/asicrw.hpp"
#include "ring.hpp"
#include <boost/multiprecision/detail/integer_ops.hpp>

using sdk::asic::asic_mem_read;
using sdk::asic::asic_mem_write;
using sdk::asic::asic_reg_write;

namespace sdk {
namespace platform {

static inline bool is_cpu_zero_copy_enabled()
{
    return (sdk::lib::gl_pal_info.platform_type ==
            platform_type_t::PLATFORM_TYPE_HW);
}


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

    /*
     * prod_cons_idx is used only for P4+ to CPU rings.
     * In case CPU is producer, prod_idx may be init to zero.
     * In case CPU is consumer, cons_idx needs to be init to num_slots.
     * With P4+ being the producer, it detects ring full if prod + 1 == cons.
     * Since prod and cons are continuosly increasing U32 numbers,
     * and do not wrap around on ring size, for detection of ring full,
     * we need to init cons this way
     */
    if (is_ring_type_send(meta)) {
        prod_cons_idx_ = 0;
    } else {
        prod_cons_idx_ = meta->num_slots;
    }

    sem_addr_ = 0;
    if (meta_.alloc_semaphore_addr &&
            ASIC_SEM_RAW_IS_PI_CI(meta_.alloc_semaphore_addr)) {
        uint32_t val32;

        // Set CI = ring size
        SDK_TRACE_DEBUG("Setting ring semaphore ci to %d", meta_.num_slots);
        val32 = meta_.num_slots;
        asic_reg_write(meta_.alloc_semaphore_addr +
                ASIC_SEM_INC_NOT_FULL_CI_OFFSET, &val32);

        // Set PI to 0
        val32 = 0;
        asic_reg_write(meta_.alloc_semaphore_addr, &val32);

        sem_addr_ = meta_.alloc_semaphore_addr;
    }
   
    ring_size_shift_ = log2(meta_.num_slots);
    slot_addr_ = base_addr_;
    slot_size_ = meta_.slot_size_in_bytes ;
     
    if  (is_cpu_zero_copy_enabled()) {
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

        virt_slot_addr_ = virt_base_addr_;
    }

    return SDK_RET_OK;
}

inline void ring::move_to_next_elem()
{
    uint32_t slot_index;

    prod_cons_idx_++;

    slot_index = prod_cons_idx_ % (1ULL << ring_size_shift_);
    if (slot_index != 0) {
        slot_addr_ += slot_size_;
        virt_slot_addr_ += slot_size_;
    } else {
        slot_addr_ = base_addr_;
        virt_slot_addr_ = virt_base_addr_;
#if 1
        // Toggle the valid bit on wrap-around
        valid_bit_val_ ^= RING_MSG_VALID_BIT_MASK;
#else
        ring->valid_bit_val =
            (ring->pc_idx - ring->wring_meta->num_slots) &
              (0x1 << ring->ring_size_shift);
        ring->valid_bit_val =
            ring->valid_bit_val << (63 - ring->ring_size_shift);
#endif
    }

    SDK_TRACE_DEBUG("updated pc_index queue: index: {} virt-slot-addr: {:#x} addr: {:#x}: valid_bit: {:#x}",
                    prod_cons_idx_, (uint64_t)virt_slot_addr_,
                    slot_addr_, valid_bit_val_);
    return;
}


sdk_ret_t ring::poll(ring_msg_batch_t *msg_batch)
{
    ring_msg_batch_t *batch = msg_batch;
    uint64_t msg_data, msg_cnt = 0;
    static int sem_write_batch = 0;
    sdk_ret_t ret;

    do {
        if (is_cpu_zero_copy_enabled()) {
            msg_data = *(uint64_t *)virt_slot_addr_;
        } else {
            if ((ret = sdk::asic::asic_mem_read(slot_addr_,
                  (uint8_t *)&msg_data,
                  sizeof(uint64_t), true)) != SDK_RET_OK) {
                SDK_TRACE_ERR("Failed to read the slot msg from the hw");
                // TODO: Is this harmless ? Can we get away without returning failure
                // Returning failure here will cause data loss
                return ret;
            }
        }

        msg_data = ntohll(msg_data);
        if (!is_valid_msg_elem(msg_data)) {
            break;
        }

        // Mask off valid bit
        msg_data &= ~RING_MSG_VALID_BIT_MASK;
        SDK_TRACE_DEBUG("msg_data: {:#x}", msg_data);

        /*
         * ASSUMPTION: Each elem in ring is just one dword.
         * May have to change in future
         */
        batch->dword[msg_cnt] = msg_data;

        /*
         * This loop can be optimized to run only up until a wrap around.
         * So that we dont have to check for wrap around condition per iteration
         * when moving to next element
         */
        move_to_next_elem();

        msg_cnt++;
    } while (msg_cnt < RING_MSG_MAX_BATCH_SIZE);

    /*
     * Update TCP ACTL Q semaphore CI. We do this in a batch to
     * reduce CSR-write overheads.
     */
    if (msg_cnt && !(sem_write_batch++ % RING_SEM_CI_BATCH_SIZE) && sem_addr_) {
        SDK_TRACE_DEBUG("updating CI: addr {:#x}, ci: {}",
		    sem_addr_,
			prod_cons_idx_);
	    if ((ret = sdk::asic::asic_reg_write(sem_addr_, &prod_cons_idx_, 1,
		      ASIC_WRITE_MODE_WRITE_THRU)) != SDK_RET_OK) {
	        SDK_TRACE_ERR("Failed to program CI semaphore");
	        return ret;
	    }
    }

    SDK_TRACE_DEBUG("Process batch: total-msgs {}", msg_cnt);

    batch->msg_cnt = msg_cnt;

    return SDK_RET_OK;
}

} // namespace platform
} // namespace sdk
