//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_TABLE_HPP__
#define __FTL_TABLE_HPP__
#include "string.h"

#include "include/sdk/mem.hpp"

#include "ftl_bucket.hpp"
#include "ftl_apictx.hpp"
#include "ftl_indexer.hpp"

namespace sdk {
namespace table {

class ftl;

namespace ftlint {

using sdk::table::ftl;

class ftl_hint_table;
class ftl_main_table;

class ftl_base_table {
public:
    friend ftl;
    static void destroy_(ftl_base_table *table);

protected:
    uint32_t table_id_;
    uint32_t table_size_;
    uint32_t num_table_index_bits_;
    ftl_bucket *buckets_;
//    sdk_spinlock_t slock_;
    volatile uint8_t slock_;

protected:
    sdk_ret_t init_(uint32_t id, uint32_t size);
    void spin_lock_(void) {
//        SDK_SPINLOCK_LOCK(&slock_);
        while(__sync_lock_test_and_set(&slock_, 1));
    }
    void spin_unlock_(void) {
//        SDK_SPINLOCK_UNLOCK(&slock_);
        __sync_lock_release(&slock_);
    }

public:
    ftl_base_table() {
        table_id_ = 0;
        table_size_ = 0;
        num_table_index_bits_ = 0;
        buckets_ = NULL;
        //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
        slock_ = 0;
    }

    ~ftl_base_table() {
        //SDK_SPINLOCK_DESTROY(&slock_);
    }

    sdk_ret_t iterate_(ftl_apictx *ctx);
};

class ftl_hint_table: public ftl_base_table {
public:
    friend ftl;
    friend ftl_main_table;
    static void destroy_(ftl_hint_table *table);

private:
    indexer indexer_;
    static thread_local uint8_t nctx_;

    ftl_apictx *ctxnew_(ftl_apictx *src) {
        if (FTL_API_CONTEXT_IS_MAIN(src)) {
            nctx_ = 0;
        } else {
            SDK_ASSERT(nctx_ < FTL_MAX_API_CONTEXTS);
        }
        auto c = src + 1;
        FTL_API_CONTEXT_INIT(c, src);
        nctx_++;
        return c;
    }

private:
    sdk_ret_t alloc_(ftl_apictx *ctx);
    sdk_ret_t dealloc_(ftl_apictx *ctx);
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(ftl_apictx *ctx);
    sdk_ret_t insert_(ftl_apictx *ctx);
    sdk_ret_t remove_(ftl_apictx *ctx);
    sdk_ret_t find_(ftl_apictx *ctx,
                    ftl_apictx **retctx);
    sdk_ret_t defragment_(ftl_apictx *ctx);
    sdk_ret_t tail_(ftl_apictx *ctx,
                    ftl_apictx **retctx);

public:
    static ftl_hint_table* factory(sdk::table::properties_t *props);
    ftl_hint_table() {
    }

    ~ftl_hint_table() {
    }
};

class ftl_main_table : public ftl_base_table {
public:
    friend ftl;
    static void destroy_(ftl_main_table *table);

private:
    ftl_hint_table *hint_table_;
    uint32_t num_hash_bits_;

private:
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(ftl_apictx *ctx);
    sdk_ret_t insert_(ftl_apictx *ctx);
    sdk_ret_t insert_with_handle_(ftl_apictx *ctx);
    sdk_ret_t remove_(ftl_apictx *ctx);
    sdk_ret_t remove_with_handle_(ftl_apictx *ctx);
    sdk_ret_t update_(ftl_apictx *ctx);
    sdk_ret_t get_(ftl_apictx *ctx);
    sdk_ret_t get_with_handle_(ftl_apictx *ctx);
    sdk_ret_t find_(ftl_apictx *ctx,
                    ftl_apictx **retctx);
    sdk_ret_t iterate_(ftl_apictx *ctx);
    void entry_lock_(ftl_apictx *ctx);
    void entry_unlock_(ftl_apictx *ctx);

public:
    static ftl_main_table* factory(sdk::table::properties_t *props);

    ftl_main_table() {
        hint_table_ = NULL;
        num_hash_bits_ = 0;
    }

    ~ftl_main_table() {
    }
};

} // namespace ftlint
} // namespace table
} // namespace sdk

#endif // __FTL_HPP__
