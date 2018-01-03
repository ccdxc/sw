//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// slab library that provides an object cache for faster alloc/free operations
//------------------------------------------------------------------------------

#ifndef __SDK_SLAB_HPP__
#define __SDK_SLAB_HPP__

#include "sdk/base.hpp"
#include "sdk/lock.hpp"
#include "sdk/mem.hpp"

namespace sdk {
namespace lib {

// slab id servers as unique identifier for a slab instance, this id is not used
// by the library itself
typedef uint32_t slab_id_t;

#define SLAB_NAME_MAX_LEN        16

typedef struct slab_block_s slab_block_t;
struct slab_block_s {
    slab_block_t    *prev_;         // previous block pointer
    slab_block_t    *next_;         // next block pointer
    void            *free_head_;    // pointer to free element in block
    uint32_t        num_in_use_;    // no. of elements in use in this block
    uint8_t         elems_[0];      // memory for block elements
} __PACK__;

class slab {
public:
    // static methods for instantiating and destroying slabs
    static slab *factory(const char *name, slab_id_t slab_id, uint32_t elem_sz,
                         uint32_t elems_per_block, bool thread_safe=false,
                         bool grow_on_demand=true, bool zero_on_alloc=false);
    static void destroy(slab *slb);

    // per instance member functions
    void *alloc(void);
    void free(void *elem);
    uint32_t num_in_use(void) const { return num_in_use_; }
    uint32_t num_allocs(void) const { return num_allocs_; }
    uint32_t num_frees(void) const { return num_frees_; }
    uint32_t num_alloc_fails(void) const { return num_alloc_fails_; }
    uint32_t num_blocks(void) const { return num_blocks_; }

    slab_id_t slab_id(void) const { return slab_id_; }
    const char *slab_name(void) const { return name_; }
    uint32_t elem_sz(void) const { return elem_sz_; }
    uint32_t elems_per_block(void) const { return elems_per_block_; }
    bool thread_safe(void) const { return thread_safe_; }
    bool grow_on_demand(void) const { return grow_on_demand_; }
    //bool delay_delete(void) const { return delay_delete_; }
    bool zero_on_alloc(void) const { return zero_on_alloc_; }

private:
    // slab private state
    sdk_spinlock_t    slock_;
    char              name_[SLAB_NAME_MAX_LEN];
    slab_id_t         slab_id_;
    uint32_t          elem_sz_;
    uint32_t          elems_per_block_;
    uint32_t          raw_block_sz_;
    bool              thread_safe_;
    bool              grow_on_demand_;
    bool              zero_on_alloc_;

    // statistics
    uint32_t          num_in_use_;
    uint32_t          num_allocs_;
    uint32_t          num_frees_;
    uint32_t          num_alloc_fails_;
    uint32_t          num_blocks_;

    // meta data
    slab_block_t      *block_head_;

private:
    slab() {};
    ~slab();
    int init(const char *name, slab_id_t slab_id, uint32_t elem_sz,
             uint32_t elems_per_block, bool thread_safe, bool grow_on_demand,
             bool zero_on_alloc);
    void free_(void *elem);
    slab_block_t *alloc_block_(void);
};

}    // namespace lib
}    // namespace sdk

#endif  // __SDK_SLAB_HPP__

