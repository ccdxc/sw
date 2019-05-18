//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLV4_BUCKET_HPP__
#define __FTLV4_BUCKET_HPP__

#include <stdint.h>
#include <stdlib.h>

#include "ftlv4.hpp"
#include "ftlv4_apictx.hpp"

namespace sdk {
namespace table {
namespace ftlint_ipv4 {

class ftlv4_base_table;
class ftlv4_main_table;
class ftlv4_hint_table;

class ftlv4_bucket {
public:
    friend ftlv4;
    friend ftlv4_base_table;
    friend ftlv4_main_table;
    friend ftlv4_hint_table;

private:
    uint8_t valid_  : 1;
    uint8_t spare_  : 7;

private:
    sdk_ret_t insert_(ftlv4_apictx *ctx);
    sdk_ret_t update_(ftlv4_apictx *ctx);
    sdk_ret_t remove_(ftlv4_apictx *ctx);
    sdk_ret_t read_(ftlv4_apictx *ctx);
    sdk_ret_t write_(ftlv4_apictx *ctx);

    sdk_ret_t compare_(ftlv4_apictx *ctx);
    sdk_ret_t append_(ftlv4_apictx *ctx);
    sdk_ret_t create_(ftlv4_apictx *ctx);
    sdk_ret_t find_first_free_hint_(ftlv4_apictx *ctx);
    sdk_ret_t find_last_hint_(ftlv4_apictx *ctx);
    sdk_ret_t find_hint_(ftlv4_apictx *ctx);
    sdk_ret_t find_(ftlv4_apictx *ctx);
    sdk_ret_t move_(ftlv4_apictx *dst_ctx,
                    ftlv4_apictx *src_ctx);
    sdk_ret_t clear_hint_(ftlv4_apictx *ctx);
    sdk_ret_t delink_(ftlv4_apictx *ctx);
    sdk_ret_t defragment_(ftlv4_apictx *ectx,
                          ftlv4_apictx *tctx);
    sdk_ret_t iterate_(ftlv4_apictx *ctx);

public:
    ftlv4_bucket() {
    }

    ~ftlv4_bucket() {
    }

} __attribute__ ((packed));

} // namespace ftlint_ipv4
} // namespace table
} // namespace sdk

#endif // __FTLV4_BUCKET_HPP__
