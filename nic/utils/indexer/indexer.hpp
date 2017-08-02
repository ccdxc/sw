#ifndef __INDEXER_HPP__
#define __INDEXER_HPP__

#include <base.h>
#include <hal_lock.hpp>

namespace hal {
namespace utils {

class indexer {
public:
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
    uint32_t        *bits_;             // bit representation
    uint32_t        last_word_off_;     // last word offset
    bool            thread_safe_;       // enable/disable thread safety
    hal_spinlock_t  slock_;             // lock for thread safety

public:
    indexer(uint32_t size, bool thread_safe = true);
    ~indexer();
    indexer::status alloc(uint32_t *index, uint32_t block_size = 1);
    indexer::status alloc_withid(uint32_t index, uint32_t block_size = 1);
    indexer::status free(uint32_t index);
    bool is_alloced(uint32_t index);
};

}    // namespace hal
}    // namespace utils

#endif // __INDEXER_HPP__

