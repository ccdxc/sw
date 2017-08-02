#include "met.hpp"
#include "repl_list.hpp"
#include "repl_entry.hpp"
#include "repl_table_entry.hpp"

using hal::pd::utils::ReplList;
using hal::pd::utils::ReplTableEntry;
using hal::pd::utils::Met;

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
    delete first_repl_tbl_entry_;
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

    repl_entry = new ReplEntry(data, met_->get_repl_entry_data_len());

    // Check if we have to create ReplTableEntry
    rs = get_repl_table_entry(&repl_te);
    if (rs != HAL_RET_OK) {
        delete repl_entry;
        goto end;
    }

    repl_te->add_replication(repl_entry);

end:
    return rs;

}


// ----------------------------------------------------------------------------
// Delete Replication Entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplList::del_replication(void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    ReplTableEntry *repl_te = NULL;

    repl_te = first_repl_tbl_entry_;
    while (repl_te) {
        rs = repl_te->del_replication(data);
        if (rs == HAL_RET_OK) { // deletion happened
            // Check if repl. table entry has to be removed.
            if (!repl_te->get_num_repl_entries()) {
                if (repl_te->get_prev() == NULL) {
                    first_repl_tbl_entry_ = repl_te->get_next();
                    if (repl_te->get_next()) {
                        repl_te->get_next()->set_prev(NULL);
                    }
                } else if (repl_te->get_next() == NULL) {
                    last_repl_tbl_entry_ = repl_te->get_prev();
                    if (repl_te->get_prev()) {
                        repl_te->get_prev()->set_next(NULL);
                    }
                } else {
                    repl_te->get_prev()->set_next(repl_te->get_next());
                    repl_te->get_next()->set_prev(repl_te->get_prev());
                }

                HAL_TRACE_DEBUG("ReplList::{}: Delete RTE: {}", __FUNCTION__, 
                                repl_te->get_repl_table_index());
                // if the repl table entry being removed has index different than 
                // the first one ... then we can free up the index.
                if (repl_te->get_repl_table_index() != repl_tbl_index_) {
                    rs = met_->free_repl_table_index(repl_te->get_repl_table_index());
                    HAL_ASSERT(rs == HAL_RET_OK);
                }
                delete repl_te;
                num_repl_tbl_entries_--;
            }
            goto end;
        }

        repl_te = repl_te->get_next();
    }

end:
    return rs;
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
    tmp_rte = new ReplTableEntry(repl_tbl_idx, this);
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

