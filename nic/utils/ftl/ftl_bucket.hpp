//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
namespace sdk {
namespace table {
namespace internal {

class Bucket {
public:
    friend BaseTable;
    friend MainTable;
    friend HintTable;

private:
    uint8_t valid_  : 1;
    uint8_t spare_  : 7;

    volatile uint8_t locked_;

private:
    sdk_ret_t insert_(Apictx *ctx);
    sdk_ret_t update_(Apictx *ctx);
    sdk_ret_t remove_(Apictx *ctx);
    sdk_ret_t read_(Apictx *ctx, bool force_hwread=FALSE);
    sdk_ret_t write_(Apictx *ctx);

    sdk_ret_t compare_(Apictx *ctx);
    sdk_ret_t append_(Apictx *ctx);
    sdk_ret_t create_(Apictx *ctx);
    sdk_ret_t find_first_free_hint_(Apictx *ctx);
    sdk_ret_t find_last_hint_(Apictx *ctx);
    sdk_ret_t find_hint_(Apictx *ctx);
    sdk_ret_t find_(Apictx *ctx);
    sdk_ret_t move_(Apictx *dst_ctx,
                    Apictx *src_ctx);
    sdk_ret_t clear_hint_(Apictx *ctx);
    sdk_ret_t delink_(Apictx *ctx);
    sdk_ret_t defragment_(Apictx *ectx,
                          Apictx *tctx);
    sdk_ret_t iterate_(Apictx *ctx);

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
    Bucket() {
        locked_ = 0;
    }

    ~Bucket() {
    }

} __attribute__ ((packed));

} // namespace internal
} // namespace table
} // namespace sdk
