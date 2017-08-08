/* ============================================================================
 |
 | DirectMap HW Table Library
 | --------- -- ----- -------
 |
 | - Helps in Programming Direct Map Tables in HW.
 | - Eventually Calls P4 APIs to program.
 |
 * ==========================================================================*/

#ifndef __DIRECT_MAP_HPP__
#define __DIRECT_MAP_HPP__


#include <base.h>
#include <string>
#include "indexer.hpp"
#include <hal_lock.hpp>

using hal::utils::indexer;

namespace hal {
namespace pd {
namespace utils {

typedef bool (*direct_map_iterate_func_t)(uint32_t index, void *data, 
                                          const void *cb_data);

/** ---------------------------------------------------------------------------
  * 
  * class DirectMap
  * 
  *  - Create an instance of this class for every Direct Map table.
  *  - Pass in the table id from p4pdtable_id generated in p4pd.h
  *  - Pass in the number of entries supported.
  *
  * ---------------------------------------------------------------------------
 */
class DirectMap {

public:
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_HW,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_WITHID_SUCCESS,
        STATS_INS_WITHID_FAIL_DUP_INS,
        STATS_INS_WITHID_FAIL_OOB,
        STATS_INS_WITHID_FAIL_HW,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_INV_ARG,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_INV_ARG,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_INV_ARG,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FAIL_HW,
        STATS_ITER_SUCCESS,
        STATS_ITER_FAIL_HW,
        STATS_MAX
    };
private:
    std::string     table_name_;    // Table Name
    uint32_t        table_id_;      // Table Id
    uint32_t        num_entries_;   // Num. of entries in table

    indexer         *dm_indexer_;   // Indexer Management
    uint32_t        hwdata_len_;    // Entry Len

    uint64_t        *stats_;         // Statistics

    bool            thread_safe_;    // Makes it thread safe

    hal_ret_t alloc_index_(uint32_t *idx);  
    hal_ret_t alloc_index_withid_(uint32_t idx);    
    hal_ret_t free_index_(uint32_t idx);
    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        INSERT_WITHID,
        UPDATE,
        REMOVE,
        RETRIEVE,
        ITERATE
    };
    void stats_update(api ap, hal_ret_t rs); 
    // Entry Trace
    hal_ret_t entry_trace_(void *data, uint32_t index);

public:
    DirectMap(std::string table_name, uint32_t table_id, uint32_t num_entries, bool thread_safe = true);
    ~DirectMap();

    // Methods
    // Inserts Entry in HW
    hal_ret_t insert(void *data, uint32_t *index);
    // Inserts Entry in HW at index
    hal_ret_t insert_withid(void *data, uint32_t index);
    // Updates Entry in HW
    hal_ret_t update(uint32_t index, void *data);
    // Removes Entry from HW
    hal_ret_t remove(uint32_t index);
    // Retrieves Entry from HW
    hal_ret_t retrieve(uint32_t index, void *data);
    // Iterates every entry and gives a call back
    hal_ret_t iterate(direct_map_iterate_func_t iterate_func, 
                      const void *cb_data);

    // Debug Stats
    hal_ret_t fetch_stats(const uint64_t **stats);
};

}   // namespace utils
}   // namespace pd
}   // namespace hal

#endif // __DIRECT_MAP_HPP__
