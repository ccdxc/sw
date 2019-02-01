//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __HBM_HASH_ENTRY_HPP__
#define __HBM_HASH_ENTRY_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"

#include "hbm_hash_mem_types.hpp"

namespace sdk {
namespace table {

class HbmHash;
class HbmHashHintGroup;
class HbmHashSpineEntry;

/** ---------------------------------------------------------------------------
  *
  * class HbmHashEntry
  *
  *     - A HbmHash entry.
  *
  * ---------------------------------------------------------------------------
*/
class HbmHashEntry {

public:
    enum Status {
        SUCCESS,
        FAILURE
    };
private:
    void                *key_;              // sw key
    uint32_t            key_len_;           // sw key len
    void                *data_;             // sw data
    uint32_t            data_len_;          // sw data len
    uint32_t            hwkey_len_;         // hw key len
    uint32_t            hash_val_;          // hash value from key
    HbmHashHintGroup    *hint_group_;       // HBM Hash hint group
    uint32_t            gl_index_;          // global index
    void                *hwkey_;            // hw key
    uint32_t            num_recircs_;       // num. of recircs

    // Location Information:
    bool                is_anchor_entry_;   // is anchor
    HbmHashSpineEntry   *spine_entry_;      // back-ptr to spine if its anchor
    uint32_t            hct_index_;         // non-anchor's coll. tbl idx

    HbmHashEntry (void *key, uint32_t key_len, void *data, uint32_t data_len,
               uint32_t hwkey_len, bool log, HbmHash *hbm_hash);
    ~HbmHashEntry ();

public:
    static HbmHashEntry *factory(void *key, uint32_t key_len, void *data,
                                 uint32_t data_len, uint32_t hwkey_len, bool log,
                                 HbmHash *hbm_hash);
    static void destroy(HbmHashEntry *fe, HbmHash *hbm_hash);

    sdk_ret_t insert(HbmHashHintGroup *hg, HbmHashSpineEntry *fse);
    sdk_ret_t update(void *data);
    sdk_ret_t remove();

    sdk_ret_t alloc_collision_index(HbmHashSpineEntry *fse, uint32_t *fhct_index);
    sdk_ret_t free_collision_index(HbmHashSpineEntry *fse, uint32_t fhct_index);
    HbmHashEntry* create_new_hbm_hash_entry(HbmHashEntry *fe);
    HbmHashTableEntry *get_bucket();
    HbmHashSpineEntry *get_eff_spine_entry();
    sdk_ret_t program_table_non_anchor_entry(HbmHashEntry *next_fe);
    sdk_ret_t deprogram_table_non_anchor_entry(HbmHashSpineEntry *fse);
    sdk_ret_t form_hw_key(uint32_t table_id, void *hwkey);
    sdk_ret_t entry_trace(uint32_t table_id, uint32_t index,
                          void *data);


    // Getters & Setters
    void *get_key() { return key_; }
    uint32_t get_key_len() { return key_len_; }
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_hash_val() { return hash_val_; }
    HbmHashHintGroup *get_fh_group() { return hint_group_; }
    uint32_t get_global_index() { return gl_index_; }
    bool get_is_anchor_entry() { return is_anchor_entry_; }
    HbmHashSpineEntry *get_spine_entry() { return spine_entry_; }
    uint32_t get_fhct_index() { return hct_index_; }
    sdk_ret_t form_action_data(HbmHashEntry *next_fe, void *swdata);
    void print_fe();
    void entry_to_str(char *buff, uint32_t buff_size);
    void *get_hwkey(void) { return hwkey_; }

    void set_key(void *key);
    void set_key_len(uint32_t key_len);
    void set_data(void *data);
    void set_data_len(uint32_t data_len);
    void set_hash_val(uint32_t hash_val);
    void set_fh_group(HbmHashHintGroup *fh_group);
    void set_global_index(uint32_t index);
    void set_is_anchor_entry(bool is_anchor);
    void set_spine_entry(HbmHashSpineEntry *spe);
    void set_fhct_index(uint32_t fhct_index);
    void set_hw_key(void *hwkey);
    void inc_recircs() { num_recircs_++; }
    void inc_recircs(uint32_t x) { num_recircs_ += x; }
    void dec_recircs() { num_recircs_--; }
    uint32_t get_recircs() { return num_recircs_; }


};

}   // namespace table
}   // namespace sdk
#endif // __HBM_HASH_ENTRY_HPP__
