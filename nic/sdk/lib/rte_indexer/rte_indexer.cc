//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <iostream>
#include <rte_memory.h>
#include <rte_bitmap.h>
#include "lib/rte_indexer/rte_indexer.hpp"
#include "lib/rte_indexer/rte_slab_op.hpp"

namespace sdk {
namespace lib {

#define INDEXER ((rte_bitmap *)indexer_)

//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
rte_indexer *
rte_indexer::factory(uint32_t size, bool thread_safe, bool skip_zero)
{
    void        *mem   = NULL;
    rte_indexer *indxr = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_INDEXER, sizeof(rte_indexer));
    if (!mem) {
        return NULL;
    }
    indxr = new (mem) rte_indexer();
    if (indxr->init(size, thread_safe, skip_zero) == false) {
        SDK_TRACE_ERR("Failed to initialize rte_indexer of size: %d\n", size);
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
rte_indexer::rte_indexer()
{

}

//---------------------------------------------------------------------------
// destructor
//---------------------------------------------------------------------------
rte_indexer::~rte_indexer()
{
    if (thread_safe_) {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
    rte_bitmap_free(INDEXER);
}

//---------------------------------------------------------------------------
// method to free & delete the object
//---------------------------------------------------------------------------
void
rte_indexer::destroy(rte_indexer *indxr)
{
    if (indxr) {
        indxr->~rte_indexer();
        SDK_FREE(SDK_MEM_ALLOC_LIB_INDEXER, indxr);
    }
}

//---------------------------------------------------------------------------
// rte indexer initialization
//---------------------------------------------------------------------------
bool
rte_indexer::init(uint32_t size, bool thread_safe, bool skip_zero)
{
    uint32_t sz;
    uint32_t indx = 0;
    rte_bitmap *indxr = NULL;
    sdk_ret_t rs = SDK_RET_OK;

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
        memset(indxr->array1, RTE_BITMAP_START_SLAB_SCAN_POS,
               (indxr->array1_size * sizeof(uint64_t)));
        memset(indxr->array2, RTE_BITMAP_START_SLAB_SCAN_POS,
               (indxr->array2_size * sizeof(uint64_t)));
        indxr->go2 = 0;
        this->curr_slab_ = 0;
        this->curr_index_ = 0;
    }

    // skip 0th entry, if asked
    skip_zero_ = skip_zero;
    if (skip_zero_) {
        rs = this->alloc(indx, 1);
    }
    usage_ = 0;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), false);
    }

    if (rs != SDK_RET_OK)
        return false;
    return true;
}


//---------------------------------------------------------------------------
// find first free bit, set it the bit and return SUCCESS with the index
//---------------------------------------------------------------------------
sdk_ret_t
rte_indexer::alloc(uint32_t *index, uint32_t block_size)
{
    uint32_t offset2;
    uint32_t  pos = 0, next_pos = 0;
    sdk_ret_t rs = SDK_RET_OK;
    rte_bitmap *indxr = (rte_bitmap *)indexer_;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SDK_RET_ERR);
    }

    if (!indexer_ || !size_ || usage_ >= size_) {
        rs = SDK_RET_NO_RESOURCE;
        goto end;
    }

    if (indxr->go2) {
        if (rte_bitmap_slab_scan(curr_slab_, curr_index_, &next_pos) == 0) {
            // no bit set in current slab, go to next slab
            curr_index_ = RTE_BITMAP_START_SLAB_SCAN_POS;
        } else {
            curr_index_ = next_pos;
        }
    }
    if (curr_index_ == 0 || curr_index_ != next_pos) {
        if (rte_bitmap_scan(indxr, &pos, &curr_slab_) == 0) {
            rs = SDK_RET_NO_RESOURCE;
            goto end;
        }
        curr_index_ = pos;
    }

    // modify curr_slab_ as well
    offset2 = curr_index_ & RTE_BITMAP_SLAB_BIT_MASK;
    curr_slab_ &= ~(1llu << offset2);

    *index = curr_index_;
    rte_bitmap_clear(INDEXER, *index);
    SDK_TRACE_DEBUG("Scan name: curr_pos %u next_pos %u idx %u slab %lx\n",
                    curr_index_, next_pos, *index, (long)curr_slab_);
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
rte_indexer::alloc(uint32_t index, uint32_t block_size)
{
    uint32_t offset2;
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

    // <curr_slab_> contains this <index> then modify
    // <curr_slab_> as well
    if (rte_compare_indexes(curr_index_, index)) {
        offset2 = index & RTE_BITMAP_SLAB_BIT_MASK;
        curr_slab_ &= ~(1llu << offset2);
    }

    rte_bitmap_clear(INDEXER, index);
    usage_++;

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
rte_indexer::free(uint32_t index)
{
    uint32_t offset2 = 0;
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

    // <curr_slab_> contains this <index> then modify
    // <curr_slab_> as well
    if (rte_compare_indexes(curr_index_, index)) {
        offset2 = index & RTE_BITMAP_SLAB_BIT_MASK;
        curr_slab_ |= (1llu << offset2);
    }

    rte_bitmap_set(INDEXER, index);
    usage_--;
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
rte_indexer::is_index_allocated(uint32_t index)
{
    uint64_t    sel_word   = 0;
    bool        is_alloced = false;

    sel_word = rte_bitmap_get(INDEXER, index);
    if (!sel_word) {
        is_alloced = true;
    }

    return is_alloced;
}

//---------------------------------------------------------------------------
// return number of indices allocate so far
//---------------------------------------------------------------------------
uint64_t
rte_indexer::compute_num_indices_allocated(void)
{
	uint32_t	usage = 0;

	for (uint32_t i = 0; i < size_; i++) {
		if (is_index_allocated(i) && !(skip_zero_ && i == 0)) {
            // Don't count 0 if skip_zero is set
			usage++;
		}
	}

    SDK_TRACE_DEBUG("Usage internal : %d Usage computed: %d", usage_, usage);
    SDK_ASSERT(usage_ == usage);
	return usage;
}

//---------------------------------------------------------------------------
// return number of indices allocate so far
//---------------------------------------------------------------------------
uint64_t
rte_indexer::num_indices_allocated(void)
{
    return usage_;
}

}    // namespace lib
}    // namespace sdk
