//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// indexer library for resource id management
//------------------------------------------------------------------------------

#ifndef __SDK_INDEXER_HPP__
#define __SDK_INDEXER_HPP__

#include <stdint.h>
#include "sdk/base.hpp"
#include "sdk/mem.hpp"
#include "sdk/lock.hpp"

namespace sdk {
namespace lib {

class indexer {
public:
    static const uint32_t k_invalid_index = 0xFFFFFFFF;
    enum status {
        SUCCESS, 
        INVALID_INDEXER,
        INDEXER_FULL,
        DUPLICATE_ALLOC,
        DUPLICATE_FREE,
        INDEX_OOB,
        SLOCK_ERR,
        UNKNOWN_STATUS
    };

private:
    uint32_t        size_;              // size of indexer
    uint32_t        num_words_;         // number of workds
    uint64_t        *bits_;             // bit representation
    bool            skip_zero_;         // skipping 0th entry
    bool            thread_safe_;       // enable/disable thread safety
    sdk_spinlock_t  slock_;             // lock for thread safety
    uint64_t        debruijn64_ = 0x022FDD63CC95386D;

    // convert debruijn idx to standard idx
    const unsigned int index64_[64] =
    {
        0,  1,  2, 53,  3,  7, 54, 27,
        4, 38, 41,  8, 34, 55, 48, 28,
       62,  5, 39, 46, 44, 42, 22,  9,
       24, 35, 59, 56, 49, 18, 29, 11,
       63, 52,  6, 26, 37, 40, 33, 47,
       61, 45, 43, 21, 23, 58, 17, 10,
       51, 25, 36, 32, 60, 20, 57, 16,
       50, 31, 19, 15, 30, 14, 13, 12,
    };

    // get index of rightmost set bit
    int get_rightmost_set_bit_ (uint64_t v) {
        return index64_[((v & (-v)) * debruijn64_) >> 58];
    }

    indexer(uint32_t size, bool thread_safe = true, bool skip_zero = false);
    ~indexer();

public:
    static indexer *factory(uint32_t size, bool thread_safe = true,
                            bool skip_zero = false);
    static void destroy(indexer *idxr);
    indexer::status alloc(uint32_t *index, bool lowest = TRUE,
                          uint32_t block_size = 1);
    indexer::status alloc_withid(uint32_t index, uint32_t block_size = 1);
    indexer::status alloc_block(uint32_t *index, uint32_t block_size);
    indexer::status free(uint32_t index);
    bool is_index_allocated(uint32_t index);
    uint32_t get_size(void) const { return size_; }
	uint32_t num_indices_allocated(void);
};

}    // namespace lib
}    // namespace sdk

#endif // __SDK_INDEXER_HPP__

