#include "nic/include/trace.hpp"
#include "nic/hal/pd/utils/flow/flow_table_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_spine_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_hint_group.hpp"
#include "nic/hal/pd/utils/flow/flow_entry.hpp"
#include "nic/hal/pd/utils/flow/flow.hpp"

using hal::pd::utils::FlowTableEntry;
using hal::pd::utils::FlowSpineEntry;
using hal::pd::utils::FlowHintGroup;
using hal::pd::utils::FlowEntry;
using hal::pd::utils::Flow;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
FlowTableEntry *
FlowTableEntry::factory(uint32_t ft_bits, Flow *flow, uint32_t mtrack_id)
{
    void            *mem = NULL;
    FlowTableEntry  *fte = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(FlowTableEntry));
    if (!mem) {
        return NULL;
    }

    fte = new (mem) FlowTableEntry(ft_bits, flow);
    return fte;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
FlowTableEntry::destroy(FlowTableEntry *fte, uint32_t mtrack_id)
{
    if (fte) {
        fte->~FlowTableEntry();
        HAL_FREE(mtrack_id, fte);
    }
}

// ---------------------------------------------------------------------------
// Constructor - Flow Table Entry
// ---------------------------------------------------------------------------
FlowTableEntry::FlowTableEntry(uint32_t ft_bits, Flow *flow)
{
    ft_bits_            = ft_bits;
    flow_               = flow;
    spine_entry_        = NULL;
    num_spine_entries_  = 0;
}

// ---------------------------------------------------------------------------
// Destructor - Flow Table Entry
// ---------------------------------------------------------------------------
FlowTableEntry::~FlowTableEntry() {}

// ---------------------------------------------------------------------------
// Insert Flow Entry to Flow Table Entry
// ---------------------------------------------------------------------------
hal_ret_t
FlowTableEntry::insert(FlowEntry *f_entry)
{
    hal_ret_t           rs = HAL_RET_OK;
    uint32_t            hint_bits = 0;
    FlowHintGroup       *fh_grp = NULL;
    FlowSpineEntry      *fse = NULL;
    bool                is_new_fse = FALSE;
    std::map<uint32_t, FlowHintGroup*>::iterator itr;

    HAL_TRACE_DEBUG("FlowTE::{}: Hash_Table_Entry:{:#x} ...", __FUNCTION__,
                    get_ft_bits());

    hint_bits = get_flow()->fetch_hint_bits_(f_entry->get_hash_val());
    itr = hint_groups_map_.find(hint_bits);
    // Check if Hint Group exists for the flow entry
    if (itr != hint_groups_map_.end()) {
        // Hint Group exists
        HAL_TRACE_DEBUG("FlowTE::{}: HG Entry exist ...", __FUNCTION__);
        fh_grp = itr->second;

        if(fh_grp->check_flow_entry_exists(f_entry)) {
            HAL_TRACE_DEBUG("FlowTE::{}: Duplicate Insert", __FUNCTION__);
            return HAL_RET_DUP_INS_FAIL;
        }

        fse = get_spine_entry_for_hg(fh_grp, &is_new_fse);
        rs = f_entry->insert(fh_grp, fse);
        if (rs != HAL_RET_OK) {
            if (is_new_fse) {
                // delete fse;
                FlowSpineEntry::destroy(fse);
                num_spine_entries_--;
            }
        }
    } else {
        // Hint Group doesnt exist
        HAL_TRACE_DEBUG("FlowTE::{}: New HG Entry ...", __FUNCTION__);
        // Check if we can put this new HG in the existing Spine Entry
        fse = get_spine_entry_for_new_hg(&is_new_fse);
        //   - Create Hint Group
        // fh_grp = new FlowHintGroup(hint_bits, NULL);
        fh_grp = FlowHintGroup::factory(hint_bits, NULL);
        rs = f_entry->insert(fh_grp, fse);

        // If insert is SUCCESS, put fh_grp into the map and list
        if (rs == HAL_RET_OK) {
            hint_groups_map_[hint_bits] = fh_grp;
        } else {
            // delete fh_grp;
            FlowHintGroup::destroy(fh_grp);
            if (is_new_fse) {
                // delete fse;
                FlowSpineEntry::destroy(fse);
                num_spine_entries_--;
            }
        }
    }
    if (rs == HAL_RET_OK) {
        if (is_new_fse) {
            if (!spine_entry_) {
                spine_entry_ = fse;
            } else {
                // Will be set in FlowEntry insert itself.
                // prev was set when its created
                // fse->get_prev()->set_next(fse);
            }
        }
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Remove Flow Entry from Flow Table Entry
// ---------------------------------------------------------------------------
hal_ret_t
FlowTableEntry::remove(FlowEntry *f_entry)
{
    hal_ret_t rs = HAL_RET_OK;
    FlowHintGroup *fhg = f_entry->get_fh_group();
    FlowSpineEntry *fspe = f_entry->get_eff_spine_entry();

    rs = f_entry->remove();

    HAL_TRACE_DEBUG("{}: After Removal: fhg_num_fes: {}, fhg_num_anchors:{}",
            __FUNCTION__, fhg->get_num_flow_entries(),
            fhg->get_num_anchor_flow_entries());
    // Check if this is last in Hint group.
    if (!fhg->get_num_flow_entries() && !fhg->get_num_anchor_flow_entries()) {
        HAL_TRACE_DEBUG("{}: hint_bits:{:#x} Removing fhg", __FUNCTION__,
                fhg->get_hint_bits());
        remove_fhg(fhg);
    }
    HAL_TRACE_DEBUG("{}: After Removal: fspe_has_anchor: {:#x}, fspe_num_hgs:{}",
            __FUNCTION__, fspe->get_anchor_entry() ? true : false,
            fspe->get_num_hgs());
    // Check if this is last in Spine entry.
    if (!fspe->get_anchor_entry() && !fspe->get_num_hgs()) {
        // Reset & Programming of Prev would have been done in FlowEntry.
        // Just free up
        HAL_TRACE_DEBUG("{}: Removing spine entry", __FUNCTION__);
        // delete fspe;
        FlowSpineEntry::destroy(fspe);
        num_spine_entries_--;
        if (!num_spine_entries_) {
            spine_entry_ = NULL;
        }
    }

    // Uncomment for debugging
    // print_fte();
    return rs;
}

// ---------------------------------------------------------------------------
// Get number of Flow Hint Groups
// ---------------------------------------------------------------------------
uint32_t
FlowTableEntry::get_num_flow_hgs()
{
    return hint_groups_map_.size();
}

// ---------------------------------------------------------------------------
// Get Spine entry for new Hint Group
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowTableEntry::get_spine_entry_for_new_hg(bool *is_new)
{
    FlowSpineEntry  *sp_entry = spine_entry_;
    FlowSpineEntry  *new_sp_entry = NULL;
    uint32_t        fse_fhct_index = 0;

    // Get to the last spine entry.
    while (sp_entry && sp_entry->get_next() != NULL) {
        sp_entry = sp_entry->get_next();
    }

    if (!sp_entry ||
            (sp_entry->get_num_hgs() ==
            flow_->get_num_hints_per_flow_entry())) {
        HAL_TRACE_DEBUG("FlowTE::{}: New Spine Entry ...", __FUNCTION__);
        *is_new = TRUE;
        // new_sp_entry = new FlowSpineEntry(this);
        new_sp_entry = FlowSpineEntry::factory(this);
        if (!sp_entry) {
            // Spine Entry will go in Flow Table
            new_sp_entry->set_is_in_ft(TRUE);
        } else {
            // Spine Entry will go in Flow Hash Collision Table
            new_sp_entry->set_is_in_ft(FALSE);
            // Setting up prev to sp_entry which is either in FT or FHCT.
            new_sp_entry->set_prev(sp_entry);
            // allocate fhct index where this spine entry will be pgmed
            flow_->alloc_fhct_index(&fse_fhct_index);
            new_sp_entry->set_fhct_index(fse_fhct_index);
        }
        num_spine_entries_++;
        return new_sp_entry;
    } else {
        HAL_TRACE_DEBUG("FlowTE::{}: Spine Entry exist...", __FUNCTION__);
        return sp_entry;
    }
}

// ---------------------------------------------------------------------------
// Get Spine entry for Hint Group
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowTableEntry::get_spine_entry_for_hg(FlowHintGroup *fhg, bool *is_new)
{
    FlowSpineEntry *sp_entry = fhg->get_fs_entry();
    if (sp_entry) {
        *is_new = FALSE;
        return sp_entry;
    } else {
        return get_spine_entry_for_new_hg(is_new);
    }
}


// ---------------------------------------------------------------------------
// Get Last Spine entry
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowTableEntry::get_last_spine_entry()
{
    FlowSpineEntry *sp_entry = spine_entry_;
    while (sp_entry && sp_entry->get_next() != NULL) {
        sp_entry = sp_entry->get_next();
    }

    return sp_entry;
}

// ---------------------------------------------------------------------------
// Get Last Flow entry
// ---------------------------------------------------------------------------
FlowEntry *
FlowTableEntry::get_last_flow_entry()
{
    FlowSpineEntry *sp_entry = get_last_spine_entry();
    return sp_entry->get_last_flow_entry();
}




/*
// ---------------------------------------------------------------------------
// Get position of hint group in list
// ---------------------------------------------------------------------------
int
FlowTableEntry::find_pos_of_hg(FlowHintGroup *hg)
{
    std::list<FlowHintGroup*>::iterator it;
    FlowHintGroup *fhg;
    int pos = -1;
    uint32_t hint_bits = hg->get_hint_bits();

    // Walk the list
    for (it = hint_groups_.begin(), pos = 0; it != hint_groups_.end(); ++it, ++pos) {
        fhg = *it;
        if (fhg->get_hint_bits() == hint_bits) {
            return pos;
        }
    }

    return -1;
}

// ---------------------------------------------------------------------------
// Get FT entry for range of HGs
// ---------------------------------------------------------------------------
void
FlowTableEntry::form_ft_entry_from_hgs(uint32_t begin, uint32_t end, void *ft_entry)
{
    std::list<FlowHintGroup*>::iterator it = hint_groups_.begin();
    std::advance(it, begin);
    uint32_t pos = begin;
    for (; it != hint_groups_.end() && pos <= end; it++, pos++) {
        if (pos % flow_->get_num_hints_per_flow_entry() == 0) {
            // Starting of FT entry

        }
    }

}
*/


// ---------------------------------------------------------------------------
// Add Flow Hint Group
// ---------------------------------------------------------------------------
void
FlowTableEntry::add_fhg(uint32_t hint_bits, FlowHintGroup *fhg)
{
    hint_groups_map_[hint_bits] = fhg;
}

// ---------------------------------------------------------------------------
// Remove fhg from groups map and clean up
// ---------------------------------------------------------------------------
void
FlowTableEntry::remove_fhg(FlowHintGroup *fhg)
{
    if (!fhg->get_num_flow_entries() && !fhg->get_num_anchor_flow_entries()) {
        // Remove Hint Group
        hint_groups_map_.erase(fhg->get_hint_bits());
        // Free Hint Group
        // delete fhg;
        FlowHintGroup::destroy(fhg);
    } else {
        // Dont call this api if fhg still has flow entries
        HAL_ASSERT(0);
    }
}


// ---------------------------------------------------------------------------
// Get Flow table Index bits
// ---------------------------------------------------------------------------
uint32_t
FlowTableEntry::get_ft_bits()
{
    return ft_bits_;
}

// ---------------------------------------------------------------------------
// Get Flow Instance
// ---------------------------------------------------------------------------
Flow *
FlowTableEntry::get_flow()
{
    return flow_;
}

// ---------------------------------------------------------------------------
// Get Spine Entry
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowTableEntry::get_spine_entry()
{
    return spine_entry_;
}

// ---------------------------------------------------------------------------
// Set Spine Entry
// ---------------------------------------------------------------------------
void
FlowTableEntry::set_spine_entry(FlowSpineEntry *sp_entry)
{
    spine_entry_ = sp_entry;
}

void
FlowTableEntry::inter_spine_str(FlowSpineEntry *eff_spine,
                                char *buff, uint32_t buff_size)
{
    char tmp_buff[8] = {0};
    uint32_t index = 0;

    FlowSpineEntry *sp_entry = spine_entry_;
    while (sp_entry) {
        index = sp_entry->get_is_in_ft() ? sp_entry->get_ft_entry()->get_ft_bits() : sp_entry->get_fhct_index();
        sprintf(tmp_buff, " %s:%d ", sp_entry->get_is_in_ft() ? "FT" : "COLL", index);
        if (sp_entry == eff_spine) {
            return;
        }
        sp_entry = sp_entry->get_next();
    }
}

// ---------------------------------------------------------------------------
// Print Flow Hint Groups
// ---------------------------------------------------------------------------
void
FlowTableEntry::print_fte()
{
    uint32_t hint_bits = 0;
    FlowHintGroup *fhg = NULL;
    FlowSpineEntry *sp_entry = spine_entry_;

    HAL_TRACE_DEBUG("Num_FSEs: {}", num_spine_entries_);
    while (sp_entry) {
        sp_entry->print_fse();
        sp_entry = sp_entry->get_next();
    }

    HAL_TRACE_DEBUG("Total Num_FHGs: {}", hint_groups_map_.size());
    for (FHGMap::const_iterator it = hint_groups_map_.begin();
            it != hint_groups_map_.end(); ++it) {
            hint_bits = it->first;
            fhg = it->second;
            HAL_TRACE_DEBUG("  hint_bits: {:#x}", hint_bits);
            fhg->print_fhg();
    }

}

void
FlowTableEntry::entry_to_str(char *buff, uint32_t buff_size)
{
    FlowSpineEntry *sp_entry = spine_entry_;

    HAL_TRACE_DEBUG("Num_FSEs: {}", num_spine_entries_);
    while (sp_entry) {
        sp_entry->entry_to_str(buff, buff_size);
        sp_entry = sp_entry->get_next();
    }
}
