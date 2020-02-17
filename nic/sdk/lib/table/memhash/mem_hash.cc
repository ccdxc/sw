//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/lock.hpp"
#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/p4/p4_api.hpp"
#include "lib/utils/crc_fast.hpp"
#include "lib/utils/time_profile.hpp"

#include "mem_hash.hpp"
#include "mem_hash_table.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_utils.hpp"
#include "mem_hash_p4pd.hpp"

using sdk::table::sdk_table_api_params_t;
using sdk::table::mem_hash;
using sdk::table::memhash::mem_hash_main_table;
using sdk::table::memhash::mem_hash_hint_table;
using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_txn;

#define MEM_HASH_API_BEGIN(_name) {\
        MEMHASH_TRACE_VERBOSE("%s memhash begin: %s %s",\
                              "--", _name, "--");\
}

#define MEM_HASH_API_END(_name, _status) {\
        MEMHASH_TRACE_VERBOSE("%s memhash end: %s (r:%d) %s",\
                              "--", _name, _status, "--");\
}

#define MEM_HASH_API_BEGIN_() {\
        MEM_HASH_API_BEGIN(props_->name.c_str());\
        SDK_SPINLOCK_LOCK(&slock_);\
}

#define MEM_HASH_API_END_(_status) {\
        MEM_HASH_API_END(props_->name.c_str(), (_status));\
        SDK_SPINLOCK_UNLOCK(&slock_);\
}

#ifdef __aarch64__
#define CRC32X(crc, value) __asm__("crc32x %w[c], %w[c], %x[v]":[c]"+r"(crc):[v]"r"(value))
#define RBIT(value) __asm__("rbit %w0, %w1": "=r"(value) : "r"(value))
uint32_t
crc32_aarch64(const uint64_t *p, uint32_t len) {
    uint32_t crc = 0;
    for (auto i = 0; i < 8; i++) {
        CRC32X(crc, *p);
    }
    RBIT(crc);
    return crc;
}
#endif

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
mem_hash *
mem_hash::factory(sdk_table_factory_params_t *params) {
    void *mem = NULL;
    mem_hash *memhash = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN("NewTable");
    mem = (mem_hash *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH, sizeof(mem_hash));
    if (mem) {
        memhash = new (mem) mem_hash();
        ret = memhash->init_(params);
        if (ret != SDK_RET_OK) {
            memhash->~mem_hash();
            SDK_FREE(SDK_MEM_ALLOC_MEM_HASH, mem);
            memhash = NULL;
        }
    } else {
        ret = SDK_RET_OOM;
    }

    MEM_HASH_API_END("NewTable", ret);
    return memhash;
}

sdk_ret_t
mem_hash::init_(sdk_table_factory_params_t *params) {
    void *mem;
    p4pd_error_t p4pdret;
    p4pd_table_properties_t tinfo, ctinfo;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_PROPERTIES,
                     sizeof(sdk::table::properties_t));
    if (mem == NULL) {
        return SDK_RET_OOM;
    }

    SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);

    props_ = new (mem) sdk::table::properties_t();
    props_->ptable_id = params->table_id;
    props_->num_hints = params->num_hints;
    props_->max_recircs = params->max_recircs;
    //props_->health_monitor_func = params->health_monitor_func;
    props_->key2str = params->key2str;
    props_->data2str = params->appdata2str;
    props_->entry_trace_en = params->entry_trace_en;

    p4pdret = p4pd_global_table_properties_get(props_->ptable_id, &tinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->name = tinfo.tablename;

    props_->ptable_size = tinfo.tabledepth;
    SDK_ASSERT(props_->ptable_size);

    props_->swkey_len = tinfo.key_struct_size;
    SDK_ASSERT(props_->swkey_len);
    SDK_ASSERT(props_->swkey_len <= SDK_TABLE_MAX_SW_KEY_LEN);

    props_->swdata_len = tinfo.actiondata_struct_size;
    SDK_ASSERT(props_->swdata_len);
    SDK_ASSERT(props_->swdata_len <= SDK_TABLE_MAX_SW_DATA_LEN);

    props_->swappdata_len =
            p4pd_global_actiondata_appdata_size_get(props_->ptable_id, 0);
    SDK_ASSERT(props_->swappdata_len &&
               props_->swappdata_len <= SDK_TABLE_MAX_SW_DATA_LEN);

    props_->hash_poly = tinfo.hash_type;

    props_->stable_id = tinfo.oflow_table_id;
    SDK_ASSERT(props_->stable_id);

    // Assumption: All mem_hash tables will have a HINT table
    SDK_ASSERT(tinfo.has_oflow_table);

    p4pdret = p4pd_global_table_properties_get(props_->stable_id, &ctinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->stable_size = ctinfo.tabledepth;
    SDK_ASSERT(props_->stable_size);

    main_table_ = mem_hash_main_table::factory(props_);
    SDK_ASSERT_RETURN(main_table_, SDK_RET_OOM);

    MEMHASH_TRACE_VERBOSE("Creating mem_hash:%s table.", props_->name.c_str());
    MEMHASH_TRACE_VERBOSE("- ptable_id:%u ptable_size:%u ",
                          props_->ptable_id, props_->ptable_size);
    MEMHASH_TRACE_VERBOSE("- stable_id:%u stable_size:%u ",
                          props_->stable_id, props_->stable_size);
    MEMHASH_TRACE_VERBOSE("- swkey_len:%uB swdata_len:%du ",
                          props_->swkey_len, props_->swdata_len);
    MEMHASH_TRACE_VERBOSE("- num_hints:%u max_recircs:%u hash_poly:%u",
                          props_->num_hints, props_->max_recircs,
                          props_->hash_poly);

    p4pd_global_hwentry_query(props_->ptable_id, &props_->hwkey_len,
                              NULL, &props_->hwdata_len);
    MEMHASH_TRACE_VERBOSE("- swkey_len:%ubits swdata_len:%ubits ",
                          props_->hwkey_len, props_->hwdata_len);

    props_->hwkey_len = SDK_TABLE_BITS_TO_BYTES(props_->hwkey_len);
    props_->hwkey_len = SDK_TABLE_ALIGN_TO_64B(props_->hwkey_len);
    SDK_ASSERT(props_->hwkey_len <= SDK_TABLE_MAX_HW_KEY_LEN);

    props_->hwdata_len = SDK_TABLE_BITS_TO_BYTES(props_->hwdata_len);
    props_->hwdata_len = SDK_TABLE_ALIGN_TO_64B(props_->hwdata_len);
    SDK_ASSERT(props_->hwdata_len <= SDK_TABLE_MAX_HW_DATA_LEN);
    MEMHASH_TRACE_VERBOSE("- hwkey_len:%uBytes hwdata_len:%uBytes ",
                          props_->hwkey_len, props_->hwdata_len);

    mem_hash_p4pd_stats_reset();
    return SDK_RET_OK;
}

mem_hash::mem_hash() {
    props_ = NULL;
    main_table_ = NULL;
    SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
}

//---------------------------------------------------------------------------
// mem_hash Destructor
//---------------------------------------------------------------------------
void
mem_hash::destroy(mem_hash *table) {
    MEM_HASH_API_BEGIN("DestroyTable");
    MEMHASH_TRACE_VERBOSE("%p", table);
    mem_hash_main_table::destroy_(static_cast<mem_hash_main_table*>(table->main_table_));

    SDK_ASSERT(table->txn_.validate() == SDK_RET_OK);
    MEM_HASH_API_END("DestroyTable", SDK_RET_OK);

    mem_hash_p4pd_stats_print();
    table->~mem_hash();
    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH, table);
}

mem_hash::~mem_hash() {
    props_->~properties_t();
    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_PROPERTIES, props_);
    SDK_SPINLOCK_DESTROY(&slock_);
}

//---------------------------------------------------------------------------
// mem_hash Insert entry with hash value
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::genhash_(sdk_table_api_params_t *params) {
    uint8_t hwkey[SDK_TABLE_MAX_HW_KEY_LEN];
    p4pd_error_t p4pdret = P4PD_SUCCESS;
    uint32_t hash_32b = 0;

    if (params->hash_valid) {
        // If hash_valid is set in the params, skip computing it.
        return SDK_RET_OK;
    }

    memset(hwkey, 0, sizeof(hwkey));

    MEMHASH_TRACE_VERBOSE("TID=%d, KL=%d, DL=%d", props_->ptable_id,
                          props_->hwkey_len, props_->hwdata_len);
    p4pdret = mem_hash_p4pd_hwkey_hwmask_build(props_->ptable_id,
                                               params->key, NULL,
                                               hwkey, NULL);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
//#ifdef SIM
    hash_32b = sdk::utils::crc32(hwkey, props_->hwkey_len, props_->hash_poly);
//#else
//    hash_32b = crc32_aarch64((uint64_t *)hwkey, props_->hwkey_len);
//#endif
    params->hash_32b = hash_32b;
    params->hash_valid = true;
    MEMHASH_TRACE_VERBOSE("[%s] => H:%#x",
                          mem_hash_utils_rawstr(hwkey, props_->hwkey_len), hash_32b);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash: Create API context. This is used by all APIs
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::ctxinit_(sdk_table_api_op_t op,
                   sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEMHASH_TRACE_VERBOSE("op:%d", op);
    if (SDK_TABLE_API_OP_IS_CRUD(op)) {
        ret = genhash_(params);
        if (ret != SDK_RET_OK) {
            MEMHASH_TRACE_ERR("failed to generate hash, ret:%d", ret);
            return ret;
        }
    }

    MEMHASH_API_CONTEXT_INIT_MAIN(apictx_, op, params,
                                  props_, &table_stats_, &txn_);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Insert entry to mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::insert(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key && params->appdata);

    time_profile_begin(sdk::utils::time_profile::TABLE_LIB_MEMHASH_INSERT);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_INSERT, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto insert_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->insert_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("MainTable: insert failed: ret:%d", ret);
    }

insert_return:
    time_profile_end(sdk::utils::time_profile::TABLE_LIB_MEMHASH_INSERT);
    api_stats_.insert(ret);
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Update entry to mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::update(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key && params->appdata);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_UPDATE, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto update_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->update_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("update_ failed. ret:%d", ret);
        goto update_return;
    }

update_return:
    api_stats_.update(ret);
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Remove entry from mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::remove(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_REMOVE, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto remove_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->remove_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("remove_ failed. ret:%d", ret);
        goto remove_return;
    }

remove_return:
    api_stats_.remove(ret);
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Get entry from mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::get(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_GET, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto get_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->get_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("remove_ failed. ret:%d", ret);
        goto get_return;
    }

get_return:
    api_stats_.get(ret);
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Reserve entry in mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::reserve(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_RESERVE, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto reserve_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->insert_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("reserve_ failed. ret:%d", ret);
    } else {
        SDK_ASSERT(txn_.is_valid());
        txn_.reserve();
        MEMHASH_TRACE_VERBOSE("handle = %s", apictx_.handle->tostr());
    }

reserve_return:
    api_stats_.reserve(ret);
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Release entry in mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::release(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key && params->handle.valid());

    ret = ctxinit_(sdk::table::SDK_TABLE_API_RELEASE, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto release_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->remove_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("release_ failed. ret:%d", ret);
    } else {
        SDK_ASSERT(txn_.is_valid());
        txn_.release();
        MEMHASH_TRACE_VERBOSE("handle = %s", apictx_.handle->tostr());
    }

release_return:
    api_stats_.release(ret);
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Get Stats from mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::stats_get(sdk_table_api_stats_t *api_stats,
                    sdk_table_stats_t *table_stats) {
    MEM_HASH_API_BEGIN_();
    api_stats_.get(api_stats);
    table_stats_.get(table_stats);
    MEM_HASH_API_END_(SDK_RET_OK);
    return SDK_RET_OK;
}

sdk_ret_t
mem_hash::state_validate() {
    auto ret = ctxinit_(sdk::table::SDK_TABLE_API_VALIDATE, NULL);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->validate_(&apictx_);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash start transaction
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::txn_start() {
    sdk_ret_t ret;
    MEM_HASH_API_BEGIN_();
    ret = txn_.start();
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash end transaction
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::txn_end() {
    sdk_ret_t ret = SDK_RET_OK;
    MEM_HASH_API_BEGIN_();
    ret = txn_.end();
    MEM_HASH_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash end transaction
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::iterate(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->itercb);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_ITERATE, params);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto iterate_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->iterate_(&apictx_);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("main table iteration failed. ret:%d", ret);
    }

iterate_return:
    MEM_HASH_API_END_(ret);
    return ret;
}
