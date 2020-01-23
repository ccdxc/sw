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

#include <stddef.h>
#include "include/sdk/base.hpp"

namespace sdk {
namespace lib {

// doubly linked list context
typedef struct dllist_ctxt_s dllist_ctxt_t;
struct dllist_ctxt_s {
    dllist_ctxt_t    *prev;
    dllist_ctxt_t    *next;
    dllist_ctxt_s() : prev(this), next(this) {}
} __PACK__;

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
         pos = n, n = (pos)?pos->next:NULL)

// reset the link information
extern void
dllist_reset (dllist_ctxt_t *dlctxt);

// reset the link information
extern void
dllist_init (dllist_ctxt_t *dlctxt);

// insert 'elem' between 'prev' and 'next'
extern void
dllist_add_between (dllist_ctxt_t *prev, dllist_ctxt_t *next,
                    dllist_ctxt_t *elem);

// insert an entry at the head of the list
extern void
dllist_add (dllist_ctxt_t *head, dllist_ctxt_t *elem);

// insert an entry at the tail of the list
extern void
dllist_add_tail (dllist_ctxt_t *head, dllist_ctxt_t *elem);

// remove an entry from the doubly linked list
extern void
dllist_del (dllist_ctxt_t *entry);

// return true if list is empty
extern bool
dllist_empty (dllist_ctxt_t *head);

// move list from src to dst
extern void
dllist_move (dllist_ctxt_t *dst, dllist_ctxt_t *src);

// count the number of elements in the list
extern uint32_t
dllist_count (dllist_ctxt_t *head);

}    // namespace lib
}    // namespace sdk

#endif //__LIST_HPP__
