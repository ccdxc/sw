// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __RING_HPP__
#define __RING_HPP__

#include "platform/utils/mpartition.hpp"

namespace sdk {
namespace platform {

#define BITS_IN_BYTE    8
// ring entry of 8 bytes
#define DEFAULT_SLOT_SIZE 8

#define RING_SEM_CI_BATCH_SIZE  1

#define RING_MSG_VALID_BIT_MASK   \
    (1ULL << ((sizeof(uint64_t) * BITS_IN_BYTE) - 1))


#define is_ring_type_send(meta) 0 // Only CPU consume/rx ring type supported currently

#define MAX_DWORD_RING_MSG  1
#define RING_MSG_MAX_BATCH_SIZE 64
#define MAX_DWORD_RING_MSG_BATCH    (MAX_DWORD_RING_MSG * RING_MSG_MAX_BATCH_SIZE)
typedef struct ring_msg_batch_s {
    uint64_t dword[MAX_DWORD_RING_MSG_BATCH];
    uint16_t msg_cnt;
} ring_msg_batch_t;

typedef struct ring_meta_s {
    std::string ring_name;
    bool        is_global;
    std::string hbm_reg_name;
    std::string obj_hbm_reg_name;
    uint32_t    num_slots;
    uint32_t    slot_size_in_bytes;
    uint32_t    obj_size;
    uint64_t    alloc_semaphore_addr;
    bool        init_slots;
    uint32_t    ring_types_in_region;
    uint32_t    ring_type_offset;
    // Max queues/rings within this ring region
    uint32_t    max_rings;
    // memory map this ring so we have a virt address for this process to
    // directly access
    bool        mmap_ring;

} ring_meta_t;

class ring {
public:
    ring() {};
    ~ring() {};
    sdk_ret_t init(ring_meta_t *meta, mpartition *mpartition);
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
    static ring_meta_t *find_ring(std::string name, int num_rings,
            ring_meta_t *ring_meta) {
        for (int i = 0; i < num_rings; i++) {
            if (name.compare(ring_meta[i].ring_name) == 0) {
                return ring_meta;
            }
        }
        return NULL;
    }
    // Poll the ring and return batch of msgs
    sdk_ret_t poll(ring_msg_batch_t *msg_batch);

private:
    ring_meta_t     meta_;
    mpartition      *mpartition_;
    uint64_t        base_addr_;
    uint64_t        obj_base_addr_;
    uint64_t        virt_base_addr_;
    uint64_t        virt_obj_base_addr_;
    uint64_t        slot_addr_;
    uint64_t        virt_slot_addr_; //mmap'ed virtual address of the ring slot
    uint64_t        valid_bit_val_;
    uint64_t        sem_addr_;
    uint32_t        prod_cons_idx_; // Consumer index in case of RXQ
    uint16_t        slot_size_; //bytes
    uint16_t        ring_size_shift_;
    inline bool is_valid_msg_elem(uint64_t msg_data)
    {
        if ((msg_data > 0) && (msg_data & RING_MSG_VALID_BIT_MASK) == valid_bit_val_) {
            return true;
        }
        return false;
    }
    inline void move_to_next_elem();
};

} // namespace platform
} // namespace sdk

#endif    // __RING_HPP__
