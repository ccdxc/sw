/* ============================================================================
 *
 * TCAM HW Table Library
 * ---- -- ----- -------
 *
 * - Helps in Programming TCAM Tables in HW.
 * - Eventually Calls P4 APIs to program.
 *
 * TODO:
 *  Optimizations:
 *      - For INSERT_FAIL_DUPLICATE, instead of walking all entries better
 *        store in some structure keyed on key, key_mask without replicating
 *        the TcamEntry
 * ============================================================================
 */
#ifndef __TCAM_HPP__
#define __TCAM_HPP__

#include "nic/include/base.h"
#include <string>
#include <map>
#include "nic/include/hal_mem.hpp"
#include "nic/utils/indexer/indexer.hpp"

using hal::utils::indexer;

namespace hal {
namespace pd {
namespace utils {

// forward declarations
class TcamEntry;

typedef std::map<uint32_t, TcamEntry*> TcamEntryMap;
typedef bool (*tcam_iterate_func_t)(const void *key,
                                    const void *key_mask,
                                    const void *data,
                                    uint32_t tcam_idx,
                                    const void *cb_data);

/** ---------------------------------------------------------------------------
  * 
  * class Tcam
  *
  * Tcam Implementation
  *
  * Assumptions:
  *     1. sw key len is same for all entries.
  *     2. sw key len and sw mask len are same.
  *
  * ---------------------------------------------------------------------------
*/
class Tcam {

public:
    // Note: Stats are mutually exclusive for every API. Only one stat will
    //       be incremented for an API call.
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_DUP_INS,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_FAIL_HW,
        STATS_INS_WITHID_SUCCESS,
        STATS_INS_WITHID_FAIL_DUP_INS,
        STATS_INS_WITHID_FAIL_HW,
        STATS_INS_WITHID_FAIL_OOB,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_OOB,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_OOB,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_OOB,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FROM_HW_SUCCESS,
        STATS_RETR_FROM_HW_FAIL_OOB,
        STATS_RETR_FROM_HW_FAIL,
        STATS_MAX
    };

private:
    std::string     table_name_;            // table name
    uint32_t        table_id_;              // table id
    uint32_t        tcam_capacity_;         // size of tcam table 
    uint32_t        swkey_len_;             // sw key len
    uint32_t        swdata_len_;            // sw data len
    bool            allow_dup_insert_;      // allow uplicate insert

    uint32_t        hwkey_len_;             // hw key len
    uint32_t        hwkeymask_len_;         // hw key mask len
    uint32_t        hwdata_len_;            // hw data len

    // TcamEntry       **tcam_table;
    TcamEntryMap    tcam_entry_map_;        // map to store entries

    indexer         *tcam_indexer_;         // tcam entry indices

    uint64_t        *stats_;                // Statistics

    hal_ret_t alloc_index_(uint32_t *idx, bool lowest);
    hal_ret_t alloc_index_withid_(uint32_t idx);
    hal_ret_t free_index_(uint32_t idx);
    bool tcam_entry_exists_(void *key, void *key_mask, uint32_t key_len,
                            TcamEntry **te);
    hal_ret_t program_table_(TcamEntry *te);
    hal_ret_t deprogram_table_(TcamEntry *te);
    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        INSERT_WITHID,
        UPDATE,
        REMOVE,
        RETRIEVE,
        RETRIEVE_FROM_HW,
        ITERATE
    };
    void stats_update(api ap, hal_ret_t rs); 
    hal_ret_t entry_trace_(TcamEntry *te);

    Tcam(std::string table_name, uint32_t table_id, uint32_t tcam_capacity, 
         uint32_t swkey_len, uint32_t swdata_len, 
         bool allow_dup_insert = false);
    ~Tcam();
public:
    static Tcam *factory(std::string table_name, uint32_t table_id, 
                         uint32_t tcam_capacity, uint32_t swkey_len, 
                         uint32_t swdata_len, bool allow_dup_insert = false,
                         uint32_t mtrack_id = HAL_MEM_ALLOC_TCAM);
    static void destroy(Tcam *tcam, uint32_t mtrack_id = HAL_MEM_ALLOC_TCAM);


    // Debug Info
    uint32_t table_id(void) { return table_id_; }
    const char *table_name(void) { return table_name_.c_str(); }
    uint32_t table_capacity(void) { return tcam_capacity_; }
    uint32_t table_num_entries_in_use(void);
    uint32_t table_num_inserts(void);
    uint32_t table_num_insert_errors(void);
    uint32_t table_num_deletes(void);
    uint32_t table_num_delete_errors(void);


    hal_ret_t insert(void *key, void *key_mask, void *data, uint32_t *index, bool lowest=true);
    hal_ret_t insert_withid(void *key, void *key_mask, void *data, uint32_t index);
    hal_ret_t update(uint32_t index, void *data);
    hal_ret_t remove(uint32_t index);
    hal_ret_t retrieve(uint32_t index, void *key, void *key_mask, 
                       void *data);
    hal_ret_t retrieve_from_hw(uint32_t index, void *key, void *key_mask, 
                               void *data);
    hal_ret_t iterate(tcam_iterate_func_t func, const void *cb_data);


    // Debug Stats
    hal_ret_t fetch_stats(const uint64_t **stats);
};

}    // namespace utils
}    // namespace pd
}    // namespace hal

#endif // __TCAM_HPP__
