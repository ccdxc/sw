//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
namespace sdk {
namespace table {
namespace FTL_MAKE_AFTYPE(internal) {

class FTL_MAKE_AFTYPE(base_table) {
public:
    friend FTL_AFPFX();
    static void destroy_(FTL_MAKE_AFTYPE(base_table) *table);

protected:
    uint32_t table_id_;
    uint32_t table_size_;
    uint32_t num_table_index_bits_;
    FTL_MAKE_AFTYPE(bucket) *buckets_;
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
    FTL_MAKE_AFTYPE(base_table)() {
        table_id_ = 0;
        table_size_ = 0;
        num_table_index_bits_ = 0;
        buckets_ = NULL;
        //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
        slock_ = 0;
    }

    ~FTL_MAKE_AFTYPE(base_table)() {
        //SDK_SPINLOCK_DESTROY(&slock_);
    }

    sdk_ret_t iterate_(FTL_MAKE_AFTYPE(apictx) *ctx);
};

class FTL_MAKE_AFTYPE(hint_table): public FTL_MAKE_AFTYPE(base_table) {
public:
    friend FTL_AFPFX();
    friend FTL_MAKE_AFTYPE(main_table);
    static void destroy_(FTL_MAKE_AFTYPE(hint_table) *table);

private:
    ftlindexer indexer_;
    static thread_local uint8_t nctx_;

private:
    sdk_ret_t alloc_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t dealloc_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t insert_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t remove_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t find_(FTL_MAKE_AFTYPE(apictx) *ctx,
                    FTL_MAKE_AFTYPE(apictx) **retctx);
    sdk_ret_t defragment_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t tail_(FTL_MAKE_AFTYPE(apictx) *ctx,
                    FTL_MAKE_AFTYPE(apictx) **retctx);
    FTL_MAKE_AFTYPE(apictx) *ctxnew_(FTL_MAKE_AFTYPE(apictx) *src);


public:
    static FTL_MAKE_AFTYPE(hint_table)* factory(sdk::table::properties_t *props);
    FTL_MAKE_AFTYPE(hint_table)() {}
    ~FTL_MAKE_AFTYPE(hint_table)() {}
};

class FTL_MAKE_AFTYPE(main_table) : public FTL_MAKE_AFTYPE(base_table) {
public:
    friend FTL_AFPFX();
    static void destroy_(FTL_MAKE_AFTYPE(main_table) *table);

private:
    FTL_MAKE_AFTYPE(hint_table) *hint_table_;
    uint32_t num_hash_bits_;

private:
    sdk_ret_t init_(sdk::table::properties_t *props);
    sdk_ret_t initctx_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t insert_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t insert_with_handle_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t remove_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t remove_with_handle_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t update_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t get_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t get_with_handle_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t find_(FTL_MAKE_AFTYPE(apictx) *ctx,
                    FTL_MAKE_AFTYPE(apictx) **retctx);
    sdk_ret_t iterate_(FTL_MAKE_AFTYPE(apictx) *ctx);
    void lock_(FTL_MAKE_AFTYPE(apictx) *ctx);
    void unlock_(FTL_MAKE_AFTYPE(apictx) *ctx);

public:
    static FTL_MAKE_AFTYPE(main_table)* factory(sdk::table::properties_t *props);

    FTL_MAKE_AFTYPE(main_table)() {
        hint_table_ = NULL;
        num_hash_bits_ = 0;
    }

    ~FTL_MAKE_AFTYPE(main_table)() {}

};

} // namespace FTL_MAKE_AFTYPE(internal)
} // namespace table
} // namespace sdk
