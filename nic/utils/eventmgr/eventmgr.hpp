// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __EVENTMGR_HPP__
#define __EVENTMGR_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/utils/slab/slab.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/utils/list/list.hpp"

namespace hal {
namespace utils {

typedef uint32_t event_id_t;
typedef void (*event_walk_cb_t)(void *lctxt);

// per (event, listener ctxt) state
typedef struct event_listener_state_s {
    void             *lctxt;    // listener context
    dllist_ctxt_t    lentry;    // link entry for next listener
} __PACK__ event_listener_state_t;

// event map entry that carries per event state, this is keyed on event id
// and is used to walk when a event needs to be notified
typedef struct event_state_s {
    event_id_t        event_id;
    hal_spinlock_t    slock;
    dllist_ctxt_t     lctxt_head;
    ht_ctxt_t         ht_ctxt;
} __PACK__ event_state_t;

#if 0
// per event, per stream state, this is used to maintain event subscriptions
typedef struct event_stream_state_s {
    event_id_t        event_id;
    void              *ctxt;
    ht_ctxt_t         ht_ctxt;
} __PACK__ event_stream_state_t;

// per stream events-of-interest, this is keyed on stream and is used when a
// stream goes away
typedef struct stream_state_s {
    void              *ctxt;
    hal_spinlock_t    slock;
    uint64_t          event_bmap;
    ht_ctxt_t         ht_ctxt;
} __PACK__ stream_state_t;
#endif

class eventmgr {
public:
    static eventmgr *factory(uint32_t max_events);
    ~eventmgr();
    hal_ret_t subscribe(event_id_t event_id, void *lctxt);
    hal_ret_t unsubscribe(event_id_t event_id, void *lctxt);
    hal_ret_t unsubscribe_listener(void *lctxt);
    hal_ret_t walk_listeners(event_id_t event_id, event_walk_cb_t walk_cb);
    slab *eventmap_slab(void) const { return eventmap_slab_; }
    slab *eventmap_listener_slab(void) const { return listener_state_slab_; }

private:
    ht      *eventmap_;
    ht      *streammap_;
    slab    *eventmap_slab_;
    slab    *listener_state_slab_;

private:
    eventmgr();
    int init(uint32_t max_events);
    hal_ret_t add_eventmap_entry_(event_id_t event_id, void *lctxt);
};

}    // namespace utils
}    // namespace hal

#endif    // __EVENTMGR_HPP__

