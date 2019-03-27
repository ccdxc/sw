//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef _SLHASH_BST_HPP_
#define _SLHASH_BST_HPP_

#include <stdint.h>
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "slhash_api_context.hpp"

namespace sdk {
namespace table {
namespace slhash_internal {

class bucket {
public:
    bool isbusy() { return inuse_ || reserved_; }
    bool isused() { return inuse_; }
    bool isreserved() { return reserved_; }
    void reserve() { reserved_ = 1; }
    void release() { reserved_ = 0; }
    void alloc() { inuse_ = 1; }
    void free() { inuse_ = 0; }

private:
    bucket() { inuse_ = 0, reserved_ = 0; }
    ~bucket() {}

private:
    uint8_t inuse_:1;
    uint8_t reserved_:1;
    uint8_t spare_:6;
} __attribute__((__packed__));

class table {
public:
    table() { max_ = 0; buckets_ = NULL; }
    ~table() { SDK_FREE(SDK_MEM_TYPE_SLHASH_BUCKETS, buckets_); }
   
    sdk_ret_t init(uint32_t max);
    sdk_ret_t insert(slhctx &ctx);
    sdk_ret_t update(slhctx &ctx);
    sdk_ret_t remove(slhctx &ctx);
    sdk_ret_t reserve(slhctx &ctx);
    sdk_ret_t release(slhctx &ctx);
    sdk_ret_t find(slhctx &ctx);
    uint32_t next(uint32_t current);
    uint32_t begin() { return 0; }
    uint32_t end() { return max_; }

private:
    uint32_t max_;
    bucket *buckets_;
};

} // namespace slhash_internal
} // namespace table
} // namespace sdk

#endif // _SLHASH_BST_HPP_
