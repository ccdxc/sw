//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "lib/table/tcam/tcam.hpp"
#include "lib/table/tcam/tcam_entry.hpp"
#include "lib/p4pd/p4pd_api.hpp"
#include <cstring>

using sdk::table::tcam_entry_t;

namespace sdk {
namespace table {

typedef struct tcam_iter_cb_s {
    tcam_iterate_func_t func;
    const void          *iter_cb_data;
} tcam_iter_cb_t;

//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
tcam *
tcam::factory(char *name, uint32_t id,
              uint32_t capacity, uint32_t swkey_len,
              uint32_t swdata_len, bool allow_dup_insert)
{
    void *mem  = NULL;
    tcam *t = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_ID_TCAM, sizeof(tcam));
    if (!mem) {
        return NULL;
    }

    t = new (mem) tcam(id, capacity, swkey_len, swdata_len, allow_dup_insert);

    t->indexer_ = indexer::factory(t->capacity_, false, false);

    t->entry_ht_ = ht::factory(capacity, 
                               tcam_entry_get_key_func,
                               tcam_entry_compute_hash_func,
                               tcam_entry_compare_key_func);

    t->name_ = (char *)SDK_CALLOC(SDK_MEM_ALLOC_ID_TCAM_NAME, 
                                  strlen(name) + 1);
    memcpy(t->name_, name, strlen(name) + 1);

    t->stats_ = (uint64_t *)SDK_CALLOC(SDK_MEM_ALLOC_ID_TCAM_STATS,
                                       sizeof(uint64_t) * STATS_MAX);

    SDK_TRACE_DEBUG("tcam::%-30s: tableid: %-3u swkey_len: %-4u "
                    "hwkey_len_: %-4u hwkeymask_len_: %-4u "
                    "hwdata_len_: %-4u \n", t->name_, t->id_, t->swkey_len_, 
                    t->hwkey_len_, t->hwkeymask_len_, t->hwdata_len_);
    return t;
}

//---------------------------------------------------------------------------
// factory method to free & delete the object
//---------------------------------------------------------------------------
void
tcam::destroy(tcam *te)
{
    if (te) {
        indexer::destroy(te->indexer_);
        SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_STATS, te->stats_);
        SDK_FREE(SDK_MEM_ALLOC_ID_TCAM_NAME, te->name_);
        te->~tcam();
        SDK_FREE(SDK_MEM_ALLOC_ID_TCAM, te);
    }
}

//---------------------------------------------------------------------------
// tcam constructor
//---------------------------------------------------------------------------
tcam::tcam(uint32_t id, uint32_t capacity, uint32_t swkey_len, 
           uint32_t swdata_len, bool allow_dup_insert)
{
    id_               = id;
    capacity_         = capacity;
    swkey_len_        = swkey_len;
    swdata_len_       = swdata_len;
    allow_dup_insert_ = allow_dup_insert;

    hwkey_len_ = 0;
    hwkeymask_len_ = 0;
    hwdata_len_ = 0;

    p4pd_hwentry_query(id_, &hwkey_len_, &hwkeymask_len_, &hwdata_len_);

    hwkey_len_ = (hwkey_len_ >> 3) + ((hwkey_len_ & 0x7) ? 1 : 0);
    hwkeymask_len_ = (hwkeymask_len_ >> 3) + ((hwkeymask_len_ & 0x7) ? 1 : 0);
    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);

}

//---------------------------------------------------------------------------
// tcam destructor
//---------------------------------------------------------------------------
tcam::~tcam() 
{
}

//---------------------------------------------------------------------------
// insert API
//---------------------------------------------------------------------------
sdk_ret_t 
tcam::insert(void *key, void *key_mask, void *data,
             uint32_t *index, bool lowest)
{
    sdk_ret_t     rs  = SDK_RET_OK;
    tcam_entry_t  *te = NULL;
    
    // check if TCAM entry already exists
    if (entry_exists_(key, key_mask, swkey_len_, &te)) {
        if (!allow_dup_insert_) {
            SDK_TRACE_DEBUG("TCAM table %s entry already exists at %d return err\n",
                            name_, te->index);
            return SDK_RET_DUPLICATE_INS;
        } else {
            // if entry exists, then increment ref-count and return the index
            SDK_TRACE_DEBUG("TCAM table %s entry exists at %d refcount %d\n",
                            name_, te->index, te->ref_cnt);
            te->ref_cnt++;
            *index = te->index;
            return SDK_RET_OK;
        }
    }

    // alloc index
    rs = alloc_index_(index, lowest);
    if (rs != SDK_RET_OK) {
        goto end;
    }

    SDK_TRACE_DEBUG("TCAM table %s insert at %d\n", name_, *index);
#if 0
    te = tcam_entry::factory(key, key_mask, swkey_len_,
                             data, swdata_len_, *index);
#endif
    te = tcam_entry_create(key, key_mask, swkey_len_,
                           data, swdata_len_, *index);
    // program hw
    rs = program_table_(te);

    if (rs == SDK_RET_OK) {
        // insert in sw
        // tcam_entry_map_[*index] = te;
        rs = entry_ht_->insert_with_key(index, te, &te->ht_ctxt);
    } else {
        // tcam_entry::destroy(te);
        tcam_entry_delete(te);
        free_index_(*index);
    }

end:

    stats_update_(INSERT, rs);
    return rs;
}

//---------------------------------------------------------------------------
// insert a tcam entry with given index
//---------------------------------------------------------------------------
sdk_ret_t 
tcam::insert_withid(void *key, void *key_mask, void *data, uint32_t index)
{
    sdk_ret_t    rs  = SDK_RET_OK;
    tcam_entry_t *te = NULL;
    
    // check if TCAM entry already exists
    if (entry_exists_(key, key_mask, swkey_len_, &te)) {
        if (!allow_dup_insert_) {
            SDK_TRACE_DEBUG("TCAM table : %s Entry exists at %d return err\n",
                             name_, index);
            return SDK_RET_DUPLICATE_INS;
        } else {
            // if entry exisits, then increment ref-count and return the index
            SDK_TRACE_DEBUG("TCAM table : %s Entry exists at %d refcount %d\n",
                            name_, index, te->ref_cnt);
            te->ref_cnt++;
            return SDK_RET_OK;
        }
    }

    // alloc index
    rs = alloc_index_withid_(index);
    if (rs != SDK_RET_OK) {
        SDK_TRACE_DEBUG("tcam table %s index %d already allocated",
                        name_, index);
        goto end;
    }

#if 0
    te = tcam_entry::factory(key, key_mask, swkey_len_, data,
                             swdata_len_, index);
#endif
    te = tcam_entry_create(key, key_mask, swkey_len_,
                           data, swdata_len_, index);
    // program hw
    rs = program_table_(te);
    if (rs == SDK_RET_OK) {
        // insert in sw DS
        // tcam_entry_map_[index] = te;
        rs = entry_ht_->insert_with_key(&index, te, &te->ht_ctxt);
    } else {
        // tcam_entry::destroy(te);
        tcam_entry_delete(te);
        free_index_(index);
    }

end:

    stats_update_(INSERT_WITHID, rs);
    return rs;
}

//---------------------------------------------------------------------------
// update tcam entry at given index with updated data
// TODO: program hw first and then update s/w copy or else if hw programming
//       fails sw copy has new data ??
//---------------------------------------------------------------------------
sdk_ret_t 
tcam::update(uint32_t tcam_idx, void *data)
{
    sdk_ret_t    rs  = SDK_RET_OK;
	tcam_entry_t *te = NULL;
    // tcam_entry_map::iterator itr;

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

    // update sw copy
    // te->update_data(data);
	memcpy(te->data, data, swdata_len_);

    // and program hw
    rs = program_table_(te);

end:

    stats_update_(UPDATE, rs);
    return rs;
}

//---------------------------------------------------------------------------
// remove entry from tcam at given index
//---------------------------------------------------------------------------
sdk_ret_t 
tcam::remove(uint32_t tcam_idx)
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
// program HW table with the given tcam entry
// TODO:
// 1. new/delete ??
// 2. std::memset() ? regular c memset() wouldn't bring std::
//----------------------------------------------------------------------------
sdk_ret_t
tcam::program_table_(tcam_entry_t *te) 
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;
    void *hwkeymask     = NULL;

    if (!te) {
        return SDK_RET_INVALID_ARG;
    }

    // build hw key & mask
    // hwkey     = ::operator new(hwkey_len_);
    // hwkeymask = ::operator new(hwkeymask_len_);
    // std::memset(hwkey, 0, hwkey_len_);
    // std::memset(hwkeymask, 0, hwkeymask_len_);
	hwkey = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_KEY, hwkey_len_);
	hwkeymask = SDK_CALLOC(SDK_MEM_ALLOC_ID_HW_KEY, hwkeymask_len_);


    pd_err = p4pd_hwkey_hwmask_build(id_, te->key, te->key_mask, 
                                     (uint8_t *)hwkey, (uint8_t *)hwkeymask);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

    // write to the actual table
    pd_err = p4pd_entry_write(id_, te->index, (uint8_t *)hwkey, 
                              (uint8_t *)hwkeymask, te->data);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

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
// allocate an index
//----------------------------------------------------------------------------
sdk_ret_t
tcam::alloc_index_(uint32_t *idx, bool lowest)
{
    sdk_ret_t   rs = SDK_RET_OK;
    
    indexer::status irs = indexer_->alloc(idx, lowest, 1);
    if (irs != indexer::SUCCESS) {
        return SDK_RET_NO_RESOURCE;
    }
    return rs;
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
        rs = (irs == indexer::DUPLICATE_ALLOC) ? SDK_RET_DUPLICATE_INS : 
			SDK_RET_OOB;
    }

    return rs;
}

//----------------------------------------------------------------------------
// free given index
//----------------------------------------------------------------------------
sdk_ret_t
tcam::free_index_(uint32_t idx)
{
    sdk_ret_t   rs = SDK_RET_OK;

    indexer::status irs = indexer_->free(idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return SDK_RET_DUPLICATE_INS;
    }
    if (irs != indexer::SUCCESS) {
        return SDK_RET_ERR;
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
        } else if (rs == SDK_RET_DUPLICATE_INS) {
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

    p4_err = p4pd_table_ds_decoded_string_get(id_, te->index,
            te->key, te->key_mask, te->data, 
            buff, sizeof(buff));
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    SDK_TRACE_DEBUG("Index: %d \n %s\n", te->index, buff);

    return SDK_RET_OK;
}

}    // namespace table
}    // namespace sdk
