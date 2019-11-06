//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <iostream>
#include <rte_memory.h>
#include <rte_common.h>
#include <rte_bitmap.h>
#include "lib/rte_indexer/rte_indexer.hpp"
#include "lib/rte_indexer/rte_slab_op.hpp"

namespace sdk {
namespace lib {

#define INDEXER ((rte_bitmap *)indexer_)

// check if <slab> has continuous <size> bits set starting from <offset>
// return 1 for success, otherwise 0
static inline bool
rte_bitmap_check_slab_set_(rte_bitmap *indxr, uint32_t offset2,
                           uint64_t slab, uint32_t size) {
    uint32_t slab_size;
    uint64_t mask;

    slab_size = RTE_BITMAP_SLAB_BIT_SIZE - offset2;
    if (slab_size > RTE_BITMAP_SLAB_BIT_MASK)
        mask = ~(0llu);
    else {
        mask = ~((~1llu) >> slab_size);
        slab = slab & (~(0llu) << offset2);
    }
    if (size < slab_size) {
        slab_size = size;
        offset2 = RTE_BITMAP_SLAB_BIT_SIZE - (size + offset2);
        mask = mask & (~(1llu) >> offset2);
    }
    slab = slab & mask;
    return (slab == mask);
}

void
rte_indexer::set_curr_slab_(uint64_t pos) {
    rte_bitmap *indxr = (rte_bitmap *)indexer_;

    this->curr_slab_ = *(indxr->array2 + (pos >> RTE_BITMAP_SLAB_BIT_SIZE_LOG2));
}

//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
rte_indexer *
rte_indexer::factory(uint32_t size, bool thread_safe, bool skip_zero) {
    void        *mem   = NULL;
    rte_indexer *indxr = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_INDEXER, sizeof(rte_indexer));
    if (!mem) {
        return NULL;
    }
    indxr = new (mem) rte_indexer();
    if (indxr->init_(size, thread_safe, skip_zero) == false) {
        SDK_TRACE_ERR("Failed to initialize rte_indexer of size %u", size);
        indxr->~rte_indexer();
        if (mem)
            SDK_FREE(SDK_MEM_ALLOC_LIB_INDEXER, indxr);
        return NULL;
    }
    return indxr;
}

//---------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------
rte_indexer::rte_indexer(void) {

}

//---------------------------------------------------------------------------
// destructor
//---------------------------------------------------------------------------
rte_indexer::~rte_indexer(void) {
    if (thread_safe_) {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
    rte_bitmap_free(INDEXER);
}

//---------------------------------------------------------------------------
// method to free & delete the object
//---------------------------------------------------------------------------
void
rte_indexer::destroy(rte_indexer *indxr) {
    if (indxr) {
        indxr->~rte_indexer();
        SDK_FREE(SDK_MEM_ALLOC_LIB_INDEXER, indxr);
    }
}

//---------------------------------------------------------------------------
// rte indexer initialization
//---------------------------------------------------------------------------
bool
rte_indexer::init_(uint32_t size, bool thread_safe, bool skip_zero) {
    uint32_t sz;
    uint32_t indx = 0;
    rte_bitmap *indxr = NULL;
    sdk_ret_t rs = SDK_RET_OK;
    uint32_t array2_64s, array2_lbits;
    uint32_t array1_64s, array1_lbits;
    uint32_t array1_nbits;

    size_ = size;
    thread_safe_ = thread_safe;

    if (thread_safe_) {
        SDK_ASSERT(!SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE));
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), false);
    }

    if (size_) {
        sz = RTE_CACHE_LINE_ROUNDUP(
                    rte_bitmap_get_memory_footprint(size_));
        posix_memalign((void **)&bits_, CACHE_LINE_SIZE, sz);
        if (!bits_) {
            return false;
        }
        indexer_ = rte_bitmap_init(size, bits_, sz);
        indxr = (rte_bitmap *)indexer_;
        // number of 64 bits chunks
        array2_64s = size / RTE_BITMAP_SLAB_BIT_SIZE;
        // number of active bits in the last 64 bit chunk.
        array2_lbits = size % RTE_BITMAP_SLAB_BIT_SIZE;
        // each bits in array1 maps to cacheline of 64 bytes of array2
        // array1_nbits = array2_bits / CACHELINE_BITS  + 1/0(reminder)
        array1_nbits =  (indxr->array2_size * RTE_BITMAP_SLAB_BIT_SIZE) /
                         RTE_BITMAP_CL_BIT_SIZE;
        array1_nbits += (((indxr->array2_size * RTE_BITMAP_SLAB_BIT_SIZE) %
                           RTE_BITMAP_CL_BIT_SIZE) != 0) ? 1 : 0;
        // Number of 64 bits chunks
        array1_64s  =  array1_nbits / RTE_BITMAP_SLAB_BIT_SIZE;
        // Number of active bits in the last 64 bit chunk
        array1_lbits = array1_nbits % RTE_BITMAP_SLAB_BIT_SIZE;

        // Set the memory
        if (array2_64s) {
            memset(indxr->array2, 0xff, array2_64s * sizeof(uint64_t));
        }
        if (array2_lbits) {
            *(indxr->array2 + array2_64s) = (1ULL << array2_lbits) - 1;
        }
        if (array1_64s) {
            memset(indxr->array1, 0xff, array1_64s * sizeof(uint64_t));
        }
        if (array1_lbits) {
            *(indxr->array1 + array1_64s) = (1ULL << array1_lbits) - 1;
        }
        rte_bitmap_set(indxr, 0);
        this->curr_index_ = 0;
        this->curr_slab_ = 0;
    }

    usage_ = 0;

    // skip 0th entry, if asked
    skip_zero_ = skip_zero;
    if (skip_zero_) {
        rs = this->alloc(indx);
        if (rs == SDK_RET_OK) {
            usage_ += 1;
            this->curr_index_ = indx;
            this->set_curr_slab_(indx);
        }
    }
    SDK_TRACE_VERBOSE("Indexer %p slab %lx current index %x", 
                      (void *)INDEXER, this->curr_slab_, this->curr_index_);
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), false);
    }

    if (rs != SDK_RET_OK)
        return false;
    return true;
}


//---------------------------------------------------------------------------
// find first free bit, right next to last allocated bit and return success
//---------------------------------------------------------------------------
sdk_ret_t
rte_indexer::alloc(uint32_t *index) {
    uint32_t nextpos = 0;
    uint64_t curr_slab = 0;
    bool go2_next = false;
    sdk_ret_t rs = SDK_RET_OK;
    rte_bitmap *indxr = (rte_bitmap *)indexer_;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SDK_RET_ERR);
    }

    if (!indexer_ || !size_ || usage_ >= size_) {
        rs = SDK_RET_NO_RESOURCE;
        goto end;
    }

    if (this->curr_slab_) {
        if (!rte_bitmap_slab_scan(this->curr_slab_, this->curr_index_,
                                  &nextpos)) {
            go2_next = true;
        }
    }

    if (this->curr_slab_ == 0 || go2_next) {
        if (!rte_bitmap_scan(indxr, &nextpos, &curr_slab)) {
            rs = SDK_RET_NO_RESOURCE;
            goto end;
        }
        this->curr_index_ = nextpos;
        rte_bsf64_safe(curr_slab, &nextpos);
        nextpos += this->curr_index_;
    }

    *index = nextpos;
    rte_bitmap_clear(INDEXER, *index);
    this->curr_index_ = nextpos;
    this->set_curr_slab_(this->curr_index_);

    SDK_TRACE_VERBOSE("Indexer %p Allocated idx %u and slab %lx curr slab %d curr index %x", 
              (void*)INDEXER, *index, (long)this->curr_slab_, curr_slab, this->curr_index_);
    usage_++;
end:
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SDK_RET_ERR);
    }
    return rs;
}

//---------------------------------------------------------------------------
// allocate a requested id by setting the corresponding bit, if its not set
// already
// if the indexed is already claimed and in use, return
//---------------------------------------------------------------------------
sdk_ret_t
rte_indexer::alloc(uint32_t index) {
    uint64_t sel_word = 0;
    sdk_ret_t rs = SDK_RET_OK;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SDK_RET_ERR);
    }

    if (index >= size_) {
        rs = SDK_RET_OOB;
        goto end;
    }

    sel_word = rte_bitmap_get(INDEXER, index);
    if (!sel_word) {
        rs = SDK_RET_ENTRY_EXISTS;
        goto end;
    }

    rte_bitmap_clear(INDEXER, index);
    if (rte_compare_indexes(this->curr_index_, index))
        this->set_curr_slab_(this->curr_index_);
    usage_++;
end:
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SDK_RET_ERR);
    }
    return rs;
}

//---------------------------------------------------------------------------
// lazy allocation strategy, used exclusively for block/s > 1
// find and allocate given size block and return success
//---------------------------------------------------------------------------
sdk_ret_t
rte_indexer::alloc_block(uint32_t *index, uint32_t size, bool wrap_around) {
    uint32_t free, curr_size, index2, offset2, slab_size;
    uint32_t start, nextpos;
    uint64_t slab2;
    sdk_ret_t rs = SDK_RET_OK;
    rte_bitmap *indxr = (rte_bitmap *)indexer_;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SDK_RET_ERR);
    }

    if (!indexer_ || !size_ || usage_ + size > size_) {
        rs = SDK_RET_NO_RESOURCE;
        goto end;
    }

    free = rte_bitmap_leftmost_slab_scan(indxr, &slab2, &start);
    if (!free) {
        rs = SDK_RET_NO_RESOURCE;
        goto end;
    }

    curr_size = size;
    while(curr_size) {
        offset2 = start & RTE_BITMAP_SLAB_BIT_MASK;
        index2 = start >> RTE_BITMAP_SLAB_BIT_SIZE_LOG2;
        slab2 = *(indxr->array2 + index2);
        slab_size = RTE_BITMAP_SLAB_BIT_SIZE - offset2;
        if (curr_size < slab_size)
            slab_size = curr_size;
        free = rte_bitmap_check_slab_set_(indxr, offset2, slab2, slab_size);
        if (free) {
            start += slab_size;
            curr_size -= slab_size;
        } else if (index2 < indxr->array2_size) {
            if (curr_size < slab_size) {
                // continue with same slab
                if (!rte_bitmap_slab_scan(slab2, start, &nextpos)) {
                    index2 += 1;
                    start = index2 << RTE_BITMAP_SLAB_BIT_SIZE_LOG2;
                } else
                    start = nextpos;
            } else {
                index2 += 1;
                start = index2 << RTE_BITMAP_SLAB_BIT_SIZE_LOG2;
            }
            curr_size = size;
        } else {
            // no slabs found of <size>
            break;
        }
    }

    if (!curr_size) {
        // alloc slabs here
        start -= size;
        rte_bitmap_clear_slabs(indxr, start, size);
        if (rte_compare_indexes(this->curr_index_, start)) {
            this->set_curr_slab_(this->curr_index_);
        }
        usage_ += size;
        *index = start;
    } else {
        rs = SDK_RET_NO_RESOURCE;
    }
end:
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SDK_RET_ERR);
    }
    return rs;
}

//---------------------------------------------------------------------------
// frees the given index, if its in use or else no-op
//---------------------------------------------------------------------------
sdk_ret_t
rte_indexer::free(uint32_t index, uint32_t block_size) {
    sdk_ret_t rs = SDK_RET_OK;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SDK_RET_ERR);
    }

    if (index >= size_) {
        rs = SDK_RET_OOB;
        goto end;
    }

    if (rte_bitmap_get(INDEXER, index)) {
        rs = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
    }

    rte_bitmap_set_slabs(INDEXER, index, block_size);
    // <curr_slab_> contains this <index> then modify
    // <curr_slab_> as well
    if (rte_compare_indexes(this->curr_index_, index)) {
        this->set_curr_slab_(this->curr_index_);
    }
    usage_ -= block_size;
    if (usage_ == 0) {
        this->curr_index_ = 0;
        this->curr_slab_ = 0;
        __rte_bitmap_scan_init(INDEXER);
    }
end:
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SDK_RET_ERR);
    }

    return rs;
}

//---------------------------------------------------------------------------
// return true if index is already allocated
//---------------------------------------------------------------------------
bool
rte_indexer::is_index_allocated(uint32_t index) {
    uint64_t sel_word   = 0;
    bool     is_allocated = false;

    sel_word = rte_bitmap_get(INDEXER, index);
    if (!sel_word) {
        is_allocated = true;
    }

    return is_allocated;
}

//---------------------------------------------------------------------------
// return number of indices allocated so far
//---------------------------------------------------------------------------
uint64_t
rte_indexer::usage(void) const {
    if (this->skip_zero_)
        return (usage_ - 1);
    return usage_;
}

}    // namespace lib
}    // namespace sdk
