//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// tcam entry
//  - represents an entry in Tcam Table
//------------------------------------------------------------------------------

#ifndef __SDK_TCAM_ENTRY_HPP__
#define __SDK_TCAM_ENTRY_HPP__

#include "sdk/base.hpp"
#include "sdk/mem.hpp"
#include "lib/ht/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace sdk {
namespace table {

typedef struct tcam_entry_s {
    void        *key;
    void        *key_mask;
    uint32_t    key_len;
    void        *data;
    uint32_t    data_len;
    uint32_t    index;
    uint32_t    priority;
    uint16_t    ref_cnt;
    ht_ctxt_t   ht_ctxt;
} tcam_entry_t;

typedef struct tcam_entry_cb_s {
    tcam_entry_t *te;
    tcam_entry_t *te_match;
    bool is_present;
} tcam_entry_cb_t;

void *tcam_entry_get_key_func (void *entry);
uint32_t tcam_entry_compute_hash_func(void *key, uint32_t ht_size);
bool tcam_entry_compare_key_func (void *key1, void *key2);

tcam_entry_t *
tcam_entry_create(void *key, void *key_mask, uint32_t key_len,
                  void *data, uint32_t data_len, uint32_t index,
                  uint32_t priority = 0);
void
tcam_entry_delete(tcam_entry_t *te);

// Deprecated. Cleanup once tcam is integrated from SDK
#if 0
class tcam_entry {

#define TCAM_ENTRY_INVALID_INDEX 0xffffffff

private:
    void        *key_;          // sw key
    void        *key_mask_;     // sw key mask
    uint32_t    key_len_;       // sw key len
    void        *data_;         // sw/hw data 
    uint32_t    data_len_;      // sw/hw data len
    uint32_t    index_;         // tcam index
    uint32_t    priority_;      // tcam entry priority
    int         refcnt_;        // tcam entry reference count

    tcam_entry (void *key, void *key_mask, uint32_t key_len, 
               void *data, uint32_t data_len, uint32_t index,
               uint32_t priority = 0);
    ~tcam_entry();
public:
    static tcam_entry *factory(void *key,                // key
                               void *key_mask,           // key's mask
                               uint32_t key_len,         // key's len
                               void *data,               // data
                               uint32_t data_len,        // data len
                               uint32_t index,           // index
                               uint32_t priority = 0,    // priority
                               uint32_t mtrack_id = 
                               SDK_MEM_ALLOC_ID_TCAM_ENTRY);
    static void destroy(tcam_entry *te, 
                        uint32_t mtrack_id = SDK_MEM_ALLOC_ID_TCAM_ENTRY); 

    void update_data(void *data);

    // Getters & Setters
    void *get_key() { return key_; };
    void *get_key_mask() { return key_mask_; }
    uint32_t get_key_len() { return key_len_; }
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_index() { return index_; }
    void set_index(uint32_t index) { index_ = index; }
    uint32_t get_priority() { return priority_; }
    int get_refcnt() { return refcnt_; }
    void set_refcnt(int refcnt) { refcnt_ = refcnt; }
    void incr_refcnt() { refcnt_++; }
    void decr_refcnt() { refcnt_--; }
};
#endif

}    // namespace table
}    // namespace sdk

#endif // __SDK_TCAM_ENTRY_HPP__
