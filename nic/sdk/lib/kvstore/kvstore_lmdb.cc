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
#include <unistd.h>
#include "include/sdk/mem.hpp"
#include "lib/kvstore/kvstore_lmdb.hpp"

namespace sdk {
namespace lib {

thread_local MDB_txn *kvstore_lmdb::t_txn_hdl_ = NULL;
thread_local MDB_dbi kvstore_lmdb::db_dbi_;

sdk_ret_t
kvstore_lmdb::init(string dbpath, size_t size) {
    int rv;
    const char *db = dbpath.c_str();

    // remove the kvstore file, if it exists
    unlink(db);
    unlink((dbpath + "-db.lock").c_str());

     rv = mdb_env_create(&env_);
     if (likely(rv == 0)) {
         rv = mdb_env_set_mapsize(env_, size);
         if (likely(rv == 0)) {
             rv = mdb_env_open(env_, db, MDB_NOSUBDIR | MDB_WRITEMAP,
                               S_IRUSR | S_IWUSR);
             if (likely(rv == 0)) {
                 // everything went through
                 return SDK_RET_OK;
             } else {
                 mdb_env_close(env_);
                 SDK_TRACE_ERR("kvstore %s (size %lu) env open failed,  err %d",
                               db, size, rv);
             goto end;
             }
         } else {
             SDK_TRACE_ERR("kvstore %s (size %lu) env set failed, err %d",
                           db, size, rv);
             goto end;
         }
     } else {
         SDK_TRACE_ERR("kvstore %s (size %lu) env create failed, err %d",
                       db, size, rv);
     }

end:

     return SDK_RET_ERR;
}

kvstore *
kvstore_lmdb::factory(string dbpath, size_t size) {
    void *mem;
    kvstore_lmdb *kvs;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_KVSTORE, sizeof(kvstore_lmdb));
    if (mem) {
        kvs = new (mem) kvstore_lmdb();
        if (kvs->init(dbpath, size) != SDK_RET_OK) {
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
kvstore_lmdb::txn_start(txn_type_t txn_type) {
    int rv;

    if (t_txn_hdl_) {
        SDK_TRACE_ERR("Failed to start transaction, transaction exists "
                      "already");
        return SDK_RET_TXN_EXISTS;
    }
    if (txn_type == TXN_TYPE_READ_WRITE) {
        rv = mdb_txn_begin(env_, NULL, 0, &t_txn_hdl_);
    } else {
        rv = mdb_txn_begin(env_, NULL, MDB_RDONLY, &t_txn_hdl_);
    }
    if (rv) {
        SDK_TRACE_ERR("Failed to start new transaction, err %d", rv);
        return SDK_RET_ERR;
    }
    rv = mdb_dbi_open(t_txn_hdl_, NULL, MDB_CREATE, &db_dbi_);
    if (rv) {
        SDK_TRACE_ERR("Failed to open the kvstore, err %d", rv);
        mdb_txn_abort(t_txn_hdl_);
        t_txn_hdl_ = NULL;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
kvstore_lmdb::txn_commit(void) {
    int rv;
    sdk_ret_t ret = SDK_RET_OK;

    if (!t_txn_hdl_) {
        SDK_TRACE_ERR("No active transaction, transaction commit failed");
        return SDK_RET_TXN_NOT_FOUND;
    }
    rv = mdb_txn_commit(t_txn_hdl_);
    if (rv < 0) {
        SDK_TRACE_ERR("Failed to commit transaction, err %d", rv);
        ret = SDK_RET_ERR;
        // fall thru
    }
    t_txn_hdl_ = NULL;
    mdb_dbi_close(env_, db_dbi_);
    return ret;
}

sdk_ret_t
kvstore_lmdb::txn_abort(void) {
    if (!t_txn_hdl_) {
        SDK_TRACE_ERR("No active transaction, transaction abort failed");
        return SDK_RET_INVALID_OP;
    }
    mdb_txn_abort(t_txn_hdl_);
    t_txn_hdl_ = NULL;
    mdb_dbi_close(env_, db_dbi_);
    return SDK_RET_OK;
}

// matches the key with the prefix and returns the key sans prefix if it matches
void *
kvstore_lmdb::key_prefix_match_(void *key, size_t key_sz,
                                std::string key_prefix) {
    char *kvs_key = (char *)key;

    if (((key_prefix.size() == 0) && (kvs_key[0] == ':')) ||
        (memcmp(kvs_key, key_prefix.c_str(), key_prefix.size()) == 0)) {
        kvs_key += (key_prefix.size() + 1);
        return kvs_key;
    } else {
        return NULL;
    }
}

sdk_ret_t
kvstore_lmdb::iterate(kvstore_iterate_cb_t cb, void *ctxt,
                      std::string key_prefix) {
    int rv;
    sdk_ret_t ret;
    void *key_data;
    MDB_cursor *db_cur;
    MDB_val db_key;
    MDB_val db_val;
    bool new_txn = false;
    bool cursor_open = false;

    // if this is not inside a transaction, open new transaction
    if (!t_txn_hdl_) {
        ret = txn_start(TXN_TYPE_READ_ONLY);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }

    rv = mdb_cursor_open(t_txn_hdl_, db_dbi_, &db_cur);
    if (rv) {
        ret = SDK_RET_ERR;
        goto end;
    }
    cursor_open = true;

    rv = mdb_cursor_get(db_cur, &db_key, &db_val, MDB_FIRST);
    if (rv && rv != MDB_NOTFOUND) {
        SDK_TRACE_ERR("kvstore iterate failed, err %d", rv);
        ret = SDK_RET_ERR;
        goto end;
    }
    while (rv != MDB_NOTFOUND) {
        key_data = key_prefix_match_(db_key.mv_data, db_key.mv_size,
                                     key_prefix);
        if (key_data) {
            cb(key_data, db_val.mv_data, ctxt);
        }
        rv = mdb_cursor_get(db_cur, &db_key, &db_val, MDB_NEXT);
        if (rv && rv != MDB_NOTFOUND) {
            SDK_TRACE_ERR("kvstore iterate failed, err %d", rv);
            ret = SDK_RET_ERR;
            goto end;
        }
    }
    ret = SDK_RET_OK;
end:
    if (cursor_open) {
        mdb_cursor_close(db_cur);
    }
    // if a new transaction is opened, close it
    if (new_txn == true) {
        if (rv) {
            txn_abort();
        } else {
            txn_commit();
        }
    }
    return ret;
}

sdk_ret_t
kvstore_lmdb::find(const void *key, size_t key_sz,
                   void *data, size_t *data_sz,
                   std::string key_prefix) {
    int rv;
    sdk_ret_t ret;
    MDB_val db_key;
    MDB_val db_val;
    std::string kvs_key;
    bool new_txn = false;

    // if this is not inside a transaction, open new transaction
    if (!t_txn_hdl_) {
        ret = txn_start(TXN_TYPE_READ_ONLY);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }
    // compute the key with prefix
    kvs_key = key_prefix + ":" + std::string((char *)key, key_sz);
    db_key.mv_data = (void *)kvs_key.c_str();
    db_key.mv_size = kvs_key.size();
    rv = mdb_get(t_txn_hdl_, db_dbi_, &db_key, &db_val);
    if (rv) {
        ret = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
    }
    if (db_val.mv_size > *data_sz) {
        ret = SDK_RET_INVALID_ARG;
        goto end;
    }
    *data_sz = db_val.mv_size;
    memcpy(data, db_val.mv_data, db_val.mv_size);
    ret = SDK_RET_OK;

end:

    // if a new transaction is opened, close it
    if (new_txn == true) {
        if (rv) {
            txn_abort();
        } else {
            txn_commit();
        }
    }
    return ret;
}

sdk_ret_t
kvstore_lmdb::insert(const void *key, size_t key_sz,
                     const void *data, size_t data_sz,
                     std::string key_prefix) {
    int rv;
    MDB_val db_key;
    MDB_val db_val;
    bool new_txn = false;
    std::string kvs_key;
    sdk_ret_t ret = SDK_RET_OK;

    // if this is not inside a transaction, open new transaction
    if (!t_txn_hdl_) {
        ret = txn_start(TXN_TYPE_READ_WRITE);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }
    // update the database
    kvs_key = key_prefix + ":" + std::string((char *)key, key_sz);
    db_key.mv_data = (void *)kvs_key.c_str();
    db_key.mv_size = kvs_key.size();
    db_val.mv_data = (void *)data;
    db_val.mv_size = data_sz;
    rv = mdb_put(t_txn_hdl_, db_dbi_, &db_key, &db_val, 0);
    if (rv) {
        SDK_TRACE_ERR("kvstore insert failed, err %d", rv);
        ret = SDK_RET_ERR;
        goto end;
    }

end:

    // if a new transaction is opened, close it
    if (new_txn == true) {
        if (rv) {
            txn_abort();
        } else {
            txn_commit();
        }
    }
    return ret;
}

sdk_ret_t
kvstore_lmdb::remove(const void *key, size_t key_sz,
                     std::string key_prefix) {
    int rv;
    MDB_val db_key;
    std::string kvs_key;
    bool new_txn = false;
    sdk_ret_t ret = SDK_RET_OK;

    // if this is not inside a transaction, open new transaction
    if (!t_txn_hdl_) {
        ret = txn_start(TXN_TYPE_READ_WRITE);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        new_txn = true;
    }
    // update the database
    kvs_key = key_prefix + ":" + std::string((char *)key, key_sz);
    db_key.mv_data = (void *)kvs_key.c_str();
    db_key.mv_size = kvs_key.size();
    rv = mdb_del(t_txn_hdl_, db_dbi_, &db_key, NULL);
    if (rv) {
        SDK_TRACE_ERR("kvstore remove failed, err %d", rv);
        ret = SDK_RET_ERR;
        goto end;
    }

end:

    // if a new transaction is opened, close it
    if (new_txn == true) {
        if (rv) {
            txn_abort();
        } else {
            txn_commit();
        }
    }
    return ret;
}

}    // namespace lib
}    // namespace sdk
