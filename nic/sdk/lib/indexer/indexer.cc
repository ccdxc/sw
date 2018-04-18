//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "lib/indexer/indexer.hpp"

namespace sdk {
namespace lib {

// NOTE: defines below ar for 64 bit word size
#define WORD_SIZE_SHIFT         6
#define WORD_SIZE_MASK          0x3F
#define WORD_SIZE_MODULO        64
#define ALL_ONES_64BIT          UINT64_MAX

//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
indexer *
indexer::factory(uint32_t size, bool thread_safe, bool skip_zero)
{
    void        *mem     = NULL;
    indexer     *indxr = NULL;

    mem = SDK_CALLOC(HAL_MEM_ALLOC_LIB_INDEXER, sizeof(indexer));
    if (!mem) {
        return NULL;
    }

    indxr = new (mem) indexer(size, thread_safe, skip_zero);
    return indxr;
}

//---------------------------------------------------------------------------
// method to free & delete the object
//---------------------------------------------------------------------------
void
indexer::destroy(indexer *indxr)
{
    if (indxr) {
        indxr->~indexer();
        SDK_FREE(HAL_MEM_ALLOC_LIB_INDEXER, indxr);
    }
}

//---------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------
indexer::indexer(uint32_t size, bool thread_safe, bool skip_zero)
{
    int   mod  = size & WORD_SIZE_MASK;

    if (mod) {
        num_words_ = (size >> WORD_SIZE_SHIFT) + 1;
    } else {
        num_words_ = size >> WORD_SIZE_SHIFT;
    }
    size_ = size;

    // allocate and initialize to 0s
    bits_ = (uint64_t*) SDK_CALLOC(HAL_MEM_ALLOC_LIB_INDEXER,
                                   sizeof(uint64_t) * num_words_);
    if (!bits_) {
        return;
    }

    // set upper bits of the odd word to ensure that they are never used
    if (mod) {
        bits_[num_words_ - 1] = ALL_ONES_64BIT << mod;
    }

    thread_safe_ = thread_safe;
    if (thread_safe_) {
        SDK_ASSERT(!SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE));
    }

    // skip 0th entry, if asked
    skip_zero_ = skip_zero;
    if (skip_zero_) {
        alloc_withid(0, 1);
    }
}

//---------------------------------------------------------------------------
// destructor
//---------------------------------------------------------------------------
indexer::~indexer()
{
    SDK_FREE(HAL_MEM_ALLOC_LIB_INDEXER, bits_);

    if (thread_safe_) {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
}

//---------------------------------------------------------------------------
// find first free bit, set it the bit and return SUCCESS with the index
// lowest - if true, return the lowest free index
//          if false, return the next higher free index
//          (useful for bottom-up allocation of indices)
//---------------------------------------------------------------------------
indexer::status
indexer::alloc(uint32_t *index, bool lowest, uint32_t block_size)
{
    uint32_t           free_idx = 0;
    int                off = 0, id = 0;
    indexer::status    rs = SUCCESS;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SLOCK_ERR);
    }

    if (num_words_ == 0 || !bits_) {
        rs = INVALID_INDEXER;
        goto end;
    }

    // find the word
    if (lowest) {
        id = 0;
        while ((uint32_t) id < num_words_ && bits_[id] == ALL_ONES_64BIT) {
            id++;
        }
    } else {
        // start searching bottom-up
        id = num_words_ - 1;
        while (id > 0 && bits_[id] == ALL_ONES_64BIT) {
            id--;
        }
    }

    if ((id < 0) || ((uint32_t) id == num_words_)) {
        rs = INDEXER_FULL;
        goto end;
    }

    // find the offset in the word
    off = get_rightmost_set_bit_(~bits_[id]);
    if (!lowest) {
        if (!bits_[id]) {
            off = ((1 << WORD_SIZE_SHIFT) - 1);
        } else {
            off = get_rightmost_set_bit_(bits_[id]);
            off--;
            if (off < 0) {
                rs = INDEXER_FULL;
                goto end;
            }
        }
    }
    free_idx = (id << WORD_SIZE_SHIFT) + off;

    // set the new index and return it
    bits_[id] |= (1ULL << off);
    *index = free_idx;

end:

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SLOCK_ERR);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// alloc_block()
//  - Finds the first free block of bits.
//  - Sets the bit and returns SUCCESS.
//  - @lowest - boolean
//          TRUE: returns the lowest free index
//          FALSE: returns the next higher free index. Useful for bottom-up
//                 allocation of indices
// ---------------------------------------------------------------------------
indexer::status
indexer::alloc_block(uint32_t *index, uint32_t block_size)
{
    uint32_t start = 0, offs = 0, word = 0, bit = 0;
    indexer::status rs = SUCCESS;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SLOCK_ERR);
    }

    if (num_words_ == 0 || !bits_) {
        rs = INVALID_INDEXER;
        goto end;
    }

    while (bit < block_size) {

        word = (start + bit) >> WORD_SIZE_SHIFT;
        while ((word < num_words_) && (bits_[word] == ALL_ONES_64BIT)) {
            word++; // Skip entire words if nothing is free in them.
            bit = 0; // But that would reset the search.
            start = word << WORD_SIZE_SHIFT;
        }

        if (word >= num_words_) {
            rs = INDEXER_FULL;
            goto end;
        }

        offs = (start + bit) & WORD_SIZE_MASK;
        if ((bits_[word] & (1ULL << offs)) == 0) {
            bit++; // This index is free. Continue to the next index.
            continue;
        }

        // This index is NOT free. Reset and search from the next index
        start = start + bit + 1;
        bit = 0;
    }

    *index = start;

    for (uint32_t idx = start; idx < (start + block_size); idx ++) {
        // Set the index as allocated
        word = idx >> WORD_SIZE_SHIFT;
        offs = idx  & WORD_SIZE_MASK;
        bits_[word] |= (1ULL << offs);
    }

end:
    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SLOCK_ERR);
    }

    return rs;
}

//---------------------------------------------------------------------------
// allocate a requested id by setting the corresponding bit, if its not set
// already
// if the indexed is already claimed and in use, return DUPLICATE_ALLOC
//---------------------------------------------------------------------------
indexer::status
indexer::alloc_withid(uint32_t index, uint32_t block_size)
{
    uint32_t word       = index >> WORD_SIZE_SHIFT;
    uint8_t pos         = index % WORD_SIZE_MODULO;
    uint64_t sel_word   = 0;
    status rs           = SUCCESS;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SLOCK_ERR);
    }
    if (bits_ && (word < num_words_)) {
        if (index >= size_) {
            rs = INDEX_OOB;
            goto end;
        }

        sel_word = bits_[word];
        if ((sel_word & ((uint64_t)(1ULL << pos)))) {
            rs = indexer::DUPLICATE_ALLOC;
            goto end;
        }
        // set the index
        bits_[word] |= ((uint64_t)(1ULL << pos));
    } else {
        rs = INDEX_OOB;
        goto end;
    }

end:

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SLOCK_ERR);
    }

    return rs;
}

//---------------------------------------------------------------------------
// frees the given index, if its in use or else no-op
//---------------------------------------------------------------------------
indexer::status
indexer::free(uint32_t index)
{
    uint32_t word       = index >> WORD_SIZE_SHIFT;
    uint8_t pos         = index % WORD_SIZE_MODULO;
    uint64_t sel_word   = 0;
    status rs           = SUCCESS;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), SLOCK_ERR);
    }

    if (bits_ && (word < num_words_)) {
        if (index >= size_) {
            rs = INDEX_OOB;
            goto end;
        }

        sel_word = bits_[word];
        if (!(sel_word & ((uint64_t) (1ULL << pos)))) {
            rs = indexer::DUPLICATE_FREE;
            goto end;
        }
        bits_[word] &= ((uint64_t) ~(1ULL << pos));
    } else {
        rs = INDEX_OOB;
    }

end:

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), SLOCK_ERR);
    }

    return rs;
}

//---------------------------------------------------------------------------
// return true if index is already allocated
//---------------------------------------------------------------------------
bool
indexer::is_index_allocated(uint32_t index)
{
    uint32_t    word       = index >> WORD_SIZE_SHIFT;
    uint8_t     pos        = index % WORD_SIZE_MODULO;
    uint64_t    sel_word   = 0;
    bool        is_alloced = false;

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_LOCK(&slock_) == 0), is_alloced);
    }

    if (bits_ && (word < num_words_)) {
        sel_word = bits_[word];
        if ((sel_word & ((uint64_t)(1ULL << pos)))) {
            is_alloced = true;
            goto end;
        }
    }

end:

    if (thread_safe_) {
        SDK_ASSERT_RETURN((SDK_SPINLOCK_UNLOCK(&slock_) == 0), is_alloced);
    }

    return is_alloced;
}

//---------------------------------------------------------------------------
// return number of indices allocate so far
//---------------------------------------------------------------------------
uint32_t
indexer::num_indices_allocated(void)
{
	uint32_t	usage = 0;

	for (uint32_t i = 0; i < size_; i++) {
		if (is_index_allocated(i) &&
            !(skip_zero_ && i == 0)) { // Don't count 0 if skip_zero is set
			usage++;
		}
	}

	return usage;
}

}    // namespace lib
}    // namespace sdk
