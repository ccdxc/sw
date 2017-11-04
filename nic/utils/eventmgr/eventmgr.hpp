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
typedef bool (*event_walk_cb_t)(event_id_t event_id_t,
                                void *event_ctxt, void *lctxt);

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

// per stream events-of-interest, this is keyed on stream and is used when a
// stream goes away
typedef struct listener_state_s {
    void         *lctxt;      // key (could be the grpc stream)
    uint64_t     event_bmap;
    ht_ctxt_t    ht_ctxt;
} __PACK__ listener_state_t;

class eventmgr {
public:
    static eventmgr *factory(uint32_t max_events);
    ~eventmgr();
    hal_ret_t subscribe(event_id_t event_id, void *lctxt);
    hal_ret_t unsubscribe(event_id_t event_id, void *lctxt);
    hal_ret_t unsubscribe_listener(void *lctxt);
    bool is_listener_active(void *lctxt);
    // for given event id, walk all the listeners (walk callback can be used to
    // send notifications to the listeners)
    hal_ret_t walk_listeners(event_id_t event_id, void *event_ctxt,
                             event_walk_cb_t walk_cb);
    slab *event_map_slab(void) const { return event_map_slab_; }
    slab *event_listener_slab(void) const { return event_listener_state_slab_; }
    slab *listener_slab(void) const { return listener_slab_; }

private:
    ht      *event_map_;
    ht      *listener_map_;
    slab    *event_map_slab_;
    slab    *event_listener_state_slab_;
    slab    *listener_slab_;

private:
    eventmgr();
    int init(uint32_t max_events);
    event_state_t *event_state_alloc_init_(event_id_t event_id);
    listener_state_t *listener_alloc_init_(void *lctxt);
    event_listener_state_t *add_listener_to_event_state_(event_state_t *event_state,
                                                         listener_state_t *listener_state);
    hal_ret_t del_event_listener_(event_listener_state_t *event_lstate);
    hal_ret_t add_eventmap_entry_(event_state_t *event_state);
    hal_ret_t del_eventmap_entry_(event_state_t *event_state);
    hal_ret_t add_listener_map_entry_(listener_state_t *lstate);
    hal_ret_t unsubscribe_(event_state_t *event_state,
                           listener_state_t *listener_state);
};

}    // namespace utils
}    // namespace hal

#endif    // __EVENTMGR_HPP__

