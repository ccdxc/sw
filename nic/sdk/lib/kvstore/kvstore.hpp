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

#include <string>
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

namespace sdk {
namespace lib {

typedef void (*kvstore_iterate_cb_t)(void *key, void *val, void *ctxt);

class kvstore {
public:
    typedef enum txn_type_e {
        TXN_TYPE_READ_ONLY,
        TXN_TYPE_READ_WRITE,
    } txn_type_t;
    static kvstore *factory(std::string dbpath, size_t size);
    static void destroy(kvstore *kvs);
    virtual sdk_ret_t txn_start(txn_type_t txn_type) = 0;
    virtual sdk_ret_t txn_commit(void) = 0;
    virtual sdk_ret_t txn_abort(void) = 0;
    virtual sdk_ret_t find(_In_ const void *key, _In_ size_t key_sz,
                           _Out_ void *data, _Inout_ size_t *data_sz,
                           std::string key_prefix) = 0;
    virtual sdk_ret_t insert(const void *key, size_t key_sz,
                             const void *data, size_t data_sz,
                             std::string key_prefix) = 0;
    virtual sdk_ret_t remove(const void *key, size_t key_sz,
                             std::string key_prefix) = 0;
    virtual sdk_ret_t iterate(kvstore_iterate_cb_t cb, void *ctxt,
                              std::string key_prefix) = 0;

protected:
    kvstore() {}
    ~kvstore() {}
};

}    // namespace lib
}    // namespace sdk

#endif    // __KVSTORE_HPP__
