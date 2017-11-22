#include "nic/hal/pd/utils/met/met.hpp"
#include "nic/hal/pd/utils/met/repl_list.hpp"
#include "nic/hal/pd/utils/met/repl_entry.hpp"
#include "nic/hal/pd/utils/met/repl_table_entry.hpp"

using hal::pd::utils::ReplList;
using hal::pd::utils::ReplTableEntry;
using hal::pd::utils::Met;

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
ReplList *
ReplList::factory(uint32_t repl_tbl_index, Met *met,
                  uint32_t mtrack_id)
{
    void        *mem = NULL;
    ReplList    *re = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(ReplList));
    if (!mem) {
        return NULL;
    }

    re = new (mem) ReplList(repl_tbl_index, met);
    return re;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
ReplList::destroy(ReplList *re, uint32_t mtrack_id) 
{
    if (re) {
        re->~ReplList();
        HAL_FREE(mtrack_id, re);
    }
}
// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
ReplList::ReplList(uint32_t repl_tbl_index, Met *met)
{
    repl_tbl_index_         = repl_tbl_index;
    met_                    = met;

    first_repl_tbl_entry_   = NULL;
    last_repl_tbl_entry_    = NULL;
    num_repl_tbl_entries_   = 0;

    // first_repl_tbl_entry_   = new ReplTableEntry(repl_tbl_index_, this);
    // last_repl_tbl_entry_    = first_repl_tbl_entry_;
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
ReplList::~ReplList() 
{
    // delete first_repl_tbl_entry_;
}


// ----------------------------------------------------------------------------
// Adding Replication Entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplList::add_replication(void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    ReplEntry *repl_entry = NULL;
    ReplTableEntry *repl_te = NULL;
    ReplTableEntry  *last_entry = last_repl_tbl_entry_;

    HAL_TRACE_DEBUG("{}: Adding replication entry to repl_list: {}",
            __FUNCTION__, repl_tbl_index_);

    // repl_entry = new ReplEntry(data, met_->get_repl_entry_data_len());
    repl_entry = ReplEntry::factory(data, 
                                    met_->get_repl_entry_data_len());


    // Check if we have to create ReplTableEntry
    rs = get_repl_table_entry(&repl_te);
    if (rs != HAL_RET_OK) {
        // delete repl_entry;
        ReplEntry::destroy(repl_entry);
        goto end;
    }

    repl_te->add_replication(repl_entry);

    if (last_entry && (last_entry != last_repl_tbl_entry_)) {
        last_entry->program_table();
    }

end:
    return rs;

}


// ----------------------------------------------------------------------------
// Delete Replication Entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplList::del_replication(void *data)
{
    hal_ret_t rs = HAL_RET_ENTRY_NOT_FOUND;
    ReplTableEntry *repl_te = NULL;

    HAL_TRACE_DEBUG("{}: Deleting replication entry from repl_list: {}",
            __FUNCTION__, repl_tbl_index_);

    repl_te = first_repl_tbl_entry_;
    while (repl_te) {
        rs = repl_te->del_replication(data);
        if (rs == HAL_RET_OK) { // deletion happened
            // Check if repl. table entry has to be removed.
            if (!repl_te->get_num_repl_entries()) {

                // Repl List will have the first entry till its deleted.
                if (repl_te->get_repl_table_index() != repl_tbl_index_) {
                    rs = met_->free_repl_table_index(
                            repl_te->get_repl_table_index());
                    HAL_ASSERT(rs == HAL_RET_OK);
                }

                // delete repl_te;
                ReplTableEntry::destroy(repl_te);
                num_repl_tbl_entries_--;
            }
            goto end;
        }

        repl_te = repl_te->get_next();
    }

end:
    return rs;
}


// ----------------------------------------------------------------------------
// Replication Table Entry getting deleted ... Process it
// - Have to be called only when there are no replication entry in rte
//
// - First RTE
//     - if next_rte:
//         - Save next rte's index.
//         - Change next rte's index to first rte's index 
//         - Move next rte to first.
//         - Set prev of new first rte to NULL.
//         - Program first rte
//         - De-program saved rte's index in first step.
//     - else:
//         - De-program at rte's index.
// - Middle RTE
//     - Change previous RTE's next to rte's next
//     - Program previous RTE.
//     - De-program at rte's index.
// - Last RTE
//     - Change previous RTE's next to rte's next
//     - Program previous RTE.
//     - De-program at rte's index.
// ----------------------------------------------------------------------------
hal_ret_t
ReplList::process_del_repl_tbl_entry(ReplTableEntry *rte)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint32_t        tmp_rte_idx = 0, free_rte_idx = 0;
    ReplTableEntry  *next_rte = NULL;

    // Check if repl. table entry has to be removed.
    if (!rte->get_num_repl_entries()) {
        if (rte->get_prev() == NULL) {
            // First RTE
            if (rte->get_next()) {
                next_rte = rte->get_next();
                // Save next rte's index
                tmp_rte_idx = next_rte->get_repl_table_index();

                // Change next rte's index to first rte's index
                next_rte->set_repl_table_index(rte->get_repl_table_index());

                // Move next rte to first
                first_repl_tbl_entry_ = next_rte;

                // Set prev of new first rte to NULL.
                first_repl_tbl_entry_->set_prev(NULL);

                // Program first rte
                first_repl_tbl_entry_->program_table();

                // De-program saved rte's index in first step
                free_rte_idx = tmp_rte_idx;
                de_program_repl_table_entry(free_rte_idx);

                // This will be freed in del_replication
                rte->set_repl_table_index(free_rte_idx);
            } else {
                // De-program at rte's index.
                free_rte_idx = rte->get_repl_table_index();
                de_program_repl_table_entry(free_rte_idx);

                // This will be freed in del_replication
                rte->set_repl_table_index(free_rte_idx);

                // Update first and last as the only RTE is going away
                first_repl_tbl_entry_ = NULL;
                last_repl_tbl_entry_ = NULL;
            }
        } else {
            // Middle RTE/Last RTE
            // Change prev and next nodes
            rte->get_prev()->set_next(rte->get_next());
            if (rte->get_next()) {
                rte->get_next()->set_prev(rte->get_prev());
            }

            // Program previous RTE
            rte->get_prev()->program_table();
            // De-program RTE
            free_rte_idx = rte->get_repl_table_index();
            de_program_repl_table_entry(free_rte_idx);

            // This will be freed in del_replication
            rte->set_repl_table_index(free_rte_idx);

            // If rte is last, update last as rte will eventually be removed
            if (rte->get_next() == NULL) {
                last_repl_tbl_entry_ = rte->get_prev();
            }
        }
    }

    return ret;
}

hal_ret_t 
ReplList::get_repl_table_entry(ReplTableEntry **rte) 
{
    hal_ret_t rs = HAL_RET_OK;
    ReplTableEntry *tmp_rte;
    uint32_t max_num_repl_entries = 0;
    uint32_t repl_tbl_idx = 0;

    max_num_repl_entries = met_->get_max_num_repls_per_entry();

    tmp_rte = first_repl_tbl_entry_;
    while(tmp_rte) {
        if (tmp_rte->get_num_repl_entries() < max_num_repl_entries) {
            *rte = tmp_rte;
            return rs;
        }
        tmp_rte = tmp_rte->get_next();
    }

    // new repl. table entry and first one is present. That means we 
    // have to create a new entry.
    if (first_repl_tbl_entry_) { 
        // Allocate an index
        rs = met_->alloc_repl_table_index(&repl_tbl_idx);
        if (rs != HAL_RET_OK) {
            goto end;
        }
    } else {
        repl_tbl_idx = repl_tbl_index_;
    }

    HAL_TRACE_DEBUG("ReplList::{}: Create RTE: {}", __FUNCTION__, 
                    repl_tbl_idx);
    // Create a new replication table entry
    // tmp_rte = new ReplTableEntry(repl_tbl_idx, this);
    tmp_rte = ReplTableEntry::factory(repl_tbl_idx, this);
    if (first_repl_tbl_entry_ == NULL) {
        first_repl_tbl_entry_ = tmp_rte;
        last_repl_tbl_entry_ = tmp_rte;
    } else {
        tmp_rte->set_prev(last_repl_tbl_entry_);
        last_repl_tbl_entry_->set_next(tmp_rte);
        last_repl_tbl_entry_ = tmp_rte;
    }
    num_repl_tbl_entries_++;

    *rte = tmp_rte;

end:
    return rs;

}

hal_ret_t 
ReplList::de_program_repl_table_entry (uint32_t index)
{
    hal_ret_t rs = HAL_RET_OK;

    // TODO: zerout the entry at index
    return rs;
}

// ----------------------------------------------------------------------------
// Trace Replication List
// ----------------------------------------------------------------------------
hal_ret_t
ReplList::trace_repl_list()
{
    hal_ret_t rs = HAL_RET_OK;
    ReplTableEntry *repl_te = NULL;

    HAL_TRACE_DEBUG("Repl List: {} Num_of_Repl_Tbl_Entries: {}", 
                    repl_tbl_index_, num_repl_tbl_entries_);

    repl_te = first_repl_tbl_entry_;
    while (repl_te) {

        repl_te->trace_repl_tbl_entry();

        repl_te = repl_te->get_next();
    }

    return rs;
}
