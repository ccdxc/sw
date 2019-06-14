//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_INDEXER_HPP__
#define __FTLITE_INDEXER_HPP__

#include "include/sdk/base.hpp"

namespace ftlite {
namespace internal {

#define WORDSIZE 64
#define I2W(_i) ((_i)/WORDSIZE)
#define W2I(_w, _o) (((_w)*WORDSIZE)+(_o))
class indexer_t {
private:
    uint64_t *pool;
    uint32_t num_words;
    uint32_t last_alloc;
    uint32_t last_free;
    volatile uint32_t lock_;

public:
    sdk_ret_t init(uint32_t num_entries) {
        pool = (uint64_t *)SDK_CALLOC(SDK_MEM_ALLOC_INDEX_POOL,
                                      num_entries / 8);
        if (pool == NULL) {
            return sdk::SDK_RET_OOM;
        }

        last_alloc = 0;
        last_free = 0;
        num_words = I2W(num_entries);
        return sdk::SDK_RET_OK;
    }

    void deinit() {
        SDK_FREE(SDK_MEM_ALLOC_INDEX_POOL, pool);
    }
    
    void lock() {
        while(__sync_lock_test_and_set(&lock_, 1));
    }

    void unlock() {
        __sync_lock_release(&lock_);
    }

    sdk_ret_t alloc(uint32_t &ret_index) {
        lock();
        uint32_t w = I2W(last_alloc);
        uint32_t index = 0;
        do {
            index = __builtin_ffsll(~pool[w]);
            if (index) {
                pool[w] |= ((uint64_t)1<<(index-1));
                last_alloc = W2I(w, index-1);
                ret_index = last_alloc;
                unlock();
                return sdk::SDK_RET_OK;
            }
            w = (w + 1) % num_words;
        } while (w != I2W(last_alloc));
        unlock();
        return sdk::SDK_RET_NO_RESOURCE;
    }
    
    void free(uint32_t index) {
        lock();
        uint64_t w = I2W(index);
        auto i = index % WORDSIZE;
        pool[w] &= ~((uint64_t)1<<(i-1));
        last_free = index;
        unlock();
    }
};

} // namespace internal
} // namespace ftlite

#endif // __FTLITE_INDEXER_HPP__
