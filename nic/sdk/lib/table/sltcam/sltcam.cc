//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "lib/p4/p4_api.hpp"

#include "sltcam.hpp"
#include "sltcam_internal.hpp"
//#include "tcam_api_context.hpp"

using sdk::table::sltcam::handle_t;

#define SLTCAM_HANDLE_TO_INDEX(_handle) \
        (((tcam_handle_t)(_handle)).index)
#define TCAM_INDEX_TO_HANDLE(

inline tcam_handle_t
tcam_index_to_handle(uint32_t index) {
    tcam_handle_t handle = { 0 };
    handle.valid = true;
    handle.index = index;
    return handle;
}
    
inline uint32_t
tcam_handle_to_index(uint64_t handle) {
    tcam_handle_t tcam_handle = { 0 };
    tcam_handle.value = handle;
    SDK_ASSERT(tcam_handle.valid);
    return tcam_handle.index;
}

namespace sdk {
namespace table {
//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
tcam *
tcam::factory(sdk_table_factory_params_t *params) {
    void *mem  = NULL;
    tcam *table = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_ID_TCAM, sizeof(tcam));
    if (!mem) {
        return NULL;
    }

    table = (tcam *) new (mem) tcam();
    
    ret = table->init_(params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("Failed to create TCAM table, ret=%d", ret);
        destroy(tcam);
        return NULL;
    }

    return table;
}

sdk_ret_t
tcam::init_(sdk_table_factory_params_t *params) {
    p4pd_error_t p4pdret;
    p4pd_table_properties_t tinfo;

    p4pdret = p4pd_table_properties_get(params->table_id, &tinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    props_->name = tinfo.tablename;
    props_->table_id = params->table_id;
    props_->table_size = tinfo.tabledepth;
    
    props_->swkey_len = tinfo.key_struct_size;
    props_->swdata_len = tinfo.actiondata_struct_size;

    p4pd_hwentry_query(id_, &props_->hwkey_len, &props_->hwkeymask_len,
                       &props_->hwdata_len);

    props_->hwkey_len = SIZE_BITS_TO_BYTES(props_->hwkey_len);
    props_->hwdata_len = SIZE_BITS_TO_BYTES(props_->hwdata_len);
    props_->hwkeymask_len = SIZE_TO_BYTPES(props_->hwkeymask_len);

    props_->entry_trace_en = params->entry_trace_en;

    props_->health_monitor_func = prams->health_monitor_func;

    indexer_ = indexer::factory(props_->table_size, false, false);

    SDK_TRACE_DEBUG("Creating TCAM table=%s table_id=%d size=%d "
                    "swkey_len=%d swdata_len=%d "
                    "hwkey_len=%d hwdata_len=%d hwkeymask_len=%d "
                    "entry_trace_en=%d",
                    props_->name, props_->table_id, props_->table_size,
                    props_->swkey_len, props_->swdata_len,
                    props_->hwkey_len, props_->hwdata_len,
                    props_->hwkeymask_len, props_->entry_trace_en);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// factory method to free & delete the object
//---------------------------------------------------------------------------
void
tcam::destroy(tcam *table)
{
    if (table)
        indexer::destroy(table->indexer_);
        table->~tcam();
        SDK_FREE(SDK_MEM_ALLOC_ID_TCAM, table);
    }
}

void
tcam::trigger_health_monitor()
{
    if (health_monitor_func_) {
        health_monitor_func_(id_, name_, health_state_, capacity_,
                             num_entries_in_use(),
                             &health_state_);
    }
}

//----------------------------------------------------------------------------
// program HW table with the given tcam entry
//----------------------------------------------------------------------------
sdk_ret_t
tcam::write_(sdk_table_api_params_t *params, bool del) {
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey = NULL;
    void *hwkeymask = NULL;
    void *hwdata = NULL;

	hwkey = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_KEY,
                       props_->hwkey_len_);
    if (whwkey == NULL) {
        return SDK_RET_OOM;
    }
	hwkeymask = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_KEY,
                           props_->hwkeymask_len);
    if (hwkeymask == NULL) {
        SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkey);
        return SDK_RET_OOM;
    }
    hwdata = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_DATA,
                        props_->swdata_len);
    if (hwdata == NULL) {
        SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkey);
        SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkeymask);
        return SDK_RET_OOM;
    }

    if (del) {
        memset(hwkey, 0xFF, props_->hwkey_len);
        memset(hwkeymask, 0xFf, props_->hwkeymask_len);
        memset(hwdata, 0, props_->swdata_len);
    } else {
        memcpy(hwdata, params->appdata, props_->swdata_len);
    }

    pd_err = p4pd_hwkey_hwmask_build(props_->table_id,
                                     params->key, params->key_mask,
                                     (uint8_t *)hwkey, (uint8_t *)hwkeymask);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

    // write to the actual table
    pd_err = p4pd_entry_write(props_->table_id,
                              tcam_handle_to_index(params->handle),
                              (uint8_t *)hwkey, (uint8_t *)hwkeymask,
                              params->appdata);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), write_return);

    if (entry_trace_en_) {
        entry_trace_(te);
    }

write_return:
    SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkey);
    SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkeymask);
    SDK_FREE(SDK_MEM_ALLOC_ID_HW_DATA, hwdata);
    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : SDK_RET_OK;
}

//----------------------------------------------------------------------------
// allocate an index
//----------------------------------------------------------------------------
sdk_ret_t
tcam::alloc_(sdk_table_api_params_t *params) {
    sdk_ret_t rs = SDK_RET_OK;
    uint32_t index = 0;

    indexer::status irs = indexer_->alloc(&index);
    if (irs != indexer::SUCCESS) {
        return SDK_RET_NO_RESOURCE;
    }
    params->handle = tcam_index_to_handle(index);
    return rs;
}

//----------------------------------------------------------------------------
// free given index
//----------------------------------------------------------------------------
sdk_ret_t
tcam::dealloc_(sdk_table_api_params_t *params) {
    sdk_ret_t rs = SDK_RET_OK;
    uint32_t index = 0;

    index = tcam_handle_to_index(params->handle);
    indexer::status irs = indexer_->free(index);
    if (irs != indexer::SUCCESS) {
        return SDK_RET_ERR;
    }

    return rs;
}

//---------------------------------------------------------------------------
// insert API
//---------------------------------------------------------------------------
sdk_ret_t
tcam::insert(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;

    TCAM_API_BEGIN_();

    if (!params->handle) {
        ret = find(params);
        if (ret == SDK_RET_OK) {
            SDK_TRACE_ERR("entry already exists");
            ret = SDK_RET_ENTRY_EXISTS
            goto insert_return;
        } else if (ret != SDK_RET_ENTRY_NOT_FOUND) {
            SDK_TRACE_ERR("failed to insert entry, ret=%d", ret);
            goto insert_return;
        }

        ret = alloc_(params);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("failed to allocate entry, ret=%d", ret);
            goto insert_return;
        }
    }

    ret = write_(params);
    if (ret != SDK_RET_OK) {
        dealloc_(params);
        SDK_TRACE_ERR("failed to write to HW, ret=%d", ret);
        goto insert_return;
    }

insert_return:
    TCAM_API_END_();
    stats_.insert(ret);
    return ret;
}

//---------------------------------------------------------------------------
// update tcam entry at given index with updated data
// TODO: program hw first and then update s/w copy or else if hw programming
//       fails sw copy has new data ??
//---------------------------------------------------------------------------
sdk_ret_t
tcam::update(uint32_t tcam_idx, void *data)
{
    sdk_ret_t ret = SDK_RET_OK;

    TCAM_API_BEGIN_();

    if (!params->handle) {
        ret = find(params);
        if (ret == SDK_RET_OK) {
            SDK_TRACE_ERR("entry already exists");
            ret = SDK_RET_ENTRY_EXISTS
            goto update_return;
        } else if (ret != SDK_RET_ENTRY_NOT_FOUND) {
            SDK_TRACE_ERR("failed to insert entry, ret=%d", ret);
            goto update_return;
        }
    }

    SDK_ASSERT(params->handle);
    ret = write_(params);
    if (ret != SDK_RET_OK) {
        dealloc_(params);
        SDK_TRACE_ERR("failed to write to HW, ret=%d", ret);
        goto insert_return;
    }

update_return:
    TCAM_API_END_();
    stats_.update(ret);
    return ret;
}

//---------------------------------------------------------------------------
// remove entry from tcam at given index
//---------------------------------------------------------------------------
sdk_ret_t
tcam::remove(uint32_t tcam_idx)
{
    sdk_ret_t ret = SDK_RET_OK;

    TCAM_API_BEGIN_();

    if (!params->handle) {
        ret = find(params);
        if (ret == SDK_RET_OK) {
            SDK_TRACE_ERR("entry already exists");
            ret = SDK_RET_ENTRY_EXISTS
            goto remove_return;
        } else if (ret != SDK_RET_ENTRY_NOT_FOUND) {
            SDK_TRACE_ERR("failed to insert entry, ret=%d", ret);
            goto remove_return;
        }
    }

    SDK_ASSERT(params->handle);
    ret = write_(params, true);
    if (ret != SDK_RET_OK) {
        dealloc_(params);
        SDK_TRACE_ERR("failed to write to HW, ret=%d", ret);
        goto remove_return;
    }

    ret = dealloc_(params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("failed to dealloc entry, ret=%d", ret);
        goto remove_return;
    }

remove_return:
    TCAM_API_END_();
    stats_.remove(ret);
    return ret;
}


    sdk_ret_t rs = SDK_RET_OK;
    // tcam_entry_map::iterator itr;
	tcam_entry_t *te = NULL;

    // check if idx is OOB
    if (tcam_idx >= capacity_) {
        rs = SDK_RET_OOB;
        goto end;
    }

#if 0
    // check if entry exists
    itr = tcam_entry_map_.find(tcam_idx);
    if (itr == tcam_entry_map_.end()) {
        rs = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
    }
#endif
    // check if entry exists
	te = (tcam_entry_t *)entry_ht_->lookup(&tcam_idx);
	if (te == NULL) {
        rs = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
	}

    if (allow_dup_insert_) {
        // check refcnt and delete only if refcnt is zero
        SDK_TRACE_DEBUG("TCAM: Table: %s Entry delete refcount %d",
                        name_, te->ref_cnt);
		te->ref_cnt--;
        // itr->second->decr_refcnt();
        if (te->ref_cnt != 0) {
            return SDK_RET_OK;
        }
    }

    // de-program hw
    rs = deprogram_table_(te);

    if (rs == SDK_RET_OK) {
        // free & remove from sw data structure
        // tcam_entry::destroy(itr->second);
        // tcam_entry_map_.erase(itr);
		te = (tcam_entry_t *)entry_ht_->remove(&tcam_idx);
        tcam_entry_delete(te);

        // free index
        rs = free_index_(tcam_idx);
        if (rs != SDK_RET_OK) {
            goto end;
        }
    }

end:

    stats_update_(REMOVE, rs);
    trigger_health_monitor();
    return rs;
}

//---------------------------------------------------------------------------
// retrieve from the s/w copy
//---------------------------------------------------------------------------
sdk_ret_t
tcam::retrieve(uint32_t tcam_idx, void *key, void *key_mask, void *data)
{
    sdk_ret_t rs = SDK_RET_OK;
    // tcam_entry_map::iterator itr;
    tcam_entry_t *te = NULL;

    // check if idx is OOB
    if (tcam_idx >= capacity_) {
        rs = SDK_RET_OOB;
        goto end;
    }

#if 0
    // check if entry exists
    itr = tcam_entry_map_.find(tcam_idx);
    if (itr == tcam_entry_map_.end()) {
        rs = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
    }
#endif
    // check if entry exists
	te = (tcam_entry_t *)entry_ht_->lookup(&tcam_idx);
	if (te == NULL) {
        rs = SDK_RET_ENTRY_NOT_FOUND;
        goto end;
	}


    // te = itr->second;
    if (key) {
        memcpy(key, te->key, te->key_len);
    }
    if (key_mask) {
        memcpy(key_mask, te->key_mask, te->key_len);
    }
    if (data) {
        memcpy(data, te->data, te->data_len);
    }

end:

    stats_update_(RETRIEVE, rs);
    return rs;
}

//---------------------------------------------------------------------------
// retrieve from hardware table
//---------------------------------------------------------------------------
sdk_ret_t
tcam::retrieve_from_hw(uint32_t tcam_idx, void *key,
                       void *key_mask, void *data)
{
    sdk_ret_t rs = SDK_RET_OK;
    p4pd_error_t pd_err = P4PD_SUCCESS;

    // check if idx is OOB
    if (tcam_idx >= capacity_) {
        rs = SDK_RET_OOB;
        goto end;
    }

    pd_err = p4pd_entry_read(id_, tcam_idx,
                             key, key_mask, data);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
    }

end:

    stats_update_(RETRIEVE_FROM_HW, rs);
    return rs;
}

bool tcam_iter_walk_cb(void *entry, void *ctxt)
{
    tcam_entry_t *te = (tcam_entry_t *)entry;
    tcam_iter_cb_t *te_cb = (tcam_iter_cb_t *)ctxt;

    te_cb->func(te->key, te->key_mask,
                te->data, te->index, te_cb->iter_cb_data);
    return false;

}
//---------------------------------------------------------------------------
// tcam iterate
//---------------------------------------------------------------------------
sdk_ret_t
tcam::iterate(tcam_iterate_func_t cb, const void *cb_data)
{
    sdk_ret_t rs = SDK_RET_OK;
    tcam_iter_cb_t te_cb  = {0};

    te_cb.func = cb;
    te_cb.iter_cb_data = cb_data;

    this->entry_ht_->walk(tcam_iter_walk_cb, &te_cb);

#if 0
    sdk_ret_t rs = SDK_RET_OK;
    tcam_entry_map::iterator itr;
    tcam_entry *te = NULL;

    for (itr = tcam_entry_map_.begin(); itr != tcam_entry_map_.end(); ++itr) {
        te = itr->second;
        cb(te->get_key(), te->get_key_mask(),
           te->get_data(), itr->first, cb_data);
    }
#endif

    return rs;
}


bool tcam_entry_walk_cb(void *entry, void *ctxt)
{
    tcam_entry_t *te = (tcam_entry_t *)entry;
    tcam_entry_cb_t *te_cb = (tcam_entry_cb_t *)ctxt;

    if ((te_cb->te->key_len == te->key_len) &&
        (!std::memcmp(te_cb->te->key, te->key, te->key_len)) &&
        (!std::memcmp(te_cb->te->key_mask, te->key_mask, te->key_len))) {
        te_cb->is_present = true;
        te_cb->te_match = (tcam_entry_t *)entry;
        return true;
    }
    return false;

}

//---------------------------------------------------------------------------
// return true if tcam entry is present already
//---------------------------------------------------------------------------
bool
tcam::entry_exists_(void *key, void *key_mask, uint32_t key_len,
                    tcam_entry_t **te)
{
    tcam_entry_t    tmp_te = {0};
    tcam_entry_cb_t te_cb  = {0};

    tmp_te.key      = key;
    tmp_te.key_mask = key_mask;
    tmp_te.key_len  = key_len;

    te_cb.te = &tmp_te;
    te_cb.is_present = false;
    this->entry_ht_->walk(tcam_entry_walk_cb, &te_cb);
    if (te_cb.is_present) {
        *te = te_cb.te_match;
        return true;
    }
    *te = NULL;
    return false;
#if 0
    tcam_entry_map::iterator itr;
    tcam_entry *tmp_te = NULL;

    for (itr = tcam_entry_map_.begin(); itr != tcam_entry_map_.end(); ++itr) {
        tmp_te = itr->second;
        if ((key_len == tmp_te->get_key_len()) &&
            (!std::memcmp(key, tmp_te->get_key(), key_len)) &&
            (!std::memcmp(key_mask, tmp_te->get_key_mask(), key_len))) {
            *te = tmp_te;
            return true;
        }
    }
    *te = NULL;
    return false;
#endif
}

//----------------------------------------------------------------------------
// invalidate gievn TCAM entry in the TCAM table
// TODO:
// 1. new/delete ??
// 2. std::memset() ? regular c memset() wouldn't bring std::
//----------------------------------------------------------------------------
sdk_ret_t
tcam::deprogram_table_(tcam_entry_t *te)
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;
    void *hwkeymask     = NULL;

    if (!te) {
        return SDK_RET_INVALID_ARG;
    }

    // build hw keys/keymasks
    // hwkey     = ::operator new(hwkey_len_);
    // hwkeymask = ::operator new(hwkeymask_len_);
	hwkey = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_KEY, hwkey_len_);
	hwkeymask = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_KEY, hwkeymask_len_);

    memset(hwkey, 0xFF, hwkey_len_);
    memset(hwkeymask, 0xFf, hwkeymask_len_);
    memset(te->data, 0, swdata_len_);

    // write to the P4 table
    pd_err = p4pd_entry_write(id_, te->index, (uint8_t *)hwkey,
                              (uint8_t *)hwkeymask, te->data);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

    SDK_TRACE_DEBUG("%s: Index: %d de-programmed\n", name_, te->index);

end:

    if (hwkey) {
        // ::operator delete(hwkey);
		SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkey);
    }
    if (hwkeymask) {
        // ::operator delete(hwkeymask);
		SDK_FREE(SDK_MEM_ALLOC_ID_HW_KEY, hwkeymask);
    }

    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : SDK_RET_OK;
}

//----------------------------------------------------------------------------
// return stats pointer
// TODO: this is a public API ?
//----------------------------------------------------------------------------
sdk_ret_t
tcam::fetch_stats(const uint64_t **stats)
{
    *stats = stats_;
    return SDK_RET_OK;
}

//----------------------------------------------------------------------------
// allocate an index with id
//----------------------------------------------------------------------------
sdk_ret_t
tcam::alloc_index_withid_(uint32_t idx)
{
    sdk_ret_t   rs = SDK_RET_OK;

    indexer::status irs = indexer_->alloc_withid(idx);
    if (irs != indexer::SUCCESS) {
        rs = (irs == indexer::DUPLICATE_ALLOC) ? SDK_RET_ENTRY_EXISTS :
			SDK_RET_OOB;
    }

    return rs;
}

//----------------------------------------------------------------------------
// increment stats
//----------------------------------------------------------------------------
void
tcam::stats_incr_(stats stat)
{
    SDK_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

//----------------------------------------------------------------------------
// decrement stats
//----------------------------------------------------------------------------
void
tcam::stats_decr_(stats stat)
{
    SDK_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

//----------------------------------------------------------------------------
// update stats
//----------------------------------------------------------------------------
void
tcam::stats_update_(tcam::api ap, sdk_ret_t rs)
{
    switch (ap) {
    case INSERT:
        if (rs == SDK_RET_OK) {
            stats_incr_(STATS_INS_SUCCESS);
        } else if (rs == SDK_RET_HW_PROGRAM_ERR) {
            stats_incr_(STATS_INS_FAIL_HW);
        } else if (rs == SDK_RET_NO_RESOURCE) {
            stats_incr_(STATS_INS_FAIL_NO_RES);
        } else {
            SDK_ASSERT(0);
        }
        break;

    case INSERT_WITHID:
        if (rs == SDK_RET_OK) {
            stats_incr_(STATS_INS_WITHID_SUCCESS);
        } else if (rs == SDK_RET_HW_PROGRAM_ERR) {
            stats_incr_(STATS_INS_WITHID_FAIL_HW);
        } else if (rs == SDK_RET_ENTRY_EXISTS) {
            stats_incr_(STATS_INS_WITHID_FAIL_DUP_INS);
        } else if (rs == SDK_RET_OOB) {
            stats_incr_(STATS_INS_WITHID_FAIL_OOB);
        } else {
            SDK_ASSERT(0);
        }
        break;

    case UPDATE:
        if (rs == SDK_RET_OK) {
            stats_incr_(STATS_UPD_SUCCESS);
        } else if (rs == SDK_RET_ENTRY_NOT_FOUND) {
            stats_incr_(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
        } else if (rs == SDK_RET_HW_PROGRAM_ERR) {
            stats_incr_(STATS_UPD_FAIL_HW);
        } else if (rs == SDK_RET_OOB) {
            stats_incr_(STATS_UPD_FAIL_OOB);
        } else {
            SDK_ASSERT(0);
        }
        break;

    case REMOVE:
        if (rs == SDK_RET_OK) {
            stats_incr_(STATS_REM_SUCCESS);
        } else if (rs == SDK_RET_ENTRY_NOT_FOUND) {
            stats_incr_(STATS_REM_FAIL_ENTRY_NOT_FOUND);
        } else if (rs == SDK_RET_HW_PROGRAM_ERR) {
            stats_incr_(STATS_REM_FAIL_HW);
        } else if (rs == SDK_RET_OOB) {
            stats_incr_(STATS_REM_FAIL_OOB);
        } else {
            SDK_ASSERT(0);
        }
        break;

    case RETRIEVE:
        if (rs == SDK_RET_OK) {
            stats_incr_(STATS_RETR_SUCCESS);
        } else if (rs == SDK_RET_OOB) {
            stats_incr_(STATS_RETR_FAIL_OOB);
        } else if (rs == SDK_RET_ENTRY_NOT_FOUND) {
            stats_incr_(STATS_RETR_FAIL_ENTRY_NOT_FOUND);
        } else {
            SDK_ASSERT(0);
        }
        break;

    case RETRIEVE_FROM_HW:
        if (rs == SDK_RET_OK) {
            stats_incr_(STATS_RETR_FROM_HW_SUCCESS);
        } else if (rs == SDK_RET_OOB) {
            stats_incr_(STATS_RETR_FROM_HW_FAIL_OOB);
        } else if (rs == SDK_RET_HW_PROGRAM_ERR) {
            stats_incr_(STATS_RETR_FROM_HW_FAIL);
        } else {
            SDK_ASSERT(0);
        }
        break;

    default:
        SDK_ASSERT(0);
    }
}

//----------------------------------------------------------------------------
// number of entries in use
//----------------------------------------------------------------------------
uint32_t
tcam::num_entries_in_use(void) const
{
    return indexer_->num_indices_allocated();
}

//----------------------------------------------------------------------------
// number of insert operations attempted
//----------------------------------------------------------------------------
uint32_t
tcam::num_inserts(void) const
{
    return stats_[STATS_INS_SUCCESS] + stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW] +
        stats_[STATS_INS_WITHID_SUCCESS] + stats_[STATS_INS_WITHID_FAIL_DUP_INS] +
        stats_[STATS_INS_WITHID_FAIL_HW] + stats_[STATS_INS_WITHID_FAIL_OOB];
}

//----------------------------------------------------------------------------
// number of failed insert operations
//----------------------------------------------------------------------------
uint32_t
tcam::num_insert_errors(void) const
{
    return stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW] +
        stats_[STATS_INS_WITHID_FAIL_DUP_INS] +
        stats_[STATS_INS_WITHID_FAIL_HW] + stats_[STATS_INS_WITHID_FAIL_OOB];
}

// ----------------------------------------------------------------------------
// number of update operations attempted
// ----------------------------------------------------------------------------
uint32_t
tcam::num_updates(void) const
{
    return stats_[STATS_UPD_SUCCESS] + stats_[STATS_UPD_FAIL_OOB] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_UPD_FAIL_HW];
}

// ----------------------------------------------------------------------------
// number of failed update operations
// ----------------------------------------------------------------------------
uint32_t
tcam::num_update_errors(void) const
{
    return stats_[STATS_UPD_FAIL_OOB] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_UPD_FAIL_HW];
}

//----------------------------------------------------------------------------
// number of delete operations attempted
//----------------------------------------------------------------------------
uint32_t
tcam::num_deletes(void) const
{
    return stats_[STATS_REM_SUCCESS] + stats_[STATS_REM_FAIL_OOB] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

//----------------------------------------------------------------------------
// number of failed delete operations
//----------------------------------------------------------------------------
uint32_t
tcam::num_delete_errors(void) const
{
    return stats_[STATS_REM_FAIL_OOB] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

//----------------------------------------------------------------------------
// dump a tcam entry
//----------------------------------------------------------------------------
sdk_ret_t
tcam::entry_trace_(tcam_entry_t *te)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    if (!te) {
        return SDK_RET_OK;
    }
    p4_err = p4pd_table_ds_decoded_string_get(id_, te->index,
            te->key, te->key_mask, te->data,
            buff, sizeof(buff));
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    SDK_TRACE_DEBUG("%s: Index: %d \n %s\n", name_, te->index, buff);

    return SDK_RET_OK;
}

// ----------------------------------------------------------------------------
// Returns string of the entry
// ----------------------------------------------------------------------------
sdk_ret_t
tcam::entry_to_str(void *key, void *key_mask, void *data, uint32_t index,
                    char *buff, uint32_t buff_size)
{
    p4pd_error_t    p4_err;

    p4_err = p4pd_global_table_ds_decoded_string_get(id_, index,
                                                     key, key_mask, data,
                                                     buff, buff_size);
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    SDK_TRACE_DEBUG("%s: Index: %d \n %s", name_, index, buff);

    return SDK_RET_OK;
}

}    // namespace table
}    // namespace sdk
