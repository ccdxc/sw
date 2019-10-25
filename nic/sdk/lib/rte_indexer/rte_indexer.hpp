//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// indexer library for resource id management
//------------------------------------------------------------------------------

#ifndef __SDK_RTE_INDEXER_HPP__
#define __SDK_RTE_INDEXER_HPP__

#include <stdint.h>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/lock.hpp"

namespace sdk {
namespace lib {

class rte_indexer {

public:
    static rte_indexer *factory(uint32_t size, bool thread_safe = true,
                                bool skip_zero = false);
    static void destroy(rte_indexer *idxr);
    sdk_ret_t alloc(uint32_t *index, uint32_t block_size = 1);
    sdk_ret_t alloc(uint32_t index, uint32_t block_size = 1);
    sdk_ret_t free(uint32_t index, uint32_t block_size = 1);
    bool is_index_allocated(uint32_t index);
    uint32_t size(void) const { return size_; }
    uint64_t usage(void) const;

private:
    rte_indexer();
    ~rte_indexer();
    bool init_(uint32_t size, bool thread_safe = true,
               bool skip_zero = false);
    void set_curr_slab_(uint64_t pos);

private:
    uint64_t        curr_slab_;      // current slab value
    uint64_t        curr_index_;     // current position of index
    uint32_t        size_;           // size of indexer
    uint8_t         *bits_;          // bit representation
    bool            skip_zero_;      // skipping 0th entry
    bool            thread_safe_;    // enable/disable thread safety
    sdk_spinlock_t  slock_;          // lock for thread safety
    uint64_t        usage_;
    void            *indexer_;       // rte_bitmap
};

}    // namespace lib
}    // namespace sdk

using sdk::lib::rte_indexer;

#endif    // __SDK_RTE_INDEXER_HPP__

