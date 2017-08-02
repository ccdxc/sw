#include "indexer.hpp"

namespace hal{
namespace utils {

// ---------------------------------------------------------------------------
// Constructor - indexer
// ---------------------------------------------------------------------------
indexer::indexer(uint32_t size, bool thread_safe)
{
    if ((size & 0x1F) != 0) {
        num_words_ = (size >> 5) + 1;
    } else {
        num_words_ = size >> 5;
    }
    size_ = size;
    last_word_off_ = size & 0x1F;

    // Allocate and Initialize to 0s
    bits_ = new uint32_t[num_words_]();

    thread_safe_ = thread_safe;
    if (thread_safe_) {
        HAL_ASSERT(!HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE));
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
// ---------------------------------------------------------------------------
indexer::status
indexer::alloc(uint32_t *index, uint32_t block_size)
{
    uint32_t id = 0, word = 0, off = 0, free_idx = 0;
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
    while (id < num_words_ && bits_[id] == 0xFFFFFFFF) {
        id++;
    }

    if (id == num_words_) {
        rs =INDEXER_FULL;
        goto end;
    }

    // Find the offset in the word
    word = bits_[id];
    while(word & 0x1) {
        word = word >> 1;
        off++;
    }

    // Check if offset is above the size of indexer
    free_idx = (id << 5) + off;
    if (free_idx >= size_) {
        rs =INDEXER_FULL;
        goto end;
    }

    // Set the new index
    bits_[id] |= (1 << off);
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
    uint32_t word       = index >> 5;
    uint8_t pos         = index % 32;
    uint32_t sel_word   = 0;
    status rs           = SUCCESS;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
    }
    if (bits_ && (word < num_words_)) {

        if ((word == num_words_ - 1) && (pos >= last_word_off_)) {
            rs = INDEX_OOB;
            goto end;
        }

        sel_word = bits_[word];
        if ((sel_word & (1 << pos))) {
            rs = indexer::DUPLICATE_ALLOC;
            goto end;
        }
        // Set the index
        bits_[word] |= (1 << pos);
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
    uint32_t word       = index >> 5;
    uint8_t pos         = index % 32;
    uint32_t sel_word   = 0;
    status rs           = SUCCESS;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
    }

    if (bits_ && (word < num_words_)) {

        if ((word == num_words_ - 1) && (pos >= last_word_off_)) {
            rs = INDEX_OOB;
            goto end;
        }

        sel_word = bits_[word];
        if (!(sel_word & (1 << pos))) {
            rs = indexer::DUPLICATE_FREE;
            goto end;
        }
        bits_[word] &= ~(1 << pos);
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
    uint32_t word       = index >> 5;
    uint8_t pos         = index % 32;
    uint32_t sel_word   = 0;
    bool is_alloced     = false;

    if (thread_safe_) {
        HAL_ASSERT_RETURN((HAL_SPINLOCK_LOCK(&slock_) == 0),
                           SLOCK_ERR);
    }

    if (bits_ && (word < num_words_)) {

        sel_word = bits_[word];
        if ((sel_word & (1 << pos))) {
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

}    // namespace hal
}    // namespace utils
