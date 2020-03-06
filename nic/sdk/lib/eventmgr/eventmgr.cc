// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "include/sdk/mem.hpp"
#include "lib/slab/slab.hpp"
#include "lib/eventmgr/eventmgr.hpp"

using sdk::lib::dllist_reset;
using sdk::lib::dllist_del;

namespace sdk {
namespace lib {

static void *
event_map_get_key_func (void *entry)
{
    return (void *)&(((event_state_t *)entry)->event_id);
}

static void *
listener_map_get_key_func (void *entry)
{
    return (void *)&(((listener_state_t *)entry)->lctxt);
}

// initialize event manager instance
bool
eventmgr::init(uint32_t max_events) {
    event_map_ = ht::factory(max_events, event_map_get_key_func,
                             sizeof(event_id_t), true);
    if (event_map_ == NULL) {
        return false;
    }

    event_map_slab_ = slab::factory("eventmgr", SDK_SLAB_ID_EVENT_MAP,
                                    sizeof(event_state_t), 16, true,
                                    true, true);
    if (event_map_slab_ == NULL) {
        goto error;
    }

    event_listener_state_slab_ =
        slab::factory("eventmgr", SDK_SLAB_ID_EVENT_LISTENER_STATE,
                      sizeof(event_listener_state_t), 16, true, true, true);
    if (event_listener_state_slab_ == NULL) {
        goto error;
    }

    listener_map_ = ht::factory(16, listener_map_get_key_func,
                                sizeof(void *), true);
    if (listener_map_ == NULL) {
        goto error;
    }

    listener_slab_ = slab::factory("eventmgr", SDK_SLAB_ID_EVENT_LISTENER,
                                   sizeof(listener_state_t), 8, true, true,
                                   true);
    if (listener_slab_ == NULL) {
        goto error;
    }
    return true;

error:

    if (event_map_) {
        ht::destroy(event_map_);
        event_map_ = NULL;
    }
    if (event_map_slab_) {
        slab::destroy(event_map_slab_);
        event_map_slab_ = NULL;
    }
    if (event_listener_state_slab_) {
        slab::destroy(event_listener_state_slab_);
        event_listener_state_slab_ = NULL;
    }
    if (listener_map_) {
        ht::destroy(listener_map_);
        listener_map_ = NULL;
    }
    return false;
}

// constructor
eventmgr::eventmgr() {
    event_map_ = NULL;
    listener_map_ = NULL;
    event_map_slab_ = NULL;
    event_listener_state_slab_ = NULL;
    listener_slab_ = NULL;
}

// factory method for event manager class
eventmgr *
eventmgr::factory(uint32_t max_events) {
    void *mem;
    eventmgr *new_eventmgr;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_EVENT_MGR, sizeof(eventmgr));
    if (!mem) {
        return NULL;
    }
    new_eventmgr = new (mem) eventmgr();
    if (new_eventmgr->init(max_events) == false) {
        new_eventmgr->~eventmgr();
        SDK_FREE(SDK_MEM_ALLOC_EVENT_MGR, new_eventmgr);
        return NULL;
    }
    return new_eventmgr;
}

eventmgr::~eventmgr() {
    if (event_map_) {
        ht::destroy(event_map_);
    }
    if (listener_map_) {
        ht::destroy(listener_map_);
    }
    if (event_map_slab_) {
        slab::destroy(event_map_slab_);
    }
    if (event_listener_state_slab_) {
        slab::destroy(event_listener_state_slab_);
    }
    if (listener_slab_) {
        slab::destroy(listener_slab_);
    }
}

void
eventmgr::destroy(eventmgr *evmgr) {
    if (!evmgr) {
        return;
    }
    evmgr->~eventmgr();
    SDK_FREE(SDK_MEM_ALLOC_EVENT_MGR, evmgr);
}

event_state_t *
eventmgr::event_state_alloc_init_(event_id_t event_id) {
    event_state_t *event_state;

    event_state = (event_state_t *)event_map_slab_->alloc();
    if (event_state == NULL) {
        SDK_TRACE_ERR("Failed to allocate memory for event map state");
        return NULL;
    }
    event_state->event_id = event_id;
    SDK_SPINLOCK_INIT(&event_state->slock, PTHREAD_PROCESS_PRIVATE);
    dllist_reset(&event_state->lctxt_head);
    event_state->ht_ctxt.reset();
    return event_state;
}

listener_state_t *
eventmgr::listener_alloc_init_(void *lctxt) {
    listener_state_t *lstate;

    lstate = (listener_state_t *)listener_slab_->alloc();
    if (lstate == NULL) {
        SDK_TRACE_ERR("Failed to allocate listener state");
        return NULL;
    }
    lstate->lctxt = lctxt;
    lstate->event_bmap = 0;
    lstate->ht_ctxt.reset();
    return lstate;
}

event_listener_state_t *
eventmgr::add_listener_to_event_state_(event_state_t *event_state,
                                       listener_state_t *listener_state) {
    event_listener_state_t *event_lstate;

    event_lstate =
        (event_listener_state_t *)event_listener_state_slab_->alloc();
    if (event_lstate == NULL) {
        SDK_TRACE_ERR("Failed to allocate memory for event listener state");
        return NULL;
    }
    event_lstate->lctxt = listener_state->lctxt;
    dllist_reset(&event_lstate->lentry);
    dllist_add(&event_state->lctxt_head, &event_lstate->lentry);
    return event_lstate;
}

sdk_ret_t
eventmgr::del_event_listener_(event_listener_state_t *event_lstate) {
    dllist_del(&event_lstate->lentry);
    event_listener_state_slab_->free(event_lstate);
    return SDK_RET_OK;
}

sdk_ret_t
eventmgr::add_eventmap_entry_(event_state_t *event_state) {
    sdk_ret_t ret;

    ret = event_map_->insert_with_key(&event_state->event_id, event_state,
                                      &event_state->ht_ctxt);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to add event map entry for event %u, err %u",
                      event_state->event_id, ret);
    }
    return ret;
}

sdk_ret_t
eventmgr::del_eventmap_entry_(event_state_t *event_state) {
    sdk_ret_t ret;

    ret = event_map_->remove_entry(event_state, &event_state->ht_ctxt);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to del event map entry for event %u, err %u",
                      event_state->event_id, ret);
    }
    return ret;
}

sdk_ret_t
eventmgr::add_listener_map_entry_(listener_state_t *lstate) {
    sdk_ret_t ret;

    ret = listener_map_->insert_with_key(&lstate->lctxt, lstate,
                                             &lstate->ht_ctxt);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to add listener map entry, err %u", ret);
    }
    return ret;
}

sdk_ret_t
eventmgr::subscribe(event_id_t event_id, void *lctxt) {
    sdk_ret_t ret;
    event_state_t *event_state;
    listener_state_t *listener_state;
    event_listener_state_t *event_lstate;
    bool new_lstate = false, new_event_state = false;

    listener_state = (listener_state_t *)listener_map_->lookup(&lctxt);
    if (listener_state == NULL) {
        // listener is new to us
        listener_state = listener_alloc_init_(lctxt);
        if (listener_state == NULL) {
            SDK_TRACE_ERR("Failed to allocate listener state, context 0x%lx",
                          lctxt);
            return SDK_RET_OOM;
        }
        new_lstate = true;

        // lookup for the event state
        event_state = (event_state_t *)event_map_->lookup(&event_id);
        if (event_state == NULL) {
            SDK_TRACE_DEBUG("First subscription to event %u, listener 0x%lx",
                            event_id, lctxt);
            event_state = event_state_alloc_init_(event_id);
            if (event_state == NULL) {
                ret = SDK_RET_OOM;
                goto error;
            }
            new_event_state = true;
        }
    } else {
        // check if listener already subscribed to this event
        if (listener_state->event_bmap & (1 << event_id)) {
            SDK_TRACE_DEBUG("Listener 0x%lx already subscribed to event %u",
                            lctxt, event_id);
            return SDK_RET_OK;
        }

        // lookup for the event state
        event_state = (event_state_t *)event_map_->lookup(&event_id);
        if (event_state == NULL) {
            SDK_TRACE_DEBUG("First subscription to event %u, listener 0x%lx",
                            event_id, lctxt);
            event_state = event_state_alloc_init_(event_id);
            if (event_state == NULL) {
                ret = SDK_RET_OOM;
                goto error;
            }
            new_event_state = true;
        }
    }

    SDK_SPINLOCK_LOCK(&event_state->slock);
    event_lstate = add_listener_to_event_state_(event_state, listener_state);
    if (event_lstate == NULL) {
        ret = SDK_RET_OOM;
        SDK_SPINLOCK_UNLOCK(&event_state->slock);
        goto error;
    }

    // we have both listener state and event state in hand now
    if (new_event_state) {
        ret = add_eventmap_entry_(event_state);
        if (ret != SDK_RET_OK) {
            del_event_listener_(event_lstate);
            SDK_SPINLOCK_UNLOCK(&event_state->slock);
            goto error;
        }
    }

    if (new_lstate) {
        ret = add_listener_map_entry_(listener_state);
        if (ret != SDK_RET_OK) {
            del_event_listener_(event_lstate);
            if (new_event_state) {
                del_eventmap_entry_(event_state);
            }
            SDK_SPINLOCK_UNLOCK(&event_state->slock);
            goto error;
        }
    }
    listener_state->event_bmap |= (1 << event_id);
    SDK_SPINLOCK_UNLOCK(&event_state->slock);
    SDK_TRACE_DEBUG("Listener 0x%lx successfully registered for event %u",
                    lctxt, event_id);

    return SDK_RET_OK;

error:

    if (new_event_state && event_state) {
        event_map_slab_->free(event_state);
    }

    if (new_lstate && listener_state) {
        listener_slab_->free(listener_state);
    }

    return ret;
}

sdk_ret_t
eventmgr::unsubscribe_(event_state_t *event_state,
                       listener_state_t *listener_state) {
    dllist_ctxt_t *curr, *next;
    event_listener_state_t *event_lstate;

    dllist_for_each_safe(curr, next, &event_state->lctxt_head) {
        event_lstate = dllist_entry(curr, event_listener_state_t, lentry);
        if (event_lstate->lctxt == listener_state->lctxt) {
            SDK_TRACE_DEBUG("Unsubscribed listener 0x%lx from event %u",
                            listener_state->lctxt, event_state->event_id);
            sdk::lib::dllist_del(&event_lstate->lentry);
            event_listener_state_slab_->free(event_lstate);
            break;
        }
    }
    listener_state->event_bmap &= ~(1 << event_state->event_id);
    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// NOTE: we are not freeing event_state_t once allocated even after all
// listeners unsubscribe for that event, which is ok as we don't have that many
// events from HAL
//------------------------------------------------------------------------------
sdk_ret_t
eventmgr::unsubscribe(event_id_t event_id, void *lctxt) {
    event_state_t *event_state;
    listener_state_t *listener_state;

    event_state = (event_state_t *)event_map_->lookup(&event_id);
    if (event_state == NULL) {
        SDK_TRACE_DEBUG("No subscribers for event %u", event_id);
        return SDK_RET_OK;
    }

    listener_state = (listener_state_t *)listener_map_->lookup(&lctxt);
    if (listener_state == NULL) {
        SDK_TRACE_DEBUG("Unknown listener 0x%lx", lctxt);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (!(listener_state->event_bmap & (1 << event_id))) {
        SDK_TRACE_ERR("Listener not subscribed to event %u", event_id);
        return SDK_RET_INVALID_OP;
    }

    SDK_TRACE_DEBUG("Unsubscribing listener 0x%lx from event %u",
                    listener_state->lctxt, event_state->event_id);
    SDK_SPINLOCK_LOCK(&event_state->slock);
    unsubscribe_(event_state, listener_state);
    SDK_SPINLOCK_UNLOCK(&event_state->slock);
    return SDK_RET_OK;
}

bool
eventmgr::is_listener_active(void *lctxt)
{
    listener_state_t    *listener_state;

    listener_state = (listener_state_t *)listener_map_->lookup(&lctxt);
    if (listener_state == NULL) {
        SDK_TRACE_DEBUG("Unknown listener 0x%lx", lctxt);
        return false;
    }
    if (listener_state->event_bmap) {
        return true;
    }
    return false;
}

sdk_ret_t
eventmgr::unsubscribe_listener(void *lctxt) {
    uint64_t event_bmap;
    event_id_t event_id = 0;
    event_state_t *event_state;
    listener_state_t *listener_state;

    SDK_TRACE_DEBUG("Unsubscribing listener 0x%lx from all events", lctxt);
    listener_state = (listener_state_t *)listener_map_->remove(&lctxt);
    if (listener_state == NULL) {
        SDK_TRACE_ERR("Listener not found");
        return SDK_RET_OK;
    }

    event_bmap = listener_state->event_bmap;
    while (event_bmap) {
        while (!(event_bmap & 0x1)) {
            event_bmap >>= 1;
            event_id++;
        }
        event_state = (event_state_t *)event_map_->lookup(&event_id);
        if (event_state == NULL) {
            SDK_TRACE_ERR("Event state for event %u not found, skipping ...",
                          event_id);
            event_bmap >>= 1;
            continue;
        }
        SDK_TRACE_DEBUG("Unsubscribing listener 0x%lx from event %u",
                        listener_state->lctxt, event_state->event_id);
        SDK_SPINLOCK_LOCK(&event_state->slock);
        unsubscribe_(event_state, listener_state);
        SDK_SPINLOCK_UNLOCK(&event_state->slock);
        event_bmap >>= 1;
        event_id++;
    }
    listener_slab_->free(listener_state);
    return SDK_RET_OK;
}

sdk_ret_t
eventmgr::walk_listeners(event_id_t event_id, void *event_ctxt,
                         event_walk_cb_t walk_cb) {
    event_state_t *event_state;
    dllist_ctxt_t *curr, *next;
    event_listener_state_t *lstate;

    event_state = (event_state_t *)event_map_->lookup(&event_id);
    if (event_state == NULL) {
        SDK_TRACE_DEBUG("event state for event %u not found",
                        event_id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    SDK_SPINLOCK_LOCK(&event_state->slock);
    dllist_for_each_safe(curr, next, &event_state->lctxt_head) {
        lstate = dllist_entry(curr, event_listener_state_t, lentry);
        if (!walk_cb(event_id, event_ctxt, lstate->lctxt)) {
            SDK_SPINLOCK_UNLOCK(&event_state->slock);
            unsubscribe_listener(lstate->lctxt);
            SDK_SPINLOCK_LOCK(&event_state->slock);
        }
    }
    SDK_SPINLOCK_UNLOCK(&event_state->slock);
    return SDK_RET_OK;
}

sdk_ret_t
eventmgr::notify_event(event_id_t event_id, void *event_ctxt,
                       event_walk_cb_t walk_cb) {
    return (walk_listeners(event_id, event_ctxt, walk_cb));
}

}    // namespace lib
}    // namespace sdk
