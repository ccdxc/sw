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

}    // namespace table
}    // namespace sdk

#endif // __SDK_TCAM_ENTRY_HPP__
