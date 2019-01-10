//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_TABLE_HPP__
#define __MEM_HASH_TABLE_HPP__

#include "include/sdk/mem.hpp"
#include "include/sdk/indexer.hpp"

#include "mem_hash.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_api_context.hpp"

namespace sdk {
namespace table {
namespace memhash {

using sdk::lib::indexer;
using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::mem_hash;

class mem_hash_base_table {
public:
    friend mem_hash;
    static void destroy_(mem_hash_base_table *table);

protected:
    uint32_t                table_id_;
    uint32_t                table_size_;
    uint32_t                num_table_index_bits_;
    mem_hash_table_bucket  *buckets_;

protected:
    sdk_ret_t   init_(uint32_t id, uint32_t size);

public:
    mem_hash_base_table() {
    }

    ~mem_hash_base_table() {
    }
};

class mem_hash_main_table : public mem_hash_base_table {
public:
    friend mem_hash;
    static void destroy_(mem_hash_main_table *table);

private:
    uint32_t            num_hash_bits_;

private:
    sdk_ret_t   init_(uint32_t id, uint32_t size);
    sdk_ret_t   initctx_(mem_hash_api_context *ctx);
    sdk_ret_t   insert_(mem_hash_api_context *ctx);
    sdk_ret_t   remove_(mem_hash_api_context *ctx);

public:
    static mem_hash_main_table* factory(uint32_t id, uint32_t size);

    mem_hash_main_table() {
    }

    ~mem_hash_main_table() {
    }
};

class mem_hash_hint_table: public mem_hash_base_table {
public:
    friend mem_hash;
    static void destroy_(mem_hash_hint_table *table);

private:
    indexer     *indexer_;

private:
    sdk_ret_t   alloc_(mem_hash_api_context *ctx);
    sdk_ret_t   dealloc_(mem_hash_api_context *ctx);
    sdk_ret_t   init_(uint32_t id, uint32_t size);
    sdk_ret_t   initctx_(mem_hash_api_context *ctx);
    sdk_ret_t   insert_(mem_hash_api_context *ctx);
    sdk_ret_t   remove_(mem_hash_api_context *ctx);
    sdk_ret_t   find_(mem_hash_api_context *ctx,
                      mem_hash_api_context **retctx);
    sdk_ret_t   defragment_(mem_hash_api_context *ctx);
    sdk_ret_t   tail_(mem_hash_api_context *ctx,
                      mem_hash_api_context **retctx);

public:
    static mem_hash_hint_table* factory(uint32_t id, uint32_t size);
    mem_hash_hint_table() {
    }

    ~mem_hash_hint_table() {
    }
};

} // namespace membash
} // namespace table
} // namespace sdk

#endif // __MEM_HASH_HPP__
