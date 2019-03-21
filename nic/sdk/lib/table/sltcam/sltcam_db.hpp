//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef _SLTCAM_BST_HPP_
#define _SLTCAM_BST_HPP_

#include <stdint.h>
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "sltcam_api_context.hpp"

namespace sdk {
namespace table {
namespace sltcam_internal {

class elem {
private:
    uint32_t data_:31;
    uint8_t valid_:1;

public:
    bool valid() { return valid_; }
    uint32_t data() { return data_; }
    void clear() { data_ = 0; valid_ = 0; }
    void set(uint32_t data) { data_ = data; valid_ = 1; }
} __attribute__((__packed__));

class db {
private:
    uint32_t max_elems_;
    uint32_t count_; // Number of elems in the db
    elem *elems_;

private:
    elem* elem_get_(uint32_t n) { return &elems_[n]; }
    sdk_ret_t grow_(sltctx *ctx);
    sdk_ret_t shrink_(sltctx *ctx);
    sdk_ret_t findslot_(sltctx *ctx);
    bool isempty() { return count_ == 0; }

public:
    db() { max_elems_ = 0; count_ = 0; elems_ = NULL; }
    ~db() { SDK_FREE(SDK_MEM_ALLOC_ABST_NODES, elems_); }
    sdk_ret_t init(uint32_t max_elems);
    sdk_ret_t insert(sltctx *ctx);
    sdk_ret_t remove(sltctx *ctx);
    sdk_ret_t find(sltctx *ctx);
    uint32_t begin() { return 0; }
    uint32_t end() { return count_; }
    uint32_t element(uint32_t iter) { return elems_[iter].data(); }
    void sanitize(sltctx *ctx);
};

} // namespace sltcam_internal
} // namespace table
} // namespace sdk

#endif // _SLTCAM_BST_HPP_
