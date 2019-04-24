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

#include "ftl.hpp"
#include "ftl_table.hpp"
#include "ftl_bucket.hpp"
#include "ftl_utils.hpp"
#include "ftl_platform.hpp"
#include "ftl_structs.hpp"

using sdk::table::ftl;
using sdk::table::sdk_table_api_params_t;
using sdk::table::ftlint::ftl_main_table;
using sdk::table::ftlint::ftl_hint_table;
using sdk::table::ftlint::ftl_bucket;

#define FTL_API_BEGIN(_name) {\
        FTL_TRACE_VERBOSE("%s ftl begin: %s %s",\
                              "--", _name, "--");\
}

#define FTL_API_END(_name, _status) {\
        FTL_TRACE_VERBOSE("%s ftl end: %s (r:%d) %s",\
                              "--", _name, _status, "--");\
}

#define FTL_API_BEGIN_() {\
        FTL_API_BEGIN(props_->name.c_str());\
        SDK_SPINLOCK_LOCK(&slock_);\
}

#define FTL_API_END_(_status) {\
        FTL_API_END(props_->name.c_str(), (_status));\
        SDK_SPINLOCK_UNLOCK(&slock_);\
}

#ifndef SIM
#define CRC32X(crc, value) __asm__("crc32x %w[c], %w[c], %x[v]":[c]"+r"(crc):[v]"r"(value))
#define RBITX(value) __asm__("rbit %x0, %x1": "=r"(value) : "r"(value))
#define RBITW(value) __asm__("rbit %w0, %w1": "=r"(value) : "r"(value))
#define REVX(value) __asm__("rev %x0, %x1": "=r"(value) : "r"(value))
uint32_t
crc32_aarch64(const uint64_t *p) {
    uint32_t crc = 0;
    for (auto i = 0; i < 8; i++) {
        auto v = p[i];
        RBITX(v);
        REVX(v);
        CRC32X(crc, v);
    }
    RBITW(crc);
    return crc;
}
#endif

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
ftl *
ftl::factory(sdk_table_factory_params_t *params) {
    void *mem = NULL;
    ftl *f = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    //FTL_API_BEGIN("NewTable");
    mem = (ftl *) SDK_CALLOC(SDK_MEM_ALLOC_FTL, sizeof(ftl));
    if (mem) {
        f = new (mem) ftl();
        ret = f->init_(params);
        if (ret != SDK_RET_OK) {
            f->~ftl();
            SDK_FREE(SDK_MEM_ALLOC_FTL, mem);
            f = NULL;
        }
    } else {
        ret = SDK_RET_OOM;
    }

    FTL_API_END("NewTable", ret);
    return f;
}

sdk_ret_t
ftl::init_(sdk_table_factory_params_t *params) {
    p4pd_error_t p4pdret;
    p4pd_table_properties_t tinfo, ctinfo;

    props_ = (sdk::table::properties_t *)SDK_CALLOC(SDK_MEM_ALLOC_FTL_PROPERTIES,
                                                 sizeof(sdk::table::properties_t));
    if (props_ == NULL) {
        return SDK_RET_OOM;
    }

    SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);

    props_->ptable_id = params->table_id;
    props_->num_hints = params->num_hints;
    props_->max_recircs = params->max_recircs;
    //props_->health_monitor_func = params->health_monitor_func;
    props_->key2str = params->key2str;
    props_->data2str = params->appdata2str;
    props_->entry_trace_en = params->entry_trace_en;

    p4pdret = p4pd_table_properties_get(props_->ptable_id, &tinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->name = tinfo.tablename;

    props_->ptable_size = tinfo.tabledepth;
    SDK_ASSERT(props_->ptable_size);

    props_->hash_poly = tinfo.hash_type;

    props_->ptable_base_mem_pa = tinfo.base_mem_pa;
    props_->ptable_base_mem_va = tinfo.base_mem_va;

    // Initialize CRC Fast
    crc32gen_ = crcFast::factory();
    SDK_ASSERT(crc32gen_);

    props_->stable_id = tinfo.oflow_table_id;
    SDK_ASSERT(props_->stable_id);

    // Assumption: All ftl tables will have a HINT table
    SDK_ASSERT(tinfo.has_oflow_table);

    p4pdret = p4pd_table_properties_get(props_->stable_id, &ctinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->stable_base_mem_pa = ctinfo.base_mem_pa;
    props_->stable_base_mem_va = ctinfo.base_mem_va;

    props_->stable_size = ctinfo.tabledepth;
    SDK_ASSERT(props_->stable_size);

    main_table_ = ftl_main_table::factory(props_);
    SDK_ASSERT_RETURN(main_table_, SDK_RET_OOM);

    FTL_TRACE_INFO("Creating Flow table.");
    FTL_TRACE_INFO("- ptable_id:%d ptable_size:%d ",
                   props_->ptable_id, props_->ptable_size);
    FTL_TRACE_INFO("- stable_id:%d stable_size:%d ",
                   props_->stable_id, props_->stable_size);
    FTL_TRACE_INFO("- num_hints:%d max_recircs:%d hash_poly:%d",
                   props_->num_hints, props_->max_recircs, props_->hash_poly);
    FTL_TRACE_INFO("- ptable base_mem_pa:%#lx base_mem_va:%#lx",
                   props_->ptable_base_mem_pa, props_->ptable_base_mem_va);
    FTL_TRACE_INFO("- stable base_mem_pa:%#lx base_mem_va:%#lx",
                   props_->stable_base_mem_pa, props_->stable_base_mem_va);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl Destructor
//---------------------------------------------------------------------------
void
ftl::destroy(ftl *table) {
    FTL_API_BEGIN("DestroyTable");
    FTL_TRACE_VERBOSE("%p", table);
    ftl_main_table::destroy_(static_cast<ftl_main_table*>(table->main_table_));
    FTL_API_END("DestroyTable", SDK_RET_OK);
}

//---------------------------------------------------------------------------
// ftl Insert entry with hash value
//---------------------------------------------------------------------------
// TODO
sdk_ret_t
ftl::genhash_(sdk_table_api_params_t *params) {
    if (params->hash_valid) {
        // If hash_valid is set in the params, skip computing it.
        return SDK_RET_OK;
    }

    ftl_entry_t hashkey;
    FTLENTRY_HASH_KEY_BUILD(&hashkey, ((ftl_entry_t*)params->entry));
    ftl_swap_bytes((uint8_t *)&hashkey);
#ifdef SIM
    params->hash_32b = crc32gen_->compute_crc((uint8_t *)&hashkey,
                                              sizeof(ftl_entry_t),
                                              props_->hash_poly);
#else
    params->hash_32b = crc32_aarch64((uint64_t *)&hashkey);
#endif

    params->hash_valid = true;
    FTL_TRACE_VERBOSE("[%s] => H:%#x",
                      ftlu_rawstr(&hashkey, sizeof(ftl_entry_t)),
                      params->hash_32b);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl: Create API context. This is used by all APIs
//---------------------------------------------------------------------------
sdk_ret_t
ftl::ctxinit_(sdk_table_api_op_t op,
              sdk_table_api_params_t *params) {
    FTL_TRACE_VERBOSE("op:%d", op);
    if (SDK_TABLE_API_OP_IS_CRUD(op)) {
        auto ret = genhash_(params);
        if (ret != SDK_RET_OK) {
            FTL_TRACE_ERR("failed to generate hash, ret:%d", ret);
            return ret;
        }
    }

    FTL_API_CONTEXT_INIT_MAIN(apictx_, op, params,
                              props_, &table_stats_);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl Insert entry to ftl table
//---------------------------------------------------------------------------
sdk_ret_t
ftl::insert(sdk_table_api_params_t *params) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->entry);

    time_profile_begin(sdk::utils::time_profile::TABLE_LIB_FTL_INSERT);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_INSERT, params);
    FTL_RET_CHECK_AND_GOTO(ret, done, "ctxinit r:%d", ret);

    ret = static_cast<ftl_main_table*>(main_table_)->insert_(&apictx_);
    FTL_RET_CHECK_AND_GOTO(ret, done, "main table insert r:%d", ret);

done:
    time_profile_end(sdk::utils::time_profile::TABLE_LIB_FTL_INSERT);
    api_stats_.insert(ret);
    FTL_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// ftl Update entry to ftl table
//---------------------------------------------------------------------------
sdk_ret_t
ftl::update(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_UPDATE, params);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto update_return;
    }

    ret = static_cast<ftl_main_table*>(main_table_)->update_(&apictx_);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("update_ failed. ret:%d", ret);
        goto update_return;
    }

update_return:
    api_stats_.update(ret);
    FTL_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// ftl Remove entry from ftl table
//---------------------------------------------------------------------------
sdk_ret_t
ftl::remove(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_REMOVE, params);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto remove_return;
    }

    ret = static_cast<ftl_main_table*>(main_table_)->remove_(&apictx_);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("remove_ failed. ret:%d", ret);
        goto remove_return;
    }

remove_return:
    api_stats_.remove(ret);
    FTL_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// ftl Get entry from ftl table
//---------------------------------------------------------------------------
sdk_ret_t
ftl::get(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_GET, params);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto get_return;
    }

    ret = static_cast<ftl_main_table*>(main_table_)->get_(&apictx_);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("remove_ failed. ret:%d", ret);
        goto get_return;
    }

get_return:
    api_stats_.get(ret);
    FTL_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// ftl Get Stats from ftl table
//---------------------------------------------------------------------------
sdk_ret_t
ftl::stats_get(sdk_table_api_stats_t *api_stats,
                    sdk_table_stats_t *table_stats) {
    FTL_API_BEGIN_();
    api_stats_.get(api_stats);
    table_stats_.get(table_stats);
    FTL_API_END_(SDK_RET_OK);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl start transaction
//---------------------------------------------------------------------------
sdk_ret_t
ftl::txn_start() {
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl end transaction
//---------------------------------------------------------------------------
sdk_ret_t
ftl::txn_end() {
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl end transaction
//---------------------------------------------------------------------------
sdk_ret_t
ftl::iterate(sdk_table_api_params_t *params) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->itercb);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_ITERATE, params);
    FTL_RET_CHECK_AND_GOTO(ret, done, "ctxinit r:%d", ret);

    ret = static_cast<ftl_main_table*>(main_table_)->iterate_(&apictx_);
    FTL_RET_CHECK_AND_GOTO(ret, done, "iterate r:%d", ret);

done:
    FTL_API_END_(ret);
    return ret;
}
