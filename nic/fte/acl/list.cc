#include "nic/fte/acl/list.hpp"
#include "nic/include/hal_mem.hpp"
#include "sdk/slab.hpp"

namespace acl {

// static memeber initializers
slab *list_t::list_item_slab_ =
    slab::factory("acl_list_item", hal::HAL_SLAB_ACL_LIST_ITEM,
                  sizeof(item_t), 1024, true, true, true);
slab *list_t::list_slab_ =
    slab::factory("acl_list", hal::HAL_SLAB_ACL_LIST,
                  sizeof(list_t), 256, true, true, true);

//------------------------------------------------------------------------
// creates new list item
//------------------------------------------------------------------------
list_t::item_t *
list_t::item_create(const ref_t *entry, const list_t::item_t *next)
{
    item_t *item = (item_t *)list_item_slab_->alloc();
    
    item->next = next;
    item->entry = entry;
    ref_init(&item->ref_count, [](const ref_t *ref) {
            item_t *item = container_of(ref, item_t, ref_count);
            
            item_deref(item->next);
            ref_dec(item->entry);
            
            list_item_slab_->free(item);
        });

    return item;
}

//------------------------------------------------------------------------
// creates a copy of the item if the item is shared
//------------------------------------------------------------------------
list_t::item_t*
list_t::item_copy(const list_t::item_t* item)
{
    item_t *copy;

    if (!ref_is_shared(&item->ref_count)) {
        return (item_t *)item;
    }

    copy = item_create(ref_clone(item->entry), item_clone(item->next));

    item_deref(item);

    return copy;
}


//------------------------------------------------------------------------
// Creates a new empty list
//------------------------------------------------------------------------
list_t *
list_t::create()
{
    list_t *list = (list_t *)list_slab_->alloc();

    list->first_ = NULL;
    list->num_entries_ = 0;
    ref_init(&list->ref_count_, [](const ref_t *ref) {
            list_t *list = container_of(ref, list_t, ref_count_);
            item_deref(list->first_);
            list_slab_->free(list);
        });

    return list;
}

//------------------------------------------------------------------------
// Creates a copy of the list if it is shared
//------------------------------------------------------------------------
list_t *
list_t::copy() const 
{
    if (!ref_is_shared(&ref_count_)) {
        return (list_t *)this;
    }

    list_t *list = create();

    list->num_entries_ = num_entries_;
    list->first_ = item_clone(first_);

    this->deref();

    return list;
}

//------------------------------------------------------------------------
// returns the first matching entry 
// returns first entry if match function is null
//------------------------------------------------------------------------
const ref_t *
list_t::find(const void *arg, cb_t match) const
{
    const item_t *item = first_;

    if (match) {
        while (item && !match(arg, item->entry)) {
            item = item->next;
        }
    }

    return item ? item->entry : NULL;
}

//------------------------------------------------------------------------
// walks the list, calling callback for each item. Aborts the walk
// if the callback returns false.
//
// returns flase if the walk is aborted
//------------------------------------------------------------------------
bool
list_t::walk(const void *arg, cb_t cb) const
{
    bool cont = cb ? true : false;

    for (const item_t *item = first_; cont && item; item = item->next) {
        cont = cb(arg, item->entry);
    }

    return cont;
}

//------------------------------------------------------------------------
// Inserts entry into the list.
// Entry is inserted before the first matching entry in the list or at the end
// if no entry matches.
// If the match function is NULL, entry is added at the front.
//
// Returns new list if the list head is shared, otherwise
//    original list is updated.
//------------------------------------------------------------------------
hal_ret_t
list_t::insert(const list_t **listp, const ref_t *entry, const void *arg, cb_t match) 
{
    const item_t **itemp;

    list_t *list = (*listp)->copy();
    *listp = list;

    itemp =  &list->first_;

    // copy the list until the matching item
    if (match) {
        while (*itemp && !match(arg, (*itemp)->entry)) {
            item_t *item = item_copy(*itemp);
            *itemp = item;
            itemp = &item->next;
        } 
    }

    *itemp = item_create(entry, *itemp);

    list->num_entries_++;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------
// deletes the first matching entry
//
// Returns new list if the list is shared, if the list is not shared
//    original list is updated.
//------------------------------------------------------------------------
hal_ret_t
list_t::remove(const list_t **listp, const void *arg, cb_t match) 
{

    const item_t **itemp, *item;
    
    // find first mathing entry
    item =  (*listp)->first_;
    if (match) {
        while (item && !match(arg, item->entry)) {
            item = item->next;
        }
    }

    // return if no matching item
    if (item == NULL){
        return HAL_RET_ENTRY_NOT_FOUND;
    }
    
    // copy the list until the matched item
    list_t *list = (*listp)->copy();
    *listp = list;

    itemp = &list->first_;
    while (*itemp && *itemp != item) {
        item_t *tmp = item_copy(*itemp);
        *itemp = tmp;
        itemp = &tmp->next;
    }

    // delete the matched item
    *itemp = item_clone(item->next);
    item_deref(item);

    list->num_entries_--;

    return HAL_RET_OK;
}

} //namespace acl
