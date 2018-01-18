//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "sdk/mem.hpp"
#include "lib/slab/slab.hpp"

namespace sdk {
namespace lib {

//------------------------------------------------------------------------------
// private function to allocate and initialize a new block
//------------------------------------------------------------------------------
slab_block_t *
slab::alloc_block_(void)
{
    slab_block_t        *block;
    uint8_t             *ptr;

    block = (slab_block_t *)SDK_MALLOC(HAL_MEM_ALLOC_LIB_SLAB, raw_block_sz_);
    if (block == NULL) {
        return NULL;
    }
    block->prev_ = block->next_ = NULL;
    block->free_head_ = block->elems_;
    block->num_in_use_ = 0;

    ptr = block->elems_;
    for (uint32_t i = 0; i < elems_per_block_ - 1; i++) {
        *(void **)ptr = (ptr + elem_sz_);
        ptr += elem_sz_;
    }
    *(void **)ptr = NULL;
    this->num_blocks_++;

    return block;
}

//------------------------------------------------------------------------------
// slab instance initialization
//------------------------------------------------------------------------------
int
slab::init(const char *name, slab_id_t slab_id, uint32_t elem_sz,
           uint32_t elems_per_block, bool thread_safe, bool grow_on_demand,
           bool zero_on_alloc)
{
    if (thread_safe) {
        SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }
    this->thread_safe_ = thread_safe;

    memset(this->name_, 0, sizeof(this->name_));
    strncpy(this->name_, name, SLAB_NAME_MAX_LEN);
    this->slab_id_ = slab_id;
    this->elem_sz_ = (elem_sz + 7) & ~0x07;
    this->elems_per_block_ = elems_per_block;
    this->raw_block_sz_ = (elem_sz_ * elems_per_block_) + sizeof(slab_block_t);
    this->grow_on_demand_ = grow_on_demand;
    this->zero_on_alloc_ = zero_on_alloc;

    this->num_in_use_ = 0;
    this->num_allocs_ = 0;
    this->num_frees_ = 0;
    this->num_alloc_fails_ = 0;
    this->num_blocks_ = 0;

#if 0
    // allocate a block and keep it ready
    this->block_head_ = alloc_block_();
    if (this->block_head_ == NULL) {
        return -1;
    }
#endif

    this->block_head_ = NULL;

    return 0;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
slab *
slab::factory(const char *name, slab_id_t slab_id, uint32_t elem_sz,
              uint32_t elems_per_block, bool thread_safe, bool grow_on_demand,
              bool zero_on_alloc)
{
    int     rv;
    void    *mem;
    slab    *new_slab = NULL;

    if ((elems_per_block <= 1) || (elem_sz < 8)) {
        return NULL;
    }

    mem = SDK_MALLOC(HAL_MEM_ALLOC_LIB_SLAB, sizeof(slab));
    if (mem == NULL) {
        return NULL;
    }

    new_slab = new (mem) slab();
    rv = new_slab->init(name, slab_id, elem_sz, elems_per_block, thread_safe,
                        grow_on_demand, zero_on_alloc);
    if (rv < 0) {
        new_slab->~slab();
        SDK_FREE(HAL_MEM_ALLOC_LIB_SLAB, new_slab);
        return NULL;
    }

    return new_slab;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
slab::~slab()
{
    slab_block_t    *block;

    // free all the blocks allocated so far
    if (thread_safe_) {
        SDK_SPINLOCK_LOCK(&slock_);
    }
    block = this->block_head_;
    while (block) {
        this->block_head_ = block->next_;
        SDK_FREE(HAL_MEM_ALLOC_LIB_SLAB, block);
        block = this->block_head_;
    }
    if (thread_safe_) {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
}

void
slab::destroy(slab *slb)
{
    if (!slb) {
        return;
    }
    slb->~slab();
    SDK_FREE(HAL_MEM_ALLOC_LIB_SLAB, slb);
}

//------------------------------------------------------------------------------
// allocate an object
//------------------------------------------------------------------------------
void *
slab::alloc(void)
{
    void            *elem = NULL;
    slab_block_t    *block;

    if (thread_safe_) {
        SDK_SPINLOCK_LOCK(&slock_);
    }

    block = this->block_head_;
    while (block && block->free_head_  == NULL) {
        block = block->next_;
    }

    // allocate a new block if all blocks are fully utilized
    if (block == NULL) {
        if (grow_on_demand_ || (this->block_head_ == NULL)) {
            block = alloc_block_();
            if (block) {
                block->next_ = this->block_head_;
                if (this->block_head_) {
                    this->block_head_->prev_ = block;
                }
                this->block_head_ = block;
            } else {
                goto cleanup;
            }
        } else {
            goto cleanup;
        }
    }

    elem = block->free_head_;
    block->free_head_ = *(void **)elem;
    this->num_allocs_++;
    this->num_in_use_++;
    block->num_in_use_++;

    if (thread_safe_) {
        SDK_SPINLOCK_UNLOCK(&slock_);
    }

    if (this->zero_on_alloc_) {
        memset(elem, 0, this->elem_sz_);
    }

    return elem;

cleanup:

    this->num_alloc_fails_++;
    if (thread_safe_) {
        SDK_SPINLOCK_UNLOCK(&slock_);
    }
    return NULL;
}

//------------------------------------------------------------------------------
// internal function to free an object
//------------------------------------------------------------------------------
void
slab::free_(void *elem)
{
    slab_block_t    *block;

    // find the block this element belongs to
    block = this->block_head_;
    while (block) {
        if ((elem > block) &&
            (elem < ((uint8_t *)block + raw_block_sz_))) {
            break;
        }
        block = block->next_;
    }

    if (block) {
        *(void **)elem = block->free_head_;
        block->free_head_ = elem;
        this->num_frees_++;
        this->num_in_use_--;
        block->num_in_use_--;

        if ((block->num_in_use_ == 0) && grow_on_demand_ && block->next_) {
            if (block_head_ == block) {
                block_head_ = block->next_;
            } else {
                block->prev_->next_ = block->next_;
                if (block->next_) {
                    block->next_->prev_ = block->prev_;
                }
            }
            SDK_FREE(HAL_MEM_ALLOC_LIB_SLAB, block);
            this->num_blocks_--;
        }
    } else {
        // this elem doesn't belong to any of this slab's active blocks
        SDK_ASSERT(FALSE);
    }
}

//------------------------------------------------------------------------------
// free an object
//------------------------------------------------------------------------------
void
slab::free(void *elem)
{
    if (thread_safe_) {
        SDK_SPINLOCK_LOCK(&slock_);
    }
    free_(elem);
    if (thread_safe_) {
        SDK_SPINLOCK_UNLOCK(&slock_);
    }
}

}    // namespace lib
}    // namespace sdk
