//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLV4_TABLE_HPP__
#define __FTLV4_TABLE_HPP__
#include "string.h"

#include "include/sdk/mem.hpp"

#include "ftlv4_bucket.hpp"
#include "ftlv4_apictx.hpp"
#include "ftlv4_indexer.hpp"

namespace sdk {
namespace table {

class ftlv4;

namespace ftlint_ipv4 {

using sdk::table::ftlv4;

class ftlv4_hint_table;
class ftlv4_main_table;

class ftlv4_base_table {
public:
    friend ftlv4;
    static void destroy_(ftlv4_base_table *table);

protected:
    uint32_t table_id_;
    uint32_t table_size_;
    uint32_t num_table_index_bits_;
    ftlv4_bucket *buckets_;

protected:
    sdk_ret_t init_(uint32_t id, uint32_t size);

public:
    ftlv4_base_table() {
        table_id_ = 0;
        table_size_ = 0;
        num_table_index_bits_ = 0;
        buckets_ = NULL;
    }

    ~ftlv4_base_table() {
    }

    sdk_ret_t iterate_(ftlv4_apictx *ctx);
};

class ftlv4_hint_table: public ftlv4_base_table {
public:
    friend ftlv4;
    friend ftlv4_main_table;
    static void destroy_(ftlv4_hint_table *table);

private:
    indexer indexer_;
    ftlv4_apictx ctxs_[FTLV4_MAX_API_CONTEXTS];
    uint8_t nctx_;

    ftlv4_apictx *ctxnew_(ftlv4_apictx *src) {
        if (FTLV4_API_CONTEXT_IS_MAIN(src)) {
            nctx_ = 0;
        } else {
            SDK_ASSERT(nctx_ < FTLV4_MAX_API_CONTEXTS);
        }
        auto c = &ctxs_[nctx_];
        FTLV4_API_CONTEXT_INIT(c, src);
        nctx_++;
        return c;
    }

private:
    sdk_ret_t alloc_(ftlv4_apictx *ctx);
    sdk_ret_t dealloc_(ftlv4_apictx *ctx);
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(ftlv4_apictx *ctx);
    sdk_ret_t insert_(ftlv4_apictx *ctx);
    sdk_ret_t remove_(ftlv4_apictx *ctx);
    sdk_ret_t find_(ftlv4_apictx *ctx,
                    ftlv4_apictx **retctx);
    sdk_ret_t defragment_(ftlv4_apictx *ctx);
    sdk_ret_t tail_(ftlv4_apictx *ctx,
                    ftlv4_apictx **retctx);

public:
    static ftlv4_hint_table* factory(sdk::table::properties_t *props);
    ftlv4_hint_table() {
        nctx_ = 0;
    }

    ~ftlv4_hint_table() {
    }
};

class ftlv4_main_table : public ftlv4_base_table {
public:
    friend ftlv4;
    static void destroy_(ftlv4_main_table *table);

private:
    ftlv4_hint_table *hint_table_;
    uint32_t num_hash_bits_;

private:
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(ftlv4_apictx *ctx);
    sdk_ret_t insert_(ftlv4_apictx *ctx);
    sdk_ret_t insert_with_handle_(ftlv4_apictx *ctx);
    sdk_ret_t remove_(ftlv4_apictx *ctx);
    sdk_ret_t remove_with_handle_(ftlv4_apictx *ctx);
    sdk_ret_t update_(ftlv4_apictx *ctx);
    sdk_ret_t get_(ftlv4_apictx *ctx);
    sdk_ret_t get_with_handle_(ftlv4_apictx *ctx);
    sdk_ret_t find_(ftlv4_apictx *ctx,
                    ftlv4_apictx **retctx);
    sdk_ret_t iterate_(ftlv4_apictx *ctx);

public:
    static ftlv4_main_table* factory(sdk::table::properties_t *props);

    ftlv4_main_table() {
        hint_table_ = NULL;
        num_hash_bits_ = 0;
    }

    ~ftlv4_main_table() {
    }
};

} // namespace ftlint_ipv4
} // namespace table
} // namespace sdk

#endif // __FTLV4_HPP__
