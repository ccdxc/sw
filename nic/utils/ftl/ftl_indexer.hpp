//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_INDEXER_HPP__
#define __FTL_INDEXER_HPP__

#include "include/sdk/base.hpp"

#define WORDSIZE 64
#define I2W(_i) ((_i)/WORDSIZE)
#define W2I(_w, _o) (((_w)*WORDSIZE)+(_o))

namespace sdk {
namespace table {
namespace ftlint {

class indexer {
private:
    uint64_t *pool;
    uint32_t num_words;
    uint32_t last_alloc;
    uint32_t last_free;

public:
    sdk_ret_t init(uint32_t num_entries) {
        pool = (uint64_t *)SDK_CALLOC(SDK_MEM_ALLOC_INDEX_POOL,
                                      num_entries / 8);
        if (pool == NULL) {
            return SDK_RET_OOM;
        }

        last_alloc = 0;
        last_free = 0;
        num_words = I2W(num_entries);
        return SDK_RET_OK;
    }

    void deinit() {
        SDK_FREE(SDK_MEM_ALLOC_INDEX_POOL, pool);
    }

    uint32_t alloc() {
        uint32_t w = I2W(last_alloc);
        uint32_t index = 0;
        do {
            index = __builtin_ffsll(~pool[w]);
            if (index) {
                pool[w] |= ((uint64_t)1<<(index-1));
                last_alloc = W2I(w, index);
                return last_alloc;
            }
            w = (w + 1) % num_words;
        } while (w != I2W(last_alloc));
        return 0;
    }
    
    void free(uint32_t index) {
        uint64_t w = I2W(index);
        auto i = index % WORDSIZE;
        pool[w] &= ~((uint64_t)1<<(i-1));
        last_free = index;
    }
};

} // namespace ftlint
} // namespace table
} // namespace sdk

#endif // __FTL_INDEXER_HPP__
