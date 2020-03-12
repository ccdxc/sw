//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lmdb based implementation of key-value store APIs
///
//----------------------------------------------------------------------------

#ifndef __KVSTORE_LMDB_HPP__
#define __KVSTORE_LMDB_HPP__

#include <sys/stat.h>
#include <lmdb.h>
#include "lib/kvstore/kvstore.hpp"

namespace sdk {
namespace lib {

class kvstore_lmdb : public kvstore {
public:
    static kvstore *factory(const char *dbpath, size_t size);
    static void destroy(kvstore *kvs);
    virtual sdk_ret_t txn_start(txn_type_t txn_type) override;
    virtual sdk_ret_t txn_commit(void) override;
    virtual sdk_ret_t txn_abort(void) override;
    virtual sdk_ret_t find(_Out_ void *key, _In_ size_t key_sz,
                           _Out_ void *data, _Inout_ size_t *data_sz) override;
    virtual sdk_ret_t insert(void *key, size_t key_sz,
                             void *data, size_t data_sz) override;
    virtual sdk_ret_t remove(void *key, size_t key_sz) override;

private:
    kvstore_lmdb() {
        env_ = NULL;
        t_txn_hdl_ = NULL;
    }
    ~kvstore_lmdb() {}
    sdk_ret_t init(const char *dbpath, size_t size);

private:
    MDB_env *env_;
    static thread_local MDB_txn *t_txn_hdl_;
    static thread_local MDB_dbi db_dbi_;
};

}    // namespace lib
}    // namespace sdk

#endif    // __KVSTORE_LMDB_HPP__
