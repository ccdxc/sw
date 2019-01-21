//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/lock.hpp"
#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/p4/p4_api.hpp"
#include "lib/utils/crc_fast.hpp"

#include "mem_hash.hpp"
#include "mem_hash_api_context.hpp"
#include "mem_hash_table.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_utils.hpp"

using sdk::table::sdk_table_api_params_t;
using sdk::table::mem_hash;
using sdk::table::memhash::mem_hash_api_context;
using sdk::table::memhash::mem_hash_main_table;
using sdk::table::memhash::mem_hash_hint_table;
using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_txn;

#define MEM_HASH_MAX_HW_KEY_LEN 128
thread_local uint8_t g_hw_key[MEM_HASH_MAX_HW_KEY_LEN];
uint32_t mem_hash_api_context::numctx_ = 0;

#define MEM_HASH_API_BEGIN(_name) {\
        const char *marker = "-------------------------"; \
        SDK_TRACE_DEBUG("%s Api Begin For Table: %s %s",\
                        marker, _name, marker);\
}

#define MEM_HASH_API_END(_name) {\
        const char *marker = "-------------------------"; \
        SDK_TRACE_DEBUG("%s Api End For Table: %s %s",\
                        marker, _name, marker);\
}

#define MEM_HASH_API_BEGIN_() {\
        MEM_HASH_API_BEGIN(props_->name.c_str());\
        SDK_SPINLOCK_UNLOCK(&slock_);\
}

#define MEM_HASH_API_END_() {\
        MEM_HASH_API_END(props_->name.c_str());\
        SDK_SPINLOCK_LOCK(&slock_);\
}

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
        }
    }

    MEM_HASH_API_END("NewTable");
    return memhash;
}

sdk_ret_t
mem_hash::init_(sdk_table_factory_params_t *params) {
    p4pd_error_t p4pdret;
    p4pd_table_properties_t tinfo, ctinfo;

    props_ = (mem_hash_properties_t *)SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_PROPERTIES,
                                                 sizeof(mem_hash_properties_t));
    if (props_ == NULL) {
        return SDK_RET_OOM;
    }

    props_->main_table_id = params->table_id;
    props_->num_hints = params->num_hints;
    props_->max_recircs = params->max_recircs;
    //props_->health_monitor_func = params->health_monitor_func;
    props_->key2str = params->key2str;
    props_->appdata2str = params->appdata2str;

    p4pdret = p4pd_table_properties_get(props_->main_table_id, &tinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->name = tinfo.tablename;

    props_->main_table_size = tinfo.tabledepth;
    SDK_ASSERT(props_->main_table_size);

    props_->key_len = tinfo.key_struct_size;
    SDK_ASSERT(props_->key_len);
    props_->data_len = tinfo.actiondata_struct_size;
    SDK_ASSERT(props_->data_len);

    props_->hash_poly = tinfo.hash_type;

    // Initialize CRC Fast
    crc32gen_ = crcFast::factory();
    SDK_ASSERT(crc32gen_);

    props_->hint_table_id = tinfo.oflow_table_id;
    SDK_ASSERT(props_->hint_table_id);

    // Assumption: All mem_hash tables will have a HINT table
    SDK_ASSERT(tinfo.has_oflow_table);

    p4pdret = p4pd_table_properties_get(props_->hint_table_id, &ctinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->hint_table_size = ctinfo.tabledepth;
    SDK_ASSERT(props_->hint_table_size);

    main_table_ = mem_hash_main_table::factory(props_);
    SDK_ASSERT_RETURN(main_table_, SDK_RET_OOM);

    SDK_TRACE_DEBUG("Creating mem_hash:%s table.", props_->name.c_str());
    SDK_TRACE_DEBUG("- main_table_id:%d main_table_size:%d ",
                    props_->main_table_id, props_->main_table_size);
    SDK_TRACE_DEBUG("- hint_table_id:%d hint_table_size:%d ",
                    props_->hint_table_id, props_->hint_table_size);
    SDK_TRACE_DEBUG("- key_len:%dB data_len:%dB ",
                    props_->key_len, props_->data_len);
    SDK_TRACE_DEBUG("- num_hints:%d max_recircs:%d hash_poly:%d",
                    props_->num_hints, props_->max_recircs, props_->hash_poly);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Destructor
//---------------------------------------------------------------------------
void
mem_hash::destroy(mem_hash *table) {
    MEM_HASH_API_BEGIN("DestroyTable");
    SDK_TRACE_DEBUG("Destroying mem_hash table %p", table);
    mem_hash_main_table::destroy_(static_cast<mem_hash_main_table*>(table->main_table_));

    SDK_TRACE_DEBUG("Number of API contexts = %d", mem_hash_api_context::count());
    SDK_ASSERT(mem_hash_api_context::count() == 0);
    SDK_ASSERT(table->txn_.validate() == SDK_RET_OK);
    MEM_HASH_API_END("DestroyTable");
}

//---------------------------------------------------------------------------
// mem_hash Insert entry with hash value
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::genhash_(sdk_table_api_params_t *params) {
    uint32_t hw_key_len = 0;
    uint32_t hw_data_len = 0;
    p4pd_error_t p4pdret = P4PD_SUCCESS;
    uint32_t hash_32b = 0;

    if (params->hash_valid) {
        // If hash_valid is set in the params, skip computing it.
        return SDK_RET_OK;
    }

    memset(g_hw_key, 0, sizeof(g_hw_key));
    p4pd_hwentry_query(props_->main_table_id, &hw_key_len,
                       NULL, &hw_data_len);
 
    SDK_TRACE_DEBUG("Generating Hash: TableID=%d, KeyLength=%d, DataLength=%d",
                    props_->main_table_id, hw_key_len, hw_data_len);
    // round off to higher byte
    hw_key_len = (hw_key_len >> 3) + ((hw_key_len & 0x7) ? 1 : 0);
    if (hw_key_len * 8 > 512) {
        if (hw_key_len % 64) {
            hw_key_len +=  (64 - (hw_key_len % 64));
        }
    }
    
    SDK_TRACE_DEBUG("Generating Hash: TableID=%d, KeyLength=%d, DataLength=%d",
                    props_->main_table_id, hw_key_len, hw_data_len);
    p4pdret = p4pd_hwkey_hwmask_build(props_->main_table_id,
                                      params->key, NULL,
                                      g_hw_key, NULL);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    SDK_TRACE_DEBUG("HW Key: [%s]",
                    mem_hash_utils_rawstr(g_hw_key, hw_key_len));
    hash_32b = crc32gen_->compute_crc(g_hw_key, hw_key_len,
                                      props_->hash_poly);
    params->hash_32b = hash_32b;
    params->hash_valid = true;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash: Create API context. This is used by all APIs
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::create_api_context_(sdk_table_api_op_t op,
                              sdk_table_api_params_t *params,
                              void **retctx) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context
    
    ret = genhash_(params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to generate hash, ret:%d", ret);
        return ret;
    }

    SDK_TRACE_DEBUG("Memhash inserting entry");
    mctx = mem_hash_api_context::factory(op, params, props_,
                                         &table_stats_, &txn_);
    if (!mctx) {
        SDK_TRACE_ERR("MainTable: create api context failed ret:%d", ret);
        return SDK_RET_OOM;
    }
    
    mctx->print_input();
    *retctx = mctx;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Insert entry to mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::insert(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key && params->appdata);

    ret = create_api_context_(sdk::table::SDK_TABLE_API_INSERT,
                              params, (void **)&mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->insert_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("MainTable: insert failed: ret:%d", ret);
    }
    
    mem_hash_api_context::destroy(mctx);

    api_stats_.insert(ret);
    MEM_HASH_API_END_();
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Update entry to mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::update(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key && params->appdata);

    ret = create_api_context_(sdk::table::SDK_TABLE_API_UPDATE,
                              params, (void **)&mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->update_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("update_ failed. ret:%d", ret);
        mem_hash_api_context::destroy(mctx);
        goto update_return;
    }

    mem_hash_api_context::destroy(mctx);

update_return:
    api_stats_.update(ret);
    MEM_HASH_API_END_();
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Remove entry from mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::remove(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = create_api_context_(sdk::table::SDK_TABLE_API_REMOVE,
                              params, (void **)&mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->remove_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("remove_ failed. ret:%d", ret);
        mem_hash_api_context::destroy(mctx);
        goto remove_return;
    }

    mem_hash_api_context::destroy(mctx);

remove_return:
    api_stats_.remove(ret);
    MEM_HASH_API_END_();
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Get entry from mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::get(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = create_api_context_(sdk::table::SDK_TABLE_API_GET,
                              params, (void **)&mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->get_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("remove_ failed. ret:%d", ret);
        mem_hash_api_context::destroy(mctx);
        goto get_return;
    }

    mem_hash_api_context::destroy(mctx);

get_return:
    api_stats_.get(ret);
    MEM_HASH_API_END_();
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash Reserve entry in mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::reserve(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = create_api_context_(sdk::table::SDK_TABLE_API_RESERVE,
                              params, (void **)&mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }
    
    ret = static_cast<mem_hash_main_table*>(main_table_)->insert_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("reserve_ failed. ret:%d", ret);
    } else {
        SDK_ASSERT(txn_.is_valid());
        txn_.reserve();
        mctx->print_handle();
    }
    
    mem_hash_api_context::destroy(mctx);

    api_stats_.reserve(ret);
    MEM_HASH_API_END_();
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Release entry in mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::release(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_API_BEGIN_();
    SDK_ASSERT(params->key && params->handle);

    ret = create_api_context_(sdk::table::SDK_TABLE_API_RELEASE,
                              params, (void **)&mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to create api context. ret:%d", ret);
        return ret;
    }
    
    ret = static_cast<mem_hash_main_table*>(main_table_)->remove_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("reserve_ failed. ret:%d", ret);
    } else {
        SDK_ASSERT(txn_.is_valid());
        txn_.release();
        mctx->print_handle();
    }
    
    mem_hash_api_context::destroy(mctx);

    api_stats_.release(sdk::SDK_RET_OK);
    MEM_HASH_API_END_();
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Get Stats from mem_hash table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::stats_get(sdk_table_api_stats_t *stats,
                    sdk_table_stats_t *table_stats) {
    MEM_HASH_API_BEGIN_();
    api_stats_.get(stats);
    table_stats_.get(table_stats);
    MEM_HASH_API_END_();
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
    MEM_HASH_API_END_();
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash start transaction
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::txn_end() {
    sdk_ret_t ret = SDK_RET_OK;
    MEM_HASH_API_BEGIN_();
    ret = txn_.end();
    MEM_HASH_API_END_();
    return ret;
}
