#pragma once

#include "nic/include/base.h"
#include "nic/fte/acl/ref.hpp"
#include "sdk/slab.hpp"

namespace acl {

using sdk::lib::slab;

class list_t {
public:
    // callback for matching entries during insert/delete/search
    typedef bool (*cb_t)(const void *arg, const ref_t *entry);

    static list_t *create();
    const list_t *clone() const { ref_inc(&ref_count_); return this; }
    void deref() const { ref_dec(&ref_count_); }
    uint32_t size() const { return num_entries_; }
    bool empty() const { return first_ == NULL; }

    const ref_t *find(const void *arg, cb_t cb) const;
    bool walk(const void *arg, cb_t cb) const;

    // list update methods, updates the list pointer
    static hal_ret_t insert(const list_t **listp, const ref_t *entry,
                            const void *arg = NULL, cb_t match = NULL);
    static hal_ret_t remove(const list_t **listp, const void *arg = NULL,
                            cb_t match = NULL);

    // debug counters
    static uint32_t num_lists_in_use() { return list_slab_->num_in_use(); }
    static uint32_t num_items_in_use() { return list_item_slab_->num_in_use(); }

private:
    struct item_t {
        ref_t         ref_count; // ref count
        const ref_t   *entry;     // pointer to entry
        const item_t  *next;      // next link
    };

    static const item_t *item_clone(const item_t *item) {
        if (item) {
            ref_inc(&item->ref_count);
        }
        return item;
    }
    static void item_deref(const item_t *item) {
        if (item) {
            ref_dec(&item->ref_count);
        }
    }
    static item_t *item_create(const ref_t *entry, const item_t *next);
    static item_t *item_copy(const item_t *item);

    list_t *copy() const;
    
    ref_t        ref_count_;      // ref count
    const item_t *first_;          // pointer to first node
    uint32_t     num_entries_;    // num entries

    static slab* list_slab_;      // slab for list head
    static slab* list_item_slab_; // slab for list ites
};

} // namespace acl
