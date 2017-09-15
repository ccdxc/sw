// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LIST_HPP__
#define __LIST_HPP__

#include <base.h>

namespace hal {
namespace utils {

// doubly linked list context
typedef struct dllist_ctxt_s dllist_ctxt_t;
struct dllist_ctxt_s {
    dllist_ctxt_t    *prev;
    dllist_ctxt_t    *next;
    dllist_ctxt_s(): prev(this), next(this){}
} __PACK__;

// reset the link information
static inline void
dllist_reset (dllist_ctxt_t *dlctxt)
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
    HAL_ASSERT((head != NULL) && (elem != NULL));
    if (head->prev == NULL || head->next == NULL) {
        dllist_reset(head);
    }
    dllist_add_between(head, head->next, elem);
}

// insert an entry at the tail of the list
static inline void
dllist_add_tail (dllist_ctxt_t *head, dllist_ctxt_t *elem)
{
    HAL_ASSERT((head != NULL) && (elem != NULL));
    if (head->prev == NULL || head->next == NULL) {
        dllist_reset(head);
    }
    dllist_add_between(head->prev, head, elem);
}

// remove an entry from the doubly linked list
static inline void
dllist_del (dllist_ctxt_t *entry)
{
    HAL_ASSERT(entry != NULL);
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = entry->next = NULL;
}

static inline bool
dllist_empty (dllist_ctxt_t *head)
{
    return head->next == head;
}

/**
 * dlist_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define dllist_entry(ptr, type, member)                 \
    ((type *)((char *)(ptr) - offsetof(type, member)))


/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define dllist_for_each(pos, head)              \
    for (pos = (head)->next; pos != (head);     \
         pos = pos->next)
/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define dllist_for_each_prev(pos, head)         \
    for (pos = (head)->prev; pos != (head);     \
         pos = pos->prev)

/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define dllist_for_each_safe(pos, n, head)                        \
    for (pos = (head)->next, n = pos->next; pos != (head);        \
         pos = n, n = pos->next)


}    // namespace utils
}    // namespace hal

#endif    // __LIST_HPP__

