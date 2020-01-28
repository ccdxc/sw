//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "ftl_base.hpp"

std::map<int, sdk::table::ftl_table_info *>
            sdk::table::ftl_base::table_info_cache;
static std::mutex mtx;

thread_local Apictx ftl_base::apictx_[FTL_MAX_API_CONTEXTS + 1];

#define FTL_API_BEGIN(_name) {\
    FTL_TRACE_VERBOSE("%s %s begin: %s %s", \
                      "--", "ftl", _name, "--");\
}

#define FTL_API_END(_name, _status) {\
   FTL_TRACE_VERBOSE("%s %s end: %s (r:%d) %s", \
                     "--", "ftl", _name, _status, "--");\
}

#define FTL_API_BEGIN_() {                           \
        FTL_API_BEGIN(props_->name.c_str());         \
}

#define FTL_API_END_(_status) {\
        FTL_API_END(props_->name.c_str(), (_status));\
}

void
ftl_base::add_table_to_cache_(uint32_t table_id,
                              sdk::table::properties_t *props,
                              void *table) {
    ftl_table_info *info = NULL;

    mtx.lock();
    if (table_info_cache.find(table_id) != table_info_cache.end()) {
        info = table_info_cache[table_id];
    } else {
        info = new ftl_table_info();
    }
    SDK_ASSERT(info);
    info->set_table(table);
    info->set_props(props);
    info->increment_ref_count();
    table_info_cache[table_id] = info;
    mtx.unlock();
}

sdk::table::ftl_table_info *
ftl_base::get_cached_table_(uint32_t table_id) {
    mtx.lock();
    if (table_info_cache.find(table_id) == table_info_cache.end()) {
        mtx.unlock();
        return NULL;
    }
    table_info_cache[table_id]->increment_ref_count();
    mtx.unlock();
    return table_info_cache[table_id];
}

bool
ftl_base::remove_table_from_cache_(uint32_t table_id) {
    ftl_table_info *info = NULL;
    mtx.lock();
    if (table_info_cache.find(table_id) == table_info_cache.end()) {
        goto error;
    }

    info = table_info_cache[table_id];
    info->decrement_ref_count();
    if (info->get_ref_count() != 0) {
        goto done;
    }
    table_info_cache.erase(table_id);
    info->~ftl_table_info();

done:
    mtx.unlock();
    return true;

error:
    mtx.unlock();
    return false;

}

sdk_ret_t
ftl_base::init_(sdk_table_factory_params_t *params) {
    p4pd_error_t p4pdret;
    p4pd_table_properties_t tinfo, ctinfo;
    thread_id_ = params->thread_id;
    ftl_table_info *info = NULL;

    info = get_cached_table_(params->table_id);
    if (info) {
        props_ = info->get_props();
        main_table_ = info->get_table();
        goto skip_tables;
    }

    props_ = (sdk::table::properties_t *)SDK_CALLOC(SDK_MEM_ALLOC_FTL_PROPERTIES,
                                                 sizeof(sdk::table::properties_t));
    if (props_ == NULL) {
        return SDK_RET_OOM;
    }

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

    main_table_ = main_table::factory(props_);
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

    add_table_to_cache_(params->table_id, props_, main_table_);

skip_tables:

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl Destructor
//---------------------------------------------------------------------------
void
ftl_base::destroy(ftl_base *t) {
    if (t != NULL) {
        FTL_API_BEGIN("DestroyTable");
        FTL_TRACE_VERBOSE("%p", t);

        if (t->remove_table_from_cache_(t->props_->ptable_id)) {
            main_table::destroy_(static_cast<main_table*>(t->main_table_));
            FTL_API_END("DestroyTable", SDK_RET_OK);
            t->main_table_ = NULL;
            if (t->props_ != NULL) {
                t->props_->~properties_t();
                SDK_FREE(SDK_MEM_ALLOC_FTL_PROPERTIES,t->props_);
                t->props_ = NULL;
            }
        }
        t->~ftl_base();
        SDK_FREE(SDK_MEM_ALLOC_FTL, t);
        t = NULL;
    }
}

//---------------------------------------------------------------------------
// ftl: Create API context. This is used by all APIs
//---------------------------------------------------------------------------
sdk_ret_t
ftl_base::ctxinit_(sdk_table_api_op_t op,
              sdk_table_api_params_t *params) {
    int index;

    FTL_TRACE_VERBOSE("op:%d", op);
    if (SDK_TABLE_API_OP_IS_CRUD(op)) {
        auto ret = genhash_(params);
        if (ret != SDK_RET_OK) {
            FTL_TRACE_ERR("failed to generate hash, ret:%d", ret);
            return ret;
        }
    }

    index = 0;
    get_apictx(index)->init(op, params, props_, &tstats_, thread_id_,
                            this, get_entry(index), params->entry_size);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl Insert entry to ftl table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_base::insert(sdk_table_api_params_t *params) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->entry);

    time_profile_begin(sdk::utils::time_profile::TABLE_LIB_FTL_INSERT);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_INSERT, params);
    FTL_RET_CHECK_AND_GOTO(ret, done, "ctxinit r:%d", ret);

    ret = static_cast<main_table*>(main_table_)->insert_(get_apictx(0));
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
ftl_base::update(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_UPDATE, params);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto update_return;
    }

    ret = static_cast<main_table*>(main_table_)->update_(get_apictx(0));
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
ftl_base::remove(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_REMOVE, params);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto remove_return;
    }

    ret = static_cast<main_table*>(main_table_)->remove_(get_apictx(0));
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
ftl_base::get(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->key);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_GET, params);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed to create api context. ret:%d", ret);
        goto get_return;
    }

    ret = static_cast<main_table*>(main_table_)->get_(get_apictx(0));
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("get_ failed. ret:%d", ret);
        goto get_return;
    }

get_return:
    api_stats_.get(ret);
    FTL_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// ftl Get Stats from ftl table
// As stats are maintained per thread, needs to call from each thread.
//---------------------------------------------------------------------------
sdk_ret_t
ftl_base::stats_get(sdk_table_api_stats_t *api_stats,
                       sdk_table_stats_t *table_stats) {
    FTL_API_BEGIN_();
    api_stats_.get(api_stats);
    tstats_.get(table_stats);
    FTL_API_END_(SDK_RET_OK);

    return SDK_RET_OK;
}

sdk_ret_t
ftl_base::iterate(sdk_table_api_params_t *params) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;

    FTL_API_BEGIN_();
    SDK_ASSERT(params->itercb);

    ret = ctxinit_(sdk::table::SDK_TABLE_API_ITERATE, params);
    FTL_RET_CHECK_AND_GOTO(ret, done, "ctxinit r:%d", ret);

    ret = static_cast<main_table*>(main_table_)->iterate_(get_apictx(0));
    FTL_RET_CHECK_AND_GOTO(ret, done, "iterate r:%d", ret);

done:
    FTL_API_END_(ret);
    return ret;
}

sdk_ret_t
ftl_base::clear(bool clear_global_state,
                bool clear_thread_local_state,
                sdk_table_api_params_t *params) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;
    FTL_API_BEGIN_();

    ret = ctxinit_(sdk::table::SDK_TABLE_API_CLEAR, params);
    FTL_RET_CHECK_AND_GOTO(ret, done, "ctxinit r:%d", ret);

    get_apictx(0)->clear_global_state = clear_global_state;
    get_apictx(0)->clear_thread_local_state = clear_thread_local_state;
    ret = static_cast<main_table*>(main_table_)->clear_(get_apictx(0));
    FTL_RET_CHECK_AND_GOTO(ret, done, "clear r:%d", ret);
    
    if (clear_thread_local_state) {
        (void)clear_stats();
    }

done:
    FTL_API_END_(ret);
    return ret;
}

sdk_ret_t
ftl_base::clear_stats(void) {
    FTL_API_BEGIN_();

    api_stats_.clear();
    tstats_.clear();

    FTL_API_END_(SDK_RET_OK);
    return SDK_RET_OK;
}
