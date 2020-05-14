//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "include/sdk/base.hpp"
#include "string.h"
#include "hbm_hash.hpp"
#include "hbm_hash_entry.hpp"
#include "hbm_hash_table_entry.hpp"
#include "hbm_hash_spine_entry.hpp"
#include "hbm_hash_hint_group.hpp"
#include "hbm_hash_mem_types.hpp"

using sdk::table::HbmHashTableEntry;
using sdk::table::HbmHashSpineEntry;
using sdk::table::HbmHashHintGroup;
using sdk::table::HbmHashEntry;
using sdk::table::HbmHash;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
HbmHashTableEntry *
HbmHashTableEntry::factory(uint32_t bucket_index, HbmHash *hbm_hash)
{
    void            *mem = NULL;
    HbmHashTableEntry  *fte = NULL;

    // mem = SDK_CALLOC(mtrack_id, sizeof(HbmHashTableEntry));
    mem = hbm_hash->hbm_hash_table_entry_alloc();
    if (!mem) {
        return NULL;
    }

    fte = new (mem) HbmHashTableEntry(bucket_index, hbm_hash);
    return fte;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
HbmHashTableEntry::destroy(HbmHashTableEntry *fte)
{
    HbmHash *hbm_hash = NULL;
    if (fte) {
        hbm_hash = fte->get_hbm_hash();
        fte->~HbmHashTableEntry();
        // SDK_FREE(mtrack_id, fte);
        hbm_hash->hbm_hash_table_entry_free(fte);
    }
}

// ---------------------------------------------------------------------------
// Constructor - HBM Hash Table Entry
// ---------------------------------------------------------------------------
HbmHashTableEntry::HbmHashTableEntry(uint32_t bucket_index, HbmHash *hbm_hash)
{
    bucket_index_       = bucket_index;
    hbm_hash_           = hbm_hash;
    spine_entry_        = NULL;
    num_spine_entries_  = 0;
}

// ---------------------------------------------------------------------------
// Destructor - HBM Hash Table Entry
// ---------------------------------------------------------------------------
HbmHashTableEntry::~HbmHashTableEntry() {}

// ---------------------------------------------------------------------------
// Insert HBM Hash Entry to HBM Hash Table Entry
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashTableEntry::insert(HbmHashEntry *h_entry)
{
    sdk_ret_t           rs = SDK_RET_OK;
    uint32_t            hint_bits = 0;
    HbmHashHintGroup    *fh_grp = NULL;
    HbmHashSpineEntry   *fse = NULL;
    bool                is_new_fse = FALSE;
    std::map<uint32_t, HbmHashHintGroup*>::iterator itr;

    SDK_TRACE_DEBUG("Hash_Table_Entry %#x ...", get_bucket_index());

    hint_bits = get_hbm_hash()->fetch_hint_bits_(h_entry->get_hash_val());
    itr = hint_groups_map_.find(hint_bits);
    // Check if Hint Group exists for the HBM Hash entry
    if (itr != hint_groups_map_.end()) {
        // Hint Group exists
        SDK_TRACE_DEBUG("HG Entry exist ...");
        fh_grp = itr->second;

        if(fh_grp->check_hbm_hash_entry_exists(h_entry)) {
            SDK_TRACE_DEBUG("Duplicate Insert");
            return SDK_RET_ENTRY_EXISTS;
        }

        rs = get_spine_entry_for_hg(fh_grp, h_entry, &is_new_fse, &fse);
        if (rs != SDK_RET_OK) {
            SDK_TRACE_DEBUG("Failed to get spine entry, ret %d", rs);
            return rs;
        }

        rs = h_entry->insert(fh_grp, fse);
        if (rs != SDK_RET_OK) {
            if (is_new_fse) {
                // delete fse;
                HbmHashSpineEntry::destroy(fse);
                num_spine_entries_--;
            }
        }
    } else {
        // Hint Group doesnt exist
        SDK_TRACE_DEBUG("New HG Entry ...");
        // Check if we can put this new HG in the existing Spine Entry
        rs = get_spine_entry_for_new_hg(&is_new_fse, h_entry, &fse);
        if (rs != SDK_RET_OK) {
            SDK_TRACE_DEBUG("Failed to create spine entry, ret %d",
                            rs);
            return rs;
        }
        //   - Create Hint Group
        // fh_grp = new HbmHashHintGroup(hint_bits, NULL);
        fh_grp = HbmHashHintGroup::factory(hint_bits, NULL, get_hbm_hash());
        rs = h_entry->insert(fh_grp, fse);

        // If insert is SUCCESS, put fh_grp into the map and list
        if (rs == SDK_RET_OK) {
            hint_groups_map_[hint_bits] = fh_grp;
        } else {
            // delete fh_grp;
            HbmHashHintGroup::destroy(fh_grp, get_hbm_hash());
            if (is_new_fse) {
                // delete fse;
                HbmHashSpineEntry::destroy(fse);
                num_spine_entries_--;
            }
        }
    }
    if (rs == SDK_RET_OK) {
        if (is_new_fse) {
            if (!spine_entry_) {
                spine_entry_ = fse;
            } else {
                // Will be set in HbmHashEntry insert itself.
                // prev was set when its created
                // fse->get_prev()->set_next(fse);
            }
        }
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Remove HBM Hash Entry from HBM Hash Table Entry
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashTableEntry::remove(HbmHashEntry *h_entry)
{
    sdk_ret_t rs = SDK_RET_OK;
    HbmHashHintGroup *hg = h_entry->get_fh_group();
    HbmHashSpineEntry *fspe = h_entry->get_eff_spine_entry();

    SDK_TRACE_DEBUG("HTE: FT bits %#x, Hint bits %#x",
                    bucket_index_,
                    hg->get_hint_bits());

    rs = h_entry->remove();

    SDK_TRACE_DEBUG("After Removal: hg_num_fes %d, hg_num_anchors %d",
                    hg->get_num_hbm_hash_entries(),
                    hg->get_num_anchor_hbm_hash_entries());
    // Check if this is last in Hint group.
    if (!hg->get_num_hbm_hash_entries() && !hg->get_num_anchor_hbm_hash_entries()) {
        SDK_TRACE_DEBUG("hint_bits:%#x Removing hg", hg->get_hint_bits());
        remove_hg(hg);
    }
    SDK_TRACE_DEBUG("After Removal: fspe_has_anchor %#x, fspe_num_hgs %d",
                    fspe->get_anchor_entry() ? true : false,
                    fspe->get_num_hgs());
    // Check if this is last in Spine entry.
    if (!fspe->get_anchor_entry() && !fspe->get_num_hgs()) {
        // Reset & Programming of Prev would have been done in HbmHashEntry.
        // Just free up
        SDK_TRACE_DEBUG("Removing spine entry");
        // delete fspe;
        HbmHashSpineEntry::destroy(fspe);
        num_spine_entries_--;
        if (!num_spine_entries_) {
            spine_entry_ = NULL;
        }
    }

    // Uncomment for debugging
    // print_hbm_hash_table_entries();
    return rs;
}

// ---------------------------------------------------------------------------
// Get number of HBM Hash Hint Groups
// ---------------------------------------------------------------------------
uint32_t
HbmHashTableEntry::get_num_hbm_hash_hgs()
{
    return hint_groups_map_.size();
}

// ---------------------------------------------------------------------------
// Get Spine entry for new Hint Group
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashTableEntry::get_spine_entry_for_new_hg(bool *is_new,
                                              HbmHashEntry *h_entry,
                                              HbmHashSpineEntry **spine_entry)
{
    sdk_ret_t           ret = SDK_RET_OK;
    HbmHashSpineEntry   *sp_entry = spine_entry_;
    HbmHashSpineEntry   *new_sp_entry = NULL;
    uint32_t            fse_fhct_index = 0;

    *spine_entry = NULL;

    /*
     * Recirc Calculation:
     * Case 1: No spine entry
     *  #recircs = 0
     * Case 2: Last spine entry with hg able to accomodate
     *  #recircs = #steps to spine entry + 1(attached entry)
     * Case 3: Last spine entry with hg not able to accomodate
     * #recircs = #steps to spine entry + 1(New spine entry) + 1(attached entry)
     */

    // Get to the last spine entry.
    while (sp_entry && sp_entry->get_next() != NULL) {
        sp_entry = sp_entry->get_next();
        h_entry->inc_recircs();
    }

    if (!sp_entry ||
            (sp_entry->get_num_hgs() ==
            hbm_hash_->get_num_hints_per_entry())) {
        //SDK_TRACE_DEBUG("New Spine Entry ...");
        *is_new = TRUE;

        if (sp_entry) {
            // Case 3:
            // For new spine
            h_entry->inc_recircs();
            // For attached entry on the new spine
            h_entry->inc_recircs();
        }

        if (h_entry->get_recircs() == hbm_hash_->max_recircs()) {
            ret = SDK_RET_MAX_RECIRC_EXCEED;
            SDK_TRACE_ERR("Unable to install flow. #recircs "
                          "exceeds max recircs %d, ret %d",
                          hbm_hash_->max_recircs(), ret);
            return ret;
        }
        new_sp_entry = HbmHashSpineEntry::factory(this);
        if (!sp_entry) {
            // Spine Entry will go in HBM Hash Table
            new_sp_entry->set_is_in_ft(TRUE);
        } else {
            // Spine Entry will go in HBM Hash Collision Table
            new_sp_entry->set_is_in_ft(FALSE);
            // Setting up prev to sp_entry which is either in FT or FHCT.
            new_sp_entry->set_prev(sp_entry);
            // allocate fhct index where this spine entry will be pgmed
            hbm_hash_->alloc_collision_index(&fse_fhct_index);
            new_sp_entry->set_fhct_index(fse_fhct_index);
        }
        num_spine_entries_++;
        *spine_entry = new_sp_entry;
        return ret;
    } else {
        if (sp_entry) {
            // Case 2:
            h_entry->inc_recircs();
            if (h_entry->get_recircs() == hbm_hash_->max_recircs()) {
                ret = SDK_RET_MAX_RECIRC_EXCEED;
                SDK_TRACE_ERR("Unable to install flow. #recircs "
                              "exceeds max recircs %d, ret %d",
                              hbm_hash_->max_recircs(), ret);
                return ret;
            }
        }
    }
    //SDK_TRACE_DEBUG("Spine Entry exist...");
    *spine_entry = sp_entry;
    return ret;
}

// ---------------------------------------------------------------------------
// Get Spine entry for Hint Group
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashTableEntry::get_spine_entry_for_hg(HbmHashHintGroup *hg,
                                          HbmHashEntry *h_entry,
                                          bool *is_new,
                                          HbmHashSpineEntry **spine_entry)
{
    sdk_ret_t ret = SDK_RET_OK;
    *spine_entry = hg->get_fs_entry();
    if (*spine_entry) {
        *is_new = FALSE;
        return SDK_RET_OK;
    } else {
        ret = get_spine_entry_for_new_hg(is_new, h_entry, spine_entry);
        return ret;
    }
}


// ---------------------------------------------------------------------------
// Get Last Spine entry
// ---------------------------------------------------------------------------
HbmHashSpineEntry *
HbmHashTableEntry::get_last_spine_entry()
{
    HbmHashSpineEntry *sp_entry = spine_entry_;
    while (sp_entry && sp_entry->get_next() != NULL) {
        sp_entry = sp_entry->get_next();
    }

    return sp_entry;
}

// ---------------------------------------------------------------------------
// Get Last HBM Hash entry
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashTableEntry::get_last_hbm_hash_entry()
{
    HbmHashSpineEntry *sp_entry = get_last_spine_entry();
    return sp_entry->get_last_hbm_hash_entry();
}

/*
// ---------------------------------------------------------------------------
// Get position of hint group in list
// ---------------------------------------------------------------------------
int
HbmHashTableEntry::find_pos_of_hg(HbmHashHintGroup *hg)
{
    std::list<HbmHashHintGroup*>::iterator it;
    HbmHashHintGroup *hg;
    int pos = -1;
    uint32_t hint_bits = hg->get_hint_bits();

    // Walk the list
    for (it = hint_groups_.begin(), pos = 0; it != hint_groups_.end(); ++it, ++pos) {
        hg = *it;
        if (hg->get_hint_bits() == hint_bits) {
            return pos;
        }
    }

    return -1;
}

// ---------------------------------------------------------------------------
// Get FT entry for range of HGs
// ---------------------------------------------------------------------------
void
HbmHashTableEntry::form_ft_entry_from_hgs(uint32_t begin, uint32_t end, void *ft_entry)
{
    std::list<HbmHashHintGroup*>::iterator it = hint_groups_.begin();
    std::advance(it, begin);
    uint32_t pos = begin;
    for (; it != hint_groups_.end() && pos <= end; it++, pos++) {
        if (pos % hbm_hash_->get_num_hints_per_entry() == 0) {
            // Starting of FT entry

        }
    }

}
*/


// ---------------------------------------------------------------------------
// Add HBM Hash Hint Group
// ---------------------------------------------------------------------------
void
HbmHashTableEntry::add_hg(uint32_t hint_bits, HbmHashHintGroup *hg)
{
    hint_groups_map_[hint_bits] = hg;
}

// ---------------------------------------------------------------------------
// Remove hg from groups map and clean up
// ---------------------------------------------------------------------------
void
HbmHashTableEntry::remove_hg(HbmHashHintGroup *hg)
{
    if (!hg->get_num_hbm_hash_entries() && !hg->get_num_anchor_hbm_hash_entries()) {
        // Remove Hint Group
        hint_groups_map_.erase(hg->get_hint_bits());
        // Free Hint Group
        // delete hg;
        HbmHashHintGroup::destroy(hg, get_hbm_hash());
    } else {
        // Dont call this api if hg still has HBM Hash entries
        SDK_ASSERT(0);
    }
}


// ---------------------------------------------------------------------------
// Get HBM Hash table Index bits
// ---------------------------------------------------------------------------
uint32_t
HbmHashTableEntry::get_bucket_index()
{
    return bucket_index_;
}

// ---------------------------------------------------------------------------
// Get HBM Hash Instance
// ---------------------------------------------------------------------------
HbmHash *
HbmHashTableEntry::get_hbm_hash()
{
    return hbm_hash_;
}

// ---------------------------------------------------------------------------
// Get Spine Entry
// ---------------------------------------------------------------------------
HbmHashSpineEntry *
HbmHashTableEntry::get_spine_entry()
{
    return spine_entry_;
}

// ---------------------------------------------------------------------------
// Set Spine Entry
// ---------------------------------------------------------------------------
void
HbmHashTableEntry::set_spine_entry(HbmHashSpineEntry *sp_entry)
{
    spine_entry_ = sp_entry;
}

void
HbmHashTableEntry::inter_spine_str(HbmHashSpineEntry *eff_spine,
                                char *buff, uint32_t buff_size,
                                uint32_t *num_recircs)
{
    char tmp_buff[32] = {0};
    uint32_t index = 0;

    HbmHashSpineEntry *sp_entry = spine_entry_;
    while (sp_entry) {
        index = sp_entry->get_is_in_ft() ? sp_entry->get_ht_entry()->get_bucket_index() : sp_entry->get_fhct_index();
        sprintf(tmp_buff, " %s:0x%x ", sp_entry->get_is_in_ft() ? "FT" : "COLL", index);
        strcat(buff, tmp_buff);
        (*num_recircs)++;
        if (sp_entry == eff_spine) {
            SDK_TRACE_DEBUG("Inter spine str:%s", buff);
            // Don't count the first one
            (*num_recircs)--;
            return;
        }
        sp_entry = sp_entry->get_next();
    }
}

// ---------------------------------------------------------------------------
// Print HBM Hash Hint Groups
// ---------------------------------------------------------------------------
void
HbmHashTableEntry::print_hbm_hash_table_entries()
{
    uint32_t hint_bits = 0;
    HbmHashHintGroup *hg = NULL;
    HbmHashSpineEntry *sp_entry = spine_entry_;

    SDK_TRACE_DEBUG("Num_FSEs:%d", num_spine_entries_);
    while (sp_entry) {
        sp_entry->print_hse();
        sp_entry = sp_entry->get_next();
    }

    SDK_TRACE_DEBUG("Total Num_HGs:%lu", hint_groups_map_.size());
    for (HGMap::const_iterator it = hint_groups_map_.begin();
            it != hint_groups_map_.end(); ++it) {
            hint_bits = it->first;
            hg = it->second;
            SDK_TRACE_DEBUG("  hint_bits:%#x", hint_bits);
            hg->print_hg();
    }
}

void
HbmHashTableEntry::entry_to_str(char *buff, uint32_t buff_size)
{
    HbmHashSpineEntry *sp_entry = spine_entry_;

    SDK_TRACE_DEBUG("Num_FSEs:%d", num_spine_entries_);
    while (sp_entry) {
        sp_entry->entry_to_str(buff, buff_size);
        sp_entry = sp_entry->get_next();
    }
}
