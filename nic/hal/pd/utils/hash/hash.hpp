/* ============================================================================
 |
 | Hash HW Table Library
 | ---- -- ----- -------
 |
 | - Helps in Programming Hash and Overflow TCAM Tables in HW.
 | - Eventually Calls P4 APIs to program.
 |
 | TODO:
 |  Optimizations:
 * ============================================================================
 */

#ifndef __HASH_HPP__
#define __HASH_HPP__

#include "nic/include/base.h"
#include <string>
#include <map>
#include <boost/crc.hpp>
#include "nic/include/hal_mem.hpp"

using namespace std;

namespace hal {
namespace pd {
namespace utils {

// forward declarations
class Tcam;
class HashEntry;

typedef std::map<uint32_t, HashEntry*> HashEntryMap;
typedef bool (*hash_iterate_func_t)(const void *key,
                                    const void *data,
                                    uint32_t hash_idx,
                                    const void *cb_data);

/** ---------------------------------------------------------------------------
  * 
  * class Hash
  *
  *     - Hash Table Implementation with Overflow TCAM
  *
  * ---------------------------------------------------------------------------
*/
class Hash {

public:
    enum HashPoly {
        // CRC_32,
        HASH_POLY0,
        HASH_POLY1,
        HASH_POLY2,
        HASH_POLY3
    };

    enum EntryType {
        DLEFT,
        OTCAM,
        BOTH
    };

    // Note: Stats are mutually exclusive for every API. Only one stat will
    //       be incremented for an API call.
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_HW,
        STATS_INS_FAIL_DUP_INS,
        STATS_INS_FAIL_NO_RES,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_OOB,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_INV_ARG,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_OOB,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_INV_ARG,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_OOB,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FAIL_INV_ARG,
        STATS_NUM_HASH,
        STATS_NUM_TCAM,

        STATS_MAX

    };
private:

    std::string     table_name_;            // table name
    uint32_t        table_id_;              // table id
    uint32_t        dleft_capacity_;        // size of dleft table
    uint32_t        swkey_len_;             // sw key len
    uint32_t        swdata_len_;            // sw data len
    HashPoly        hash_poly_;             // hash polynomial

    uint32_t        hwkey_len_;             // hw key len
    uint32_t        hwdata_len_;            // hw data len

    Tcam            *otcam_;                // overflow tcam

    // Hash Value => Hash Entry
    HashEntryMap  hash_entry_map_;

    // Static Declarations
    static const uint8_t otcam_bit_ = 28;    // Dleft or OTcam ?

    uint64_t        *stats_;                 // Statistics

    // overflow tcam presence
    bool has_otcam_();  
    // Generates Hash 
    uint32_t generate_hash_(void *key, uint32_t key_len);
    // hw program 
    hal_ret_t program_table_(HashEntry *he, void *hwkey);
    hal_ret_t deprogram_table_(HashEntry *he);


    static uint32_t form_hash_idx_from_dleft_id_(uint32_t dleft_id);
    static uint32_t form_hash_idx_from_otcam_id_(uint32_t otcam_id);
    typedef struct otcam_iterate_cb_s {
        hash_iterate_func_t cb;
        const void *cb_data;
    } otcam_iterate_cb_t;
    static bool otcam_iterate_(const void *key, 
                               const void *key_mask, 
                               const void *data, 
                               uint32_t tcam_idx, 
                               const void *cb_data);
    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        UPDATE,
        REMOVE,
        RETRIEVE
    };
    void stats_update(api ap, hal_ret_t rs); 
    // Entry Trace
    hal_ret_t entry_trace_(HashEntry *he);

    Hash(std::string table_name, uint32_t dleft_table_id, 
         uint32_t otcam_table_id, uint32_t dleft_capacity,
         uint32_t otcam_capacity, uint32_t swkey_len, uint32_t swdata_len, 
         Hash::HashPoly hash_poly = HASH_POLY0);
    ~Hash();
public:
	static Hash *factory(std::string table_name, uint32_t dleft_table_id,
						 uint32_t otcam_table_id, uint32_t dleft_capacity,
						 uint32_t otcam_capacity, uint32_t swkey_len,
						 uint32_t swdata_len, Hash::HashPoly hash_poly = HASH_POLY0,
						 uint32_t mtrack_id = HAL_MEM_ALLOC_HASH);
	static void destroy(Hash *hash, uint32_t mtrack_id = HAL_MEM_ALLOC_HASH);

    // Debug Info
    uint32_t table_id(void) { return table_id_; }
    const char *table_name(void) { return table_name_.c_str(); }
    uint32_t table_capacity(void) { return dleft_capacity_; }
    uint32_t oflow_table_capacity(void);
    uint32_t table_num_entries_in_use(void);
    uint32_t oflow_table_num_entries_in_use(void);
    uint32_t table_num_inserts(void);
    uint32_t table_num_insert_errors(void);
    uint32_t table_num_deletes(void);
    uint32_t table_num_delete_errors(void);

    hal_ret_t insert(void *key, void *data, uint32_t *index, 
                     bool direct_to_otcam = FALSE);
    hal_ret_t update(uint32_t index, void *data);
    hal_ret_t remove(uint32_t index);
    hal_ret_t retrieve(uint32_t index, void *key, void *data);
    hal_ret_t iterate(hash_iterate_func_t cb, const void *cb_data,
                      Hash::EntryType type);

    // For Debugging
    static bool is_dleft(uint32_t hash_idx);
    static uint32_t get_dleft_id_from_hash_idx_(uint32_t hash_idx);
    static uint32_t get_otcam_id_from_hash_idx_(uint32_t hash_idx);
    hal_ret_t fetch_stats(const uint64_t **stats);

};

}    // namespace hal
}    // namespace pd
}    // namespace utils

#endif // __HASH_HPP__
