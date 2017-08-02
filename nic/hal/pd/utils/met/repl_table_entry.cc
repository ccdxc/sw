#include "repl_table_entry.hpp"
#include "repl_list.hpp"
#include "repl_entry.hpp"
#include "met.hpp"
#include "trace.hpp"

using hal::pd::utils::ReplTableEntry;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
ReplTableEntry::ReplTableEntry(uint32_t repl_table_index, ReplList *repl_list) 
{

    repl_table_index_ = repl_table_index;
    repl_list_        = repl_list;
    num_repl_entries_ = 0;
    first_repl_entry_ = NULL;
    prev_             = NULL;
    next_             = NULL;

}

// ----------------------------------------------------------------------------
// Destructor 
// ----------------------------------------------------------------------------
ReplTableEntry::~ReplTableEntry() {}


// ----------------------------------------------------------------------------
// Add Replication entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplTableEntry::add_replication(ReplEntry *re)
{
    hal_ret_t rs = HAL_RET_OK;
#if 0
    HAL_TRACE_DEBUG("RTE:{}: num_repl_entries_: {}, max_repl_entries: {}",
            __FUNCTION__, num_repl_entries_, 
            repl_list_->get_met()->get_num_repl_entries());
#endif
    HAL_ASSERT_GOTO(num_repl_entries_ < 
            repl_list_->get_met()->get_max_num_repls_per_entry(), end);

    // Add replication entry
    if (first_repl_entry_ == NULL) {
        first_repl_entry_ = re;
        last_repl_entry_ = re;
    } else {
        re->set_prev(last_repl_entry_);
        last_repl_entry_->set_next(re);
        last_repl_entry_ = re;
    }

    num_repl_entries_++;
    program_table();
end:
    return rs;
}

// ----------------------------------------------------------------------------
// Del Replication entry
// ----------------------------------------------------------------------------
hal_ret_t
ReplTableEntry::del_replication(void *data)
{
    hal_ret_t rs = HAL_RET_ENTRY_NOT_FOUND;
    ReplEntry *re = NULL;

    HAL_ASSERT_GOTO(num_repl_entries_ != 0, end);
    re = first_repl_entry_;
    while(re) {

        if (!memcmp(data, re->get_data(), re->get_data_len())) {
            // match
            if (re->get_prev() == NULL) {
                first_repl_entry_ = re->get_next();
                if (re->get_next()) {
                    re->get_next()->set_prev(NULL);
                }
            } else if (re->get_next() == NULL) {
                last_repl_entry_ = re->get_prev();
                if (re->get_prev()) {
                    re->get_prev()->set_next(NULL);
                }
            } else {
                re->get_prev()->set_next(re->get_next());
                re->get_next()->set_prev(re->get_prev());
            }
            num_repl_entries_--;
            program_table();
            delete re;
            rs = HAL_RET_OK;
            goto end;
        }

        re = re->get_next();
    }

    

end:
    return rs;
}


// ----------------------------------------------------------------------------
// Programs Replication table
// ----------------------------------------------------------------------------
hal_ret_t
ReplTableEntry::program_table()
{
    hal_ret_t rs = HAL_RET_OK;

    // TODO: Walk replication entries. Pgm at repl_table_index_

    return rs;
}

// ----------------------------------------------------------------------------
// DePrograms Replication table
// ----------------------------------------------------------------------------
hal_ret_t
ReplTableEntry::deprogram_table()
{
    hal_ret_t rs = HAL_RET_OK;

    // TODO: zerout the entry at repl_table_index_
    return rs;
}
