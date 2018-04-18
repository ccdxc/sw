#include "flow_hint_group.hpp"
#include "flow_table_entry.hpp"
#include "flow.hpp"
#include "flow_entry.hpp"

using hal::pd::utils::FlowSpineEntry;
using hal::pd::utils::FlowHintGroup;
using hal::pd::utils::FlowEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
FlowHintGroup *
FlowHintGroup::factory(uint32_t hint_bits, FlowSpineEntry *fs_entry,
                       uint32_t mtrack_id)
{
    void            *mem = NULL;
    FlowHintGroup   *fhg = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(FlowHintGroup));
    if (!mem) {
        return NULL;
    }

    fhg = new (mem) FlowHintGroup(hint_bits, fs_entry);
    return fhg;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
FlowHintGroup::destroy(FlowHintGroup *fhg, uint32_t mtrack_id)
{
    if (fhg) {
        fhg->~FlowHintGroup();
        HAL_FREE(mtrack_id, fhg);
    }
}

// ---------------------------------------------------------------------------
// Constructor - Flow Hint Group
// ---------------------------------------------------------------------------
FlowHintGroup::FlowHintGroup(uint32_t hint_bits, FlowSpineEntry *fs_entry)
{
    hint_bits_ = hint_bits;
    fs_entry_ = fs_entry;
}

// ---------------------------------------------------------------------------
// Destructor - Flow Hint Group
// ---------------------------------------------------------------------------
FlowHintGroup::~FlowHintGroup() {}


// ---------------------------------------------------------------------------
// Add Flow entry into the list - Called after insert to HW is SUCCESS
// ---------------------------------------------------------------------------
void
FlowHintGroup::add_flow_entry(FlowEntry *f_entry)
{
    flow_entry_list_.push_back(f_entry);
}

// ---------------------------------------------------------------------------
// Del Flow entry into the list
// ---------------------------------------------------------------------------
void
FlowHintGroup::del_flow_entry(FlowEntry *f_entry)
{
    flow_entry_list_.remove(f_entry);
}

// ---------------------------------------------------------------------------
// Add Flow entry into the anchor list
// ---------------------------------------------------------------------------
void
FlowHintGroup::add_anchor_flow_entry(FlowEntry *f_entry)
{
    anchor_list_.push_back(f_entry);
}

// ---------------------------------------------------------------------------
// Del Flow entry from the anchor list
// ---------------------------------------------------------------------------
void
FlowHintGroup::del_anchor_flow_entry(FlowEntry *f_entry)
{
    anchor_list_.remove(f_entry);
}

// ---------------------------------------------------------------------------
// Get First Flow Entry in the list
// ---------------------------------------------------------------------------
FlowEntry *
FlowHintGroup::get_first_flow_entry()
{
    return flow_entry_list_.front();
}

// ---------------------------------------------------------------------------
// Get Last Flow Entry in the list
// ---------------------------------------------------------------------------
FlowEntry *
FlowHintGroup::get_last_flow_entry()
{
    return flow_entry_list_.back();
}

// ---------------------------------------------------------------------------
// Get Prev. Flow Entry
// ---------------------------------------------------------------------------
FlowEntry *
FlowHintGroup::get_next_flow_entry(FlowEntry *fe)
{
    std::list<FlowEntry*>::iterator itr;
    for (itr = flow_entry_list_.begin();
            itr != flow_entry_list_.end(); itr++) {
        if (*itr == fe) {
            if (std::next(itr) == flow_entry_list_.end()) {
                HAL_TRACE_DEBUG("FE:{}: Last element", __FUNCTION__);
                return NULL;
            } else {
                return (*std::next(itr));
            }
        }
    }

    return NULL;
}

// ---------------------------------------------------------------------------
// Get Next. Flow Entry
// ---------------------------------------------------------------------------
FlowEntry *
FlowHintGroup::get_prev_flow_entry(FlowEntry *fe)
{
    std::list<FlowEntry*>::iterator itr;
    for (itr = flow_entry_list_.begin();
            itr != flow_entry_list_.end(); itr++) {
        if (*itr == fe) {
            if (std::prev(itr) == flow_entry_list_.begin()) {
                return NULL;
            } else {
                return (*std::prev(itr));
            }
        }
    }

    return NULL;
}

// ---------------------------------------------------------------------------
// Checks if Flow Entry passed already exists
// ---------------------------------------------------------------------------
bool
FlowHintGroup::check_flow_entry_exists(FlowEntry *fe)
{
    std::list<FlowEntry*>::iterator itr;
    FlowEntry *tmp_fe = NULL;
    for (itr = flow_entry_list_.begin();
            itr != flow_entry_list_.end(); itr++) {
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

        HAL_TRACE_DEBUG("FlowHG:: key1: {} key2: {}, key_len: {}",
                fe->get_key(), tmp_fe->get_key(), tmp_fe->get_key_len());
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
FlowHintGroup::inter_hg_str(FlowEntry *f_entry,
                            char *inter_hg_buff, uint32_t inter_hg_size,
                            char *entry_buff, uint32_t entry_size)
{
    char tmp_buff[32] = {0};
    uint32_t index = 0;
    std::list<FlowEntry*>::iterator itr;
    FlowEntry *tmp_fe = NULL;
    for (itr = flow_entry_list_.begin();
            itr != flow_entry_list_.end(); itr++) {
        tmp_fe = (*itr);
        HAL_ASSERT(!tmp_fe->get_is_anchor_entry());
        index = tmp_fe->get_fhct_index();
        sprintf(tmp_buff, " %s:0x%x ", "COLL", index);
        strcat(inter_hg_buff, tmp_buff);
        if (tmp_fe == f_entry) {
            tmp_fe->entry_to_str(entry_buff, entry_size);
            HAL_TRACE_DEBUG("Inter HG str: {}", inter_hg_buff);
            HAL_TRACE_DEBUG("Entry..: {}", entry_buff);
            return;
        }
    }

}


// ---------------------------------------------------------------------------
// Prints all flow entries
// ---------------------------------------------------------------------------
void
FlowHintGroup::print_fhg()
{
    HAL_TRACE_DEBUG("  FHG:: hint_bits: {:#x}, num_anchors: {}, num_fes: {}",
            hint_bits_, anchor_list_.size(), flow_entry_list_.size());
    std::list<FlowEntry*>::iterator itr;
    FlowEntry *tmp_fe = NULL;
    for (itr = anchor_list_.begin();
            itr != anchor_list_.end(); itr++) {
        tmp_fe = (*itr);
        HAL_TRACE_DEBUG("    FE: Anchors");
        tmp_fe->print_fe();
    }
    for (itr = flow_entry_list_.begin();
            itr != flow_entry_list_.end(); itr++) {
        tmp_fe = (*itr);
        HAL_TRACE_DEBUG("    FE: Flow Entries");
        tmp_fe->print_fe();
    }
}


// ---------------------------------------------------------------------------
// Get Hint Bits
// ---------------------------------------------------------------------------
uint32_t
FlowHintGroup::get_hint_bits()
{
    return hint_bits_;
}

// ---------------------------------------------------------------------------
// Get the Flow Spine Entry
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowHintGroup::get_fs_entry()
{
    return fs_entry_;
}

// ---------------------------------------------------------------------------
// Set the Flow Spine Entry
// ---------------------------------------------------------------------------
void
FlowHintGroup::set_fs_entry(FlowSpineEntry *fs_entry)
{
    fs_entry_ = fs_entry;
}

// ---------------------------------------------------------------------------
// Get the Flow Spine Entry
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowHintGroup::get_delayed_del_fs_entry()
{
    return delayed_del_fs_entry_;
}

// ---------------------------------------------------------------------------
// Set the Flow Spine Entry
// ---------------------------------------------------------------------------
void
FlowHintGroup::set_delayed_del_fs_entry(FlowSpineEntry *delayed_del_fs_entry)
{
    delayed_del_fs_entry_ = delayed_del_fs_entry;
}


// ---------------------------------------------------------------------------
// Get number of flow entries
// ---------------------------------------------------------------------------
uint32_t
FlowHintGroup::get_num_flow_entries()
{
    return flow_entry_list_.size();
}

// ---------------------------------------------------------------------------
// Get number of anchor flow entries
// ---------------------------------------------------------------------------
uint32_t
FlowHintGroup::get_num_anchor_flow_entries()
{
    return anchor_list_.size();
}
