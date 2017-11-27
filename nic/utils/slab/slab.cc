#include <string.h>
#include <stdlib.h>
#include "nic/utils/slab/slab.hpp"
#include "nic/include/hal_mem.hpp"
#include <cxxabi.h>
#include <execinfo.h>
#include <iostream>

namespace hal {

namespace periodic {
hal_ret_t delay_delete_to_slab(hal_slab_t slab_id_, void *elem);
}    // namespace periodic

namespace utils {

bool slab::g_delay_delete = true;

std::string slab_demangle( const char* const symbol )
{
    const std::unique_ptr< char, decltype( &std::free ) > demangled(
            abi::__cxa_demangle( symbol, 0, 0, 0 ), &std::free );
    if( demangled ) {
        return demangled.get();
    }
    else {
        return symbol;
    }
}

// ----------------------------------------------------------------------------
// Prints the 2nd frame in the BT.
// - x -> y -> custom_backtrace
//   - prints the x frame
// ----------------------------------------------------------------------------
void slab_custom_backtrace()
{
    // TODO: replace hardcoded limit?
    void* addresses[ 256 ];
    const int n = ::backtrace( addresses, std::extent< decltype( addresses ) >::value );
    const std::unique_ptr< char*, decltype( &std::free ) > symbols(
            ::backtrace_symbols( addresses, n ), &std::free );
    for( int i = 0; i < n; ++i ) {
        if (i != 2) {
            continue;
        }
        // we parse the symbols retrieved from backtrace_symbols() to
        // extract the "real" symbols that represent the mangled names.
        char* const symbol = symbols.get()[ i ];
        char* end = symbol;
        while( *end ) {
            ++end;
        }
        // scanning is done backwards, since the module name
        // might contain both '+' or '(' characters.
        while( end != symbol && *end != '+' ) {
            --end;
        }
        char* begin = end;
        while( begin != symbol && *begin != '(' ) {
            --begin;
        }

        if( begin != symbol ) {
            // std::cout << std::string( symbol, ++begin - symbol );
            *end++ = '\0';
            std::cout << slab_demangle( begin ) << '+' << end;
        }
        else {
            std::cout << symbol;
        }
        // Revisit: Line number not working.
#if 0
        // For line number
        size_t p = 0;
        while(symbol[p] != '(' && symbol[p] != ' '
                && symbol[p] != 0)
            ++p;
        char syscom[256];
        sprintf(syscom,"addr2line %p -e %.*s", addresses[i], (int)p, symbol);
        //last parameter is the file name of the symbol
        system(syscom);
#endif
        std::cout << std::endl;
    }
}


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
        *(void **)ptr = (ptr + elem_sz_);
        ptr += elem_sz_;
    }
    *(void **)ptr = NULL;
    this->num_blocks_++;

    HAL_TRACE_DEBUG("{}:slab_id:{} Allocated new block:{:#x}", __FUNCTION__, 
                    slab_id_, 
                    (uint64_t)block);
    HAL_TRACE_DEBUG("{}:slab_id:{} free elems:", __FUNCTION__, slab_id_);
    print_free_elem_ptrs_(block);

    return block;
}

void
slab::print_free_elem_ptrs_(slab_block_t *block)
{
    uint64_t                *ptr = NULL;
    // uint64_t            tmp = (uint64_t)NULL;

    if (slab_id_ != 55) {
        return;
    }

    ptr = (uint64_t *)block->free_head_;
    // memcpy(&tmp, ptr, sizeof(uint32_t *));
    HAL_TRACE_DEBUG("{:#x} => ", (uint64_t)ptr);
    // while(tmp != (uint64_t)NULL) {
    while(ptr != NULL) {
        ptr = (uint64_t *)*ptr;
        HAL_TRACE_DEBUG("{:#x} => ", (uint64_t)ptr);
    }
    HAL_TRACE_DEBUG("NULL");
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
    this->elem_sz_ = (elem_sz + 7) & ~0x07;
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
    HAL_TRACE_DEBUG("{}:slab_id:{} block_head:{:#x}", __FUNCTION__, slab_id, 
                    (uint64_t)this->block_head_);
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

    //new_slab = new slab();
    void *mem = HAL_MALLOC(HAL_MEM_ALLOC_LIB_SLAB, sizeof(slab));
    if (mem == NULL) {
        return NULL;
    }
    new_slab = new (mem) slab();
    rv = new_slab->init(name, slab_id, elem_sz, elems_per_block, thread_safe,
                        grow_on_demand, delay_delete, zero_on_alloc);
    if (rv < 0) {
        new_slab->~slab();
        HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, new_slab);
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

void
slab::destroy(slab *slb)
{
    if (!slb) {
        return;
    }
    slb->~slab();
    HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, slb);
}

//------------------------------------------------------------------------------
// allocate an object
//------------------------------------------------------------------------------
void *
slab::alloc(void)
{
    void            *elem = NULL;
    slab_block_t    *block;

    // Uncomment to debug
    // HAL_TRACE_DEBUG("slaballoc:slab_id:{} ", slab_id_);
	// slab_custom_backtrace();
    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&slock_);
    }

    block = this->block_head_;
    while (block && block->free_head_  == NULL) {
        block = block->next_;
    }

    HAL_TRACE_DEBUG("{}:block_head: {:#x}, block: {:#x}", __FUNCTION__, 
                    (uint64_t)this->block_head_, (uint64_t)block);
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
    block->free_head_ = *(void **)elem;
    this->num_allocs_++;
    this->num_in_use_++;
    block->num_in_use_++;


    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&slock_);
    }

    HAL_TRACE_DEBUG("{}:slab_id:{} num_in_use:{}", __FUNCTION__, slab_id_, 
                    block->num_in_use_);
    HAL_TRACE_DEBUG("{}:slab_id:{} free elems:", __FUNCTION__, slab_id_);
    print_free_elem_ptrs_(block);
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

        HAL_TRACE_DEBUG("{}:slab_id:{} free elems:", __FUNCTION__, slab_id_);
        print_free_elem_ptrs_(block);

        HAL_TRACE_DEBUG("{}:slab_id:{} num_in_use:{}, grow_on_demand_:{}, prev:{:#x}, block_head:{:#x}", 
                        __FUNCTION__, slab_id_, 
                        block->num_in_use_, grow_on_demand_, (uint64_t)block->prev_,
                        (uint64_t)this->block_head_);

        if ((block->num_in_use_ == 0) && grow_on_demand_ && block->next_) {
            HAL_TRACE_DEBUG("{}:slab_id:{} Freeing block:{:#x}", __FUNCTION__, 
                            slab_id_, 
                            (uint64_t)block);

            if (block_head_ == block) {
                block_head_ = block->next_;
            } else {
                block->prev_->next_ = block->next_;
                if (block->next_) {
                    block->next_->prev_ = block->prev_;
                }
            }
            HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, block);
            this->num_blocks_--;
            HAL_TRACE_DEBUG("After freeing the block: block_head:{:#x}",
                            (uint64_t)this->block_head_);
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
    // Uncomment to debug
    // HAL_TRACE_DEBUG("slabfree:slab_id:{} ", slab_id_);
	// slab_custom_backtrace();
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
