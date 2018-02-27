#include <cstring>
// #include <ctime>
// #include <cstdlib>

#include "nic/hal/pd/utils/flow/flow.hpp"
#include "nic/hal/pd/utils/flow/flow_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_table_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_spine_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_hint_group.hpp"
#include "nic/include/trace.hpp"

using hal::pd::utils::FlowEntry;
using hal::pd::utils::FlowTableEntry;
using hal::pd::utils::FlowSpineEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
FlowEntry *
FlowEntry::factory(void *key, uint32_t key_len, void *data, uint32_t data_len,
                   uint32_t hwkey_len, bool log, uint32_t mtrack_id)
{
    void        *mem = NULL;
    FlowEntry   *fe = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(FlowEntry));
    if (!mem) {
        return NULL;
    }

    fe = new (mem) FlowEntry(key, key_len, data, data_len, hwkey_len, log);
    return fe;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
FlowEntry::destroy(FlowEntry *re, uint32_t mtrack_id) 
{
    if (re) {
        re->~FlowEntry();
        HAL_FREE(mtrack_id, re);
    }
}

// ---------------------------------------------------------------------------
// Constructor - Flow Entry
// ---------------------------------------------------------------------------
FlowEntry::FlowEntry(void *key, uint32_t key_len, 
                     void *data, uint32_t data_len,
                     uint32_t hwkey_len, bool log)
{
    key_len_ = key_len;
    data_len_ = data_len;
    hwkey_len_ = hwkey_len;

    // key_    = ::operator new(key_len);
    // data_   = ::operator new(data_len);
    key_ = HAL_MALLOC(HAL_MEM_ALLOC_FLOW_ENTRY_KEY, key_len);
    data_ = HAL_MALLOC(HAL_MEM_ALLOC_FLOW_ENTRY_DATA, data_len);

    std::memcpy(key_, key, key_len);
    std::memcpy(data_, data, data_len);

    if (log) {
        //HAL_TRACE_DEBUG("FlowE:: key_len: {}, data_len: {}", key_len, data_len);
        //HAL_TRACE_DEBUG("FlowE:: key: {} data: {}", key_, data_);
    }

    hash_val_ = 0;
    fh_group_ = NULL;
    gl_index_ = 0;
    is_anchor_entry_ = FALSE;
    spine_entry_ = NULL;
    fhct_index_ = 0;
}

// ---------------------------------------------------------------------------
// Destructor - Flow Entry
// ---------------------------------------------------------------------------
FlowEntry::~FlowEntry()
{
    // ::operator delete(key_);
    // ::operator delete(data_);
    HAL_FREE(HAL_MEM_ALLOC_FLOW_ENTRY_KEY, key_);
    HAL_FREE(HAL_MEM_ALLOC_FLOW_ENTRY_DATA, data_);
}

// ---------------------------------------------------------------------------
// Inserts Flow Entry in HW
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::insert(FlowHintGroup *fhg, FlowSpineEntry *fse)
{
    hal_ret_t rs = HAL_RET_OK;

    HAL_TRACE_DEBUG("FlowE::{}: Insert ...", __FUNCTION__);

    if (fse->get_is_in_ft() && !fse->get_anchor_entry()) {
        // Case 1: First Entry in Flow Table Entry. Anchor Entry
        HAL_TRACE_DEBUG("FlowE::{}: Insert:Anchor ...", __FUNCTION__);
        // Install FSE in FT.
    
        // Set fields in FlowEntry
        is_anchor_entry_ = TRUE;
        spine_entry_ = fse;
        fh_group_ = fhg;
        fhct_index_ = 0; // Will be filled with valid value for non-anchor
        // Set Fields in HintGroup
        fhg->add_anchor_flow_entry(this);
        // Set Fields in FlowSpineEntry
        fse->set_anchor_entry(this);

        // Program HW: FT 
        fse->program_table();

    } else if (!fhg->get_fs_entry()) {
        // Case 2: Hint group doesnt have a spine. 
        //       - HG has only anchors.
        //       - HG doesnt have anchor and this is first entry.
        // Install FEntry in FHCT
        // Install FSE in FT pointing to the above entry in FHCT
        HAL_TRACE_DEBUG("FlowE::{}: Insert:HG SE Attach Entry...", __FUNCTION__);

        // Allocate an entry in Flow Hash Coll. Table.
        rs = alloc_fhct_index(fse, &fhct_index_);
        if (rs != HAL_RET_OK) {
            HAL_TRACE_DEBUG("FlowE::{}: Failed to alloc fhct idx", __FUNCTION__);
            return rs;
        } 

        // once the index is alloced successfully ... it should not ideally fail...
        // so setting up connections.

        // Set fields in FlowEntry
        is_anchor_entry_ = FALSE;
        fh_group_ = fhg;
        // Set Fields in HintGoup
        fhg->add_flow_entry(this);
        fhg->set_fs_entry(fse);
        // Set Fields in FlowSpineEntry
        fse->add_fhg(fhg);

        // Write the flow entry in FHCT
        // P4-PI: fhct_index_ => [hw_key_, data_,] 
        rs = program_table_non_anchor_entry(NULL);
        HAL_TRACE_DEBUG("FlowE::{}: {} Done Programming non-anchor entry", 
                __FUNCTION__, rs);
        if (rs != HAL_RET_OK) {
            free_fhct_index(fse, fhct_index_);
            goto end;
        }

        HAL_TRACE_DEBUG("FlowE::{}: Program Spine Entry ", __FUNCTION__);
        // Program FT / FHCT Spine Entry
        fse->program_table();

        // Have to check if this is the first entry in spine then reprogram prev as well
        // Prevent for first spine entry
        if (fse->get_num_hgs() == 1 && fse->get_prev()) {
            HAL_TRACE_DEBUG("FlowE::{}: FSE being programmed for first time...");
            // Link prev to this
            fse->get_prev()->set_next(fse);
            // Reprogram prev FSE
            fse->get_prev()->program_table();
        }

    } else {
        // Case 3: FHG has spine. 
        //      - FEntry will be installed at the end of HG.
        // Install FEntry in FHCT.
        // Re-Install last entry in the FHG to point to the above entry.
        HAL_TRACE_DEBUG("FlowE::{}: Insert:HG Chain Entry ...", __FUNCTION__);

        // Allocate an entry in Flow Hash Coll. Table.
        rs = alloc_fhct_index(fse, &fhct_index_);
        if (rs != HAL_RET_OK) {
            return rs;
        } 

        // storing the last entry which has to be re-programmed
        FlowEntry * fhg_last_entry = fhg->get_last_flow_entry();

        // Set fields in FlowEntry
        is_anchor_entry_ = FALSE;
        fh_group_ = fhg;
        // Set Fields in HintGroup
        fhg->add_flow_entry(this);

        // Write the flow entry in FHCT
        // P4-PI: FHCT Write fhct_index_ => [hw_key_, data_] 
        rs = program_table_non_anchor_entry(NULL);
        if (rs != HAL_RET_OK) {
            free_fhct_index(fse, fhct_index_);
            goto end;
        }

        // fhct_index_ = fhg_last_entry->get_fhct_index();
        // P4-PI: FHCT Write
        //      fhg_last_entry->hw_key_, fhg_last_entry->data, 
        //        fhg->get_hint_bits(), this->fhct_index_
        rs = fhg_last_entry->program_table_non_anchor_entry(this);
        if (rs != HAL_RET_OK) {
            free_fhct_index(fse, fhct_index_);
            goto end;
        }
    }

end:
    return rs;
}

// ---------------------------------------------------------------------------
// Update Flow Entry in HW
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::update(void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    HAL_TRACE_DEBUG("FlowE::{}: Update...", __FUNCTION__);

    // Updates Data in the entry
    std::memcpy(data_, data, data_len_);

    if (is_anchor_entry_) { // Case 1: FT Entry Re-Install

        HAL_TRACE_DEBUG("FlowE::{}: Update anchor", __FUNCTION__);
        
        // Program FT/FHCT Spine entry.
        spine_entry_->program_table();

    } else { // Case 2: FHCT Single Flow Entry Rewrite

        HAL_TRACE_DEBUG("FlowE::{}: Update FHCT: {}", 
                __FUNCTION__, fhct_index_);
        // P4-PI: FHCT Table Write
        FlowEntry *next_fe = fh_group_->get_next_flow_entry(this);
        program_table_non_anchor_entry(next_fe);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Remove this entry.
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::remove()
{
    hal_ret_t rs = HAL_RET_OK;
    FlowEntry *last_flow_entry = NULL, *new_last_flow_entry = NULL;
    FlowSpineEntry *fse_last = NULL, *eff_spine_entry = get_eff_spine_entry();
    FlowSpineEntry *fse_prev = NULL;
    FlowHintGroup *last_fhg = NULL;
    // FlowTableEntry *fte = eff_spine_entry->get_ft_entry();
    std::list<FlowHintGroup*>::iterator itr;

    // Step 1: Replace the entry & reprogram
    if (is_anchor_entry_) {
        // Case 1: Anchor entry.
        HAL_TRACE_DEBUG("FlowE:{} Removing anchor entry", __FUNCTION__);

        // Get the entry which will be moved
        last_flow_entry = eff_spine_entry->get_ft_entry()->get_last_flow_entry();

        if (this == last_flow_entry) {
            // No need to move.
            // This is the last entry in the spine entry.
            HAL_TRACE_DEBUG("FlowE:{} Removing the only existing entry", __FUNCTION__);
            
            // Program previous spine entry as this spine entry is going away
            // May not happen as anchor is only in FT's spine entry
            fse_prev = eff_spine_entry->get_prev();
            if (fse_prev) {
                fse_prev->set_next(NULL);
                fse_prev->program_table();
            }

            // Reset anchor entry in spine entry
            eff_spine_entry->set_anchor_entry(NULL);
            // Remove from FHG's anchor flow entry list
            fh_group_->del_anchor_flow_entry(this);
            // Program spine entry with all 0s.
            eff_spine_entry->program_table();

#if 0
            // Anchor entry is present only in FT.
            // Not FT then its FHCT spine entry.
            if (!spine_entry_->get_is_in_ft()) {
                // Set Prev spine entry to point to NULL
                FlowSpineEntry *fse_prev = spine_entry_->get_prev();
                fse_prev->set_next(NULL);

                // Program FHCT/FT Spine Entry
                fse_prev->program();
            }
#endif
        } else {

            if (!eff_spine_entry->get_ft_entry()->get_flow()->
                    get_delayed_del_en()) {
                // Remove from FHG's flow entry list and put in anchor list.
                // Put last flow entry as this->eff_spine anchor.
                // If its last entry in FHG chain:
                //      - Remove FHG From last_entry->eff_spine list
                //      - Reprogram last_entry->eff_spine.
                // else:
                //      - Get Prev flow entry in FHG chain.
                //      - Program the prev_flow_entry 

                // Reset anchor entry in spine entry
                eff_spine_entry->set_anchor_entry(last_flow_entry);
                // Remove from FHG's anchor flow entry list
                fh_group_->del_anchor_flow_entry(this);
                // Program spine entry.
                eff_spine_entry->program_table();

                // Removing Last Flow Entry 
                FlowHintGroup *last_fhg = last_flow_entry->get_fh_group();
                FlowSpineEntry *last_fse = last_fhg->get_fs_entry();
                HAL_TRACE_DEBUG("last_fhg:");
                last_fhg->print_fhg();
                // Add last_flow_entry to the anchor list of FHG
                last_fhg->add_anchor_flow_entry(last_flow_entry);
                // Rem last_flow_entry from flow_entry list of last FHG
                last_fhg->del_flow_entry(last_flow_entry);

                if (last_fhg->get_num_flow_entries()) {
                    // ----------
                    HAL_TRACE_DEBUG("FlowE:{} Move: Hint Group Chain prev entry "
                            "reprogram & deprogram ", __FUNCTION__);
                    // Re-program prev entry in the chain.
                    last_fhg->get_last_flow_entry()->
                        program_table_non_anchor_entry(NULL);

                    // Un-program last_flow_entry
                    last_flow_entry->deprogram_table_non_anchor_entry();
                    // Free up FHCT index of last flow
                    free_fhct_index(last_fse, last_flow_entry->get_fhct_index());
                } else {
                    // last_fhg will always have anchor entry. So it wont go away.
                    // Since there are no flow entries, it will be removed from
                    // fse.
                    last_fse->del_fhg(last_fhg);

                    // Remove FHG from FSE and FTE
                    // fte->remove_fhg(last_fhg);
                    if (last_fse->get_num_hgs() || last_fse->get_anchor_entry()) {
                        HAL_TRACE_DEBUG("FlowE:{} Move: HG Remove: "
                                "Reprogram last spine", __FUNCTION__);
                        // Re-program last_fse
                        last_fse->program_table();
                        // Un-program last_flow_entry
                        last_flow_entry->deprogram_table_non_anchor_entry();
                        // Free up FHCT index of last flow
                        free_fhct_index(last_fse, last_flow_entry->get_fhct_index());
                    } else {
                        HAL_TRACE_DEBUG("FlowE:{} Move: HG Remove, Spine Remove: "
                                "Reprogram last spine, Deprogram prev spine", __FUNCTION__);
                        // Reprogram prev. last fse entry as last fse is going away.
                        FlowSpineEntry *last_fse_prev = last_fse->get_prev();
                        last_fse_prev->set_next(NULL);
                        last_fse_prev->program_table();

                        // Re-program last_fse with all 0s
                        last_fse->program_table();
                        // Free FHCT index of last_fse
                        free_fhct_index(last_fse, last_fse->get_fhct_index());
                        // delete last_fse;
                        FlowSpineEntry::destroy(last_fse);
                        eff_spine_entry->get_ft_entry()->dec_num_spine_entries();
                    }
                }

                // Update last flow entry as anchor entry
                last_flow_entry->set_is_anchor_entry(true);
                last_flow_entry->set_spine_entry(eff_spine_entry);
                last_flow_entry->set_fhct_index(0);

            } else {

                // Create a copy of new last_flow_entry with location as "this".
                new_last_flow_entry = create_new_flow_entry(last_flow_entry);

                // Replace "this" with new_last_flow_entry as spine_entry's anchor.
                eff_spine_entry->set_anchor_entry(new_last_flow_entry);

                // Remove from FHG's anchor flow entry list
                fh_group_->del_anchor_flow_entry(this);

                // Re-Install spine_entry
                eff_spine_entry->program_table();

                // Add new_last_flow_entry to the anchor list of FHG
                last_flow_entry->get_fh_group()->
                    add_anchor_flow_entry(new_last_flow_entry);

                // Replace last_flow_entry with new_last_flow_entry in global map
                eff_spine_entry->get_ft_entry()->get_flow()->
                    add_flow_entry_global_map(new_last_flow_entry, 
                            last_flow_entry->get_global_index());


                // Put last_flow_entry in delayed_delete list of Flow.
                eff_spine_entry->get_ft_entry()->get_flow()->
                    push_fe_delete_q(last_flow_entry);
            }
        }
    } else if (fh_group_->get_first_flow_entry() == this) {
        // Case 2: Flow entry is attached to spine entry.
        HAL_TRACE_DEBUG("FlowE:{} Removing flow attached to spine entry", 
                __FUNCTION__);

        if (fh_group_->get_num_flow_entries() > 1) {
            // ----------
            // No need to move
            // At fhct_index_ program all 0s
            //    TODO-P4-PI: fhct_index_ => [hw_key_, data_,] 
            HAL_TRACE_DEBUG("FlowE:{} HS has extra entries..."
                    "just reprogram spine entry", 
                    __FUNCTION__);

            // Remove flow entry from FHG flow_entry_list_
            fh_group_->del_flow_entry(this);

            // Re-program FSE
            eff_spine_entry->program_table();

        } else {
            // FHG rooted on spine entry is getting detached.
            HAL_TRACE_DEBUG("FlowE:{} FHG rooted on spine entry "
                    "is getting detached",
                    __FUNCTION__);

            // Remove flow entry from FHG flow_entry_list_
            fh_group_->del_flow_entry(this);

            fse_last = fh_group_->get_fs_entry()->
                get_ft_entry()->get_last_spine_entry();
            // Get Last FHG
            last_fhg = fse_last->get_last_fhg();
            if (last_fhg == fh_group_) {
                HAL_TRACE_DEBUG("FlowE:{} Current is last FHG: Last FHG is removed."
                        "Reprogram last spine entry",
                        __FUNCTION__);
                // No need to move
                // Remove fhg from fse
                eff_spine_entry->del_fhg(fh_group_);
                // Re-program FSE
                eff_spine_entry->program_table();
                if (!eff_spine_entry->get_anchor_entry() && 
                        !eff_spine_entry->get_num_hgs()) {
                HAL_TRACE_DEBUG("FlowE:{} Current is last FHG: Last Spine entry is removed."
                        "Reprogram last prev. spine entry",
                        __FUNCTION__);
                    fse_prev = eff_spine_entry->get_prev();
                    if (fse_prev) {
                        fse_prev->set_next(NULL);
                        fse_prev->program_table();
                    }
                }

            } else {
                // Need to move last_fhg to here
                if (!eff_spine_entry->get_ft_entry()->get_flow()->
                        get_delayed_del_en()) {

#if 0
                    if (fse_last == eff_spine_entry) {
                        // Remove last fhg 
                        fse_last->del_fhg(last_fhg);
                    }
#endif
                    // Remove last_fhg from fse_last
                    fse_last->del_fhg(last_fhg);

                    // Change parent of fhg to current spine entry
                    last_fhg->set_fs_entry(eff_spine_entry);
                    // Put last_fhg in the current FSE
                    eff_spine_entry->replace_fhg(fh_group_, last_fhg);
                    // Re-program FSE
                    eff_spine_entry->program_table();

                    // Check if we have to program fse_last
                    if (fse_last != eff_spine_entry) {
                        // ----------
                        // Handle last_fhg move
                        if (fse_last->get_num_hgs() || fse_last->get_anchor_entry()) {
                            // Remove last_fhg from fse_last
                            fse_last->del_fhg(last_fhg);
                            // Re-program fse_last
                            fse_last->program_table();
                        } else {
                            // Reprogram prev. last fse entry as last fse is going away.
                            FlowSpineEntry *fse_last_prev = fse_last->get_prev();
                            fse_last_prev->set_next(NULL);
                            fse_last_prev->program_table();

                            // Re-program fse_last with all 0s
                            fse_last->program_table();
                            // Free FHCT index of fse_last
                            free_fhct_index(fse_last, fse_last->get_fhct_index());
                            // delete fse_last;
                            FlowSpineEntry::destroy(fse_last);
                            eff_spine_entry->get_ft_entry()->dec_num_spine_entries();
                        }
                    }

                } else {

                    // Changing the spine entry of last_fhg.
                    last_fhg->set_delayed_del_fs_entry(last_fhg->get_fs_entry());
                    last_fhg->set_fs_entry(eff_spine_entry);
                    // Put last_fhg in the current FSE
                    eff_spine_entry->replace_fhg(fh_group_, last_fhg);

                    // Re-program FSE
                    eff_spine_entry->program_table();

                    // Add last_fhg to delayed delete flow entry.
                    eff_spine_entry->get_ft_entry()->get_flow()->
                        push_fhg_delete_q(last_fhg);
                }

            }
        }
        // Re-program FHCT with 0s
        deprogram_table_non_anchor_entry();
        // Free up fhct_index_ from Indexer
        free_fhct_index(eff_spine_entry, fhct_index_);
    } else {
        // ----------
        // Case 3: Flow entry is in FHG list, and its not attached to Spine.
        
        // Re-program prev. FHCT entry to point to next FHCT entry.
        FlowEntry *prev_fe = fh_group_->get_prev_flow_entry(this);
        // FlowEntry *next_fe = fh_group_->get_next_flow_entry(this);
        // Remove this from fhg
        fh_group_->del_flow_entry(this);
        // Program the Prev. Flow Entry in FHG list
        prev_fe->deprogram_table_non_anchor_entry();
        // Re-program FHCT with 0s
        deprogram_table_non_anchor_entry();
        // Free up the FHCT Indexer
        free_fhct_index(eff_spine_entry, fhct_index_);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Allocate FHCT index
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::alloc_fhct_index(FlowSpineEntry *fse, uint32_t *fhct_index)
{
    hal_ret_t rs = HAL_RET_OK;
    rs = fse->get_ft_entry()->get_flow()->alloc_fhct_index(fhct_index);
    return rs;
}

// ---------------------------------------------------------------------------
// Free FHCT index
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::free_fhct_index(FlowSpineEntry *fse, uint32_t fhct_index)
{
    hal_ret_t rs = HAL_RET_OK;
    rs = fse->get_ft_entry()->get_flow()->free_fhct_index(fhct_index);
    return rs;
}

// ---------------------------------------------------------------------------
// Create a new flow entry with all fields other than location. Loc. of this.
// ---------------------------------------------------------------------------
FlowEntry *
FlowEntry::create_new_flow_entry(FlowEntry *fe)
{
    // FlowEntry *new_fe = new FlowEntry(fe->get_key(), fe->get_key_len(),
    //                                   fe->get_data(), fe->get_data_len(), 
    //                                   hwkey_len_, true);
    FlowEntry *new_fe = FlowEntry::factory(fe->get_key(), fe->get_key_len(),
                                           fe->get_data(), fe->get_data_len(),
                                           hwkey_len_, true);
    *new_fe = *fe;

    // Take location information from "this"
    new_fe->set_is_anchor_entry(is_anchor_entry_);
    new_fe->set_spine_entry(spine_entry_);
    new_fe->set_fhct_index(fhct_index_);

    return new_fe;
}

// ---------------------------------------------------------------------------
// Get Flow Table Entry
// ---------------------------------------------------------------------------
FlowTableEntry *
FlowEntry::get_flow_table_entry()
{
    return get_eff_spine_entry()->get_ft_entry();
}

// ---------------------------------------------------------------------------
// Get Effective Spine Entry
// ---------------------------------------------------------------------------
FlowSpineEntry *
FlowEntry::get_eff_spine_entry()
{
    if (is_anchor_entry_) {
        return spine_entry_;
    } else {
        return fh_group_->get_fs_entry();
    }
}

// ---------------------------------------------------------------------------
// Program non-anchor entry
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::program_table_non_anchor_entry(FlowEntry *next_fe)
{
    hal_ret_t       rs = HAL_RET_OK;
    p4pd_error_t    pd_err = P4PD_SUCCESS;
    uint32_t        oflow_table_id = 0;
    uint32_t        entire_data_len;
    void            *hwkey = NULL;
    void            *swdata = NULL;
    uint8_t         *action_id;
    uint8_t         *entry_valid;
    void            *data;
    void            *first_hash_hint;
    uint8_t         *more_hashs;
    void            *more_hints;
    uint32_t        hint_bits = 0;
    uint32_t        fhct_idx = 0; 
    uint32_t        hint_mem_len_B = 0;

    oflow_table_id = get_flow_table_entry()->get_flow()->get_oflow_table_id();
    entire_data_len = get_flow_table_entry()->get_flow()->
                      get_flow_entire_data_len();
    hint_mem_len_B = get_flow_table_entry()->get_flow()->get_hint_mem_len_B();

    swdata = HAL_CALLOC(HAL_MEM_ALLOC_ENTIRE_FLOW_ENTRY_DATA, 
                        entire_data_len);
    get_flow_table_entry()->get_flow()->
        flow_action_data_offsets(swdata,
                                 &action_id,
                                 &entry_valid,
                                 &data,
                                 &first_hash_hint,
                                 &more_hashs,
                                 &more_hints);

    *action_id = 0;
    *entry_valid = 1;
    memcpy(data, data_, data_len_); // export_en + flow_index is data

    if (next_fe) {
        // first_hash_hint->hash = next_fe->get_fh_group()->get_hint_bits();
        // first_hash_hint->hint = next_fe->get_fhct_index();
        char *loc = (char *)first_hash_hint;
        hint_bits = next_fe->get_fh_group()->get_hint_bits();
        fhct_idx = next_fe->get_fhct_index();
        memcpy(loc, &hint_bits, 2);
        memcpy(loc + 2, &fhct_idx, hint_mem_len_B);
    }

    HAL_TRACE_DEBUG("FE::{}: OflowTID: {} P4 FHCT Write: {}", 
                    __FUNCTION__, oflow_table_id, fhct_index_);
    
    hwkey = HAL_CALLOC(HAL_MEM_ALLOC_FLOW_ENTRY_HW_KEY, hwkey_len_);
    p4pd_hwkey_hwmask_build(oflow_table_id, key_,
                            NULL, (uint8_t *)hwkey, NULL);

    // P4-API: Oflow Table Write
    pd_err = p4pd_entry_write(oflow_table_id, fhct_index_, (uint8_t*)hwkey, NULL,
                              swdata);
    HAL_TRACE_DEBUG("Done programming Flow Hash Overflow {}", pd_err);

    // Free
    HAL_FREE(HAL_MEM_ALLOC_FLOW_ENTRY_HW_KEY, hwkey);
    HAL_FREE(HAL_MEM_ALLOC_ENTIRE_FLOW_ENTRY_DATA, swdata);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
}


// ---------------------------------------------------------------------------
// DeProgram Non-anchor entry
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::deprogram_table_non_anchor_entry()
{
    hal_ret_t                       rs = HAL_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    uint32_t                        oflow_table_id = 0;
    uint32_t                        entire_data_len = 0;
    void                            *swdata = NULL;

    HAL_TRACE_DEBUG("{}: Deprogram Coll. Table idx: {}", __FUNCTION__, 
            fhct_index_);

    entire_data_len = get_flow_table_entry()->get_flow()->
                      get_flow_entire_data_len();
    swdata = HAL_CALLOC(HAL_MEM_ALLOC_ENTIRE_FLOW_ENTRY_DATA, 
                        entire_data_len);

    oflow_table_id = get_flow_table_entry()->get_flow()->get_oflow_table_id();


    // P4-API: Oflow Table Write
    pd_err = p4pd_entry_write(oflow_table_id, fhct_index_, NULL, NULL,
                              swdata);

    entry_trace(oflow_table_id, fhct_index_, swdata);

    HAL_FREE(HAL_MEM_ALLOC_ENTIRE_FLOW_ENTRY_DATA, swdata);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
}



// ---------------------------------------------------------------------------
// Sets Key Pointer
// ---------------------------------------------------------------------------
void
FlowEntry::set_key(void *key)
{
    key_ = key;
}

// ---------------------------------------------------------------------------
// Sets Key Len
// ---------------------------------------------------------------------------
void
FlowEntry::set_key_len(uint32_t key_len)
{
    key_len_ = key_len;
}


// ---------------------------------------------------------------------------
// Sets Data Pointer
// ---------------------------------------------------------------------------
void
FlowEntry::set_data(void *data)
{
    data_ = data;;
}

// ---------------------------------------------------------------------------
// Sets Data Len
// ---------------------------------------------------------------------------
void
FlowEntry::set_data_len(uint32_t data_len)
{
    data_len_ = data_len;
}

// ---------------------------------------------------------------------------
// Sets Hash Value
// ---------------------------------------------------------------------------
void
FlowEntry::set_hash_val(uint32_t hash_val)
{
    hash_val_ = hash_val;
}

// ---------------------------------------------------------------------------
// Set Flow Hash Collision Table Index
// ---------------------------------------------------------------------------
void
FlowEntry::set_fhct_index(uint32_t fhct_index)
{
    fhct_index_ = fhct_index;
}

// ---------------------------------------------------------------------------
// Sets Flow Hint Group
// ---------------------------------------------------------------------------
void
FlowEntry::set_fh_group(FlowHintGroup *fh_group)
{
    fh_group_ = fh_group;
}

// ---------------------------------------------------------------------------
// Sets Global Index
// ---------------------------------------------------------------------------
void
FlowEntry::set_global_index(uint32_t index)
{
    gl_index_ = index;
}


// ---------------------------------------------------------------------------
// Sets is_anchor_entry
// ---------------------------------------------------------------------------
void
FlowEntry::set_is_anchor_entry(bool is_anchor)
{
    is_anchor_entry_ = is_anchor;
}

// ---------------------------------------------------------------------------
// Set Spine entry
// ---------------------------------------------------------------------------
void
FlowEntry::set_spine_entry(FlowSpineEntry *spe)
{
    spine_entry_ = spe;
}

// ---------------------------------------------------------------------------
// Forms hw key
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::form_hw_key(uint32_t table_id, void *hwkey)
{
    hal_ret_t                       rs = HAL_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;

    pd_err = p4pd_hwkey_hwmask_build(table_id, key_, NULL, 
                                     (uint8_t *)hwkey, NULL);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
}

// ---------------------------------------------------------------------------
// P4Pd trace of the entry
// ---------------------------------------------------------------------------
hal_ret_t
FlowEntry::entry_trace(uint32_t table_id, uint32_t index, 
                             void *data)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(table_id, index, key_, NULL, 
                                              data, buff, sizeof(buff));
    HAL_ASSERT(p4_err == P4PD_SUCCESS);

    HAL_TRACE_DEBUG("Index: {} \n {}", index, buff);

    return HAL_RET_OK;
}

// ---------------------------------------------------------------------------
// Prints Flow Entry
// ---------------------------------------------------------------------------
void
FlowEntry::print_fe() 
{
    HAL_TRACE_DEBUG("      flow_entry: fe_idx: {}, is_anchor: {}, "
            "fhct_index: {}, fhg_bits: {:#x}", 
            gl_index_, is_anchor_entry_, 
            fhct_index_, fh_group_->get_hint_bits());
}
