//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_BUCKET_HPP__
#define __FTL_BUCKET_HPP__

#include <stdint.h>
#include <stdlib.h>

#include "ftl.hpp"
#include "ftl_apictx.hpp"

namespace sdk {
namespace table {
namespace ftlint {

class ftl_base_table;
class ftl_main_table;
class ftl_hint_table;

class ftl_bucket {
public:
    friend ftl;
    friend ftl_base_table;
    friend ftl_main_table;
    friend ftl_hint_table;

private:
    uint8_t valid_  : 1;
//    uint8_t locked_ : 1;
    uint8_t spare_  : 6;

    volatile uint8_t locked_;

private:
    sdk_ret_t insert_(ftl_apictx *ctx);
    sdk_ret_t update_(ftl_apictx *ctx);
    sdk_ret_t remove_(ftl_apictx *ctx);
    sdk_ret_t read_(ftl_apictx *ctx);
    sdk_ret_t write_(ftl_apictx *ctx);

    sdk_ret_t compare_(ftl_apictx *ctx);
    sdk_ret_t append_(ftl_apictx *ctx);
    sdk_ret_t create_(ftl_apictx *ctx);
    sdk_ret_t find_first_free_hint_(ftl_apictx *ctx);
    sdk_ret_t find_last_hint_(ftl_apictx *ctx);
    sdk_ret_t find_hint_(ftl_apictx *ctx);
    sdk_ret_t find_(ftl_apictx *ctx);
    sdk_ret_t move_(ftl_apictx *dst_ctx,
                    ftl_apictx *src_ctx);
    sdk_ret_t clear_hint_(ftl_apictx *ctx);
    sdk_ret_t delink_(ftl_apictx *ctx);
    sdk_ret_t defragment_(ftl_apictx *ectx,
                          ftl_apictx *tctx);
    sdk_ret_t iterate_(ftl_apictx *ctx);

    bool is_locked_() {
        return (locked_ ? true : false);
    }
    void lock_(void) {
        while(__sync_lock_test_and_set(&locked_, 1));
//        locked_ = 1;
    }
    void unlock_(void) {
        __sync_lock_release(&locked_);
//        locked_ = 0;
    }
public:
    ftl_bucket() {
        locked_ = 0;
    }

    ~ftl_bucket() {
    }

} __attribute__ ((packed));

} // namespace ftlint
} // namespace table
} // namespace sdk

#endif // __FTL_BUCKET_HPP__
