/*
 * ACL TCAM HW Table Library
 * --------------------------
 *
 * - Helps in programming ACL TCAM tables in HW
 * - Inserts entries in the order of priority
 * - Entry insertion might involve moving the entries around
 * - Eventually Calls P4 APIs to program.
 *
 * acl_tcam.hpp
 * Vasanth Kumar (Pensando Systems)
 */
#ifndef __ACL_TCAM_HPP__
#define __ACL_TCAM_HPP__

#include "nic/include/base.h"
#include <string>
#include <map>
#include "nic/utils/bitmap/bitmap.hpp"

using hal::utils::bitmap;

namespace hal {
namespace pd {
namespace utils {

typedef uint64_t acl_tcam_entry_handle_t;
typedef uint32_t priority_t;
typedef uint32_t move_chain_t;

#define ACL_TCAM_ENTRY_INVALID_HANDLE UINT64_MAX

// forward declarations
class TcamEntry;

typedef struct prio_range_s {
    uint32_t start;
    uint32_t end;
} prio_range_t;

typedef std::map<acl_tcam_entry_handle_t, TcamEntry*> tcam_entry_map;

typedef std::map<priority_t, prio_range_t*,bool(*)(priority_t,priority_t)> tcam_prio_map;

typedef bool (*acl_tcam_iterate_func_t)(const void *key,
                                        const void *key_mask,
                                        const void *data,
                                        acl_tcam_entry_handle_t handle,
                                        uint32_t index,
                                        const void *cb_data);


class acl_tcam {
public:
    // Note: Stats are mutually exclusive for every API. Only one stat will
    //       be incremented for an API call.
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_INVALID_ARG,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_FAIL_HW,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_INVALID_ARG,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_NO_RES,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FROM_HW_SUCCESS,
        STATS_RETR_FROM_HW_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FROM_HW_FAIL_HW,
        STATS_MAX
    };

    static acl_tcam *factory(std::string table_name, uint32_t table_id, 
                             uint32_t table_size,
                             uint32_t swkey_len, uint32_t swdata_len,
                             bool priority_0_lowest = true,
                             bool allow_same_priority = true);
    ~acl_tcam();

    hal_ret_t insert(void *key, void *key_mask, void *data,
                     priority_t priority, acl_tcam_entry_handle_t *handle);
    hal_ret_t update(acl_tcam_entry_handle_t handle, 
                     void *key, void *key_mask, void *data,
                     priority_t priority);
    hal_ret_t remove(acl_tcam_entry_handle_t handle);
    hal_ret_t retrieve(acl_tcam_entry_handle_t handle, void *key, 
                      void *key_mask, void *data);
    hal_ret_t retrieve_from_hw(acl_tcam_entry_handle_t handle, void *key, 
                               void *key_mask, void *data);
    hal_ret_t iterate(acl_tcam_iterate_func_t func, const void *cb_data);
    hal_ret_t get_index(acl_tcam_entry_handle_t handle, uint32_t *index_p);

    // Debug Info
    uint32_t table_id(void) { return table_id_; }
    const char *table_name(void) { return table_name_.c_str(); }
    uint32_t table_capacity(void) { return tcam_size_; }
    uint32_t table_num_entries_in_use(void);
    uint32_t table_num_inserts(void);
    uint32_t table_num_insert_errors(void);
    uint32_t table_num_deletes(void);
    uint32_t table_num_delete_errors(void);

    // Debug Stats
    hal_ret_t fetch_stats(const uint64_t **stats);

private:
    std::string             table_name_;         // table name
    uint32_t                table_id_;           // table id
    uint32_t                tcam_size_;          // size of tcam table
    uint32_t                swkey_len_;          // sw key len
    uint32_t                swdata_len_;         // sw data len

    uint32_t                hwkey_len_;          // hw key len
    uint32_t                hwkeymask_len_;      // hw key mask len
    uint32_t                hwdata_len_;         // hw data len
    bool                    allow_same_priority_;// Allow entries with same priority
    acl_tcam_entry_handle_t handle_allocator;    // handle allocator

    TcamEntry               **tcam_entries_;     // array of entry pointers
    tcam_prio_map*          tcam_prio_map_;      // ordered map to store the
                                                 // start and end indexes based on
                                                 // priority of the entry
    bitmap                  *inuse_bmp_;         // bitmap of indexes in use
    tcam_entry_map          tcam_entry_map_;     // map to store entries

    uint64_t                *stats_;             // Statistics


    acl_tcam() {};

    hal_ret_t init(std::string table_name, uint32_t table_id, uint32_t table_size,
                   uint32_t swkey_len, uint32_t swdata_len, 
                   bool priority_0_lowest, bool allow_same_priority);

    hal_ret_t place_entry_(void *key, void *key_mask, void *data,
                           priority_t priority, TcamEntry **tentry_p);
    hal_ret_t cleanup_entry_(TcamEntry *tentry);

    acl_tcam_entry_handle_t alloc_handle_(void) { return ++handle_allocator; }

    void set_entry_(uint32_t index, TcamEntry *tentry);
    void clear_entry_(TcamEntry *tentry);
    TcamEntry* get_entry_(uint32_t index) { return tcam_entries_[index]; }

    void set_prio_range_(priority_t priority, prio_range_t *prio_range);

    prio_range_t *get_prio_range_(priority_t priority);

    void update_priority_range_for_insert_(priority_t priority, uint32_t index);
    void update_priority_range_for_clear_(priority_t priority, uint32_t index);

    hal_ret_t find_allowed_range_(priority_t priority,
                                  bool *prev_exists_p, uint32_t *prev_end_p,
                                  bool *next_exists_p, uint32_t *next_start_p,
                                  bool *cur_exists_p);
    move_chain_t* create_move_chain_(uint32_t target_up,
                                     uint32_t target_down,
                                     uint32_t *free_spot_p,
                                     uint32_t *num_moves_p);

    void unroll_moves_(move_chain_t *move_chain,
                       uint32_t num_moves,
                       uint32_t completed_moves);
    hal_ret_t move_entries_(move_chain_t *move_chain,
                            uint32_t free_spot,
                            uint32_t num_moves);

    hal_ret_t program_table_(TcamEntry *te, uint32_t index,
                             void *key = NULL, void *key_mask = NULL, 
                             void *data = NULL);
    hal_ret_t deprogram_table_(uint32_t index) ;
    hal_ret_t entry_trace_(void *key, void *key_mask, void *data, uint32_t index);

    void move_chain_state_update_(move_chain_t *move_chain,
                                  uint32_t free_spot,
                                  uint32_t num_moves);

    hal_ret_t populate_move_chain_(move_chain_t *move_chain, uint32_t num_moves,
                                   bool move_up, uint32_t target);
    uint32_t count_moves_up_(uint32_t up_free, uint32_t target);
    uint32_t count_moves_down_(uint32_t down_free, uint32_t target);

    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        UPDATE,
        REMOVE,
        RETRIEVE,
        RETRIEVE_FROM_HW,
        ITERATE
    };
    void stats_update(api ap, hal_ret_t rs); 

};

}    // namespace utils
}    // namespace pd
}    // namespace hal

#endif // __ACL_TCAM_HPP__
