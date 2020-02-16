//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lmdb based implementation of kvstore interface
///
//----------------------------------------------------------------------------

#include <string.h>
#include "include/sdk/mem.hpp"
#include "lib/kvstore/kvstore_lmdb.hpp"

namespace sdk {
namespace lib {

sdk_ret_t
kvstore_lmdb::init(const char *dbpath) {
    int rv;

     rv = mdb_env_create(&env_);
     if (likely(rv == 0)) {
         rv = mdb_env_set_mapsize(env_, 20 << 20);
         if (likely(rv == 0)) {
             rv = mdb_env_open(env_, dbpath, MDB_NOSUBDIR, S_IRUSR | S_IWUSR);
             if (likely(rv == 0)) {
                 return SDK_RET_OK;
             }
             mdb_env_close(env_);
         }
     }
     SDK_TRACE_ERR("kvstore init with %s failed, err %d", dbpath, rv);
     return SDK_RET_ERR;
}

kvstore *
kvstore_lmdb::factory(const char *dbpath) {
    void *mem;
    kvstore_lmdb *kvs;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_KVSTORE, sizeof(kvstore_lmdb));
    if (mem) {
        kvs = new (mem) kvstore_lmdb();
        if (kvs->init(dbpath) != SDK_RET_OK) {
            kvs->~kvstore_lmdb();
            SDK_FREE(SDK_MEM_ALLOC_KVSTORE, mem);
            return NULL;
        }
        return kvs;
    }
    return NULL;
}

void
kvstore_lmdb::destroy(kvstore *kvs) {
    kvstore_lmdb *lmdb = (kvstore_lmdb *)kvs;

    // if there is any outstanding transaction, abort it
    lmdb->txn_abort();
    mdb_env_close(lmdb->env_);
    lmdb->~kvstore_lmdb();
    SDK_FREE(SDK_MEM_ALLOC_KVSTORE, lmdb);
}

sdk_ret_t
kvstore_lmdb::txn_start(void) {
    int rv;

    if (txn_hdl_) {
        SDK_TRACE_ERR("Failed to start transaction, transaction exists "
                      "already");
        return SDK_RET_TXN_EXISTS;
    }
    rv = mdb_txn_begin(env_, NULL, 0, &txn_hdl_);
    if (rv) {
        SDK_TRACE_ERR("Failed to start new transaction, err %d", rv);
        return SDK_RET_ERR;
    }
    rv = mdb_dbi_open(txn_hdl_, NULL, MDB_CREATE, &db_dbi_);
    if (rv) {
        SDK_TRACE_ERR("Failed to open the kvstore, err %d", rv);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
kvstore_lmdb::txn_commit(void) {
    int rv;
    sdk_ret_t ret = SDK_RET_OK;

    if (!txn_hdl_) {
        SDK_TRACE_ERR("No active transaction, transaction commit failed");
        return SDK_RET_TXN_NOT_FOUND;
    }
    rv = mdb_txn_commit(txn_hdl_);
    if (rv < 0) {
        SDK_TRACE_ERR("Failed to commit transaction, err %d", rv);
        ret = SDK_RET_ERR;
        // fall thru
    }
    txn_hdl_ = NULL;
    mdb_dbi_close(env_, db_dbi_);
    return ret;
}

sdk_ret_t
kvstore_lmdb::txn_abort(void) {
    if (!txn_hdl_) {
        SDK_TRACE_ERR("No active transaction, transaction abort failed");
        return SDK_RET_INVALID_OP;
    }
    mdb_txn_abort(txn_hdl_);
    txn_hdl_ = NULL;
    mdb_dbi_close(env_, db_dbi_);
    return SDK_RET_OK;
}

sdk_ret_t
kvstore_lmdb::find(void *key, size_t key_sz, void *data, size_t *data_sz) {
    int rv;
    sdk_ret_t ret;
    MDB_val db_key;
    MDB_val db_val;
    bool new_txn = false;

    // if this is not inside a transaction, open new transaction
    if (!txn_hdl_) {
        ret = txn_start();
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }
    // lookup the key
    db_key.mv_data = key;
    db_key.mv_size = key_sz;
    rv = mdb_get(txn_hdl_, db_dbi_, &db_key, &db_val);
    if (rv) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    // if a new transaction is opened, close it
    if (new_txn == true) {
        txn_commit();
    }
    if (db_val.mv_size > *data_sz) {
        return SDK_RET_INVALID_ARG;
    }
    *data_sz = db_val.mv_size;
    memcpy(data, db_val.mv_data, db_val.mv_size);
    return SDK_RET_OK;
}

sdk_ret_t
kvstore_lmdb::insert(void *key, size_t key_sz, void *data, size_t data_sz) {
    int rv;
    MDB_val db_key;
    MDB_val db_val;
    bool new_txn = false;
    sdk_ret_t ret = SDK_RET_OK;

    // if this is not inside a transaction, open new transaction
    if (!txn_hdl_) {
        ret = txn_start();
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }
    // update the database
    db_key.mv_data = key;
    db_key.mv_size = key_sz;
    db_val.mv_data = data;
    db_val.mv_size = data_sz;
    rv = mdb_put(txn_hdl_, db_dbi_, &db_key, &db_val, 0);
    if (rv) {
        SDK_TRACE_ERR("kvstore insert failed, err %d", rv);
        ret = SDK_RET_ERR;
        // fall thru
    }
    // if a new transaction is opened, close it
    if (new_txn == true) {
        txn_commit();
    }
    return ret;
}

sdk_ret_t
kvstore_lmdb::remove(void *key, size_t key_sz) {
    int rv;
    MDB_val db_key;
    bool new_txn = false;
    sdk_ret_t ret = SDK_RET_OK;

    // if this is not inside a transaction, open new transaction
    if (!txn_hdl_) {
        ret = txn_start();
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }
    // update the database
    db_key.mv_data = key;
    db_key.mv_size = key_sz;
    rv = mdb_del(txn_hdl_, db_dbi_, &db_key, NULL);
    if (rv) {
        SDK_TRACE_ERR("kvstore insert failed, err %d", rv);
        ret = SDK_RET_ERR;
        // fall thru
    }
    // if a new transaction is opened, close it
    if (new_txn == true) {
        txn_commit();
    }
    return ret;
}

}    // namespace lib
}    // namespace sdk
