//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>
#include "hbm_hash.hpp"
#include "hbm_hash_entry.hpp"
#include "hbm_hash_hint_group.hpp"
#include "hbm_hash_table_entry.hpp"
#include "hbm_hash_spine_entry.hpp"
#include "hbm_hash_mem_types.hpp"

using sdk::table::HbmHashSpineEntry;
using sdk::table::HbmHashHintGroup;
using sdk::table::HbmHashEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
HbmHashHintGroup *
HbmHashHintGroup::factory(uint32_t hint_bits, HbmHashSpineEntry *fs_entry,
                          HbmHash *hbm_hash)
{
    void            *mem = NULL;
    HbmHashHintGroup   *hg = NULL;

    // mem = SDK_CALLOC(mtrack_id, sizeof(HbmHashHintGroup));
    mem = hbm_hash->hbm_hash_hint_group_alloc();
    if (!mem) {
        return NULL;
    }

    hg = new (mem) HbmHashHintGroup(hint_bits, fs_entry);
    return hg;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
HbmHashHintGroup::destroy(HbmHashHintGroup *hg, HbmHash *hbm_hash)
{
    if (hg) {
        hg->~HbmHashHintGroup();
        // SDK_FREE(mtrack_id, hg);
        hbm_hash->hbm_hash_hint_group_free(hg);
    }
}

// ---------------------------------------------------------------------------
// Constructor - HBM Hash Hint Group
// ---------------------------------------------------------------------------
HbmHashHintGroup::HbmHashHintGroup(uint32_t hint_bits, HbmHashSpineEntry *fs_entry)
{
    hint_bits_ = hint_bits;
    fs_entry_ = fs_entry;
    delayed_del_fs_entry_ = NULL;
}

// ---------------------------------------------------------------------------
// Destructor - HBM Hash Hint Group
// ---------------------------------------------------------------------------
HbmHashHintGroup::~HbmHashHintGroup() {}


// ---------------------------------------------------------------------------
// Add HBM Hash entry into the list - Called after insert to HW is SUCCESS
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::add_hbm_hash_entry(HbmHashEntry *h_entry)
{
    hbm_hash_entry_list_.push_back(h_entry);
}

// ---------------------------------------------------------------------------
// Del HBM Hash entry from the list
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::del_hbm_hash_entry(HbmHashEntry *h_entry)
{
    hbm_hash_entry_list_.remove(h_entry);
}

// ---------------------------------------------------------------------------
// Add HBM Hash entry into the anchor list
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::add_anchor_hbm_hash_entry(HbmHashEntry *h_entry)
{
    anchor_list_.push_back(h_entry);
}

// ---------------------------------------------------------------------------
// Del HBM Hash entry from the anchor list
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::del_anchor_hbm_hash_entry(HbmHashEntry *h_entry)
{
    anchor_list_.remove(h_entry);
}

// ---------------------------------------------------------------------------
// Get First HBM Hash Entry in the list
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashHintGroup::get_first_hbm_hash_entry()
{
    return hbm_hash_entry_list_.front();
}

// ---------------------------------------------------------------------------
// Get Last HBM Hash Entry in the list
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashHintGroup::get_last_hbm_hash_entry()
{
    return hbm_hash_entry_list_.back();
}

// ---------------------------------------------------------------------------
// Get Prev. HBM Hash Entry
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashHintGroup::get_next_hbm_hash_entry(HbmHashEntry *fe)
{
    std::list<HbmHashEntry*>::iterator itr;
    for (itr = hbm_hash_entry_list_.begin();
            itr != hbm_hash_entry_list_.end(); itr++) {
        if (*itr == fe) {
            if (std::next(itr) == hbm_hash_entry_list_.end()) {
                SDK_TRACE_DEBUG("Last element");
                return NULL;
            } else {
                return (*std::next(itr));
            }
        }
    }

    return NULL;
}

// ---------------------------------------------------------------------------
// Get Next. HBM Hash Entry
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashHintGroup::get_prev_hbm_hash_entry(HbmHashEntry *fe)
{
    std::list<HbmHashEntry*>::iterator itr;
    for (itr = hbm_hash_entry_list_.begin();
            itr != hbm_hash_entry_list_.end(); itr++) {
        if (*itr == fe) {
            if (std::prev(itr) == hbm_hash_entry_list_.begin()) {
                return NULL;
            } else {
                return (*std::prev(itr));
            }
        }
    }

    return NULL;
}

// ---------------------------------------------------------------------------
// Checks if HBM Hash Entry passed already exists
// ---------------------------------------------------------------------------
bool
HbmHashHintGroup::check_hbm_hash_entry_exists(HbmHashEntry *fe)
{
    std::list<HbmHashEntry*>::iterator itr;
    HbmHashEntry *tmp_fe = NULL;
    for (itr = hbm_hash_entry_list_.begin();
            itr != hbm_hash_entry_list_.end(); itr++) {
        tmp_fe = (*itr);

        if ((tmp_fe->get_key_len() == fe->get_key_len()) &&
                (!memcmp(tmp_fe->get_key(), fe->get_key(),
                        tmp_fe->get_key_len()))) {
            return TRUE;
        }
    }
    for (itr = anchor_list_.begin();
            itr != anchor_list_.end(); itr++) {
        tmp_fe = (*itr);

        //SDK_TRACE_DEBUG("HbmHashHG:: key1:%#x key2:%#x, key_len:%d",
                          //fe->get_key(), tmp_fe->get_key(), tmp_fe->get_key_len());
        if ((tmp_fe->get_key_len() == fe->get_key_len()) &&

                (!memcmp(fe->get_key(),
                         tmp_fe->get_key(),
                         tmp_fe->get_key_len()))) {
            return TRUE;
        }
    }
    return FALSE;
}

void
HbmHashHintGroup::inter_hg_str(HbmHashEntry *h_entry,
                            char *inter_hg_buff, uint32_t inter_hg_size,
                            char *entry_buff, uint32_t entry_size,
                            uint32_t *num_recircs)
{
    char tmp_buff[32] = {0};
    uint32_t index = 0;
    std::list<HbmHashEntry*>::iterator itr;
    HbmHashEntry *tmp_fe = NULL;
    for (itr = hbm_hash_entry_list_.begin();
            itr != hbm_hash_entry_list_.end(); itr++) {
        tmp_fe = (*itr);
        SDK_ASSERT(!tmp_fe->get_is_anchor_entry());
        index = tmp_fe->get_fhct_index();
        sprintf(tmp_buff, " %s:0x%x ", "COLL", index);
        strcat(inter_hg_buff, tmp_buff);
        (*num_recircs)++;
        if (tmp_fe == h_entry) {
            tmp_fe->entry_to_str(entry_buff, entry_size);
            SDK_TRACE_DEBUG("Inter HG str:%s", inter_hg_buff);
            SDK_TRACE_DEBUG("Entry..:%s", entry_buff);
            return;
        }
    }

}


// ---------------------------------------------------------------------------
// Prints all HBM Hash entries
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::print_hg()
{
    SDK_TRACE_DEBUG("  HG:: hint_bits:%#x, num_anchors:%lu, num_fes:%lu",
            hint_bits_, anchor_list_.size(), hbm_hash_entry_list_.size());
    std::list<HbmHashEntry*>::iterator itr;
    HbmHashEntry *tmp_fe = NULL;
    for (itr = anchor_list_.begin();
            itr != anchor_list_.end(); itr++) {
        tmp_fe = (*itr);
        SDK_TRACE_DEBUG("    HE: Anchors");
        tmp_fe->print_fe();
    }
    for (itr = hbm_hash_entry_list_.begin();
            itr != hbm_hash_entry_list_.end(); itr++) {
        tmp_fe = (*itr);
        SDK_TRACE_DEBUG("    HE: Entries");
        tmp_fe->print_fe();
    }
}


// ---------------------------------------------------------------------------
// Get Hint Bits
// ---------------------------------------------------------------------------
uint32_t
HbmHashHintGroup::get_hint_bits()
{
    return hint_bits_;
}

// ---------------------------------------------------------------------------
// Get the HBM Hash Spine Entry
// ---------------------------------------------------------------------------
HbmHashSpineEntry *
HbmHashHintGroup::get_fs_entry()
{
    return fs_entry_;
}

// ---------------------------------------------------------------------------
// Set the HBM Hash Spine Entry
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::set_fs_entry(HbmHashSpineEntry *fs_entry)
{
    fs_entry_ = fs_entry;
}

// ---------------------------------------------------------------------------
// Get the HBM Hash Spine Entry
// ---------------------------------------------------------------------------
HbmHashSpineEntry *
HbmHashHintGroup::get_delayed_del_fs_entry()
{
    return delayed_del_fs_entry_;
}

// ---------------------------------------------------------------------------
// Set the HBM Hash Spine Entry
// ---------------------------------------------------------------------------
void
HbmHashHintGroup::set_delayed_del_fs_entry(HbmHashSpineEntry *delayed_del_fs_entry)
{
    delayed_del_fs_entry_ = delayed_del_fs_entry;
}


// ---------------------------------------------------------------------------
// Get number of HBM Hash entries
// ---------------------------------------------------------------------------
uint32_t
HbmHashHintGroup::get_num_hbm_hash_entries()
{
    return hbm_hash_entry_list_.size();
}

// ---------------------------------------------------------------------------
// Get number of anchor HBM Hash entries
// ---------------------------------------------------------------------------
uint32_t
HbmHashHintGroup::get_num_anchor_hbm_hash_entries()
{
    return anchor_list_.size();
}
