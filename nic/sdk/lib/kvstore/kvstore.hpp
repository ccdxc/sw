//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Generic key-value store interface APIs
///
//----------------------------------------------------------------------------

#ifndef __KVSTORE_HPP__
#define __KVSTORE_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

namespace sdk {
namespace lib {

class kvstore {
public:
    static kvstore *factory(const char *dbpath);
    static void destroy(kvstore *kvs);
    virtual sdk_ret_t insert(void *key, size_t key_sz,
                             void *data, size_t data_sz) = 0;
    virtual sdk_ret_t remove(void *key, size_t key_sz) = 0;
    virtual sdk_ret_t txn_start(void) = 0;
    virtual sdk_ret_t txn_commit(void) = 0;
    virtual sdk_ret_t txn_abort(void) = 0;

protected:
    kvstore() {}
    ~kvstore() {}
};

}    // namespace lib
}    // namespace sdk

#endif    // __KVSTORE_HPP__
