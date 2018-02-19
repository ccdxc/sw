//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// hash entry
//  - represents an entry in hash table
//------------------------------------------------------------------------------

#ifndef __SDK_HASH_ENTRY_HPP__
#define __SDK_HASH_ENTRY_HPP__

#include "sdk/base.hpp"
#include "sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace sdk {
namespace table {


typedef struct hash_entry_s {
    void        *key;
    uint32_t    key_len;
    void        *data;
    uint32_t    data_len;
    uint32_t    index;
    ht_ctxt_t   ht_ctxt;
} hash_entry_t;

typedef struct hash_entry_cb_s {
    hash_entry_t *he;
    hash_entry_t *he_match;
    bool is_present;
} hash_entry_cb_t;

void *hash_entry_get_key_func (void *entry);
uint32_t hash_entry_compute_hash_func(void *key, uint32_t ht_size);
bool hash_entry_compare_key_func (void *key1, void *key2);

hash_entry_t *
hash_entry_create(void *key, uint32_t key_len,
                  void *data, uint32_t data_len, uint32_t index);
void
hash_entry_delete(hash_entry_t *he);

void hash_entry_update_data(hash_entry_t *he, void *data);

#if 0
class hash_entry {

private:
    void        *key_;          // sw key
    uint32_t    key_len_;       // sw key len
    void        *data_;         // sw/hw data
    uint32_t    data_len_;      // sw/hw data len
    uint32_t    index_;         // hash index

    hash_entry (void *key, uint32_t key_len, void *data, uint32_t data_len,
               uint32_t index);
    ~hash_entry();
public:
    static hash_entry *factory(void *key, uint32_t key_len, void *data, 
                              uint32_t data_len, uint32_t index);
    static void destroy(hash_entry *he);

    void update_data(void *data);

    // Getters & Setters
    void *get_key(void) { return key_; }
    uint32_t get_key_len(void) { return key_len_; }
    void *get_data(void) { return data_; }
    uint32_t get_data_len(void) { return data_len_; }
    uint32_t get_index(void) { return index_; }

};
#endif

}    // namespace table
}    // namespace sdk
#endif // __SDK_HASH_ENTRY_HPP__
