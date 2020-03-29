//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_TABLE_HPP__
#define __FTL_TABLE_HPP__

#define PDS_FLOW_HINT_POOL_COUNT_MAX    256
#define PDS_FLOW_HINT_POOLS_MAX 8

namespace sdk {
namespace table {
namespace internal {

typedef struct ftl_flow_hint_id_thr_local_pool_s {
    int16_t         pool_count;
    uint32_t        hint_ids[PDS_FLOW_HINT_POOL_COUNT_MAX];
} ftl_flow_hint_id_thr_local_pool_t;

class base_table {
public:
    friend ftl_base;
    static void destroy_(base_table *table);

protected:
    uint32_t table_id_;
    uint32_t table_size_;
    uint32_t num_table_index_bits_;
    Bucket *buckets_;
    volatile uint8_t slock_;

protected:
    sdk_ret_t init_(uint32_t id, uint32_t size);
    void spin_lock_(void) {
        while(__sync_lock_test_and_set(&slock_, 1));
    }
    void spin_unlock_(void) {
        __sync_lock_release(&slock_);
    }

public:
    base_table() {
        table_id_ = 0;
        table_size_ = 0;
        num_table_index_bits_ = 0;
        buckets_ = NULL;
        //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
        slock_ = 0;
    }

    ~base_table() {
        //SDK_SPINLOCK_DESTROY(&slock_);
    }

    sdk_ret_t iterate_(Apictx *ctx);
    sdk_ret_t clear_(Apictx *ctx);
};

class hint_table: public base_table {
public:
    friend ftl_base;
    friend main_table;
    static void destroy_(hint_table *table);

private:
    ftlindexer indexer_;
    static thread_local uint8_t nctx_;
    ftl_flow_hint_id_thr_local_pool_t thr_local_pools_[PDS_FLOW_HINT_POOLS_MAX];

private:
    sdk_ret_t alloc_(Apictx *ctx);
    sdk_ret_t dealloc_(Apictx *ctx);
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(Apictx *ctx);
    sdk_ret_t initctx_with_handle_(Apictx *ctx);
    sdk_ret_t insert_(Apictx *ctx);
    sdk_ret_t remove_(Apictx *ctx);
    sdk_ret_t find_(Apictx *ctx,
                    Apictx **retctx);
    sdk_ret_t get_with_handle_(Apictx *ctx);
    sdk_ret_t iterate_(Apictx *ctx);
    sdk_ret_t clear_(Apictx *ctx);
    sdk_ret_t defragment_(Apictx *ctx);
    sdk_ret_t tail_(Apictx *ctx,
                    Apictx **retctx);
    Apictx *ctxnew_(Apictx *src);
    void lock_(Apictx *ctx);
    void unlock_(Apictx *ctx);


public:
    static hint_table* factory(sdk::table::properties_t *props);
    hint_table() {}
    ~hint_table() {}
};

class main_table : public base_table {
public:
    friend ftl_base;
    static void destroy_(main_table *table);

private:
    hint_table *hint_table_;
    uint32_t num_hash_bits_;

private:
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(Apictx *ctx);
    sdk_ret_t initctx_with_handle_(Apictx *ctx);
    sdk_ret_t insert_(Apictx *ctx);
    sdk_ret_t insert_with_handle_(Apictx *ctx);
    sdk_ret_t remove_(Apictx *ctx);
    sdk_ret_t remove_with_handle_(Apictx *ctx);
    sdk_ret_t update_(Apictx *ctx);
    sdk_ret_t get_(Apictx *ctx);
    sdk_ret_t get_with_handle_(Apictx *ctx);
    sdk_ret_t find_(Apictx *ctx,
                    Apictx **retctx);
    sdk_ret_t iterate_(Apictx *ctx);
    sdk_ret_t clear_(Apictx *ctx);
    void lock_(Apictx *ctx);
    void unlock_(Apictx *ctx);

public:
    static main_table* factory(sdk::table::properties_t *props);

    main_table() {
        hint_table_ = NULL;
        num_hash_bits_ = 0;
    }

    ~main_table() {}

};

} // namespace internal
} // namespace table
} // namespace sdk

#endif    // __FTL_TABLE_HPP__
