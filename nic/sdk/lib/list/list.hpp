//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// doubly inked list implementation
// NOTE: this is not a thread safe library ... as this is an intrusive linked
//       list, the contained structure needs to have a lock (e.g., spinlock)
//       and it has to be locked by the app if the list is a shared list across
//       multiple threads
//------------------------------------------------------------------------------

#ifndef __LIST_HPP__
#define __LIST_HPP__

#include "sdk/base.hpp"

namespace sdk {
namespace lib {

// doubly linked list context
typedef struct dllist_ctxt_s dllist_ctxt_t;
struct dllist_ctxt_s {
    dllist_ctxt_t    *prev;
    dllist_ctxt_t    *next;
    dllist_ctxt_s() : prev(this), next(this) {}
} __PACK__;

// reset the link information
static inline void
dllist_reset (dllist_ctxt_t *dlctxt)
{
    dlctxt->prev = dlctxt->next = dlctxt;
}

// reset the link information
static inline void
dllist_init (dllist_ctxt_t *dlctxt)
{
    dlctxt->prev = dlctxt->next = dlctxt;
}

// insert 'elem' between 'prev' and 'next'
static inline void
dllist_add_between (dllist_ctxt_t *prev, dllist_ctxt_t *next,
                    dllist_ctxt_t *elem)
{
    elem->next = next;
    elem->prev = prev;
    next->prev = elem;
    prev->next = elem;
}

// insert an entry at the head of the list
static inline void
dllist_add (dllist_ctxt_t *head, dllist_ctxt_t *elem)
{
    SDK_ASSERT((head != NULL) && (elem != NULL));
    if (head->prev == NULL || head->next == NULL) {
        dllist_reset(head);
    }
    dllist_add_between(head, head->next, elem);
}

// insert an entry at the tail of the list
static inline void
dllist_add_tail (dllist_ctxt_t *head, dllist_ctxt_t *elem)
{
    SDK_ASSERT((head != NULL) && (elem != NULL));
    if (head->prev == NULL || head->next == NULL) {
        dllist_reset(head);
    }
    dllist_add_between(head->prev, head, elem);
}

// remove an entry from the doubly linked list
static inline void
dllist_del (dllist_ctxt_t *entry)
{
    SDK_ASSERT(entry != NULL);
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = entry->next = NULL;
}

// return true if list is empty
static inline bool
dllist_empty (dllist_ctxt_t *head)
{
    return head->next == head;
}

// move list from src to dst
static inline void
dllist_move (dllist_ctxt_t *dst, dllist_ctxt_t *src)
{
    if ((dst == NULL) || (src == NULL) ||
        ((src->prev == src->next) &&
         (src == src->prev))) {
        // src list is empty
        return;
    }
    dst->prev = src->prev;
    dst->next = src->next;
    src->next->prev = dst;
    src->prev->next = dst;
    dllist_reset(src);
}

//------------------------------------------------------------------------------
// get the container structure for this entry
// ptr    - the &struct list_head pointer
// type   - type of the struct this is embedded in
// member - the name of the list_struct within the struct
//------------------------------------------------------------------------------
#define dllist_entry(ptr, type, member)                 \
    ((type *)((char *)(ptr) - offsetof(type, member)))

//------------------------------------------------------------------------------
//  iterate over a list
//  pos  - the &struct list_head to use as a loop counter
//  head - the head for your list
//------------------------------------------------------------------------------
#define dllist_for_each(pos, head)              \
    for (pos = (head)->next; pos != (head);     \
         pos = pos->next)
//------------------------------------------------------------------------------
// iterate over a list backwards
// pos  - the &struct list_head to use as a loop counter
// head - the head for your list
//------------------------------------------------------------------------------
#define dllist_for_each_prev(pos, head)         \
    for (pos = (head)->prev; pos != (head);     \
         pos = pos->prev)

//------------------------------------------------------------------------------
// iterate over a list safe against removal of list entry
// pos   - the &struct list_head to use as a loop counter
// n     - another &struct list_head to use as temporary storage
// @head - the head for your list
//------------------------------------------------------------------------------
#define dllist_for_each_safe(pos, n, head)                              \
    for (pos = head ? (head)->next : NULL, n = pos ? pos->next : NULL;  \
            pos != (head) && pos != NULL;                              \
         pos = n, n = pos->next)

// count the number of elements in the list
static inline uint32_t
dllist_count (dllist_ctxt_t *head)
{
    dllist_ctxt_t   *lnode = NULL;
    uint32_t        count = 0;

    if (!head) {
        return count;
    }
    dllist_for_each(lnode, head) {
        count++;
    }

    return count;
}

}    // namespace lib
}    // namespace sdk

#endif    // __LIST_HPP__

