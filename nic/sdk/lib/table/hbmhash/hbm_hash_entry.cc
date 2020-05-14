//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <cstring>
#include "include/sdk/base.hpp"
#include "hbm_hash.hpp"
#include "hbm_hash_entry.hpp"
#include "hbm_hash_table_entry.hpp"
#include "hbm_hash_spine_entry.hpp"
#include "hbm_hash_hint_group.hpp"
#include "hbm_hash_mem_types.hpp"

using sdk::table::HbmHash;
using sdk::table::HbmHashEntry;
using sdk::table::HbmHashTableEntry;
using sdk::table::HbmHashSpineEntry;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
HbmHashEntry *
HbmHashEntry::factory(void *key, uint32_t key_len, void *data, uint32_t data_len,
                      uint32_t hwkey_len, bool log, HbmHash *hbm_hash)
{
    void        *mem = NULL;
    HbmHashEntry   *fe = NULL;

    mem = hbm_hash->hbm_hash_entry_alloc();
    // mem = SDK_CALLOC(mtrack_id, sizeof(HbmHashEntry));
    if (!mem) {
        return NULL;
    }

    fe = new (mem) HbmHashEntry(key, key_len, data, data_len,
                                hwkey_len, log, hbm_hash);
    return fe;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
HbmHashEntry::destroy(HbmHashEntry *re, HbmHash *hbm_hash)
{
    if (re) {
        re->~HbmHashEntry();
        hbm_hash->hbm_sw_key_free(re->get_key());
        hbm_hash->hbm_sw_data_free(re->get_data());
        hbm_hash->hbm_hw_key_free(re->get_hwkey());
        // SDK_FREE(mtrack_id, re);
        hbm_hash->hbm_hash_entry_free(re);
    }
}

// ---------------------------------------------------------------------------
// Constructor - HBM Hash Entry
// ---------------------------------------------------------------------------
HbmHashEntry::HbmHashEntry(void *key, uint32_t key_len,
                     void *data, uint32_t data_len,
                     uint32_t hwkey_len, bool log,
                     HbmHash *hbm_hash)
{
    key_len_ = key_len;
    data_len_ = data_len;
    hwkey_len_ = hwkey_len;

    key_ = hbm_hash->hbm_sw_key_alloc();
    data_ = hbm_hash->hbm_sw_data_alloc();
    hwkey_ = hbm_hash->hbm_hw_key_alloc();

    // key_ = SDK_MALLOC(SDK_MEM_ALLOC_HBM_HASH_ENTRY_KEY, key_len);
    // data_ = SDK_MALLOC(SDK_MEM_ALLOC_HBM_HASH_ENTRY_DATA, data_len);
    // hwkey_ = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_HW_KEY, hwkey_len_);

    std::memcpy(key_, key, key_len);
    std::memcpy(data_, data, data_len);

    if (log) {
        //SDK_TRACE_DEBUG("HbmHashE:: key_len: %d, data_len: %d", key_len, data_len);
        //SDK_TRACE_DEBUG("HbmHashE:: key: %p data: %p", key_, data_);
    }

    hash_val_ = 0;
    hint_group_ = NULL;
    gl_index_ = 0;
    is_anchor_entry_ = FALSE;
    spine_entry_ = NULL;
    hct_index_ = 0;
    num_recircs_ = 0;
}

// ---------------------------------------------------------------------------
// Destructor - HBM Hash Entry
// ---------------------------------------------------------------------------
HbmHashEntry::~HbmHashEntry()
{
#if 0
    get_bucket()->get_hbm_hash()->hbm_sw_key_free(key_);
    get_bucket()->get_hbm_hash()->hbm_sw_data_free(data_);
    get_bucket()->get_hbm_hash()->hbm_hw_key_free(hwkey_);
#endif

    // SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_ENTRY_KEY, key_);
    // SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_ENTRY_DATA, data_);
    // SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_HW_KEY, hwkey_);
}

// ---------------------------------------------------------------------------
// Inserts HBM Hash Entry in HW
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::insert(HbmHashHintGroup *hg, HbmHashSpineEntry *fse)
{
    sdk_ret_t rs = SDK_RET_OK;

    SDK_TRACE_DEBUG("Insert ...");

    if (fse->get_is_in_ft() && !fse->get_anchor_entry()) {
        // Case 1: First Entry in HBM Hash Table Entry. Anchor Entry
        SDK_TRACE_DEBUG("Insert:Anchor ...");
        // Install FSE in FT.

        // Set fields in HbmHashEntry
        is_anchor_entry_ = TRUE;
        spine_entry_ = fse;
        hint_group_ = hg;
        hct_index_ = 0; // Will be filled with valid value for non-anchor
        // Set Fields in HintGroup
        hg->add_anchor_hbm_hash_entry(this);
        // Set Fields in HbmHashSpineEntry
        fse->set_anchor_entry(this);

        // Program HW: FT
        fse->program_table();

    } else if (!hg->get_fs_entry()) {
        // Case 2: Hint group doesnt have a spine.
        //       - HG has only anchors.
        //       - HG doesnt have anchor and this is first entry.
        // Install FEntry in FHCT
        // Install FSE in FT pointing to the above entry in FHCT
        SDK_TRACE_DEBUG("Insert:HG SE Attach Entry...");

        // Allocate an entry in HBM Hash Coll. Table.
        rs = alloc_collision_index(fse, &hct_index_);
        if (rs != SDK_RET_OK) {
            SDK_TRACE_DEBUG("Failed to alloc fhct idx");
            return rs;
        }

        // once the index is alloced successfully ... it should not ideally fail...
        // so setting up connections.

        // Set fields in HbmHashEntry
        is_anchor_entry_ = FALSE;
        hint_group_ = hg;
        // Set Fields in HintGoup
        hg->add_hbm_hash_entry(this);
        hg->set_fs_entry(fse);
        // Set Fields in HbmHashSpineEntry
        fse->add_hg(hg);

        // Write the HBM Hash Entry in FHCT
        // P4-PI: hct_index_ => [hw_key_, data_,]
        rs = program_table_non_anchor_entry(NULL);
        SDK_TRACE_DEBUG("%d Done Programming non-anchor entry", rs);
        if (rs != SDK_RET_OK) {
            free_collision_index(fse, hct_index_);
            goto end;
        }

        SDK_TRACE_DEBUG("Program Spine Entry ");
        // Program FT / FHCT Spine Entry
        fse->program_table();

        // Have to check if this is the first entry in spine then reprogram prev as well
        // Prevent for first spine entry
        if (fse->get_num_hgs() == 1 && fse->get_prev()) {
            SDK_TRACE_DEBUG("FSE being programmed for first time...");
            // Link prev to this
            fse->get_prev()->set_next(fse);
            // Reprogram prev FSE
            fse->get_prev()->program_table();
        }

    } else {
        // Case 3: HG has spine.
        //      - FEntry will be installed at the end of HG.
        // Install FEntry in FHCT.
        // Re-Install last entry in the HG to point to the above entry.
        SDK_TRACE_DEBUG("Insert:HG Chain Entry ...");

        /*
         * 1 recirc of attached entry is already incremented.
         * To add to the end of list, the following is the
         * number of recircs.
         */
        inc_recircs(hg->get_num_hbm_hash_entries() - 1);
        if (get_recircs() == fse->get_ht_entry()->
            get_hbm_hash()->max_recircs()) {
            SDK_TRACE_ERR("Unable to install flow. #recircs "
                          "exceeds max recircs: %d. ret: %d",
                          fse->get_ht_entry()->
                          get_hbm_hash()->max_recircs(),
                          SDK_RET_MAX_RECIRC_EXCEED);
            rs = SDK_RET_MAX_RECIRC_EXCEED;
            return rs;
        }

        // Allocate an entry in HBM Hash Coll. Table.
        rs = alloc_collision_index(fse, &hct_index_);
        if (rs != SDK_RET_OK) {
            return rs;
        }

        // storing the last entry which has to be re-programmed
        HbmHashEntry * hg_last_entry = hg->get_last_hbm_hash_entry();

        // Set fields in HbmHashEntry
        is_anchor_entry_ = FALSE;
        hint_group_ = hg;
        // Set Fields in HintGroup
        hg->add_hbm_hash_entry(this);

        // Write the HBM Hash entry in FHCT
        // P4-PI: FHCT Write hct_index_ => [hw_key_, data_]
        rs = program_table_non_anchor_entry(NULL);
        if (rs != SDK_RET_OK) {
            free_collision_index(fse, hct_index_);
            goto end;
        }

        // hct_index_ = hg_last_entry->get_fhct_index();
        // P4-PI: FHCT Write
        //      hg_last_entry->hw_key_, hg_last_entry->data,
        //        hg->get_hint_bits(), this->hct_index_
        rs = hg_last_entry->program_table_non_anchor_entry(this);
        if (rs != SDK_RET_OK) {
            free_collision_index(fse, hct_index_);
            goto end;
        }
    }

end:
    return rs;
}

// ---------------------------------------------------------------------------
// Update HBM Hash Entry in HW
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::update(void *data)
{
    sdk_ret_t rs = SDK_RET_OK;
    SDK_TRACE_DEBUG("Update...");

    // Updates Data in the entry
    std::memcpy(data_, data, data_len_);

    if (is_anchor_entry_) { // Case 1: FT Entry Re-Install

        SDK_TRACE_DEBUG("Update anchor");

        // Program FT/FHCT Spine entry.
        spine_entry_->program_table();

    } else { // Case 2: FHCT Single HBM Hash Entry Rewrite

        SDK_TRACE_DEBUG("Update FHCT: %d", hct_index_);
        // P4-PI: FHCT Table Write
        HbmHashEntry *next_fe = hint_group_->get_next_hbm_hash_entry(this);
        program_table_non_anchor_entry(next_fe);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Remove this entry.
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::remove()
{
    sdk_ret_t rs = SDK_RET_OK;
    HbmHashEntry *last_hbm_hash_entry = NULL, *new_last_hbm_hash_entry = NULL;
    HbmHashSpineEntry *fse_last = NULL, *eff_spine_entry = get_eff_spine_entry();
    HbmHashSpineEntry *fse_prev = NULL;
    HbmHashHintGroup *last_hg = NULL;
    // HbmHashTableEntry *fte = eff_spine_entry->get_ht_entry();
    std::list<HbmHashHintGroup*>::iterator itr;

    SDK_TRACE_DEBUG("Removing an entry from HG: %p", hint_group_);

    // Step 1: Replace the entry & reprogram
    if (is_anchor_entry_) {
        // Case 1: Anchor entry.
        SDK_TRACE_DEBUG("Removing anchor entry");

        // Get the entry which will be moved
        last_hbm_hash_entry = eff_spine_entry->get_ht_entry()->get_last_hbm_hash_entry();

        if (this == last_hbm_hash_entry) {
            // No need to move.
            // This is the last entry in the spine entry.
            SDK_TRACE_DEBUG("Removing the only existing entry");

            // Program previous spine entry as this spine entry is going away
            // May not happen as anchor is only in FT's spine entry
            fse_prev = eff_spine_entry->get_prev();
            if (fse_prev) {
                fse_prev->set_next(NULL);
                fse_prev->program_table();
            }

            // Reset anchor entry in spine entry
            eff_spine_entry->set_anchor_entry(NULL);
            // Remove from HG's anchor HBM Hash entry list
            hint_group_->del_anchor_hbm_hash_entry(this);
            // Program spine entry with all 0s.
            eff_spine_entry->program_table();

#if 0
            // Anchor entry is present only in FT.
            // Not FT then its FHCT spine entry.
            if (!spine_entry_->get_is_in_ft()) {
                // Set Prev spine entry to point to NULL
                HbmHashSpineEntry *fse_prev = spine_entry_->get_prev();
                fse_prev->set_next(NULL);

                // Program FHCT/FT Spine Entry
                fse_prev->program();
            }
#endif
        } else {

            if (!eff_spine_entry->get_ht_entry()->get_hbm_hash()->
                    get_delayed_del_en()) {
                // Remove from HG's HBM Hash entry list and put in anchor list.
                // Put last HBM Hash entry as this->eff_spine anchor.
                // If its last entry in HG chain:
                //      - Remove HG From last_entry->eff_spine list
                //      - Reprogram last_entry->eff_spine.
                // else:
                //      - Get Prev HBM Hash entry in HG chain.
                //      - Program the prev_hbm_hash_entry

                // Reset anchor entry in spine entry
                eff_spine_entry->set_anchor_entry(last_hbm_hash_entry);
                // Remove from HG's anchor HBM Hash entry list
                hint_group_->del_anchor_hbm_hash_entry(this);
                // Program spine entry.
                eff_spine_entry->program_table();

                // Removing Last HBM Hash Entry
                HbmHashHintGroup *last_hg = last_hbm_hash_entry->get_fh_group();
                HbmHashSpineEntry *last_fse = last_hg->get_fs_entry();
                SDK_TRACE_DEBUG("last_hg:");
                last_hg->print_hg();
                // Add last_hbm_hash_entry to the anchor list of HG
                last_hg->add_anchor_hbm_hash_entry(last_hbm_hash_entry);
                // Rem last_hbm_hash_entry from hbm_hash_entry list of last HG
                last_hg->del_hbm_hash_entry(last_hbm_hash_entry);

                if (last_hg->get_num_hbm_hash_entries()) {
                    // ----------
                    SDK_TRACE_DEBUG("Move: Hint Group Chain prev entry "
                                    "reprogram & deprogram ");
                    // Re-program prev entry in the chain.
                    last_hg->get_last_hbm_hash_entry()->
                        program_table_non_anchor_entry(NULL);

                    // Un-program last_hbm_hash_entry
                    last_hbm_hash_entry->deprogram_table_non_anchor_entry(eff_spine_entry);
                    // Free up FHCT index of last hbm_hash_entry
                    free_collision_index(last_fse, last_hbm_hash_entry->get_fhct_index());
                } else {
                    // last_hg will always have anchor entry. So it wont go away.
                    // Since there are no hbm hash entries, it will be removed from
                    // fse.
                    last_fse->del_hg(last_hg);

                    // Remove HG from FSE and FTE
                    // fte->remove_hg(last_hg);
                    if (last_fse->get_num_hgs() || last_fse->get_anchor_entry()) {
                        SDK_TRACE_DEBUG("Move: HG Remove: Reprogram last spine");
                        // Re-program last_fse
                        last_fse->program_table();
                        // Un-program last_hbm_hash_entry
                        last_hbm_hash_entry->deprogram_table_non_anchor_entry(eff_spine_entry);
                        // Free up FHCT index of last hbm hash entry
                        free_collision_index(last_fse, last_hbm_hash_entry->get_fhct_index());
                    } else {
                        SDK_TRACE_DEBUG("Move: HG Remove, Spine Remove: "
                                        "Reprogram last spine, Deprogram prev spine");
                        // Reprogram prev. last fse entry as last fse is going away.
                        HbmHashSpineEntry *last_fse_prev = last_fse->get_prev();
                        last_fse_prev->set_next(NULL);
                        last_fse_prev->program_table();

                        // Re-program last_fse with all 0s
                        last_fse->program_table();
                        // Free FHCT index of last_fse
                        free_collision_index(last_fse, last_fse->get_fhct_index());
                        // delete last_fse;
                        HbmHashSpineEntry::destroy(last_fse);
                        eff_spine_entry->get_ht_entry()->dec_num_spine_entries();
                    }
                }

                // Update last hbm hash entry as anchor entry
                last_hbm_hash_entry->set_is_anchor_entry(true);
                last_hbm_hash_entry->set_spine_entry(eff_spine_entry);
                last_hbm_hash_entry->set_fhct_index(0);

            } else {

                // Create a copy of new last_hbm_hash_entry with location as "this".
                new_last_hbm_hash_entry = create_new_hbm_hash_entry(last_hbm_hash_entry);

                // Replace "this" with new_last_hbm_hash_entry as spine_entry's anchor.
                eff_spine_entry->set_anchor_entry(new_last_hbm_hash_entry);

                // Remove from HG's anchor hbm hash entry list
                hint_group_->del_anchor_hbm_hash_entry(this);

                // Re-Install spine_entry
                eff_spine_entry->program_table();

                // Add new_last_hbm_hash_entry to the anchor list of HG
                last_hbm_hash_entry->get_fh_group()->
                    add_anchor_hbm_hash_entry(new_last_hbm_hash_entry);

                // Replace last_hbm_hash_entry with new_last_hbm_hash_entry in global map
                eff_spine_entry->get_ht_entry()->get_hbm_hash()->
                    add_hbm_hash_entry_global_map(new_last_hbm_hash_entry,
                            last_hbm_hash_entry->get_global_index());


                // Put last_hbm_hash_entry in delayed_delete list of HBM Hash.
                eff_spine_entry->get_ht_entry()->get_hbm_hash()->
                    push_fe_delete_q(last_hbm_hash_entry);
            }
        }
    } else if (hint_group_->get_first_hbm_hash_entry() == this) {
        // Case 2: HBM hash entry is attached to spine entry.
        SDK_TRACE_DEBUG("Removing HBM Hash entry attached to spine entry");

        if (hint_group_->get_num_hbm_hash_entries() > 1) {
            // ----------
            // No need to move
            // At hct_index_ program all 0s
            //    TODO-P4-PI: hct_index_ => [hw_key_, data_,]
            SDK_TRACE_DEBUG("HS has extra entries..."
                            "just reprogram spine entry");

            // Remove HBM Hash entry from HG hbm_hash_entry_list_
            hint_group_->del_hbm_hash_entry(this);

            // Re-program FSE
            eff_spine_entry->program_table();

        } else {
            // HG rooted on spine entry is getting detached.
            SDK_TRACE_DEBUG("HG rooted on spine entry is getting detached");

            // Remove HBM Hash entry from HG hbm_hash_entry_list_
            hint_group_->del_hbm_hash_entry(this);

            fse_last = hint_group_->get_fs_entry()->
                get_ht_entry()->get_last_spine_entry();
            // Get Last HG
            last_hg = fse_last->get_last_hg();
            if (last_hg == hint_group_) {
                SDK_TRACE_DEBUG("Current is last HG: Last HG is removed."
                                "Reprogram last spine entry");
                // No need to move
                // Remove hg from fse
                eff_spine_entry->del_hg(hint_group_);
                // Re-program FSE
                eff_spine_entry->program_table();
                if (!eff_spine_entry->get_anchor_entry() &&
                        !eff_spine_entry->get_num_hgs()) {
                    SDK_TRACE_DEBUG("Current is last HG: Last Spine entry is removed."
                                    "Reprogram last prev. spine entry");
                    fse_prev = eff_spine_entry->get_prev();
                    if (fse_prev) {
                        fse_prev->set_next(NULL);
                        fse_prev->program_table();
                    }
                }

            } else {
                // Need to move last_hg to here
                if (!eff_spine_entry->get_ht_entry()->get_hbm_hash()->
                        get_delayed_del_en()) {
                    SDK_TRACE_DEBUG("Moving last hg to here");

#if 0
                    if (fse_last == eff_spine_entry) {
                        // Remove last hg
                        fse_last->del_hg(last_hg);
                    }
#endif
                    // Remove last_hg from fse_last
                    fse_last->del_hg(last_hg);

                    // Change parent of hg to current spine entry
                    last_hg->set_fs_entry(eff_spine_entry);
                    // Put last_hg in the current FSE
                    eff_spine_entry->replace_hg(hint_group_, last_hg);
                    // Re-program FSE
                    eff_spine_entry->program_table();

                    // Check if we have to program fse_last
                    if (fse_last != eff_spine_entry) {
                        SDK_TRACE_DEBUG("Programming last spine entry");
                        // ----------
                        // Handle last_hg move
                        if (fse_last->get_num_hgs() || fse_last->get_anchor_entry()) {
                            // Remove last_hg from fse_last
                            SDK_TRACE_DEBUG("Last spine entry has FGs. Reprogramming it");
                            // fse_last->del_hg(last_hg);
                            // Re-program fse_last
                            fse_last->program_table();
                        } else {
                            SDK_TRACE_DEBUG("Last spine entry has no FGS or anchors. Reprogramming prev.");
                            // Reprogram prev. last fse entry as last fse is going away.
                            HbmHashSpineEntry *fse_last_prev = fse_last->get_prev();
                            fse_last_prev->set_next(NULL);
                            fse_last_prev->program_table();

                            // Re-program fse_last with all 0s
                            fse_last->program_table();
                            // Free FHCT index of fse_last
                            free_collision_index(fse_last, fse_last->get_fhct_index());
                            // delete fse_last;
                            HbmHashSpineEntry::destroy(fse_last);
                            eff_spine_entry->get_ht_entry()->dec_num_spine_entries();
                        }
                    }

                } else {

                    // Changing the spine entry of last_hg.
                    last_hg->set_delayed_del_fs_entry(last_hg->get_fs_entry());
                    last_hg->set_fs_entry(eff_spine_entry);
                    // Put last_hg in the current FSE
                    eff_spine_entry->replace_hg(hint_group_, last_hg);

                    // Re-program FSE
                    eff_spine_entry->program_table();

                    // Add last_hg to delayed delete HBM Hash entry.
                    eff_spine_entry->get_ht_entry()->get_hbm_hash()->
                        push_hg_delete_q(last_hg);
                }

            }

            // Hint group detaches from spine entry. So setting spine entry to NULL.
            hint_group_->set_fs_entry(NULL);
        }
        // Re-program FHCT with 0s
        deprogram_table_non_anchor_entry(eff_spine_entry);
        // Free up hct_index_ from Indexer
        free_collision_index(eff_spine_entry, hct_index_);
    } else {
        // ----------
        // Case 3: HBM Hash entry is in HG list, and its not attached to Spine.

        // Re-program prev. FHCT entry to point to next FHCT entry.
        HbmHashEntry *prev_fe = hint_group_->get_prev_hbm_hash_entry(this);
        // HbmHashEntry *next_fe = hint_group_->get_next_hbm_hash_entry(this);
        // Remove this from hg
        hint_group_->del_hbm_hash_entry(this);
        // Program the Prev. HBM Hash Entry in HG list
        prev_fe->deprogram_table_non_anchor_entry(prev_fe->get_eff_spine_entry());
        // Re-program FHCT with 0s
        deprogram_table_non_anchor_entry(eff_spine_entry);
        // Free up the FHCT Indexer
        free_collision_index(eff_spine_entry, hct_index_);
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Allocate FHCT index
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::alloc_collision_index(HbmHashSpineEntry *fse, uint32_t *fhct_index)
{
    sdk_ret_t rs = SDK_RET_OK;
    rs = fse->get_ht_entry()->get_hbm_hash()->alloc_collision_index(fhct_index);
    return rs;
}

// ---------------------------------------------------------------------------
// Free FHCT index
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::free_collision_index(HbmHashSpineEntry *fse, uint32_t fhct_index)
{
    sdk_ret_t rs = SDK_RET_OK;
    rs = fse->get_ht_entry()->get_hbm_hash()->free_collision_index(fhct_index);
    return rs;
}

// ---------------------------------------------------------------------------
// Create a new hbm_hash entry with all fields other than location. Loc. of this.
// ---------------------------------------------------------------------------
HbmHashEntry *
HbmHashEntry::create_new_hbm_hash_entry(HbmHashEntry *fe)
{
    // HbmHashEntry *new_fe = new HbmHashEntry(fe->get_key(), fe->get_key_len(),
    //                                   fe->get_data(), fe->get_data_len(),
    //                                   hwkey_len_, true);
    HbmHashEntry *new_fe = HbmHashEntry::factory(fe->get_key(), fe->get_key_len(),
                                           fe->get_data(), fe->get_data_len(),
                                           hwkey_len_, true,
                                           get_bucket()->get_hbm_hash());
    *new_fe = *fe;

    // Take location information from "this"
    new_fe->set_is_anchor_entry(is_anchor_entry_);
    new_fe->set_spine_entry(spine_entry_);
    new_fe->set_fhct_index(hct_index_);

    return new_fe;
}

// ---------------------------------------------------------------------------
// Get HBM Hash Table Entry
// ---------------------------------------------------------------------------
HbmHashTableEntry *
HbmHashEntry::get_bucket()
{
    return get_eff_spine_entry()->get_ht_entry();
}

// ---------------------------------------------------------------------------
// Get Effective Spine Entry
// ---------------------------------------------------------------------------
HbmHashSpineEntry *
HbmHashEntry::get_eff_spine_entry()
{
    if (is_anchor_entry_) {
        return spine_entry_;
    } else {
        return hint_group_->get_fs_entry();
    }
}

sdk_ret_t
HbmHashEntry::form_action_data(HbmHashEntry *next_fe, void *swdata)
{
    uint8_t         *action_id;
    uint8_t         *entry_valid;
    void            *data;
    void            *first_hash_hint;
    uint8_t         *more_hashs;
    void            *more_hints;
    uint32_t        hint_bits = 0;
    uint32_t        fhct_idx = 0;
    uint32_t        hint_mem_len_B = 0;

    hint_mem_len_B = get_bucket()->get_hbm_hash()->get_hint_mem_len_B();

    get_bucket()->get_hbm_hash()->
        hbm_hash_action_data_offsets(swdata,
                                 &action_id,
                                 &entry_valid,
                                 &data,
                                 &first_hash_hint,
                                 &more_hashs,
                                 &more_hints);

    *action_id = 0;
    *entry_valid = 1;
    memcpy(data, data_, data_len_); // export_en + hbm_hash_index is data

    if (next_fe) {
        // first_hash_hint->hash = next_fe->get_fh_group()->get_hint_bits();
        // first_hash_hint->hint = next_fe->get_fhct_index();
        char *loc = (char *)first_hash_hint;
        hint_bits = next_fe->get_fh_group()->get_hint_bits();
        fhct_idx = next_fe->get_fhct_index();
        memcpy(loc, &hint_bits, 2);
        memcpy(loc + 2, &fhct_idx, hint_mem_len_B);
    }

    return SDK_RET_OK;
}


// ---------------------------------------------------------------------------
// Program non-anchor entry
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::program_table_non_anchor_entry(HbmHashEntry *next_fe)
{
    sdk_ret_t       rs = SDK_RET_OK;
    p4pd_error_t    pd_err = P4PD_SUCCESS;
    uint32_t        coll_table_id = 0;
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

    coll_table_id = get_bucket()->get_hbm_hash()->get_collision_table_id();
    entire_data_len = get_bucket()->get_hbm_hash()->
                      get_entire_data_len();
    hint_mem_len_B = get_bucket()->get_hbm_hash()->get_hint_mem_len_B();

    swdata = SDK_CALLOC(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
                        entire_data_len);
    get_bucket()->get_hbm_hash()->
        hbm_hash_action_data_offsets(swdata,
                                 &action_id,
                                 &entry_valid,
                                 &data,
                                 &first_hash_hint,
                                 &more_hashs,
                                 &more_hints);

    *action_id = 0;
    *entry_valid = 1;
    memcpy(data, data_, data_len_); // export_en + hbm_hash_index is data

    if (next_fe) {
        // first_hash_hint->hash = next_fe->get_fh_group()->get_hint_bits();
        // first_hash_hint->hint = next_fe->get_fhct_index();
        char *loc = (char *)first_hash_hint;
        hint_bits = next_fe->get_fh_group()->get_hint_bits();
        fhct_idx = next_fe->get_fhct_index();
        memcpy(loc, &hint_bits, 2);
        memcpy(loc + 2, &fhct_idx, hint_mem_len_B);
    }

    SDK_TRACE_DEBUG("CollisionTID: %d P4 FHCT Write: %d",
                    coll_table_id, hct_index_);

    hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_ENTRY_HW_KEY, hwkey_len_);
    p4pd_hwkey_hwmask_build(coll_table_id, key_,
                            NULL, (uint8_t *)hwkey, NULL);

    // P4-API: Collision Table Write
    pd_err = p4pd_entry_write(coll_table_id, hct_index_, (uint8_t*)hwkey, NULL,
                              swdata);
    SDK_TRACE_DEBUG("Done programming HBM Hash Collision %d", pd_err);

    // Free
    SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_ENTRY_HW_KEY, hwkey);
    SDK_FREE(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA, swdata);

    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : rs;
}


// ---------------------------------------------------------------------------
// DeProgram Non-anchor entry
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::deprogram_table_non_anchor_entry(HbmHashSpineEntry *fse)
{
    sdk_ret_t                       rs = SDK_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;
    uint32_t                        coll_table_id = 0;
    uint32_t                        entire_data_len = 0;
    void                            *swdata = NULL;

    SDK_TRACE_DEBUG("Deprogram Coll. Table idx: %d", hct_index_);

    entire_data_len = fse->get_ht_entry()->get_hbm_hash()->
                      get_entire_data_len();
    swdata = SDK_CALLOC(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
                        entire_data_len);

    coll_table_id = fse->get_ht_entry()->get_hbm_hash()->get_collision_table_id();


    // P4-API: Collision Table Write
    pd_err = p4pd_entry_write(coll_table_id, hct_index_, NULL, NULL,
                              swdata);

    entry_trace(coll_table_id, hct_index_, swdata);

    SDK_FREE(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA, swdata);

    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : rs;
}



// ---------------------------------------------------------------------------
// Sets Key Pointer
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_key(void *key)
{
    key_ = key;
}

// ---------------------------------------------------------------------------
// Sets Key Len
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_key_len(uint32_t key_len)
{
    key_len_ = key_len;
}


// ---------------------------------------------------------------------------
// Sets Data Pointer
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_data(void *data)
{
    data_ = data;;
}

// ---------------------------------------------------------------------------
// Sets Data Len
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_data_len(uint32_t data_len)
{
    data_len_ = data_len;
}

// ---------------------------------------------------------------------------
// Sets Hash Value
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_hash_val(uint32_t hash_val)
{
    hash_val_ = hash_val;
}

// ---------------------------------------------------------------------------
// Set HBM Hash Collision Table Index
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_fhct_index(uint32_t fhct_index)
{
    hct_index_ = fhct_index;
}

// ---------------------------------------------------------------------------
// Sets HBM Hash Hint Group
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_fh_group(HbmHashHintGroup *fh_group)
{
    hint_group_ = fh_group;
}

// ---------------------------------------------------------------------------
// Sets Global Index
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_global_index(uint32_t index)
{
    gl_index_ = index;
}


// ---------------------------------------------------------------------------
// Sets is_anchor_entry
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_is_anchor_entry(bool is_anchor)
{
    is_anchor_entry_ = is_anchor;
}

// ---------------------------------------------------------------------------
// Set Spine entry
// ---------------------------------------------------------------------------
void
HbmHashEntry::set_spine_entry(HbmHashSpineEntry *spe)
{
    spine_entry_ = spe;
}

// ---------------------------------------------------------------------------
// Forms hw key
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::form_hw_key(uint32_t table_id, void *hwkey)
{
    sdk_ret_t                       rs = SDK_RET_OK;
    p4pd_error_t                    pd_err = P4PD_SUCCESS;

    pd_err = p4pd_hwkey_hwmask_build(table_id, key_, NULL,
                                     (uint8_t *)hwkey, NULL);

    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : rs;
}

// ---------------------------------------------------------------------------
// P4Pd trace of the entry
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHashEntry::entry_trace(uint32_t table_id, uint32_t index,
                             void *data)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(table_id, index, key_, NULL,
                                              data, buff, sizeof(buff));
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    SDK_TRACE_DEBUG("Index: %d %s", index, buff);

    return SDK_RET_OK;
}

// ---------------------------------------------------------------------------
// Prints HBM Hash Entry
// ---------------------------------------------------------------------------
void
HbmHashEntry::print_fe()
{
    SDK_TRACE_DEBUG("      hbm_hash_entry: fe_idx: %d, is_anchor: %d, "
                    "fhct_index: %d, hg_bits: %#x",
                    gl_index_, is_anchor_entry_,
                    hct_index_, hint_group_->get_hint_bits());
}

void
HbmHashEntry::entry_to_str(char *buff, uint32_t buff_size)
{
    p4pd_error_t    p4_err;
    void            *swdata;
    uint32_t        entire_data_len;
    uint32_t        coll_table_id = 0;

    coll_table_id = get_bucket()->get_hbm_hash()->get_collision_table_id();
    entire_data_len = get_bucket()->get_hbm_hash()->
                      get_entire_data_len();

    swdata = SDK_CALLOC(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA,
                        entire_data_len);

    form_action_data(hint_group_->get_next_hbm_hash_entry(this), swdata);

    p4_err = p4pd_table_ds_decoded_string_get(coll_table_id, hct_index_, key_, NULL,
                                              swdata, buff, buff_size);
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    SDK_FREE(SDK_MEM_ALLOC_ENTIRE_HBM_HASH_ENTRY_DATA, swdata);
}
