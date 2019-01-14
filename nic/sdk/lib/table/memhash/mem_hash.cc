//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "lib/utils/crc_fast.hpp"

#include "mem_hash.hpp"
#include "mem_hash_api_context.hpp"
#include "mem_hash_table.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_utils.hpp"

using sdk::table::mem_hash;
using sdk::table::memhash::mem_hash_api_context;
using sdk::table::memhash::mem_hash_main_table;
using sdk::table::memhash::mem_hash_hint_table;
using sdk::table::memhash::mem_hash_table_bucket;

uint32_t mem_hash_api_context::numctx_ = 0;


//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
mem_hash *
mem_hash::factory(mem_hash_factory_params_t *params) {
    void *mem = NULL;
    mem_hash *memhash = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    MEM_HASH_TRACE_API_BEGIN();
    mem = (mem_hash *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH, sizeof(mem_hash));
    if (mem) {
        memhash = new (mem) mem_hash();
        ret = memhash->init_(params);
        if (ret != SDK_RET_OK) {
            memhash->~mem_hash();
            SDK_FREE(SDK_MEM_ALLOC_MEM_HASH, mem);
        }
    }

    MEM_HASH_TRACE_API_END();
    return memhash;
}

sdk_ret_t
mem_hash::init_(mem_hash_factory_params_t *params) {
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
    props_->health_monitor_func = params->health_monitor_func;
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
    props_->appdata_len = tinfo.appdata_struct_size;
    SDK_ASSERT(props_->appdata_len);

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

    hint_table_ = mem_hash_hint_table::factory(props_->hint_table_id,
                                               props_->hint_table_size);
    SDK_ASSERT_RETURN(hint_table_, SDK_RET_OOM);

    main_table_ = mem_hash_main_table::factory(props_->main_table_id,
                                               props_->main_table_size,
                                               hint_table_);
    SDK_ASSERT_RETURN(main_table_, SDK_RET_OOM);


    SDK_TRACE_DEBUG("Creating mem_hash:%s table.", props_->name.c_str());
    SDK_TRACE_DEBUG("- main_table_id:%d main_table_size:%d ",
                    props_->main_table_id, props_->main_table_size);
    SDK_TRACE_DEBUG("- hint_table_id:%d hint_table_size:%d ",
                    props_->hint_table_id, props_->hint_table_size);
    SDK_TRACE_DEBUG("- key_len:%dB data_len:%dB appdata_len:%dB ",
                    props_->key_len, props_->data_len,
                    props_->appdata_len);
    SDK_TRACE_DEBUG("- num_hints:%d max_recircs:%d hash_poly:%d",
                    props_->num_hints, props_->max_recircs, props_->hash_poly);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Destructor
//---------------------------------------------------------------------------
void
mem_hash::destroy(mem_hash *table) {
    MEM_HASH_TRACE_API_BEGIN();
    SDK_TRACE_DEBUG("Destroying mem_hash table %p", table);
    mem_hash_main_table::destroy_(static_cast<mem_hash_main_table*>(table->main_table_));
    mem_hash_hint_table::destroy_(static_cast<mem_hash_hint_table*>(table->hint_table_));

    SDK_TRACE_DEBUG("Number of API contexts = %d", mem_hash_api_context::count());
    SDK_ASSERT(mem_hash_api_context::count() == 0);
    MEM_HASH_TRACE_API_END();
}

//---------------------------------------------------------------------------
// mem_hash Insert entry with hash value
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::genhash_(mem_hash_api_params_t *params) {
    if (params->hash_valid) {
        // Hash is already valid for this params.
        return SDK_RET_OK;
    }
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash Insert entry with hash value
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::insert(mem_hash_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_TRACE_API_BEGIN();

    ret = genhash_(params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to generate hash, ret:%d", ret);
        return ret;
    }

    SDK_TRACE_DEBUG("Memhash inserting entry, Hash32bits:%#x", params->hash_32b);
    SDK_TRACE_DEBUG("- Key:[%s]", props_->key2str(params->key));
    SDK_TRACE_DEBUG("- AppData:[%s]", props_->appdata2str(params->appdata));
    mctx = mem_hash_api_context::factory(params, props_);
    if (!mctx) {
        SDK_TRACE_ERR("MainTable: create api context failed ret:%d", ret);
        ret = SDK_RET_OOM;
        goto insert_return;
    }

    // INSERT SEQUENCE:
    // 1) Insert to Main Table
    // 2) If COLLISION:
    //      2.1) Call Hint Table insert api
    //          2.1.1) Insert to Hint Table
    //          2.1.2) If COLLISION:
    //              2.1.2.1) Recursive call to (2.1)
    //          2.1.3) If SUCCESS, write Hint Table bucket to HW
    //      2.2) If Hint Table insert is Successful,
    //           Write the Main Table bucket to HW
    // 3) Else if SUCCESS, insert is complete.

    ret = static_cast<mem_hash_main_table*>(main_table_)->insert_(mctx);
    if (ret != SDK_RET_COLLISION) {
        SDK_TRACE_DEBUG("MainTable: insert status: ret:%d", ret);
    } else {
        // COLLISION case
        ret = static_cast<mem_hash_hint_table*>(hint_table_)->insert_(mctx);
        // Now that all downstream nodes are written to HW, we can update the
        // main entry. This will ensure make before break for any downstream
        // changes.
        if (ret == SDK_RET_OK) {
            ret = static_cast<mem_hash_table_bucket*>(mctx->bucket)->write_(mctx);
        }
    }

    mem_hash_api_context::destroy(mctx);

insert_return:
    MEM_HASH_TRACE_API_END();
    return ret;
}

sdk_ret_t
mem_hash::update(mem_hash_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_TRACE_API_BEGIN();

    SDK_TRACE_DEBUG("Memhash updating entry, Hash32bits:%#x", params->hash_32b);
    SDK_ASSERT(params->key);
    SDK_TRACE_DEBUG("- Key:[%s]", props_->key2str(params->key));
    SDK_ASSERT(params->appdata);
    SDK_TRACE_DEBUG("- AppData:[%s]", props_->appdata2str(params->appdata));

    ret = genhash_(params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to generate hash, ret:%d", ret);
        return ret;
    }

    mctx = mem_hash_api_context::factory(params, props_);
    if (!mctx) {
        SDK_TRACE_ERR("MainTable: create api context failed ret:%d", ret);
        ret = SDK_RET_OOM;
        goto update_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->update_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("update_ failed. ret:%d", ret);
        mem_hash_api_context::destroy(mctx);
        goto update_return;
    }

    mem_hash_api_context::destroy(mctx);

update_return:
    MEM_HASH_TRACE_API_END();
    return ret;
}

sdk_ret_t
mem_hash::remove(mem_hash_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *mctx = NULL;   // Main Table Context

    MEM_HASH_TRACE_API_BEGIN();

    SDK_TRACE_DEBUG("Memhash removing entry, Hash32bits:%#x", params->hash_32b);
    SDK_TRACE_DEBUG("- Key:[%s]", props_->key2str(params->key));
    SDK_ASSERT(params->key);

    ret = genhash_(params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to generate hash, ret:%d", ret);
        return ret;
    }

    mctx = mem_hash_api_context::factory(params, props_);
    if (!mctx) {
        SDK_TRACE_ERR("MainTable: create api context failed ret:%d", ret);
        ret = SDK_RET_OOM;
        goto remove_return;
    }

    ret = static_cast<mem_hash_main_table*>(main_table_)->remove_(mctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("remove_ failed. ret:%d", ret);
        mem_hash_api_context::destroy(mctx);
        goto remove_return;
    }

    SDK_ASSERT(mctx->match_type);

    if (mctx->is_exact_match()) {
        // This means there was an exact match in the main table and
        // it was removed. Check and defragment the hints if required.
        if (mctx->is_hint_valid()) {
            ret = static_cast<mem_hash_hint_table*>(hint_table_)->defragment_(mctx);
            if (ret != SDK_RET_OK) {
                SDK_TRACE_DEBUG("defragment_ failed, ret:%d", ret);
            }
        }
    } else {
        // We have a hint match, traverse the hints to remove the entry.
        ret = static_cast<mem_hash_hint_table*>(hint_table_)->remove_(mctx);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_DEBUG("remove_ failed, ret:%d", ret);
        }
    }

    mem_hash_api_context::destroy(mctx);

remove_return:
    MEM_HASH_TRACE_API_END();
    return ret;
}
