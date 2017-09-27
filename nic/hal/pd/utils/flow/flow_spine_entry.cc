#include "nic/hal/pd/utils/flow/flow.hpp"
#include "nic/hal/pd/utils/flow/flow_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_spine_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_table_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_hint_group.hpp"
#include "nic/include/trace.hpp"

using hal::pd::utils::FlowHintGroup;
using hal::pd::utils::FlowSpineEntry;
using hal::pd::utils::FlowEntry;

// ---------------------------------------------------------------------------
// Constructor - Flow Spine Entry
// ---------------------------------------------------------------------------
FlowSpineEntry::FlowSpineEntry(FlowTableEntry *ft_entry)
{
    ft_entry_       = ft_entry;
    anchor_entry_   = NULL;
    is_in_ft_       = false;
    fhct_index_     = 0;
    prev_           = NULL;
    next_           = NULL;
}
           
#define flow_hash_info action_data->flow_hash_action_u.flow_hash_flow_hash_info
#define flow_hash_info1 action_data.flow_hash_action_u.flow_hash_flow_hash_info
// ---------------------------------------------------------------------------
// Form the HW entry for this spine entry
// ---------------------------------------------------------------------------
void 
FlowSpineEntry::form_action_data(flow_hash_actiondata *action_data)
{
    FlowHintGroup               *fhg = NULL;
    FlowEntry                   *root_fe = NULL;
    uint32_t                    hint_bits = 0;
    uint32_t                    fhct_idx = 0; 
    // uint32_t                    next_spe_fhct_idx = 0;
    hg_root_t                   hg_root;
    char                        *loc = NULL;
    FlowHintGroupList::iterator itr;

    HAL_TRACE_DEBUG("FlowSE::{}: P4-API: Table: FT Call ...", __FUNCTION__);

    // Form Anchor entry.
    // For each element in hg_list, 
    //      - Get hint_bits from hg 
    //      - Get first element in hg->flow_entry_list. 
    //        From it get fhct_index_. 
    // If next_:
    //      - Get fhct_index_ from next_
    //      - Set the bit indicating that there is next_

    memset(action_data, 0, sizeof(flow_hash_actiondata));
    if (anchor_entry_ || hg_list_.size()) {
        // Don't have to set the entry valid if this is deprogram
        flow_hash_info.entry_valid = 1;
    }
    if (anchor_entry_) {
        // +1 for entry_valid. export_en + flow_index is data
        memcpy((char *)&(flow_hash_info.entry_valid) + 1, anchor_entry_->get_data(), 
                anchor_entry_->get_data_len());
    }

    // After this loc will point to hash1 
    loc = (char*)&(flow_hash_info.hash1);

    for (itr = hg_list_.begin();itr != hg_list_.end(); itr++) {
        fhg = (*itr);
        hint_bits = fhg->get_hint_bits();
        root_fe = fhg->get_first_flow_entry();
        fhct_idx = root_fe->get_fhct_index();

        hg_root.hash = hint_bits;
        hg_root.hint = fhct_idx;
        memcpy(loc, &hg_root, sizeof(hg_root));
        loc += sizeof(hg_root);
    }
    if (next_) {
        flow_hash_info.more_hashs = 1;    
        flow_hash_info.more_hints = next_->get_fhct_index();
    }
}

#define oflow_hash_info action_data->flow_hash_overflow_action_u.flow_hash_overflow_flow_hash_info
#define oflow_hash_info1 action_data.flow_hash_overflow_action_u.flow_hash_overflow_flow_hash_info
// ---------------------------------------------------------------------------
// Form the HW entry for this spine entry
// ---------------------------------------------------------------------------
void 
FlowSpineEntry::form_oflow_action_data(flow_hash_overflow_actiondata *action_data)
{
    FlowHintGroup               *fhg = NULL;
    FlowEntry                   *root_fe = NULL;
    uint32_t                    hint_bits = 0;
    uint32_t                    fhct_idx = 0; 
    // uint32_t                    next_spe_fhct_idx = 0;
    hg_root_t                   hg_root;
    char                        *loc = NULL;
    FlowHintGroupList::iterator itr;

    HAL_TRACE_DEBUG("FlowSE::{}: P4-API: Table: FT Call ...", __FUNCTION__);

    // Form Anchor entry.
    // For each element in hg_list, 
    //      - Get hint_bits from hg 
    //      - Get first element in hg->flow_entry_list. 
    //        From it get fhct_index_. 
    // If next_:
    //      - Get fhct_index_ from next_
    //      - Set the bit indicating that there is next_

    memset(action_data, 0, sizeof(flow_hash_actiondata));
    if (anchor_entry_ || hg_list_.size()) {
        // Don't have to set the entry valid if this is deprogram
        oflow_hash_info.entry_valid = 1;
    }
    if (anchor_entry_) {
        // +1 for entry_valid. export_en + flow_index is data
        memcpy((char *)&(oflow_hash_info.entry_valid) + 1, anchor_entry_->get_data(), 
                anchor_entry_->get_data_len());
    }

    // After this loc will point to hash1 
    loc = (char*)&(oflow_hash_info.hash1);

    for (itr = hg_list_.begin();itr != hg_list_.end(); itr++) {
        fhg = (*itr);
        hint_bits = fhg->get_hint_bits();
        root_fe = fhg->get_first_flow_entry();
        fhct_idx = root_fe->get_fhct_index();

        hg_root.hash = hint_bits;
        hg_root.hint = fhct_idx;
        memcpy(loc, &hg_root, sizeof(hg_root));
        loc += sizeof(hg_root);
    }
    if (next_) {
        oflow_hash_info.more_hashs = 1;    
        oflow_hash_info.more_hints = next_->get_fhct_index();
    }
}
// ---------------------------------------------------------------------------
// Program the entry into HW
// ---------------------------------------------------------------------------
hal_ret_t
FlowSpineEntry::program_table()
{
    hal_ret_t                       rs = HAL_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    uint32_t                        table_id = 0;
    uint32_t                        oflow_table_id = 0;
    uint32_t                        hw_key_len = 0, sw_key_len = 0;
    void                            *hwkey = NULL, *sw_key = NULL;
    flow_hash_actiondata            action_data = {0};
    flow_hash_overflow_actiondata   oflow_act_data = {0};

    table_id = get_ft_entry()->get_flow()->get_table_id();
    oflow_table_id = get_ft_entry()->get_flow()->get_oflow_table_id();
    hw_key_len = get_ft_entry()->get_flow()->get_hwkey_len();
    sw_key_len = get_ft_entry()->get_flow()->get_key_len();

    sw_key = ::operator new(sw_key_len);
    memset(sw_key, 0, sw_key_len);

    HAL_TRACE_DEBUG("FSE::{}: Before forming action data", __FUNCTION__);
    if (is_in_ft_) {
        form_action_data(&action_data);
        // Form Hw key for anchor
        hwkey = ::operator new(hw_key_len);
        memset(hwkey, 0, hw_key_len);
        if (anchor_entry_) {
            p4pd_hwkey_hwmask_build(table_id, anchor_entry_->get_key(),
                                    NULL, (uint8_t *)hwkey, NULL);
        }

        uint32_t ft_index = get_ft_entry()->get_ft_bits();
        HAL_TRACE_DEBUG("FSE::{}: P4 FT Write: {}", __FUNCTION__, ft_index);

        // Entry trace
        if (anchor_entry_) {
            entry_trace(table_id, ft_index, (void *)anchor_entry_->get_key(), 
                        (void *)&action_data);
        } 

		// P4-API: Flow Table Write
        pd_err = p4pd_entry_write(table_id, ft_index, (uint8_t*)hwkey, NULL,
                                  (void *)&action_data);
        ::operator delete(hwkey);
	} else {
        form_oflow_action_data(&oflow_act_data);
        // Form Hw key for anchor
        hwkey = ::operator new(hw_key_len);
        memset(hwkey, 0, hw_key_len);
        if (anchor_entry_) {
            p4pd_hwkey_hwmask_build(oflow_table_id, anchor_entry_->get_key(),
                                    NULL, (uint8_t *)hwkey, NULL);
        }

        HAL_TRACE_DEBUG("FSE::{}: P4 FHCT Write: {}", 
                        __FUNCTION__, fhct_index_);

        // Entry trace
        if (anchor_entry_) {
            entry_trace(oflow_table_id, fhct_index_, (void *)anchor_entry_->get_key(), 
                        (void *)&oflow_act_data);
        } else {
            entry_trace(oflow_table_id, fhct_index_, (void *)sw_key, (void *)&oflow_act_data);
        }

		// P4-API: OFlow Table Write
        pd_err = p4pd_entry_write(oflow_table_id, fhct_index_, (uint8_t*)hwkey, NULL,
                                  (void *)&oflow_act_data);
        ::operator delete(hwkey);
	}

	return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
}


// ---------------------------------------------------------------------------
// De-Program the entry into HW
// ---------------------------------------------------------------------------
hal_ret_t
FlowSpineEntry::deprogram_table()
{
    hal_ret_t                       rs = HAL_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    void                            *hw_key = NULL;
    uint32_t                        hw_key_len = 0;
    uint32_t                        table_id = 0;
    uint32_t                        oflow_table_id = 0;
    flow_hash_actiondata            action_data;
    flow_hash_overflow_actiondata   oflow_act_data;
    
    hw_key_len = get_ft_entry()->get_flow()->get_hwkey_len();
    table_id = get_ft_entry()->get_flow()->get_table_id();
    oflow_table_id = get_ft_entry()->get_flow()->get_oflow_table_id();

	if (is_in_ft_) {
        hw_key = ::operator new(hw_key_len);
        memset(hw_key, 0, hw_key_len);
        memset(&action_data, 0, sizeof(action_data));
        uint32_t ft_index = get_ft_entry()->get_ft_bits();

        // Entry trace
        entry_trace(table_id, ft_index, NULL, NULL);

		// P4-API: Flow Table Write
        pd_err = p4pd_entry_write(table_id, ft_index, (uint8_t *)hw_key, NULL, 
                                  (void *)&action_data);
        ::operator delete(hw_key);
    } else {
        memset(&oflow_act_data, 0, sizeof(oflow_act_data));

        // Entry trace
        entry_trace(oflow_table_id, fhct_index_, NULL, NULL);

		// P4-API: Oflow Table Write
        pd_err = p4pd_entry_write(oflow_table_id, fhct_index_, NULL, NULL, 
                                  (void *)&oflow_act_data);
    }

	return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
}





// ---------------------------------------------------------------------------
// Get number of Hint Groups
// ---------------------------------------------------------------------------
uint32_t
FlowSpineEntry::get_num_hgs()
{
    return hg_list_.size();
}

// ---------------------------------------------------------------------------
// Adds Flow Hint Group to the list
// ---------------------------------------------------------------------------
void
FlowSpineEntry::add_fhg(FlowHintGroup *fhg)
{
    hg_list_.push_back(fhg);
}

// ---------------------------------------------------------------------------
// Del Flow Hint Group from the list
// ---------------------------------------------------------------------------
void
FlowSpineEntry::del_fhg(FlowHintGroup *fhg)
{
    hg_list_.remove(fhg);
}

// ---------------------------------------------------------------------------
// Get Last Flow Hint Group in the list
// ---------------------------------------------------------------------------
FlowHintGroup *
FlowSpineEntry::get_last_fhg()
{
	return hg_list_.back();
}

// ---------------------------------------------------------------------------
// Get Last Flow Entry
// ---------------------------------------------------------------------------
FlowEntry *
FlowSpineEntry::get_last_flow_entry()
{
	if (!get_num_hgs()) {
		return anchor_entry_;
	} else {
		FlowHintGroup *hg = get_last_fhg();
		return hg->get_last_flow_entry();
	}
}



// ---------------------------------------------------------------------------
// Replace the FHG with new FHG
// ---------------------------------------------------------------------------
void
FlowSpineEntry::replace_fhg(FlowHintGroup *fhg, FlowHintGroup *new_fhg)
{
	FlowHintGroupList::iterator itr;
	for (itr = hg_list_.begin(); itr != hg_list_.end(); itr++) {
		if ((*itr) == fhg) {
			(*itr) = new_fhg;
		}
	}
}

// ---------------------------------------------------------------------------
// Set anchor entry
// ---------------------------------------------------------------------------
void
FlowSpineEntry::set_anchor_entry(FlowEntry *fl_entry)
{
    anchor_entry_ = fl_entry;
}


// ---------------------------------------------------------------------------
// Set prev entry
// ---------------------------------------------------------------------------
void
FlowSpineEntry::set_prev(FlowSpineEntry *prev)
{
    prev_ = prev;
}

// ---------------------------------------------------------------------------
// Set next entry
// ---------------------------------------------------------------------------
void
FlowSpineEntry::set_next(FlowSpineEntry *next)
{
    next_ = next;
}

// ---------------------------------------------------------------------------
// Set is_in_ft
// ---------------------------------------------------------------------------
void 
FlowSpineEntry::set_is_in_ft(bool is_in_ft)
{
    is_in_ft_ = is_in_ft;
}

// ---------------------------------------------------------------------------
// Set Flow Hash Collision Table index
// ---------------------------------------------------------------------------
void
FlowSpineEntry::set_fhct_index(uint32_t idx)
{
    fhct_index_ = idx;
}

// ---------------------------------------------------------------------------
// Set Flow Table Entry
// ---------------------------------------------------------------------------
void
FlowSpineEntry::set_ft_entry(FlowTableEntry *ft_entry)
{
    ft_entry_ = ft_entry;
}

hal_ret_t
FlowSpineEntry::entry_trace(uint32_t table_id, uint32_t index, 
                             void *key, void *data)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(table_id, key, NULL, 
                                              data, buff, sizeof(buff));
    HAL_ASSERT(p4_err == P4PD_SUCCESS);

    if (!key && !data) {
        HAL_TRACE_DEBUG("Clearing entry at Index: {}", index);
    } else {
        HAL_TRACE_DEBUG("Index: {} \n {}", index, buff);
    }

    return HAL_RET_OK;
}

void
FlowSpineEntry::print_fse()
{
    FlowHintGroup               *fhg = NULL;
    FlowHintGroupList::iterator itr;
    HAL_TRACE_DEBUG("FSE:: is_in_ft:{}, fhct_index:{}, anc_entry_present:{}, "
            "prev: {}, next: {}",
            is_in_ft_, fhct_index_, 
            ((anchor_entry_ != NULL) ? true : false),
            prev_ ? prev_->get_fhct_index() : -1,
            next_ ? next_->get_fhct_index() : -1); 

    for (itr = hg_list_.begin();itr != hg_list_.end(); itr++) {
        fhg = (*itr);
        fhg->print_fhg();
    }
}
