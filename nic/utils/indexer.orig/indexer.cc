#include "indexer.hpp"

namespace hal{
namespace utils {

/* DEFINES BELOW ARE FOR 64 BIT WORD SIZE */
#define WORD_SIZE_SHIFT         6
#define WORD_SIZE_MASK          0x3F
#define WORD_SIZE_MODULO        64
#define ALL_ONES_64BIT          UINT64_MAX

// ---------------------------------------------------------------------------
// Constructor - indexer
// ---------------------------------------------------------------------------
indexer::indexer(uint32_t size, bool thread_safe, bool skip_zero)
{
    int mod = size & WORD_SIZE_MASK;

    if (mod) {
        num_words_ = (size >> WORD_SIZE_SHIFT) + 1;
    } else {
        num_words_ = size >> WORD_SIZE_SHIFT;
    }
    size_ = size;

    // Allocate and Initialize to 0s
    bits_ = new uint64_t[num_words_]();
    
    // Set the upper bits of the odd word to ensure
    // that they never get returned
    if (mod) {
        bits_[num_words_ - 1] = ALL_ONES_64BIT << mod;
    }

    thread_safe_ = thread_safe;
    if (thread_safe_) {
        HAL_ASSERT(!HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE));
    }

    // Skipping 0th entry. Note: Uses up one entry
    skip_zero_ = skip_zero;
    if (skip_zero_) {
        alloc_withid(0, 1);
    }

}

// ---------------------------------------------------------------------------
// Destructor - indexer
// ---------------------------------------------------------------------------
indexer::~indexer() 
{
    delete[] bits_;
    if (thread_safe_) {
        HAL_SPINLOCK_DESTROY(&slock_);
    }
}

// ---------------------------------------------------------------------------
// alloc()
//  - Finds the first free bit. 
//  - Sets the bit and returns SUCCESS.
//  - @lowest - boolean
//          TRUE: returns the lowest free index
//          FALSE: returns the next higher free index. Useful for bottom-up
//                 allocation of indices
// ---------------------------------------------------------------------------
indexer::status
indexer::alloc(uint32_t *index, bool lowest, uint32_t block_size)
{
    uint32_t free_idx = 0;
    int off = 0, id = 0;
    indexer::status rs = SUCCESS;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
    }

    if (num_words_ == 0 || !bits_) {
        rs = INVALID_INDEXER;
        goto end;
    }

    // Find the word
    if (lowest) {
        id = 0;
        while ((uint32_t) id < num_words_ && bits_[id] == ALL_ONES_64BIT) {
            id++;
        }
    } else {
        // Start searching bottom-up
        id = num_words_ - 1;
        while (id > 0 && bits_[id] == ALL_ONES_64BIT) {
            id--;
        }
    }

    if ((id < 0) || ((uint32_t) id == num_words_)) {
        rs = INDEXER_FULL;
        goto end;
    }

    // Find the offset in the word
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
    // Set the new index
    bits_[id] |= (1ULL << off);
    *index = free_idx;

end:
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&slock_) == 0),
                SLOCK_ERR);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// alloc_withid
//  - Sets the bit if its not already set.
//  - If its already set, returns DUPLICATE_ALLOC
// ---------------------------------------------------------------------------
indexer::status
indexer::alloc_withid(uint32_t index, uint32_t block_size)
{
    uint32_t word       = index >> WORD_SIZE_SHIFT;
    uint8_t pos         = index % WORD_SIZE_MODULO;
    uint64_t sel_word   = 0;
    status rs           = SUCCESS;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
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
        // Set the index
        bits_[word] |= ((uint64_t)(1ULL << pos));
    } else {
        rs = INDEX_OOB;
        goto end;
    }

end:
    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&slock_) == 0),
                SLOCK_ERR);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// free()
//  - Frees the index passed.
// ---------------------------------------------------------------------------
indexer::status
indexer::free(uint32_t index)
{
    uint32_t word       = index >> WORD_SIZE_SHIFT;
    uint8_t pos         = index % WORD_SIZE_MODULO;
    uint64_t sel_word   = 0;
    status rs           = SUCCESS;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
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
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&slock_) == 0),
                SLOCK_ERR);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// is_alloced()
//  - TRUE: if index is alloced
// ---------------------------------------------------------------------------
bool
indexer::is_alloced(uint32_t index)
{
    uint32_t word       = index >> WORD_SIZE_SHIFT;
    uint8_t pos         = index % WORD_SIZE_MODULO;
    uint64_t sel_word   = 0;
    bool is_alloced     = false;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
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
        HAL_ASSERT_RETURN((HAL_SPINLOCK_UNLOCK(&slock_) == 0),
                SLOCK_ERR);
    }

    return is_alloced;
}

// ---------------------------------------------------------------------------
// usage()
//  - Returns the number of entries being used (or bits set)
// ---------------------------------------------------------------------------
uint32_t
indexer::usage()
{
	uint32_t	usage = 0;
	for (uint32_t i = 0; i < size_; i++) {
		if (is_alloced(i)) {
			usage++;
		}
	}

	return usage;
}
}    // namespace hal
}    // namespace utils
