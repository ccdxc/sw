//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
namespace sdk {
namespace table {
namespace FTL_MAKE_AFTYPE(internal) {

class FTL_MAKE_AFTYPE(bucket) {
public:
    friend FTL_MAKE_AFTYPE(base_table);
    friend FTL_MAKE_AFTYPE(main_table);
    friend FTL_MAKE_AFTYPE(hint_table);

private:
    uint8_t valid_  : 1;
    uint8_t spare_  : 7;

    volatile uint8_t locked_;

private:
    sdk_ret_t insert_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t update_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t remove_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t read_(FTL_MAKE_AFTYPE(apictx) *ctx, bool force_hwread=FALSE);
    sdk_ret_t write_(FTL_MAKE_AFTYPE(apictx) *ctx);

    sdk_ret_t compare_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t append_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t create_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t find_first_free_hint_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t find_last_hint_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t find_hint_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t find_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t move_(FTL_MAKE_AFTYPE(apictx) *dst_ctx,
                    FTL_MAKE_AFTYPE(apictx) *src_ctx);
    sdk_ret_t clear_hint_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t delink_(FTL_MAKE_AFTYPE(apictx) *ctx);
    sdk_ret_t defragment_(FTL_MAKE_AFTYPE(apictx) *ectx,
                          FTL_MAKE_AFTYPE(apictx) *tctx);
    sdk_ret_t iterate_(FTL_MAKE_AFTYPE(apictx) *ctx, bool force_hwread=FALSE);

    bool is_locked_() {
        return (locked_ ? true : false);
    }
    void lock_(void) {
        while(__sync_lock_test_and_set(&locked_, 1));
    }
    void unlock_(void) {
        __sync_lock_release(&locked_);
    }
public:
    FTL_MAKE_AFTYPE(bucket)() {
        locked_ = 0;
    }

    ~FTL_MAKE_AFTYPE(bucket)() {
    }

} __attribute__ ((packed));

} // namespace FTL_MAKE_AFTYPE(internal)
} // namespace table
} // namespace sdk
