//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// doubly inked list implementation
// NOTE: this is not a thread safe library ... as this is an intrusive linked
//       list, the contained structure needs to have a lock (e.g., spinlock)
//       and it has to be locked by the app if the list is a shared list across
//       multiple threads
//------------------------------------------------------------------------------

#include "include/sdk/base.hpp"
#include "include/sdk/list.hpp"

namespace sdk {
namespace lib {

// reset the link information
void
dllist_reset (dllist_ctxt_t *dlctxt)
{
    dlctxt->prev = dlctxt->next = dlctxt;
}

// reset the link information
void
dllist_init (dllist_ctxt_t *dlctxt)
{
    dlctxt->prev = dlctxt->next = dlctxt;
}

// insert 'elem' between 'prev' and 'next'
void
dllist_add_between (dllist_ctxt_t *prev, dllist_ctxt_t *next,
                    dllist_ctxt_t *elem)
{
    elem->next = next;
    elem->prev = prev;
    next->prev = elem;
    prev->next = elem;
}

// insert an entry at the head of the list
void
dllist_add (dllist_ctxt_t *head, dllist_ctxt_t *elem)
{
    SDK_ASSERT((head != NULL) && (elem != NULL));
    if (head->prev == NULL || head->next == NULL) {
        dllist_reset(head);
    }
    dllist_add_between(head, head->next, elem);
}

// insert an entry at the tail of the list
void
dllist_add_tail (dllist_ctxt_t *head, dllist_ctxt_t *elem)
{
    SDK_ASSERT((head != NULL) && (elem != NULL));
    if (head->prev == NULL || head->next == NULL) {
        dllist_reset(head);
    }
    dllist_add_between(head->prev, head, elem);
}

// remove an entry from the doubly linked list
void
dllist_del (dllist_ctxt_t *entry)
{
    SDK_ASSERT(entry != NULL);
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = entry->next = NULL;
}

// return true if list is empty
bool
dllist_empty (dllist_ctxt_t *head)
{
    return head->next == head;
}

// move list from src to dst
void
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

// count the number of elements in the list
uint32_t
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
