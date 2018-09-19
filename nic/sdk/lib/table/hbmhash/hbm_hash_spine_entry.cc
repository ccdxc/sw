//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "string.h"
#include "include/sdk/base.hpp"
#include "hbm_hash.hpp"
#include "hbm_hash_entry.hpp"
#include "hbm_hash_table_entry.hpp"
#include "hbm_hash_spine_entry.hpp"
#include "hbm_hash_hint_group.hpp"
#include "hbm_hash_mem_types.hpp"

using sdk::table::HbmHashHintGroup;
using sdk::table::HbmHashSpineEntry;
using sdk::table::HbmHashEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
HbmHashSpineEntry *
HbmHashSpineEntry::factory(HbmHashTableEntry *ht_entry, uint32_t mtrack_id)
{
    void            *mem = NULL;
    HbmHashSpineEntry  *hse = NULL;

    mem = SDK_CALLOC(mtrack_id, sizeof(HbmHashSpineEntry));
    if (!mem) {
        return NULL;
    }

    hse = new (mem) HbmHashSpineEntry(ht_entry);
    return hse;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
HbmHashSpineEntry::destroy(HbmHashSpineEntry *hse, uint32_t mtrack_id)
{
    if (hse) {
        hse->~HbmHashSpineEntry();
        SDK_FREE(mtrack_id, hse);
    }
}

// ---------------------------------------------------------------------------
// Constructor - HBM Hash Spine Entry
// ---------------------------------------------------------------------------
HbmHashSpineEntry::HbmHashSpineEntry(HbmHashTableEntry *ht_entry)
{
    ht_entry_       = ht_entry;
    anchor_entry_   = NULL;
    is_in_ht_       = false;
    hct_index_     = 0;
    prev_           = NULL;
    next_           = NULL;
}

// ---------------------------------------------------------------------------
// Forms action data
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::form_action_data(void *action_data)
{
    HbmHashHintGroup   *hg = NULL;
    HbmHashEntry       *root_fe = NULL;
    uint32_t        hint_bits = 0;
    uint32_t        fhct_idx = 0;
    // hg_root_t       hg_root;
    uint8_t         *action_id;
    uint8_t         *entry_valid;
    void            *data;
    void            *first_hash_hint;
    uint8_t         *more_hashs;
    void            *more_hints;
    char            *loc = NULL;
    uint32_t        hint_mem_len_B = get_ht_entry()->get_hbm_hash()->get_hint_mem_len_B();
    HbmHashHintGroupList::iterator itr;

    SDK_TRACE_DEBUG("P4-API: Table: FT Call ...");
    get_ht_entry()->get_hbm_hash()->hbm_hash_action_data_offsets(action_data,
                                                         &action_id,
                                                         &entry_valid,
                                                         &data,
                                                         &first_hash_hint,
                                                         &more_hashs,
                                                         &more_hints);

    *action_id = 0;
    if (anchor_entry_ || hg_list_.size()) {
        // Don't have to set the entry valid if this is deprogram
        *entry_valid = 1;
    }
    if (anchor_entry_) {
        // Enable only for debugging
#if 0
        fmt::MemoryWriter buf;
        uint8_t *tmp = (uint8_t *)anchor_entry_->get_data();
        for (uint32_t i = 0; i < anchor_entry_->get_data_len(); i++, tmp++) {
            buf.write("{:#x} ", (uint8_t)*tmp);
        }
        SDK_TRACE_DEBUG("Data:");
        SDK_TRACE_DEBUG("%s", buf.c_str());
#endif
        // +1 for entry_valid. export_en + hbm_hash_index is data
        memcpy(data, anchor_entry_->get_data(), anchor_entry_->get_data_len());
    }

    // After this loc will point to hash1
    loc = (char*)(first_hash_hint);
    for (itr = hg_list_.begin();itr != hg_list_.end(); itr++) {
        hg = (*itr);
        hint_bits = hg->get_hint_bits();
        root_fe = hg->get_first_hbm_hash_entry();
        fhct_idx = root_fe->get_fhct_index();

        // hg_root.hash = hint_bits;
        // hg_root.hint = fhct_idx;
        // memcpy(loc, &hg_root, sizeof(hg_root));
        // loc += sizeof(hg_root);
        memcpy(loc, &hint_bits, 2);
        memcpy(loc + 2, &fhct_idx, hint_mem_len_B);
        loc += (2 + hint_mem_len_B);
    }
    if (next_) {
        *more_hashs = 1;
        // *more_hints = next_->get_fhct_index();
        fhct_idx = next_->get_fhct_index();
        memcpy(more_hints, &fhct_idx, hint_mem_len_B);
    }
}

// ---------------------------------------------------------------------------
// Program the entry into HW
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashSpineEntry::program_table()
{
    sdk_ret_t                       rs = SDK_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    uint32_t                        table_id = 0;
    uint32_t                        coll_table_id = 0;
    uint32_t                        /*hw_key_len = 0,*/ sw_key_len = 0;
    void                            *hwkey = NULL, *sw_key = NULL;
    void                            *action_data;
    uint32_t                        entire_data_len;

    table_id = get_ht_entry()->get_hbm_hash()->get_table_id();
    coll_table_id = get_ht_entry()->get_hbm_hash()->get_collision_table_id();
    // hw_key_len = get_ht_entry()->get_hbm_hash()->get_hwkey_len();
    sw_key_len = get_ht_entry()->get_hbm_hash()->get_key_len();
    entire_data_len = get_ht_entry()->get_hbm_hash()->
                      get_entire_data_len();

    action_data = SDK_CALLOC(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
                             entire_data_len);
    sw_key = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_SW_KEY,
                        sw_key_len);

    SDK_TRACE_DEBUG("Before forming action data");
    if (is_in_ht_) {
        form_action_data(action_data);
        // Form Hw key for anchor
#if 0
        hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY,
                           hw_key_len);
#endif
        if (anchor_entry_) {
            hwkey = anchor_entry_->get_hwkey();
#if 0
            p4pd_hwkey_hwmask_build(table_id, anchor_entry_->get_key(),
                                    NULL, (uint8_t *)hwkey, NULL);
#endif
        }

        uint32_t ft_index = get_ht_entry()->get_bucket_index();
        SDK_TRACE_DEBUG("P4 FT Write:%d", ft_index);

        // Entry trace
        if (anchor_entry_ &&
            get_ht_entry()->get_hbm_hash()->get_entry_trace_en()) {
            entry_trace(table_id, ft_index, (void *)anchor_entry_->get_key(),
                        action_data);
        }

		// P4-API: HBM Hash Table Write
        pd_err = p4pd_entry_write(table_id, ft_index, (uint8_t*)hwkey, NULL,
                                  action_data);
        // SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY, hwkey);
	} else {
        form_action_data(action_data);
        // Form Hw key for anchor
#if 0
        hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY,
                           hw_key_len);
#endif
        if (anchor_entry_) {
            hwkey = anchor_entry_->get_hwkey();
#if 0
            p4pd_hwkey_hwmask_build(coll_table_id, anchor_entry_->get_key(),
                                    NULL, (uint8_t *)hwkey, NULL);
#endif
        }

        SDK_TRACE_DEBUG("P4 FHCT Write:%d", hct_index_);

        // Entry trace
        if (get_ht_entry()->get_hbm_hash()->get_entry_trace_en()) {
            if (anchor_entry_) {
                entry_trace(coll_table_id, hct_index_,
                            (void *)anchor_entry_->get_key(), action_data);
            } else {
                entry_trace(coll_table_id, hct_index_, (void *)sw_key,
                            action_data);
            }
        }

		// P4-API: Collision Table Write
        pd_err = p4pd_entry_write(coll_table_id, hct_index_, (uint8_t*)hwkey, NULL,
                                  action_data);
        // SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY, hwkey);
	}

    SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_SW_KEY, sw_key);
    SDK_FREE(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA, action_data);
	return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : rs;
}

// ---------------------------------------------------------------------------
// De-Program the entry into HW
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashSpineEntry::deprogram_table()
{
    sdk_ret_t                       rs = SDK_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    void                            *hw_key = NULL;
    uint32_t                        hw_key_len = 0;
    uint32_t                        table_id = 0;
    uint32_t                        coll_table_id = 0;
    void                            *action_data;
    uint32_t                        entire_data_len;

    hw_key_len = get_ht_entry()->get_hbm_hash()->get_hwkey_len();
    table_id = get_ht_entry()->get_hbm_hash()->get_table_id();
    coll_table_id = get_ht_entry()->get_hbm_hash()->get_collision_table_id();
    entire_data_len = get_ht_entry()->get_hbm_hash()->
                      get_entire_data_len();
    action_data = SDK_CALLOC(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
                             entire_data_len);

	if (is_in_ht_) {
        hw_key = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY, hw_key_len);
        uint32_t ft_index = get_ht_entry()->get_bucket_index();

        // Entry trace
        if (get_ht_entry()->get_hbm_hash()->get_entry_trace_en()) {
            entry_trace(table_id, ft_index, NULL, NULL);
        }

		// P4-API: HBM Hash Table Write
        pd_err = p4pd_entry_write(table_id, ft_index, (uint8_t *)hw_key, NULL,
                                  action_data);
        SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_HW_KEY, hw_key);
    } else {
        // Entry trace
        if (get_ht_entry()->get_hbm_hash()->get_entry_trace_en()) {
            entry_trace(coll_table_id, hct_index_, NULL, NULL);
        }

		// P4-API: Collision Table Write
        pd_err = p4pd_entry_write(coll_table_id, hct_index_, NULL, NULL,
                                  action_data);
    }

    SDK_FREE(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA, action_data);

	return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : rs;
}





// ---------------------------------------------------------------------------
// Get number of Hint Groups
// ---------------------------------------------------------------------------
uint32_t
HbmHashSpineEntry::get_num_hgs()
{
    return hg_list_.size();
}

// ---------------------------------------------------------------------------
// Adds HBM Hash Hint Group to the list
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::add_hg(HbmHashHintGroup *hg)
{
    hg_list_.push_back(hg);
}

// ---------------------------------------------------------------------------
// Del HBM Hash Hint Group from the list
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::del_hg(HbmHashHintGroup *hg)
{
    hg_list_.remove(hg);
}

// ---------------------------------------------------------------------------
// Get Last HBM Hash Hint Group in the list
// ---------------------------------------------------------------------------
HbmHashHintGroup *
HbmHashSpineEntry::get_last_hg()
{
	return hg_list_.back();
}

// ---------------------------------------------------------------------------
// Get Last HBM Hash Entry
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashSpineEntry::get_last_hbm_hash_entry()
{
	if (!get_num_hgs()) {
		return anchor_entry_;
	} else {
		HbmHashHintGroup *hg = get_last_hg();
		return hg->get_last_hbm_hash_entry();
	}
}



// ---------------------------------------------------------------------------
// Replace the HG with new HG
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::replace_hg(HbmHashHintGroup *hg, HbmHashHintGroup *new_hg)
{
	HbmHashHintGroupList::iterator itr;
	for (itr = hg_list_.begin(); itr != hg_list_.end(); itr++) {
		if ((*itr) == hg) {
			(*itr) = new_hg;
		}
	}
}

// ---------------------------------------------------------------------------
// Set anchor entry
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::set_anchor_entry(HbmHashEntry *fl_entry)
{
    anchor_entry_ = fl_entry;
}


// ---------------------------------------------------------------------------
// Set prev entry
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::set_prev(HbmHashSpineEntry *prev)
{
    prev_ = prev;
}

// ---------------------------------------------------------------------------
// Set next entry
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::set_next(HbmHashSpineEntry *next)
{
    next_ = next;
}

// ---------------------------------------------------------------------------
// Set is_in_ft
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::set_is_in_ft(bool is_in_ft)
{
    is_in_ht_ = is_in_ft;
}

// ---------------------------------------------------------------------------
// Set HBM Hash Hash Collision Table index
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::set_fhct_index(uint32_t idx)
{
    hct_index_ = idx;
}

// ---------------------------------------------------------------------------
// Set HBM Hash Table Entry
// ---------------------------------------------------------------------------
void
HbmHashSpineEntry::set_ht_entry(HbmHashTableEntry *ht_entry)
{
    ht_entry_ = ht_entry;
}

sdk_ret_t
HbmHashSpineEntry::entry_trace(uint32_t table_id, uint32_t index,
                             void *key, void *data)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(table_id, index, key, NULL,
                                              data, buff, sizeof(buff));
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    if (!key && !data) {
        SDK_TRACE_DEBUG("Clearing entry at Index:%d", index);
    } else {
        SDK_TRACE_DEBUG("Index:%d %s", index, buff);
    }

    return SDK_RET_OK;
}

void
HbmHashSpineEntry::print_hse()
{
    HbmHashHintGroup               *hg = NULL;
    HbmHashHintGroupList::iterator itr;
    SDK_TRACE_DEBUG("hse:: is_in_ft:%d, fhct_index:%d, anc_entry_present:%d, "
                    "prev:%d, next:%d",
                    is_in_ht_, hct_index_,
                    ((anchor_entry_ != NULL) ? true : false),
                    prev_ ? prev_->get_fhct_index() : -1,
                    next_ ? next_->get_fhct_index() : -1);

    for (itr = hg_list_.begin();itr != hg_list_.end(); itr++) {
        hg = (*itr);
        hg->print_hg();
    }
}

void
HbmHashSpineEntry::entry_to_str(char *buff, uint32_t buff_size)
{
    p4pd_error_t    p4_err;
    uint32_t        table_id = 0;
    uint32_t        index = 0;
    void            *action_data;
    void            *sw_key;
    uint32_t        entire_data_len;
    uint32_t        sw_key_len = 0;

    table_id = is_in_ht_ ? get_ht_entry()->get_hbm_hash()->get_table_id() :
        get_ht_entry()->get_hbm_hash()->get_collision_table_id();
    sw_key_len = get_ht_entry()->get_hbm_hash()->get_key_len();
    entire_data_len = get_ht_entry()->get_hbm_hash()->
                      get_entire_data_len();
    index = is_in_ht_ ? get_ht_entry()->get_bucket_index() : hct_index_;

    action_data = SDK_CALLOC(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
                             entire_data_len);
    sw_key = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_SPINE_ENTRY_SW_KEY,
                        sw_key_len);

    form_action_data(action_data);
    if (is_in_ht_) {
        p4_err = p4pd_table_ds_decoded_string_get(table_id, index,
                                                  (void *)anchor_entry_->get_key(),
                                                  NULL,
                                                  action_data, buff, buff_size);
    } else {
        if (anchor_entry_) {
            p4_err = p4pd_table_ds_decoded_string_get(table_id, index,
                                                      (void *)anchor_entry_->get_key(),
                                                      NULL,
                                                      action_data, buff, buff_size);
        } else {
            p4_err = p4pd_table_ds_decoded_string_get(table_id, index,
                                                      (void *)sw_key,
                                                      NULL,
                                                      action_data, buff, buff_size);
        }
    }
    SDK_ASSERT(p4_err == P4PD_SUCCESS);
}
