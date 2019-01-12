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
mem_hash::factory(uint32_t table_id,
                  uint32_t max_recircs,
                  table_health_monitor_func_t health_monitor_func,
                  key2str_t key2str,
                  data2str_t data2str) {
    void                    *mem = NULL;
    mem_hash                 *memhash = NULL;
    sdk_ret_t               ret = SDK_RET_OK;

    MEM_HASH_TRACE_API_BEGIN();
    mem = (mem_hash *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH, sizeof(mem_hash));
    if (mem) {
        memhash = new (mem) mem_hash();
        ret = memhash->init_(table_id, max_recircs, health_monitor_func,
                             key2str, data2str);
        if (ret != SDK_RET_OK) {
            memhash->~mem_hash();
            SDK_FREE(SDK_MEM_ALLOC_MEM_HASH, mem);
        }
    }

    MEM_HASH_TRACE_API_END();
    return memhash;
}

sdk_ret_t
mem_hash::init_(uint32_t table_id,
                uint32_t max_recircs,
                table_health_monitor_func_t health_monitor_func,
                key2str_t key2str,
                data2str_t data2str) {
    p4pd_error_t            p4pdret;
    p4pd_table_properties_t tinfo, ctinfo;

    main_table_id_ = table_id;
    max_recircs_ = max_recircs;
    health_monitor_func_ = health_monitor_func;
    key2str_ = key2str;
    data2str_ = data2str;

    p4pdret = p4pd_table_properties_get(main_table_id_, &tinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    name_ = tinfo.tablename;

    main_table_size_ = tinfo.tabledepth;
    SDK_ASSERT(main_table_size_);

    key_len_ = tinfo.key_struct_size;
    SDK_ASSERT(key_len_);
    data_len_ = tinfo.actiondata_struct_size;
    SDK_ASSERT(data_len_);

    hash_poly_ = tinfo.hash_type;
    // Initialize CRC Fast
    crc32gen_ = crcFast::factory();
    if (crc32gen_ == NULL) {
        return SDK_RET_OOM;
    }

    hint_table_id_ = tinfo.oflow_table_id;
    SDK_ASSERT(hint_table_id_);

    // Assumption: All mem_hash tables will have a HINT table
    SDK_ASSERT(tinfo.has_oflow_table);

    p4pdret = p4pd_table_properties_get(hint_table_id_, &ctinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    hint_table_size_ = ctinfo.tabledepth;
    SDK_ASSERT(hint_table_size_);

    main_table_ = mem_hash_main_table::factory(main_table_id_, main_table_size_);
    SDK_ASSERT_RETURN(main_table_, SDK_RET_OOM);

    hint_table_ = mem_hash_hint_table::factory(hint_table_id_, hint_table_size_);
    SDK_ASSERT_RETURN(hint_table_, SDK_RET_OOM);

    SDK_TRACE_DEBUG("Creating mem_hash:%s main_table_id:%d main_table_size:%d "
                    "hint_table_id:%d hint_table_size:%d key_len:%dB "
                    "data_len:%dB max_recircs:%d hash_poly:%d",
                    name_.c_str(), main_table_id_, main_table_size_,
                    hint_table_id_, hint_table_size_, key_len_, data_len_,
                    max_recircs, hash_poly_);

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
uint32_t
mem_hash::genhash_(void *key) {
    return 0;
}

//---------------------------------------------------------------------------
// mem_hash Insert entry with hash value
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash::insert(void *key, void *data, uint32_t crc32) {
    sdk_ret_t               ret = SDK_RET_OK;
    mem_hash_api_context    *mctx = NULL;   // Main Table Context

    MEM_HASH_TRACE_API_BEGIN();

    SDK_TRACE_DEBUG("Memhash inserting entry, Crc32:%#x", crc32);
    SDK_TRACE_DEBUG("- Key:[%s]", key2str_ ? key2str_(key) : "NA");
    SDK_TRACE_DEBUG("- Data:[%s]", data2str_ ? data2str_(data) : "NA");
    mctx = mem_hash_api_context::factory(key, key_len_, data, data_len_,
                                         max_recircs_, crc32, key2str_, data2str_);
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

//---------------------------------------------------------------------------
// mem_hash Insert entry without hash value
//---------------------------------------------------------------------------
inline sdk_ret_t
mem_hash::insert(void *key, void *data) {
    return insert(key, data, genhash_(key));
}

sdk_ret_t
mem_hash::update(void *key, void *data, uint32_t crc32) {
    MEM_HASH_TRACE_API_BEGIN();
    MEM_HASH_TRACE_API_END();
    return SDK_RET_OK;
}

sdk_ret_t
mem_hash::update(void *key, void *data) {
    return update(key, data, genhash_(key));
}

sdk_ret_t
mem_hash::remove(void *key, uint32_t crc32) {
    sdk_ret_t               ret = SDK_RET_OK;
    mem_hash_api_context    *mctx = NULL;   // Main Table Context

    MEM_HASH_TRACE_API_BEGIN();

    SDK_TRACE_DEBUG("Memhash removing entry, Crc32:%#x", crc32);
    SDK_TRACE_DEBUG("- Key:[%s]", key2str_ ? key2str_(key) : "NA");

    mctx = mem_hash_api_context::factory(key, key_len_, NULL, data_len_,
                                         max_recircs_, crc32, key2str_, data2str_);
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

sdk_ret_t
mem_hash::remove(void *key) {
    return remove(key, genhash_(key));
}
