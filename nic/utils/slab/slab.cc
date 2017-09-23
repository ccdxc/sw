#include <string.h>
#include <stdlib.h>
#include <slab.hpp>
#include <hal_mem.hpp>
#include <periodic.hpp>

namespace hal {
namespace utils {

bool slab::g_delay_delete = true;

//------------------------------------------------------------------------------
// private function to allocate and initialize a new block
//------------------------------------------------------------------------------
slab_block_t *
slab::alloc_block_(void)
{
    slab_block_t        *block;
    uint8_t             *ptr;

    block = (slab_block_t *)HAL_MALLOC(HAL_MEM_ALLOC_LIB_SLAB, raw_block_sz_);
    if (block == NULL) {
        return NULL;
    }
    block->prev_ = block->next_ = NULL;
    block->free_head_ = block->elems_;
    block->num_in_use_ = 0;

    ptr = block->elems_;
    for (uint32_t i = 0; i < elems_per_block_ - 1; i++) {
        *(uint32_t **)ptr = (uint32_t *)(ptr + elem_sz_);
        ptr += elem_sz_;
    }
    *(uint32_t **)ptr = NULL;
    this->num_blocks_++;

    return block;
}

//------------------------------------------------------------------------------
// slab instance initialization
//------------------------------------------------------------------------------
int
slab::init(const char *name, hal_slab_t slab_id, uint32_t elem_sz,
           uint32_t elems_per_block, bool thread_safe, bool grow_on_demand,
           bool delay_delete, bool zero_on_alloc)
{
    if (thread_safe) {
        HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }
    this->thread_safe_ = thread_safe;

    memset(this->name_, 0, sizeof(this->name_));
    strncpy(this->name_, name, SLAB_NAME_MAX_LEN);
    this->slab_id_ = slab_id;
    this->elem_sz_ = (elem_sz + 3) & ~0x03;
    this->elems_per_block_ = elems_per_block;
    this->raw_block_sz_ = (elem_sz_ * elems_per_block_) + sizeof(slab_block_t);
    this->grow_on_demand_ = grow_on_demand;
    this->delay_delete_ = delay_delete;
    if (delay_delete && slab::g_delay_delete && (slab_id == HAL_SLAB_RSVD)) {
        return -1;
    }
    this->zero_on_alloc_ = zero_on_alloc;

    this->num_in_use_ = 0;
    this->num_allocs_ = 0;
    this->num_frees_ = 0;
    this->num_alloc_fails_ = 0;
    this->num_blocks_ = 0;

    this->block_head_ = alloc_block_();
    if (this->block_head_ == NULL) {
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
slab *
slab::factory(const char *name, hal_slab_t slab_id,
              uint32_t elem_sz, uint32_t elems_per_block,
              bool thread_safe, bool grow_on_demand,
              bool delay_delete, bool zero_on_alloc)
{
    int     rv;
    slab    *new_slab = NULL;

    if ((elems_per_block <= 1) || (elem_sz < 8)) {
        return NULL;
    }

    new_slab = new slab();
    if (new_slab == NULL) {
        return NULL;
    }
    rv = new_slab->init(name, slab_id, elem_sz, elems_per_block, thread_safe,
                        grow_on_demand, delay_delete, zero_on_alloc);
    if (rv < 0) {
        delete new_slab;
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
        HAL_SPINLOCK_LOCK(&slock_);
    }
    block = this->block_head_;
    while (block) {
        this->block_head_ = block->next_;
        HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, block);
        block = this->block_head_;
    }
    if (thread_safe_) {
        HAL_SPINLOCK_DESTROY(&slock_);
    }
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
        HAL_SPINLOCK_LOCK(&slock_);
    }

    block = this->block_head_;
    while (block && block->free_head_  == NULL) {
        block = block->next_;
    }

    // allocate a new block if all blocks are fully utilized
    if (block == NULL) {
        if (grow_on_demand_) {
            block = alloc_block_();
            if (block) {
                block->next_ = this->block_head_;
                this->block_head_->prev_ = block;
                this->block_head_ = block;
            } else {
                goto cleanup;
            }
        } else {
            goto cleanup;
        }
    }

    elem = block->free_head_;
    block->free_head_ = *(uint32_t **)elem;
    this->num_allocs_++;
    this->num_in_use_++;
    block->num_in_use_++;
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&slock_);
    }

    if (this->zero_on_alloc_) {
        memset(elem, 0, this->elem_sz_);
    }
    return elem;

cleanup:

    this->num_alloc_fails_++;
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&slock_);
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
        if ((block->num_in_use_ == 0) && grow_on_demand_ && block->prev_) {
            // except 1st block, we can release all other blocks while shrinking
            block->prev_->next_ = block->next_;
            if (block->next_) {
                block->next_->prev_ = block->prev_;
            }
            HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, block);
            this->num_blocks_--;
        }
    } else {
        // this elem doesn't belong to any of this slab's active blocks
        HAL_ASSERT(FALSE);
    }
}

//------------------------------------------------------------------------------
// free an object
//------------------------------------------------------------------------------
void
slab::free(void *elem)
{
    if (delay_delete_ && g_delay_delete) {
        hal::periodic::delay_delete_to_slab(slab_id_, elem);
    } else {
        if (thread_safe_) {
            HAL_SPINLOCK_LOCK(&slock_);
        }
        free_(elem);
        if (thread_safe_) {
            HAL_SPINLOCK_UNLOCK(&slock_);
        }
    }
}

}    // namespace hal
}    // namespace utils
